#include "cache.h"  // implementa

t_tabla_entradas* tlb;
uint32_t cant_entradas_tlb = 0;
char* algoritmo_reemplazo_tlb = NULL;
uint32_t num_pag_solicitado_tlb;

// Auxiliares para LRU

uint64_t obtenerTiempoAhora() {
	struct timeval tv;
	gettimeofday(&tv, NULL);

    // Devuelve el tiempo actual en milisegundos
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000); 
}

void* elegir_mas_antigua(void* entrada1, void* entrada2) {
    t_entrada_tlb* e1 = (t_entrada_tlb*)entrada1;
    t_entrada_tlb* e2 = (t_entrada_tlb*)entrada2;

    // Compara el uso reciente de las dos entradas y devuelve el menor
    return e1->tiempo_acceso <= e2->tiempo_acceso ? entrada1 : entrada2; 
}


// Agregar entradas a la TLB dado un algoritmo

void agregar_bajo_fifo(t_entrada_tlb* nueva_entrada) {

    // Si la TLB esta llena, se elimina la primera entrada de la lista (la mas antigua)
    if(list_size(tlb->entradas) >= cant_entradas_tlb) {
        t_entrada_tlb* entrada_a_reemplazar = list_remove(tlb->entradas, 0);
        free(entrada_a_reemplazar);
    }
    // De lo contrario, solo se agrega la entrada
    list_add(tlb->entradas, nueva_entrada);
}

void agregar_bajo_lru(t_entrada_tlb* nueva_entrada) {

    nueva_entrada->tiempo_acceso  = obtenerTiempoAhora(); // Se asigna el uso más reciente (valor mas alto) a la nueva entrada
    
    // Si la TLB esta llena, se elimina la entrada con el uso mas antiguo (valor más bajo con timestamp)
    if(list_size(tlb->entradas) >= cant_entradas_tlb) {
        
        // Se obtiene la que tenga el uso reciente mas bajo
        t_entrada_tlb* entrada_a_reemplazar = list_get_minimum(tlb->entradas, elegir_mas_antigua);
        // Para eliminar el elemento, se usa list_remove_element que devuelve true si lo pudo eliminar.
        if (list_remove_element(tlb->entradas, entrada_a_reemplazar)) free(entrada_a_reemplazar);
    }
    // De lo contrario, solo agrega la entrada
    list_add(tlb->entradas, nueva_entrada);
}

void agregar_a_tlb(uint32_t pagina, uint32_t frame) {

    if(cant_entradas_tlb == 0) { // Verifica que la TLB este activada
        return;
    }

    t_entrada_tlb* nuevaEntrada = malloc(sizeof(t_entrada_tlb));
    nuevaEntrada->pagina = pagina;
    nuevaEntrada->frame  = frame;

    if (strcmp(algoritmo_reemplazo_tlb, "FIFO") == 0) {
        agregar_bajo_fifo(nuevaEntrada);
    } else if (strcmp(algoritmo_reemplazo_tlb, "LRU") == 0) {
        agregar_bajo_lru(nuevaEntrada);
    } else {
        // Implementar otro algoritmo de reemplazo si es necesario
        log_error(logger, "Algoritmo de reemplazo no soportado: %s", algoritmo_reemplazo_tlb);
        free(nuevaEntrada);
        return;
    }
}

// Busqueda de paginas

bool pagina_esta_en_tlb(void* entrada) {
    t_entrada_tlb* e = (t_entrada_tlb*)entrada;
    return e->pagina == num_pag_solicitado_tlb;
}

t_entrada_tlb* verificar_en_tlb(uint32_t num_pagina) {
    t_entrada_tlb* entrada = NULL;

    if(cant_entradas_tlb != 0) { // Verifica que la TLB este activada
        num_pag_solicitado_tlb = num_pagina;

        entrada = list_find(tlb->entradas, pagina_esta_en_tlb);

        if(entrada != NULL && strcmp(algoritmo_reemplazo_tlb, "LRU") == 0) {
            // Si se usa LRU, se actualiza el uso reciente de la entrada encontrada
            entrada->tiempo_acceso = obtenerTiempoAhora();  // Se asigna el uso más reciente (valor mas alto)
        }
    }

    return entrada;  // Retorna NULL si no se encuentra la pagina en la TLB
}

// Creacion de TLB

void crear_TLB(int cant_entradas, char* algoritmo) {
    t_tabla_entradas* tabla = malloc(sizeof(t_tabla_entradas));

    tabla->entradas = list_create();

    cant_entradas_tlb = cant_entradas;  // Se guarda la cantidad de entradas de la TLB
    algoritmo_reemplazo_tlb = algoritmo;  // Se guarda el algoritmo de reemplazo para la TLB
    tlb = tabla;  // la tabla creada sera la tlb del modulo
}

// Limpiar paginas de/y TLB

void vaciar_entradas_tlb(uint32_t pid) {
    if (tlb != NULL) {
        list_clean_and_destroy_elements(tlb->entradas, free);  // Libera las entradas de la TLB
        log_info(logger, "Entradas de TLB eliminadas correctamente para el PID: %d", pid);
    }
}

void vaciar_tlb() {
    if(tlb != NULL) {
        list_destroy_and_destroy_elements(tlb->entradas, free);  // Libera las entradas de la TLB
        free(tlb);  // Libera la estructura de la tabla de entradas
        tlb = NULL;  // Resetea el puntero a NULL
        log_info(logger, "Tabla de TLB eliminada correctamente"); 
    }
}

// Log para debugging

void log_tlb(uint32_t pid, const char* momento) {
    if (tlb == NULL || list_is_empty(tlb->entradas)) {
        log_debug(logger, "PID: %d - DUMP TLB (%s): La TLB está vacía.", pid, momento);
        return;
    }

    log_debug(logger, "--- Inicio DUMP TLB para PID: %d (%s) ---", pid, momento);
    int cantidad_entradas = list_size(tlb->entradas);

    for (int i = 0; i < cantidad_entradas; i++) {
        t_entrada_tlb* entrada = (t_entrada_tlb*)list_get(tlb->entradas, i);
        if (entrada != NULL) {
            log_debug(logger, "  [TLB] Entrada %d: | Pagina: %-5u | Frame: %-5u", i, entrada->pagina, entrada->frame);
        }
    }
    log_debug(logger, "--- Fin DUMP TLB ---");
}