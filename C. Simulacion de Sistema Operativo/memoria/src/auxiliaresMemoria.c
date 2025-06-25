#include <auxiliaresMemoria.h>
#include <mostrarDiccionarioYTablas.h>

void *atender_cliente(void *datosSinTipo)
{
	datos_cliente_t *datos = (datos_cliente_t *)datosSinTipo;
	int fd_conexion = *(datos->fd_conexion_ptr);
	char *servidor = datos->servidor;
	//char *pathInstrucciones; // Esto ya no es necesario, el path esta en configs->pathInstrucciones

	// (*datos).fd_conexion_ptr == datos->fd_conexion_ptr   (RECORDANDO UN POCO LA SINTAXIS)
	
	free(datos->fd_conexion_ptr);
	free(datos); // Liberamos memoria reservada
	char* respuesta_operacion = NULL; // Para mensajes como "OK", "ERROR"

	while (1) //habria que agregar una condicion de salida para cortar el programa manualmente
	{
		uint32_t cod_op = recibir_operacion(fd_conexion);

		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(fd_conexion, servidor);
			char* mensaje = "Hola desde Memoria";
			enviar_mensaje(mensaje,fd_conexion);									
			break;
		case PROCESO:
			t_info_nuevo_proceso* info_proceso = recibir_nuevo_proceso(fd_conexion); //leer descripcion en recibir_nuevo_proceso

			// "guardar_proceso" retorna "OK" o "ERROR", por eso esta dentro de enviar_mensaje, para mas info ver definicion
            pthread_mutex_lock(&mutex_frames_y_diccionario);
			respuesta_operacion = guardar_proceso(info_proceso); //hacer dinamico
            pthread_mutex_unlock(&mutex_frames_y_diccionario);

			enviar_mensaje(respuesta_operacion, fd_conexion); //Enviar mensaje de confirmación a Kernel, OK o ERROR
			
			//imprimir_diccionario_completo();
			//imprimir_estado_frames();

			if (strcmp(respuesta_operacion, "OK") == 0)
			log_info(logger, "## PID: %d - Proceso Creado - Tamaño: %d", info_proceso->pid, info_proceso->tamanio_bytes_archivo);

			free(info_proceso->nombre_archivo);
			free(info_proceso);
			break;
		case DESTRUCCION_PROCESO:
			//Recibo el pid del proceso a destruir
			uint32_t pid_entero = recibir_pid(fd_conexion);
			//Transformo el pid a string
			char* pid_str = string_itoa(pid_entero);

            pthread_mutex_lock(&mutex_frames_y_diccionario); // <-- LOCK UNA SOLA VEZ

            t_value_proceso* value = dictionary_get(dicc_pids_con_instrucciones, pid_str);
            
            // Es más seguro verificar si el proceso existe ANTES de intentar acceder a sus métricas
            if (value == NULL) {
                respuesta_operacion = "ERROR_PROCESO_NO_EXISTE";
            } else {
                t_metricas_proceso metricasLog = *(value->metricas); // Copia segura
                respuesta_operacion = destruirProceso(pid_str); // Ahora destruirProceso solo remueve y libera
                
                if (strcmp(respuesta_operacion, "OK") == 0) {
                    log_info(logger, "## PID: %s - Proceso Destruido - Metricas - Acc.T.Pag: %d; Inst.Sol.: %d; SWAP: %d; Mem.Prin.: %d; Lec.Mem.: %d; Esc.Mem.: %d",
                        pid_str,metricasLog.cant_accesos_a_TTDPP,metricasLog.cant_instruccs_solicitadas,
                        metricasLog.cant_bajadas_swap,metricasLog.cant_subidas_memoria,
                        metricasLog.cant_lecturas_memoria,metricasLog.cant_escrituras_memoria);
                }
            }
            
            pthread_mutex_unlock(&mutex_frames_y_diccionario);
            
            //imprimir_diccionario_completo();
            //imprimir_estado_frames();
			
			enviar_mensaje(respuesta_operacion, fd_conexion); //Enviar mensaje de confirmación a Kernel, OK o ERROR
			
			free(pid_str); //libero el pid que recibi como string
			break;
        case PETICION_INSTRUCCION:
            t_peticion_instruccion* info_instruccion = recibir_peticion_instruccion(fd_conexion); //recibo PID y PC del CPU
			
			//buscar_instruccion busca la instruccion en el diccionario y la devuelve como char*
			char* instr_a_enviar = buscar_instruccion(info_instruccion->pid, info_instruccion->pc);
			
            enviar_mensaje_codigo_operacion(instr_a_enviar, fd_conexion, LINEA_DE_INSTRUCCION);

            log_info(logger, "## PID: %d - Obtener instrucción: %d - Instrucción: %s",
                info_instruccion->pid,info_instruccion->pc,instr_a_enviar);

			free(info_instruccion);
            break;
		case PETICION_LECTURA:                    
			//Ante un pedido de lectura, devolver el valor que se encuentra en la posición pedida. (osea recibo direccion fisica)
			t_recepcion_lectura* recepcion_L = recibir_instruccion_lectura(fd_conexion);
			char* datos = leer_direccion(recepcion_L); //busco el dato en la direccion fisica del void
            
            pthread_mutex_lock(&mutex_diccionario_instrucciones);
            t_value_proceso* valueL = dictionary_get(dicc_pids_con_instrucciones, string_itoa(recepcion_L->pid));
            valueL->metricas->cant_lecturas_memoria++;
            pthread_mutex_unlock(&mutex_diccionario_instrucciones);

			enviar_mensaje(datos, fd_conexion); //Envia los datos leídos al CPU (Cuando CPU lo implemente)
            
            log_info(logger, "## PID: %d - Lectura - Dir. Física: %d - Tamaño: %d",
                recepcion_L->pid, recepcion_L->direccion, recepcion_L->tamanio);
            
            free(datos); // Tal vez no deberia ir esto
            free(recepcion_L);
			break;
		case PETICION_ESCRITURA:
			/*Ante un pedido de escritura, escribir lo indicado en la posición pedida. En caso satisfactorio se responderá un mensaje de "OK".*/
			t_recepcion_escritura* recepcion_E = recibir_instruccion_escritura(fd_conexion);
    
            // Llamar a la función que maneja la escritura
            atender_peticion_escritura(recepcion_E);

            pthread_mutex_lock(&mutex_diccionario_instrucciones);
            t_value_proceso* valueE = dictionary_get(dicc_pids_con_instrucciones, string_itoa(recepcion_E->pid));
            valueE->metricas->cant_escrituras_memoria++;
            pthread_mutex_unlock(&mutex_diccionario_instrucciones);
            
            enviar_mensaje("OK", fd_conexion); // Cuando CPU lo implemente, enviar mensaje de confirmación
            log_info(logger, "## PID: %d - Escritura - Dir. Física: %d - Tamaño: %d",
                    recepcion_E->pid, recepcion_E->direccion, recepcion_E->tamanio);
            
            // Liberar recursos
            free(recepcion_E->datos);
            free(recepcion_E);
            break;
		case PETICION_MARCO:
			// CPU pasa, pid, entrada 1,2,3
			// Memoria responde con el nro de frame solicitado
            t_peticion_marco* peticion = recibir_peticion_marco(fd_conexion);

            uint32_t frame_encontrado = buscar_frame_para_pagina(peticion);

            enviar_frame_a_cpu(fd_conexion, frame_encontrado);

            log_info(logger, "## PID: %d - Petición de Marco: %d",
                     peticion->pid, frame_encontrado);
            
            list_destroy_and_destroy_elements(peticion->entradas_por_nivel, free);
            free(peticion);
			break;
		case SOLICITUD_DATOS_PAGINAS:
			recibir_mensaje(fd_conexion,servidor); //Solo para recibir el string, pero desecho los datos
			enviar_info_configs_a_mmu(fd_conexion);
			break;
		case PETICION_DUMP:
			uint32_t pid_dump = recibir_proceso_dump(fd_conexion);
			char* respuesta_dump = realizar_dump(pid_dump);
			//enviamos la respuesta de finalizacion
			enviar_mensaje(respuesta_dump,fd_conexion);

			break;
		case -1:
			log_warning(logger, "Cliente desconectado: fd=%d", fd_conexion);
			close(fd_conexion); // Cerramos conexión
			return NULL;
		default:
			log_warning(logger, "Operación desconocida recibida del cliente: fd=%d", fd_conexion);
			break;
		}
	}

	return NULL;
}

