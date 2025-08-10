#include "swap.h"

//Debe ser usado entre mutex_frames_y_diccionario
char* sacar_de_swap(uint32_t pid_a_sacar){

     //simulamos retardo SWAP
    usleep(configs->retardo_swap * 1000);
    char* pid_str = string_itoa(pid_a_sacar);
     pthread_mutex_lock(&mutex_frames_y_diccionario);
    //aux contiene los datos del proceso
    t_value_proceso* aux = dictionary_get(dicc_pids_con_instrucciones, pid_str);
    
    free(pid_str);

    if (!aux) {
        log_error(logger, "PID %d no encontrado en diccionario", pid_a_sacar);
        return "ERROR";
    }

    int pag_necesarias = (int)ceil((double)aux->tamanio_proceso / configs->tamanio_pagina);

    
    bool hay_espacio = contar_frames_libres_en_lista() >= pag_necesarias;
    

    if (!hay_espacio) {
        log_warning(logger, "No hay suficiente espacio para traer al PID %d desde SWAP", pid_a_sacar);
        return "ERROR";
    }

    asignar_frames_a_proceso(aux, pag_necesarias, pid_a_sacar);

   
    for (int pagina = 0; pagina < pag_necesarias; pagina++) {
        
        int nro_bloque = encontrar_pagina_en_swap(pid_a_sacar);
        

        if (nro_bloque == -1) {
            log_error(logger, "No se encontró bloque de SWAP para la página %d del PID %d", pagina, pid_a_sacar);
            return "ERROR";
        }

        void* buffer = malloc(configs->tamanio_pagina);
        off_t offset = nro_bloque * configs->tamanio_pagina;

        pthread_mutex_lock(&mutex_fd_swap);
        lseek(fd_swap, offset, SEEK_SET);
        read(fd_swap, buffer, configs->tamanio_pagina);
        pthread_mutex_unlock(&mutex_fd_swap);

        int frame_asignado = obtener_frame_asignado(aux->tabla_primer_nvl, pagina);

        
        memcpy(memoria_usuario + frame_asignado * configs->tamanio_pagina, buffer, configs->tamanio_pagina);
        

        free(buffer);

        liberar_bloque(nro_bloque);
    }
    
    aux->metricas->cant_subidas_memoria++;
    

    return "OK";
}

int obtener_frame_asignado(t_tabla_de_pagina* tabla_L1, int pagina_logica) {
    int indices[configs->cant_niveles];
    int pagina_actual = pagina_logica;

    for (int nivel = configs->cant_niveles; nivel >= 1; nivel--) {
        indices[nivel - 1] = pagina_actual % configs->entradas_por_tabla;
        pagina_actual /= configs->entradas_por_tabla;
    }

    t_tabla_de_pagina* tabla_actual = tabla_L1;
    for (int nivel = 1; nivel < configs->cant_niveles; nivel++) {
        t_entrada* entrada_intermedia = list_get(tabla_actual->entradas, indices[nivel - 1]);
        tabla_actual = entrada_intermedia->ptrTabla;
    }

    t_entrada_ult_tabla* entrada_final = list_get(tabla_actual->entradas, indices[configs->cant_niveles - 1]);
    return entrada_final->nroFrame;
}

void liberar_bloque(int indice){
      if (indice < 0 || indice >= list_size(lista_bloques_swap)) {
        log_error(logger, "Índice inválido al liberar bloque de SWAP: %d", indice);
        return;
    }
     pthread_mutex_lock(&mutex_bloques);

    int* libre = malloc(sizeof(int));
    *libre = -1;

    list_replace_and_destroy_element(lista_bloques_swap, indice, libre, free);

    pthread_mutex_unlock(&mutex_bloques);

    log_debug(logger, "Bloque %d liberado de SWAP", indice);
}

int encontrar_pagina_en_swap(uint32_t pid_a_sacar){
    pthread_mutex_lock(&mutex_bloques);
    for (int i = 0; i < list_size(lista_bloques_swap); i++) {
          int* valor = list_get(lista_bloques_swap, i);
        if (*valor == pid_a_sacar) {
            pthread_mutex_unlock(&mutex_bloques);
            return i;
        }
    }
    pthread_mutex_unlock(&mutex_bloques);
    return -1; // No se encontró ninguna página para ese PID
}


