#include "cache.h"  // implementa

t_tabla_entradas* cache = NULL;

uint32_t cant_entradas_cache = 0;
uint32_t retardo_cache = 0;
uint32_t tamanio_pagina = 0;
uint32_t num_pag_solicitado_cache;
char* algoritmo_reemplazo_cache = NULL;

// Auxiliares para CLOCK y CLOCK-M

bool tiene_bit_ref_cero(t_entrada_cache* entrada) {
    return entrada->bit_referencia == 0;
}

bool tiene_bit_mod_cero(t_entrada_cache* entrada) {
    return entrada->bit_modificado == 0;
}

// Agregar a cache dado un algoritmo

void agregar_bajo_clock(t_entrada_cache* nueva_entrada) {

    if(list_size(cache->entradas) >= cant_entradas_cache) {
        // Si la cache esta llena, se busca una entrada con bit_referencia = 0
        t_list_iterator* iterador = list_iterator_create(cache->entradas);

        while(list_iterator_has_next(iterador)) {
            t_entrada_cache* entrada_actual = (t_entrada_cache*)list_iterator_next(iterador);

            if(tiene_bit_ref_cero(entrada_actual)) {  // Ref = 0 -> Mejor victima

                // Se reemplaza la entrada actual con la nueva entrada
                entrada_actual->pagina = nueva_entrada->pagina;
                entrada_actual->direccion_fisica = nueva_entrada->direccion_fisica; 
                entrada_actual->contenido = nueva_entrada->contenido; 
                entrada_actual->bit_referencia = 0; // Reiniciar bit de referencia
                list_iterator_destroy(iterador);    // liberar el iterador
                return;                             // Salir del bucle -> De esta forma sale cuando encuentra la primera entrada con bit_ref = 0
            }

            // Si no tiene bit_ref = 0, se lo setea a 0 y se sigue buscando
            entrada_actual->bit_referencia = 0;
        }
    } else {
        nueva_entrada->bit_referencia = 0;
        list_add(cache->entradas, nueva_entrada); // Agregar la nueva entrada a la cache
    }    
}

void agregar_bajo_clock_modificado(t_entrada_cache* nueva_entrada) {

    if(list_size(cache->entradas) >= cant_entradas_cache) {
        // Si la cache esta llena, se busca una entrada con bit_referencia = 0
        t_list_iterator* iterador = list_iterator_create(cache->entradas);

        while(list_iterator_has_next(iterador)) {
            t_entrada_cache* entrada_actual = (t_entrada_cache*)list_iterator_next(iterador);

            if(tiene_bit_ref_cero(entrada_actual) && tiene_bit_mod_cero(entrada_actual)) {  // Ref = 0 ; Mod = 0 -> Mejor victima
                // Se reemplaza la entrada actual con la nueva entrada
                entrada_actual->pagina = nueva_entrada->pagina;
                entrada_actual->direccion_fisica = nueva_entrada->direccion_fisica;
                entrada_actual->contenido = nueva_entrada->contenido;
                entrada_actual->bit_referencia = 0; // Reiniciar bit de referencia
                entrada_actual->bit_modificado = 0; // Reiniciar bit de modificado
                list_iterator_destroy(iterador);    // liberar el iterador
                return;                             // Salir del bucle -> De esta forma sale cuando encuentra la primera entrada con bit_ref = 0

            } else if (tiene_bit_ref_cero(entrada_actual) && !tiene_bit_mod_cero(entrada_actual)) { // Ref = 0 ; Mod = 1 -> Se le da una oportunidad
                entrada_actual->bit_modificado = 0; 

            } else if (!tiene_bit_ref_cero(entrada_actual) && tiene_bit_mod_cero(entrada_actual)) { // Ref = 1 ; Mod = 0 -> Se le da una oportunidad
                entrada_actual->bit_referencia = 0; 

            } else { // Ref = 1 ; Mod = 1 -> Peor victima
                entrada_actual->bit_referencia = 0;

            }
        }
    } else {
        nueva_entrada->bit_referencia = 0;
        nueva_entrada->bit_modificado = 0;
        list_add(cache->entradas, nueva_entrada); // Agregar la nueva entrada a la cache
    }    
}