void atender_peticion_escritura(t_recepcion_escritura* recepcion_E){
    uint32_t direccion = recepcion_E->direccion; //direccion fisica
    char* datos = recepcion_E->datos; //datos a escribir
    uint32_t tamanio = recepcion_E->tamanio; //tamanio de la escritura

    if (direccion + tamanio > configs->tamanio_memoria) {
        log_error(logger, "Intento de escritura fuera de los límites de la memoria: Dirección %d, Tamaño %d", direccion, tamanio);
        return; // o manejar el error de otra manera
    }
    
    usleep(configs->retardo_memoria * 1000); // Simular retardo

    pthread_mutex_lock(&mutex_memoria_usuario);
    memcpy(memoria_usuario + direccion, datos, tamanio); //escribo en la memoria
    pthread_mutex_unlock(&mutex_memoria_usuario);
}

char* leer_direccion(t_recepcion_lectura* recepcionL){ //lee la direccion de la memoria segun el tamanio
    uint32_t direccion = recepcionL->direccion; //direccion fisica
    uint32_t tamanio = recepcionL->tamanio; //tamanio de la lectura

    if (direccion + tamanio > configs->tamanio_memoria) {
        log_error(logger, "Intento de lectura fuera de los límites de la memoria: Dirección %d, Tamaño %d", direccion, tamanio);
        return NULL; // o manejar el error de otra manera
    }
    usleep(configs->retardo_memoria * 1000);

    char* datos = malloc(tamanio+1);
    memcpy(datos, memoria_usuario + direccion, tamanio);
    datos[tamanio] = '\0'; // Asegurar que la cadena esté terminada en nulo

    return datos;
}