//Debe ser usado entre mutex_frames_y_diccionario
void pasar_a_swap(uint32_t proceso){
    
    struct stat st;
    char* pid_str = string_itoa(proceso);
    //Busco el proceso para saber cuantas paginas ocupa
    
    t_value_proceso* value = dictionary_get(dicc_pids_con_instrucciones, pid_str);
    
    
    int cant_paginas = (int)ceil((double)value->tamanio_proceso/configs->tamanio_pagina);
    
    if(bloques_swap_libres()< cant_paginas){
        int bloques_faltantes = cant_paginas - bloques_swap_libres();
        //entonces truncamos el archivo con la cantidad de bloques
        //que faltan al final del archivo
        pthread_mutex_lock(&mutex_fd_swap);
        fstat(fd_swap, &st);  // Obtener tamaño actual
        off_t tamanio_actual = st.st_size;

        off_t nuevo_tamano = tamanio_actual + (bloques_faltantes * configs->tamanio_pagina);
        ftruncate(fd_swap, nuevo_tamano);
        pthread_mutex_unlock(&mutex_fd_swap);
        
        agregar_a_lista_bloques(bloques_faltantes);
        

    }
    
    free(pid_str);
    escribir_en_swap(proceso);
    
}
int bloques_swap_libres(){
   pthread_mutex_lock(&mutex_bloques); // si tenés un mutex para la lista

    int libres = 0;
    for (int i = 0; i < list_size(lista_bloques_swap); i++) {
        int* bloque = list_get(lista_bloques_swap, i);
        if (*bloque == -1) {
            libres++;
        }
    }

    pthread_mutex_unlock(&mutex_bloques);
    return libres; 
}

void agregar_a_lista_bloques(int cantidad_bloques){
    pthread_mutex_lock(&mutex_bloques);
    for (int i = 0; i < cantidad_bloques; i++) {
        int* libre = malloc(sizeof(int));
        *libre = -1;
        list_add(lista_bloques_swap, libre);
    }
    pthread_mutex_unlock(&mutex_bloques);
}

int encontrar_asignar_bloque_libre(uint32_t pid){

     int bloque_encontrado = -1;
    //si no se encuentra nada devuelve -1
    pthread_mutex_lock(&mutex_bloques); // 

    for (int i = 0; i < list_size(lista_bloques_swap); i++) {
        int* valor = list_get(lista_bloques_swap, i);
        if (*valor == -1) {
            int* nuevo_pid = malloc(sizeof(int));
            *nuevo_pid = pid;

            list_replace_and_destroy_element(lista_bloques_swap, i, nuevo_pid, free);
            bloque_encontrado = i;

            log_info(logger, "Bloque %d asignado a PID %d", i, pid); 

            break;
        }
    }

    pthread_mutex_unlock(&mutex_bloques); 

    return bloque_encontrado;

}
//debe ser llamada dentro de un mutex_frames_y_diccionario
void escribir_en_swap(uint32_t proceso){

    usleep(configs->retardo_swap*1000);

    for(int nro_frame = 0; nro_frame < list_size(lista_frames); nro_frame++ ){

        t_info_frame* aux_frame = list_get(lista_frames,nro_frame);
        
        if(aux_frame->pid_proceso == proceso){
            int bloque_libre = encontrar_asignar_bloque_libre(proceso);
            
            char* buffer = leer_frame(nro_frame);
            int offset = bloque_libre * configs->tamanio_pagina;

            pthread_mutex_lock(&mutex_fd_swap);
            lseek(fd_swap, offset, SEEK_SET);
            write(fd_swap, buffer, configs->tamanio_pagina);
            pthread_mutex_unlock(&mutex_fd_swap);
            
            free(buffer);
        }
    }

    

    liberar_frames_de_proceso(proceso);
    char* pid_str = string_itoa(proceso);
    
    t_value_proceso* auxiliar = dictionary_get(dicc_pids_con_instrucciones, pid_str);
    auxiliar->metricas->cant_bajadas_swap++;

    free(pid_str);
    
}
char* leer_frame(int nro_frame){
    uint32_t direccion = nro_frame * configs->tamanio_pagina;

    char* datos = malloc(configs->tamanio_pagina); // debo liberar
    
    memcpy(datos, memoria_usuario + direccion, configs->tamanio_pagina);
    

    return datos; // quien lo recibe debe hacer free()
}


void iniciar_swap(){
    
    fd_swap = open(configs->pathSwapfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_swap == -1) {
        log_error(logger, "No se pudo crear el archivo %s", configs->pathSwapfile);
    }
    
    ftruncate(fd_swap, configs->tamanio_memoria);
    int cant_bloques_swap = configs->tamanio_memoria/ configs->tamanio_pagina;
    lista_bloques_swap = list_create();
    //inicializamos la lista_bloques_swap con -1
     agregar_a_lista_bloques(cant_bloques_swap);
    
}


bool esta_en_swap(uint32_t pid){
    bool tiene_bloques_asignados(void* ptr){
        int* aux = (int*) ptr;
        return *aux == pid;
    }
    return list_any_satisfy(lista_bloques_swap,tiene_bloques_asignados);
}

void liberar_bloques_swap_de_pid(uint32_t pid){

    for (int i = 0; i < list_size(lista_bloques_swap); i++) {
        int* valor = list_get(lista_bloques_swap, i);
        if (*valor == (int)pid) {
            liberar_bloque(i);
        }
    }

}