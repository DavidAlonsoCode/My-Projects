#include "cache.h"  // implementa

t_tabla_entradas* cache = NULL;

int fd_conexion_memoria = -1;
uint32_t cant_entradas_cache = 0;
uint32_t retardo_cache = 0;
uint32_t tamanio_pagina = 0;
uint32_t num_pag_solicitado_cache;

char* algoritmo_reemplazo_cache = NULL;
t_list_iterator* iterador;

// Actualizacion de paginas

void actualizar_pagina_en_memoria(t_entrada_cache* entrada_cache){
    
    uint32_t pid = entrada_cache->pid; // PID de la entrada cacheada
    uint32_t pagina = entrada_cache->pagina; // Necesario para el log
    uint32_t direccion = entrada_cache->direccion_fisica;
    char* contenido = entrada_cache->contenido;

    // Enviar la solicitud de escritura a memoria para actualizar los datos
    enviar_solicitud_escritura(fd_conexion_memoria, direccion, contenido, pid);

    // Confirmar que la operacion se realizo correctamente
    uint32_t codOp = EN_ESPERA;
    uint32_t cadenaSize;
    uint32_t frame = (uint32_t)floor((double)(direccion / tamanio_pagina)); 

    while(codOp != MENSAJE) {
        codOp = recibir_operacion(fd_conexion_memoria);
    }
    char* respuesta = recibir_buffer(&cadenaSize, fd_conexion_memoria);
    
    strcmp(respuesta, "OK") == 0 ? 
        log_info(logger, "PID: %d - Memory Update - Pagina: %d - Frame: %d", pid, pagina, frame) : 
        log_error(logger, "Error al actualizar la pagina %d en memoria para el PID: %d", pagina, pid);
    
    free(respuesta);
}

// Auxiliares para CLOCK y CLOCK-M

bool tiene_bit_ref_cero(t_entrada_cache* entrada) {
    return entrada->bit_referencia == 0;
}

bool tiene_bit_mod_cero(t_entrada_cache* entrada) {
    return entrada->bit_modificado == 0;
}

void reemplazar_entrada_en_cache(t_entrada_cache* nueva_entrada, t_entrada_cache* entrada_actual) {

    //Antes de reemplazar, se revisa si la pagina de la entrada actual fue modificada
    if(entrada_actual->bit_modificado) {
        actualizar_pagina_en_memoria(entrada_actual); 
    }

    // Se reemplaza la entrada actual con la nueva entrada
    entrada_actual->pagina = nueva_entrada->pagina;
    entrada_actual->direccion_fisica = nueva_entrada->direccion_fisica;

    free(entrada_actual->contenido); // Se libera el espacio del anterior puntero antes de asignar el nuevo
    entrada_actual->contenido = nueva_entrada->contenido; 

    entrada_actual->bit_referencia = nueva_entrada->bit_referencia; // Reiniciar bit de referencia
    entrada_actual->bit_modificado = nueva_entrada->bit_modificado; // Reiniciar bit de modificado
    free(nueva_entrada);
}

void verificar_contadores(uint8_t* iteracion, uint16_t* accesos) {
    // Se incrementa iteracion cuando se recorren todos los accesos
    if((*accesos) == cant_entradas_cache) { 
        (*iteracion) = 2;
    // Si se recorren todos los accesos de nuevo, se resetea al estado inicial
    } else if ((*accesos) == 2* cant_entradas_cache) { 
        (*iteracion) = 1;
        (*accesos)   = 0;
    }
}

// Agregar a cache dado un algoritmo

void agregar_bajo_clock(t_entrada_cache* nueva_entrada) {

    // Si la cache esta llena, se busca una victima para reemplazar
    if(list_size(cache->entradas) >= cant_entradas_cache) {

        if(iterador == NULL) {
            iterador = list_iterator_create(cache->entradas); // Inicializa el iterador por unica vez
        }
        
        while(1) {
            if(!list_iterator_has_next(iterador)) {
                list_iterator_destroy(iterador);
                iterador = list_iterator_create(cache->entradas);
            } // se resetea la iteracion si llega al final

            t_entrada_cache* entrada_actual = (t_entrada_cache*)list_iterator_next(iterador);
            // Se busca una entrada con Ref = 0 -> Mejor victima
            if(tiene_bit_ref_cero(entrada_actual)) {  
                reemplazar_entrada_en_cache(nueva_entrada, entrada_actual);
                return; // Salir del bucle -> De esta forma sale cuando encuentra la primera entrada con bit_ref = 0
            }
            // Si no tiene bit_ref = 0, se lo setea a 0 y se sigue buscando
            entrada_actual->bit_referencia = 0;
        }
    // De lo contrario, solo se agrega la nueva entrada a la cache
    } else {
        //nueva_entrada->bit_referencia = 1;
        //nueva_entrada->bit_modificado = 0;
        list_add(cache->entradas, nueva_entrada); // Agregar la nueva entrada a la cache
    }    
}