void enviar_frame_a_cpu(int socket_cpu, uint32_t nro_frame) {
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = PETICION_MARCO; // El mismo con el que se recibe la peticion? revisar
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(uint32_t);
    paquete->buffer->stream = malloc(paquete->buffer->size);

    memcpy(paquete->buffer->stream, &nro_frame, sizeof(uint32_t));

    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}

uint32_t buscar_frame_para_pagina(t_peticion_marco* peticion) { // [pid,nroPag,entradasPorNivel] -> entradasPorNivel = [0,2,1]
    char key_pid[11];
    sprintf(key_pid, "%d", peticion->pid);

    // 1. Obtener el proceso y su tabla de primer nivel del diccionario.
    pthread_mutex_lock(&mutex_diccionario_instrucciones);
    t_value_proceso* value = dictionary_get(dicc_pids_con_instrucciones, key_pid);
    pthread_mutex_unlock(&mutex_diccionario_instrucciones);
    
    t_tabla_de_pagina* tabla_actual = value->tabla_primer_nvl; // Tabla de primer nivel del proceso

    // El acceso a tablas de paginas ahora lo hare dinamico usando la lista de entradas por nivel y un for
    for(int i=0; i < configs->cant_niveles - 1; i++) { //recorro hasta la anteultima entrada guardada en la lista
        int nivel = i+1; //solo para claridad
        
        uint32_t* ptr_indice_entrada_tabla = list_get(peticion->entradas_por_nivel, i); // puntero al cero
        uint32_t indice_entrada_tabla = *ptr_indice_entrada_tabla; // obtengo al cero
        t_entrada* entrada_tabla = list_get(tabla_actual->entradas, indice_entrada_tabla); // uso el cero como indice para entrar en la entrada de tabla de nvl 1

        // Simular retardo y actualizar métricas para el último acceso
        usleep(configs->retardo_memoria * 1000);
        value->metricas->cant_accesos_a_TTDPP++;

        log_info(logger, "PID: %d - Acceso a Tabla Nivel %d - Entrada nro: %d -> Puntero a Tabla Nivel %d",
        peticion->pid, nivel, indice_entrada_tabla, nivel+1);

        tabla_actual = entrada_tabla->ptrTabla; // a traves de la entrada ingreso a la proxima tabla
    }
    // Ahora tabla_actual apunta a la tabla de ULTIMO NIVEL
    int i = configs->cant_niveles - 1;
    uint32_t* ptr_indice_entrada_tabla_final = list_get(peticion->entradas_por_nivel, i);
    uint32_t indice_entrada_tabla_final = *ptr_indice_entrada_tabla_final;

    t_entrada_ult_tabla* entrada_tabla_final = list_get(tabla_actual->entradas, indice_entrada_tabla_final);

    // Simular retardo y actualizar métricas para el último acceso
    usleep(configs->retardo_memoria * 1000);
    value->metricas->cant_accesos_a_TTDPP++;
    
    log_info(logger, "PID: %d - Acceso a Tabla Nivel %d - Entrada nro: %d -> Frame Encontrado: %d",
        peticion->pid, configs->cant_niveles, indice_entrada_tabla_final, entrada_tabla_final->nroFrame);

    return entrada_tabla_final->nroFrame;
}