void agregar_a_cache(uint32_t direccion, char* datos, uint32_t pid) {

    if(cant_entradas_cache == 0) { // Verifica que la cache este activada
        return;
    }
    
    t_entrada_cache* nuevaEntrada = malloc(sizeof(t_entrada_cache));
    nuevaEntrada->pagina = num_pag_solicitado_cache; // Revisar misma logica de asignacion en TLB
    nuevaEntrada->direccion_fisica = direccion; // Direccion fisica donde se guarda la pagina
    nuevaEntrada->contenido = datos; // Asignar el contenido de la pagina

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
                                                                              
// Creacion de cache de paginas

void crear_cache_paginas(int cant_entradas, char* algoritmo, int retardo, uint32_t tamanioPag) {
    t_tabla_entradas* tabla = malloc(sizeof(t_tabla_entradas));

    tabla->entradas = list_create();

    cant_entradas_cache = cant_entradas;
    algoritmo_reemplazo_cache = algoritmo;
    retardo_cache = retardo;
    tamanio_pagina = tamanioPag;
    cache = tabla;
}

// Limpieza de cache

void vaciar_entradas_cache(uint32_t pid, int conexion) {

    if (cache != NULL) {
        uint32_t pagina, direccion;
        char* contenido;
        
        // Antes de vaciar la cache, se actualizan las paginas en memoria
        for(int i = 0; i < list_size(cache->entradas); i++) {
            t_entrada_cache* entrada_cache = (t_entrada_cache*)list_get(cache->entradas, i);
            pagina = entrada_cache->pagina; // Necesario para el log
            direccion = entrada_cache->direccion_fisica;
            contenido = entrada_cache->contenido;

            // Enviar la solicitud de escritura a memoria para actualizar los datos
            enviar_solicitud_escritura(conexion, direccion, contenido, pid);

            // Confirmar que la operacion se realizo correctamente
            uint32_t codOp = EN_ESPERA;
            uint32_t cadenaSize;

            while(codOp != MENSAJE) {
                codOp = recibir_operacion(conexion);
            }
            char* respuesta = recibir_buffer(&cadenaSize, conexion);
            
            strcmp(respuesta, "OK") == 0 ? 
                log_info(logger, "PID: %d - Memory Update - Pagina: %d - Frame: %d", pid, pagina, direccion) : 
                log_error(logger, "Error al actualizar la pagina %d en memoria para el PID: %d", pagina, pid);
            
            free(respuesta);
        }

        list_clean_and_destroy_elements(cache->entradas, free);  // Libera las entradas de la TLB
        log_info(logger, "Entradas de cache eliminadas correctamente para el PID: %d", pid);
    }
}

void vaciar_cache() {
    if(cache != NULL) {
        list_destroy_and_destroy_elements(cache->entradas, free);  // Libera las entradas de la TLB
        free(cache);  // Libera la estructura de la tabla de entradas
        cache = NULL;  // Resetea el puntero a NULL
        log_info(logger, "Cache de paginas eliminada correctamente");
    }
}

// Log para debugging

void log_cache(uint32_t pid, const char* momento) {
    if (cache == NULL || list_is_empty(cache->entradas)) {
        log_info(logger, "PID: %d - DUMP Cache (%s): La Cache de páginas está vacía.", pid, momento);
        return;
    }

    log_info(logger, "--- Inicio DUMP Cache para PID: %d (%s) ---", pid, momento);
    int cantidad_entradas = list_size(cache->entradas);

    for (int i = 0; i < cantidad_entradas; i++) {
        t_entrada_cache* entrada = (t_entrada_cache*)list_get(cache->entradas, i);
        if (entrada != NULL) {
            // Calculamos el frame a partir de la dirección física y el tamaño de página
            uint32_t frame = entrada->direccion_fisica / tamanio_pagina;
            log_info(logger, "  [Cache] Entrada %d: | Pagina: %-5u | Frame: %-5u | R: %d | M: %d", 
                     i, entrada->pagina, frame, entrada->bit_referencia, entrada->bit_modificado);
        }
    }
    log_info(logger, "--- Fin DUMP Cache ---");
}