void agregar_bajo_clock_modificado(t_entrada_cache* nueva_entrada) {

    // Si la cache esta llena, se busca una victima para reemplazar
    if(list_size(cache->entradas) >= cant_entradas_cache) {
        uint8_t cont_iteracion = 1;
        uint16_t cont_accesos  = 0;

        if(iterador == NULL) {
            iterador = list_iterator_create(cache->entradas); // Inicializa el iterador por unica vez
        }

        while(1) {
            if(!list_iterator_has_next(iterador)) {
                list_iterator_destroy(iterador);
                iterador = list_iterator_create(cache->entradas);
            } // se resetea la iteracion si llega al final
            verificar_contadores(&cont_iteracion, &cont_accesos);

            t_entrada_cache* entrada_actual = (t_entrada_cache*)list_iterator_next(iterador);
            // Primera iteracion -> se busca un par (0,0) -> mejor victima - sin modificaciones
            if(cont_iteracion == 1) {
                if(tiene_bit_ref_cero(entrada_actual) && tiene_bit_mod_cero(entrada_actual)) {
                    reemplazar_entrada_en_cache(nueva_entrada, entrada_actual);
                    return; // Salir del bucle -> De esta forma sale cuando encuentra la primera entrada con bit_ref = 0
                } 
                cont_accesos++;

            // Segunda iteracion -> se busca un par (0,1) -> victima - se modifica el bit de ref   
            } else if(cont_iteracion == 2) {
                if(tiene_bit_ref_cero(entrada_actual) && !tiene_bit_mod_cero(entrada_actual)) {
                    reemplazar_entrada_en_cache(nueva_entrada, entrada_actual);
                    return; // Salir del bucle -> De esta forma sale cuando encuentra la primera entrada con bit_ref = 0
                } else {
                    // Si no tiene bit_ref = 0, se lo setea a 0 y se sigue buscando
                    entrada_actual->bit_referencia = 0;
                    cont_accesos++;
                }
            } 
        }
    // De lo contrario, solo se agrega la nueva entrada a la cache
    } else {
        //nueva_entrada->bit_referencia = 1;
        //nueva_entrada->bit_modificado = 0; // Se entiende que las nuevas entradas no fueron modificadas
        list_add(cache->entradas, nueva_entrada); // Agregar la nueva entrada a la cache
    }    
}

void agregar_a_cache(uint32_t direccion, char* datos, uint32_t pid, char accion) {

    if(cant_entradas_cache == 0) { // Verifica que la cache este activada
        return;
    }
    
    t_entrada_cache* nuevaEntrada = malloc(sizeof(t_entrada_cache));
    nuevaEntrada->pid = pid; // Asignar el PID para el log
    nuevaEntrada->pagina = num_pag_solicitado_cache; // Revisar misma logica de asignacion en TLB
    nuevaEntrada->direccion_fisica = direccion; // Direccion fisica donde se guarda la pagina
    nuevaEntrada->contenido = malloc(strlen(datos) + 1);
    strcpy(nuevaEntrada->contenido, datos); // Asignar el contenido de la pagina
    nuevaEntrada->bit_referencia = 1; // Marcar como referenciada

    if (accion == 'W') {
        nuevaEntrada->bit_modificado = 1; // Marcar como modificada si es una escritura
    } else {
        nuevaEntrada->bit_modificado = 0; // No modificada si no es una escritura
    }

    log_info(logger, "PID: %d - Cache Add - Pagina: %d", pid, nuevaEntrada->pagina);

    if (strcmp(algoritmo_reemplazo_cache, "CLOCK") == 0) {
        agregar_bajo_clock(nuevaEntrada);
    } else if (strcmp(algoritmo_reemplazo_cache, "CLOCK-M") == 0) {
        agregar_bajo_clock_modificado(nuevaEntrada);
    } 
}

// Busqueda de paginas

bool pagina_esta_en_cache(void* entrada) {
    t_entrada_cache* e = (t_entrada_cache*)entrada;
    return e->pagina == num_pag_solicitado_cache;
}