t_peticion_marco* recibir_peticion_marco(int socket_cliente) {
    uint32_t size;
    void* buffer = recibir_buffer(&size, socket_cliente);

    t_peticion_marco* peticion = malloc(sizeof(t_peticion_marco));
    int desplazamiento = 0;

    // Deserializar PID
    memcpy(&peticion->pid, buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    
    // Deserializar Entradas por Nivel
    peticion->entradas_por_nivel = list_create(); // la de nvl 1 se va a guardar en indice 0, la de nvl2 en el 1, etc
    for(int i = 0; i < configs->cant_niveles; i++) { //yo se que al fin y al cabo van a ser 3 niveles, osea hasta indice 2
        uint32_t* entrada_actual = malloc(sizeof(uint32_t));

        memcpy(entrada_actual, buffer + desplazamiento, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);

        list_add(peticion->entradas_por_nivel,entrada_actual);
    }
    
    free(buffer);
    return peticion;
}

char* destruirProceso(char* pid) {
    uint32_t pid_entero = atoi(pid);
    
    // Ya no se verifica si existe, se asume que el llamador lo hizo.
    t_value_proceso* value = dictionary_remove(dicc_pids_con_instrucciones, pid);

    liberar_frames_de_proceso(pid_entero);
    destruir_tablas_recursivamente(value->tabla_primer_nvl);
    list_destroy_and_destroy_elements(value->instrucciones, free);
    
    free(value->metricas);
    free(value);

    return "OK";
}

char* realizar_dump(uint32_t pid_a_dump){
    log_info(logger,"## PID: %d - Memory Dump solicitado",pid_a_dump);

	char* timestamp = temporal_get_string_time("%d%m%y%H%M%S");
	char* path_dump_completo = string_from_format("%s/%d-%s.dmp", configs->dump_path, pid_a_dump, timestamp);

	FILE* archivo_dump = fopen(path_dump_completo, "wb");
    if (!archivo_dump) {
        log_error(logger, "No se pudo crear el archivo %s", path_dump_completo);
        free(path_dump_completo);
        return "ERROR";
    }

	//error memoria no asignada  
	//if(cantidad_paginas_pid == 0){
	//	return "Error_dump";
	//}
	//BUSCAR QUE OTROS CASOS DE ERROR PODRIA HABER	
    //en el modulo memoria no hay casos en los que se 

		//un mutex para recorrer la lista_frames ?
		pthread_mutex_lock(&mutex_frames);
		//recorremos los frames en busca del que tiene la pagina i del proceso
		for(int nro_frame = 0; nro_frame < list_size(lista_frames); nro_frame++){
			t_info_frame* aux_frame = list_get(lista_frames,nro_frame);
		
			//suponiendo que todos estan en memoria_usuario y no en swap
			if(aux_frame->pid_proceso==pid_a_dump){
                //log_info(logger, "Frame %d: PID=%d", nro_frame, aux_frame->pid_proceso);
                char* contenido = mem_hexstring(inicio_frame(nro_frame),configs->tamanio_pagina);
                //for (int i = 0; i < configs->tamanio_pagina; i++) {
                fprintf(archivo_dump,"%s\n", contenido);
                free(contenido);
                //    }
                //fprintf(archivo_dump, "\n");
			    }
 
		}
		pthread_mutex_unlock(&mutex_frames);
	


	fclose(archivo_dump);
	free(timestamp);
	free(path_dump_completo);

	return "FIN_DUMP";
}


//retorna un puntero al inicio del frame pasado por parametro indice_frame
void* inicio_frame(int indice_frame){
	//deberia poner mutex de memoria_usuario
    return (void*)(char*)memoria_usuario + (indice_frame * (configs->tamanio_pagina));
}

void liberar_frames_de_proceso(uint32_t pid) {
    void _liberar_si_pertenece(void* elemento) {
        t_info_frame* frame_info = (t_info_frame*) elemento;
        if (frame_info->pid_proceso == pid) { // Liberar solo los frames que pertenecen al PID
            frame_info->libre = true;
            frame_info->pid_proceso = -1;
            frame_info->nro_pagina = -1;
        }
    }
    list_iterate(lista_frames, _liberar_si_pertenece);
    log_info(logger, "Todos los frames del PID %d han sido liberados.", pid);
}

void destruir_tablas_recursivamente(t_tabla_de_pagina* tabla) {
    // Si la tabla es NULL (entrada no utilizada), no hacer nada.
    if (tabla == NULL) {
        return;
    }

    // Si no es la tabla del último nivel, sus entradas apuntan a otras tablas.
    if (tabla->nivel < configs->cant_niveles) {
        // Recorremos las entradas y llamamos recursivamente para destruir las tablas hijas.
        void destruir_tabla_hija(void* elemento) {
            t_entrada* entrada = (t_entrada*) elemento;
            destruir_tablas_recursivamente(entrada->ptrTabla);
        }
        list_iterate(tabla->entradas, destruir_tabla_hija);
    }
    
    // Destruir la lista de entradas de la tabla actual y la tabla en sí.
    // list_destroy_and_destroy_elements se encarga de liberar cada struct de entrada y la lista.
    list_destroy_and_destroy_elements(tabla->entradas, free);
    free(tabla);
}

char* buscar_instruccion(uint32_t pid, uint32_t pc){
	//Convierto el pid a string para usarlo como key en el diccionario
	char key_pid[11]; //como maximo el pid, en formato string, tiene 11 digitos
	sprintf(key_pid, "%d", pid); //paso el pid a string

	//Busco el pid en el diccionario
	pthread_mutex_lock(&mutex_diccionario_instrucciones);
	t_value_proceso* value = dictionary_get(dicc_pids_con_instrucciones, key_pid);

	if (pc >= list_size(value->instrucciones)) {
        return "INSTRUCCION_INVALIDA";
    }

	t_list* instrucciones = value->instrucciones;
    value->metricas->cant_instruccs_solicitadas++;
    pthread_mutex_unlock(&mutex_diccionario_instrucciones);

	return list_get(instrucciones, pc); //retorna la instruccion en el indice indicado por pc
}

char* guardar_proceso(t_info_nuevo_proceso* info_proceso){
	//El proceso se guarda en un diccionario, donde la key es el pid y el value es un t_value_proceso

    //Convierto el pid a string
    char key_pid[11]; //como maximo el pid, en formato string, tiene 11 digitos
    sprintf(key_pid, "%d", info_proceso->pid); //paso el pid a string

    //Genero el path para obtener instrucciones
    char* path_completo = malloc(strlen(configs->pathInstrucciones) + strlen(info_proceso->nombre_archivo) + 1); //sumo uno para el "\0"
    sprintf(path_completo, "%s%s", configs->pathInstrucciones, info_proceso->nombre_archivo); //en path_completo concateno las dos cadenas (la del config y la del nombre del archivo)

    //Abro el archivo para leer las instrucciones
    FILE* archivo = fopen(path_completo, "r");
    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo %s", path_completo);
        free(path_completo);
        return "ERROR";
    }

	//Si el archivo se abrio correctamente, leo las instrucciones
    t_list* instrucciones = list_create(); 
	leer_lineas_archivo(archivo, instrucciones); //lee el archivo y guarda las lineas en la lista de instrucciones
    fclose(archivo);
	free(path_completo);
	
	// Calcular páginas y verificar espacio
    int paginas_necesarias = (int)ceil((double)info_proceso->tamanio_bytes_archivo / configs->tamanio_pagina);
	
    if (contar_frames_libres_en_lista() < paginas_necesarias) {
		log_error(logger, "No hay suficientes frames libres para el proceso %d. Necesarios: %d, Libres: %d", 
				info_proceso->pid, paginas_necesarias, contar_frames_libres_en_lista());
		list_destroy_and_destroy_elements(instrucciones, free);
		return "ERROR_NO_HAY_ESPACIO";
	}
	
	//t_tabla_de_pagina* tabla_global_primer_nvl = crear_tablas_de_paginas(); ANTES
    t_tabla_de_pagina* tabla_L1 = crear_arbol_de_paginacion_completo();
    if (tabla_L1 == NULL) {
        log_error(logger, "Fallo al crear la estructura de paginación para el PID: %d", info_proceso->pid);
        list_destroy_and_destroy_elements(instrucciones, free);
        return "ERROR_PAGINACION";
    }

	t_value_proceso* value_proceso = llenar_contenido_proceso(info_proceso, instrucciones,tabla_L1);
    
    //asignar_frames_requeridos(tabla_global_primer_nvl, paginas_necesarias, info_proceso->pid); ANTES
    asignar_frames_a_proceso(value_proceso, paginas_necesarias, info_proceso->pid);

    //Guardo en diccionario
    dictionary_put(dicc_pids_con_instrucciones, key_pid, value_proceso);

    log_info(logger, "Se guardo el proceso %d en la memoria", info_proceso->pid);

    return "OK";
}

t_tabla_de_pagina* crear_arbol_de_paginacion_completo() {
    uint32_t total_frames = configs->tamanio_memoria / configs->tamanio_pagina;
    int cant_niveles = configs->cant_niveles;
    int entradas_por_tabla = configs->entradas_por_tabla;

    // Calcular cuántas tablas se necesitan por nivel
    t_list* cant_tablas_por_nivel = list_create();
    double elementos_a_direccionar = total_frames;

    for (int i = 0; i < cant_niveles; i++) {
        uint32_t tablas_necesarias = (uint32_t)ceil(elementos_a_direccionar / entradas_por_tabla);
        uint32_t* ptr_tablas = malloc(sizeof(uint32_t));
        *ptr_tablas = tablas_necesarias;
        list_add(cant_tablas_por_nivel, ptr_tablas);
        elementos_a_direccionar = tablas_necesarias;
    }

    // Ahora cant_tablas_por_nivel contiene [cant_ultimo_nvl, cant_anteultimo_nvl, ..., cant_primer_nvl]
    if (*((uint32_t*)list_get(cant_tablas_por_nivel, cant_niveles - 1)) > 1) {
        log_error(logger, "Configuración inválida: se necesitaría más de una tabla de Nivel 1.");
        list_destroy_and_destroy_elements(cant_tablas_por_nivel, free);
        return NULL;
    }

    // Crear todas las tablas para cada nivel
    t_list* listas_de_tablas = list_create(); // Contendrá N listas, una por cada nivel
    for (int nivel = 1; nivel <= cant_niveles; nivel++) {
        t_list* tablas_del_nivel_actual = list_create();
        uint32_t cantidad = *((uint32_t*)list_get(cant_tablas_por_nivel, cant_niveles - nivel));
        
        for (int i = 0; i < cantidad; i++) {
            list_add(tablas_del_nivel_actual, crear_tabla_de_nivel(nivel));
        }
        list_add(listas_de_tablas, tablas_del_nivel_actual);
    }

    // Enlazar las tablas entre sí (de arriba hacia abajo) ---
    for (int nivel_padre = 1; nivel_padre < cant_niveles; nivel_padre++) {
        t_list* tablas_padre = list_get(listas_de_tablas, nivel_padre - 1);
        t_list* tablas_hijo = list_get(listas_de_tablas, nivel_padre);

        for (int i = 0; i < list_size(tablas_hijo); i++) {
            int indice_tabla_padre = i / entradas_por_tabla;
            int indice_entrada_en_padre = i % entradas_por_tabla;

            t_tabla_de_pagina* tabla_padre_actual = list_get(tablas_padre, indice_tabla_padre);
            t_entrada* entrada_actual = list_get(tabla_padre_actual->entradas, indice_entrada_en_padre);
            
            entrada_actual->ptrTabla = list_get(tablas_hijo, i);
        }
    }

    // Limpieza y retorno de la raíz ---
    t_tabla_de_pagina* raiz = list_get(list_get(listas_de_tablas, 0), 0);
    
    // Liberamos las listas contenedoras, pero no su contenido (las tablas y las listas de tablas)
    list_destroy_and_destroy_elements(cant_tablas_por_nivel, free);
    for(int i = 0; i < list_size(listas_de_tablas); i++){
        list_destroy(list_get(listas_de_tablas, i));
    }
    list_destroy(listas_de_tablas);
    
    return raiz;
}