t_entrada_cache* verificar_en_cache(uint32_t direccion) {
    t_entrada_cache* entrada = NULL;

    if(cant_entradas_cache != 0) { // Verifica que la cache este activada
        num_pag_solicitado_cache = direccion / tamanio_pagina;

        entrada = list_find(cache->entradas, pagina_esta_en_cache); 
    }
    
    return entrada; // Retorna NULL si no se encuentra la pagina en la cache
}

// Escritura y lectura en cache

void ejecutar_write_en_cache(t_entrada_cache* entrada_cache, char* datos) {

    usleep(retardo_cache * 1000);
    // Actualiza el contenido de la entrada de cache con los nuevos datos
    free(entrada_cache->contenido);
    entrada_cache->contenido = malloc(strlen(datos) + 1);
    strcpy(entrada_cache->contenido, datos); // Asignar el contenido de la pagina
    
    entrada_cache->bit_referencia = 1; // Marcar como referenciada
    entrada_cache->bit_modificado = 1; // Marcar como modificado
}

char* ejecutar_read_en_cache(t_entrada_cache* entrada_cache, uint32_t tamanio) {

    usleep(retardo_cache * 1000);
    // Retorna los datos leidos desde la cache
    char* datos = malloc(tamanio + 1); // +1 para el terminador \0
    memcpy(datos, entrada_cache->contenido, tamanio);
    datos[tamanio] = '\0'; // Asegurar que la cadena este terminada en \0
    entrada_cache->bit_referencia = 1;
    return datos;
}
                                                                              
// Creacion de cache de paginas

void crear_cache_paginas(int cant_entradas, char* algoritmo, int retardo, uint32_t tamanioPag, int conexion) {
    t_tabla_entradas* tabla = malloc(sizeof(t_tabla_entradas));

    tabla->entradas = list_create();
    cant_entradas_cache = cant_entradas;
    algoritmo_reemplazo_cache = algoritmo;
    retardo_cache = retardo;
    tamanio_pagina = tamanioPag;
    fd_conexion_memoria = conexion;
    cache = tabla; // Asigna la tabla global de la cache
}

// Limpieza de cache

void entrada_destroyer(void* e) {
    t_entrada_cache* entrada = (t_entrada_cache*) e;
    free(entrada->contenido);
    free(entrada);
}

void vaciar_entradas_cache(uint32_t pid) {

    if (cache != NULL) {
        // Antes de vaciar la cache, se actualizan las paginas en memoria
        for(int i = 0; i < list_size(cache->entradas); i++) {
            t_entrada_cache* entrada_cache = (t_entrada_cache*)list_get(cache->entradas, i);
            if(entrada_cache->bit_modificado){
                actualizar_pagina_en_memoria(entrada_cache);
            }
        }

        list_clean_and_destroy_elements(cache->entradas, entrada_destroyer);  // Libera las entradas de la TLB
        list_iterator_destroy(iterador); // Resetea el iterador
        iterador = NULL;
        log_info(logger, "Entradas de cache eliminadas correctamente para el PID: %d", pid);
    }
}

void vaciar_cache() {
    if(cache != NULL) {
        list_destroy_and_destroy_elements(cache->entradas, entrada_destroyer);  // Libera las entradas de la TLB
        list_iterator_destroy(iterador); // Resetea el iterador
        iterador = NULL;
        free(cache);  // Libera la estructura de la tabla de entradas
        cache = NULL;  // Resetea el puntero a NULL
        log_info(logger, "Cache de paginas eliminada correctamente");
    }
}

// Log para debugging

void log_cache(uint32_t pid, const char* momento) {
    if (cache == NULL || list_is_empty(cache->entradas)) {
        log_debug(logger, "PID: %d - DUMP Cache (%s): La Cache de páginas está vacía.", pid, momento);
        return;
    }

    log_debug(logger, "--- Inicio DUMP Cache para PID: %d (%s) ---", pid, momento);
    int cantidad_entradas = list_size(cache->entradas);

    for (int i = 0; i < cantidad_entradas; i++) {
        t_entrada_cache* entrada = (t_entrada_cache*)list_get(cache->entradas, i);
        if (entrada != NULL) {
            // Calculamos el frame a partir de la dirección física y el tamaño de página
            uint32_t frame = entrada->direccion_fisica / tamanio_pagina;
            log_debug(logger, "  [Cache] Entrada %d: | Pagina: %-5u | Frame: %-5u | R: %d | M: %d", 
                     i, entrada->pagina, frame, entrada->bit_referencia, entrada->bit_modificado);
        }
    }
    log_debug(logger, "--- Fin DUMP Cache ---");
}