void asignar_frames_a_proceso(t_value_proceso* value_proceso, int paginas_necesarias, uint32_t pid) {
    t_tabla_de_pagina* tabla_L1 = value_proceso->tabla_primer_nvl;

    for (int i = 0; i < paginas_necesarias; i++) {
        // --- Cálculo dinámico de índices para N niveles ---
        int indices[configs->cant_niveles];
        int pagina_logica_actual = i;

        for (int nivel = configs->cant_niveles; nivel >= 1; nivel--) {
            indices[nivel - 1] = pagina_logica_actual % configs->entradas_por_tabla;
            pagina_logica_actual = floor(pagina_logica_actual / configs->entradas_por_tabla);
        }

        // --- Navegación dinámica del árbol ---
        t_tabla_de_pagina* tabla_actual = tabla_L1;
        for (int nivel = 1; nivel < configs->cant_niveles; nivel++) {
            t_entrada* entrada_intermedia = list_get(tabla_actual->entradas, indices[nivel - 1]);
            tabla_actual = entrada_intermedia->ptrTabla;
        }
        
        // --- Asignación en la tabla de último nivel ---
        t_entrada_ult_tabla* entrada_final = list_get(tabla_actual->entradas, indices[configs->cant_niveles - 1]);
        
        int frame_libre = encontrar_y_asignar_primer_frame_libre(pid, i);
        if (frame_libre == -1) {
            log_error(logger, "INCONSISTENCIA: No se encontró frame libre a pesar del chequeo inicial. Abortando.");
            exit(EXIT_FAILURE); 
        }
        entrada_final->nroFrame = frame_libre;
    }
}

t_value_proceso* llenar_contenido_proceso(t_info_nuevo_proceso* info_proceso, t_list* instrucciones, t_tabla_de_pagina* tabla_global_primer_nvl) {
	//Creo Value del diccionario, que es un t_value_proceso
	t_value_proceso* value_proceso = malloc(sizeof(t_value_proceso)); //LIBERAR MEMORIA DESPUES <-----------------
	
	//Meto tamanio del proceso e instrucciones en el value_proceso
	value_proceso->tamanio_proceso = info_proceso->tamanio_bytes_archivo;
	value_proceso->instrucciones = instrucciones;
	//Asigno la tabla de paginas de nivel 1 al value_proceso
	value_proceso->tabla_primer_nvl = tabla_global_primer_nvl;

	//Metricas del proceso
	value_proceso->metricas = malloc(sizeof(t_metricas_proceso)); //LIBERAR MEMORIA DESPUES <-----------------
	memset(value_proceso->metricas, 0, sizeof(t_metricas_proceso)); // Iguala cada metrica a 0

	return value_proceso;
}

int encontrar_y_asignar_primer_frame_libre(uint32_t pid, uint32_t nro_pagina) {
    for (int i = 0; i < list_size(lista_frames); i++) {
        t_info_frame* frame_info = list_get(lista_frames, i);
        if (frame_info->libre) {
            frame_info->libre = false;
            frame_info->pid_proceso = pid;
            frame_info->nro_pagina = nro_pagina;
            log_info(logger, "Frame %d asignado al PID: %d, Página: %d.", i, pid, nro_pagina);
            return i; // Retorna el índice del frame, que es su número
        }
    }
    return -1; // No hay frames libres
}

uint32_t contar_frames_libres_en_lista() {
    uint32_t contador = 0;
    
    // Función closure para verificar si un frame está libre
    bool esta_libre(void* elemento) {
        t_info_frame* frame = (t_info_frame*) elemento;
        return frame->libre;
    }

    contador = list_count_satisfying(lista_frames, esta_libre);
    
    return contador;
}

t_tabla_de_pagina* crear_tabla_de_nivel(uint32_t nivelTabla) {
	t_tabla_de_pagina* tabla = malloc(sizeof(t_tabla_de_pagina)); //LIBERAR MEMORIA DESPUES <-----------------
	tabla->nivel = nivelTabla; //asigno el nivel de la tabla
	tabla->entradas = list_create(); //creo la lista de entradas de la tabla

	//Creo las entradas de la tabla
	if(nivelTabla!=configs->cant_niveles){ //CANT DE NIVELES es 3 en el caso este
		for (int i = 0; i < configs->entradas_por_tabla; i++) { //CANT_ENTRADAS es 4
			t_entrada* entrada = malloc(sizeof(t_entrada)); //LIBERAR MEMORIA DESPUES <-----------------
			entrada->ptrTabla = NULL;
			list_add(tabla->entradas, entrada);
		}
	}
	else { // Tabla de ultimo nivel
		for (int i = 0; i < configs->entradas_por_tabla; i++) { //CANT_ENTRADAS es 4
			t_entrada_ult_tabla* entrada = malloc(sizeof(t_entrada_ult_tabla)); //LIBERAR MEMORIA DESPUES <-----------------
			entrada->nroFrame = -1; // -1 indica que el frame no está asignado
			list_add(tabla->entradas, entrada);
		}
	}

	return tabla; 
}

void leer_lineas_archivo(FILE* archivo, t_list* instrucciones) {
	char* linea = NULL; //Para que getline asigne memoria para la linea que lee
    size_t len = 0; //tamanio buffer al q apunte linea

    while (getline(&linea, &len, archivo) != -1) { //getline lee linea (en la var linea) hasta el \n o devuelve -1(EOF).
        linea[strcspn(linea, "\n")] = '\0'; //eliminamos el \n al final de la linea (si existe)
        list_add(instrucciones, strdup(linea)); //strdup es necesario para guardar la cadena en espacio dinamico
    }
    free(linea);
}

//recibe el pid del proceso a hacer dump
uint32_t recibir_proceso_dump(int conexion_kernel){

	uint32_t* pid_proceso = malloc(sizeof(uint32_t));
	uint32_t size;

	void *buffer = recibir_buffer(&size, conexion_kernel);

	memcpy(pid_proceso, buffer, sizeof(uint32_t)); //recibo el pid
    uint32_t pid = *pid_proceso;

    free(pid_proceso); 
    return pid;
}

t_peticion_instruccion* recibir_peticion_instruccion(int socket_cliente){
    t_peticion_instruccion* info_peticion_instruccion = malloc(sizeof(t_peticion_instruccion)); //LIBERAR MEMORIA DESPUES <-----------------
	uint32_t size;
	void *buffer = recibir_buffer(&size, socket_cliente);

    uint32_t desplazamiento = 0;
	desplazamiento = des_serializar_buffer_peticion_instruccion(buffer, size, info_peticion_instruccion); //meto en el ultimo argumento todo

    if(desplazamiento != size){
		log_error(logger, "Error al recibir el proceso, tamanios no coinciden");
		free(buffer);
		free(info_peticion_instruccion);
		return NULL;
	}

    free(buffer);
	return info_peticion_instruccion;
}

uint32_t des_serializar_buffer_peticion_instruccion(void* buffer, uint32_t size, t_peticion_instruccion* info_peticion_instruccion)
{
    uint32_t desplazamiento = 0;
    memcpy(&(info_peticion_instruccion->pid), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
    desplazamiento += sizeof(uint32_t);
    memcpy(&(info_peticion_instruccion->pc), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pc
    desplazamiento += sizeof(uint32_t);

    return desplazamiento; //devuelvo el desplazamiento para saber si se recibio todo el buffer (comparando con el size)
}

t_recepcion_lectura* recibir_instruccion_lectura(int socket_cliente) { 
    //se recibe pid, direccion y tamanio de la lectura
	uint32_t size;
    t_recepcion_lectura* recepcion = malloc(sizeof(t_recepcion_lectura));

	void *buffer = recibir_buffer(&size, socket_cliente);

	int desplazamiento = 0;
    memcpy(&recepcion->pid, buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
    
    desplazamiento += sizeof(uint32_t);
	memcpy(&recepcion->direccion, buffer + desplazamiento, sizeof(uint32_t)); //recibo la direccion
	
	desplazamiento += sizeof(uint32_t);
	memcpy(&recepcion->tamanio, buffer + desplazamiento, sizeof(uint32_t)); //recibo el tamanio

	free(buffer);
	
    return recepcion; 
}

t_recepcion_escritura* recibir_instruccion_escritura(int socket_cliente) { 
    //se recibe pid, direccion, tamanio de la cadena y la cadena de datos
	uint32_t size;
    t_recepcion_escritura* recepcion = malloc(sizeof(t_recepcion_escritura)); 

    void *buffer = recibir_buffer(&size, socket_cliente);

    int desplazamiento = 0;
    memcpy(&recepcion->pid, buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
    
    desplazamiento += sizeof(uint32_t);
    memcpy(&recepcion->direccion, buffer + desplazamiento, sizeof(uint32_t)); //recibo la direccion

    desplazamiento += sizeof(uint32_t);
    memcpy(&recepcion->tamanio, buffer + desplazamiento, sizeof(uint32_t)); //recibo el tamanio de la cadena

	recepcion->datos = malloc(sizeof(recepcion->tamanio));
	desplazamiento += sizeof(uint32_t);
	memcpy(recepcion->datos, buffer + desplazamiento, recepcion->tamanio); //recibo la cadena de datos

    free(buffer);
	
    return recepcion;
}

void enviar_info_configs_a_mmu(int conexion) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    
    paquete->codigo_operacion = SOLICITUD_DATOS_PAGINAS;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(uint32_t) * 3; 

    paquete->buffer->stream = malloc(paquete->buffer->size);

    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &(configs->cant_niveles), sizeof(uint32_t));

    offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(configs->entradas_por_tabla), sizeof(uint32_t));

	offset += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &(configs->tamanio_pagina), sizeof(uint32_t));

    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}