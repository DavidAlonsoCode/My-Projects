#include "mmu.h"      // implementa
#include "cache.h"    // importa

uint32_t cantNiveles = 0;
uint32_t cantEntradasTabla = 0;
uint32_t tamanioPagina = 0;

int conexion_memoria;


void recibir_datos_memoria(uint32_t* cantNiveles, uint32_t* cantEntradasTabla, uint32_t* tamanioPagina, int conexion) { 

    uint32_t size;
    void* buffer = recibir_buffer(&size, conexion);

    int offset = 0;
    memcpy(cantNiveles, buffer + offset, sizeof(uint32_t));

    offset += sizeof(uint32_t);
    memcpy(cantEntradasTabla, buffer + offset, sizeof(uint32_t));

    offset += sizeof(uint32_t);
    memcpy(tamanioPagina, buffer + offset, sizeof(uint32_t));

    free(buffer);
}

void enviar_entradas_memoria(uint32_t pid, t_list* entradas, int conexion) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    uint32_t cantEntradas = list_size(entradas);
    
	paquete->codigo_operacion = (uint32_t)PETICION_MARCO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t) + sizeof(uint32_t) * cantEntradas;

	paquete->buffer->stream = malloc(paquete->buffer->size);

    //	op_code | size buffer | pid  | entrada1 | entrada2 ... -> las entradas se envian en orden lv1 -> lv2 -> lv...

    int offset = 0;
    memcpy(paquete->buffer->stream , &pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // Se copian todas las entradas de la lista
    for(int i = 0; i < list_size(entradas); i++) {
        uint32_t entrada = *((uint32_t*)list_get(entradas, i)); // se castea a uint32_t para luego desreferenciarlo con el ultimo *
        memcpy(paquete->buffer->stream + offset, &entrada, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }
	
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}


/* Funciones principal para traducciones */
uint32_t traducir_a_direccion_fisica(uint32_t direccion, uint32_t pid) {
    uint32_t size;
    uint32_t codOp = EN_ESPERA;

    // Datos para acceder a las tablas de paginas
    uint32_t num_pagina = floor(direccion / tamanioPagina);
    log_info(logger, "PID: %d - Cache Miss - Pagina: %d", pid, num_pagina);

    // Datos para acceder a los frames por cualquiera de los casos
    t_entrada_tlb* entrada_tlb;
    uint32_t frame;

    // Primero se verifica la cache de paginas, luego la TLB y finalmente se consulta a la memoria
    if((entrada_tlb = verificar_en_tlb(num_pagina)) != NULL) { 
        // TLB hit, se hace el calculo con la entrada de la TLB 
        log_info(logger, "PID: %d - TLB HIT - Pagina: %d", pid, num_pagina);
        frame = entrada_tlb->frame;

    } else {
        // TLB miss, se debe consultar a la memoria
        log_info(logger, "PID: %d - TLB MISS - Pagina: %d", pid, num_pagina);
        t_list* entrada_niveles = list_create();
        uint32_t idNivel = 1;
        for(int i = 0; i < cantNiveles; i++) {
            // Reservamos memoria para el entero
            uint32_t* entrada_ptr = malloc(sizeof(uint32_t)); 
        
            *entrada_ptr = (uint32_t)floor(num_pagina / pow(cantEntradasTabla, (cantNiveles - idNivel))) % cantEntradasTabla;
        
            // Guardamos el puntero a la memoria reservada
            list_add(entrada_niveles, entrada_ptr); 
            idNivel++;
        }

        enviar_entradas_memoria(pid, entrada_niveles, conexion_memoria);

        while(codOp != PETICION_MARCO) {
            codOp = recibir_operacion(conexion_memoria);
        }

        frame = *((uint32_t*)recibir_buffer(&size, conexion_memoria)); // se castea a uint32_t para luego desreferenciarlo con el ultimo *
        log_info(logger, "PID: %d - OBTENER MARCO - Pagina: %d - Marco: %d", pid, num_pagina, frame);

        agregar_a_tlb(num_pagina, frame); // Como hubo un TLB miss, se agrega la entrada a la TLB
        list_destroy_and_destroy_elements(entrada_niveles, free);
    }

    uint32_t desplazamiento = direccion % tamanioPagina;

    uint32_t direccionFisica = frame * tamanioPagina + desplazamiento; // Calculo final de la direccion fisica -> tamanioPagina es igual al tamanio de cada frame

    return direccionFisica;
}


/* Funcion de inicio de MMU */
void iniciar_mmu(int conexion, valores_cache* config) {
    int entradas_tlb      = atoi(config->entradas_tlb);
    char* reemplazo_tlb   = config->reemplazo_tlb;
    int entradas_cache    = atoi(config->entradas_cache);
    char* reemplazo_cache = config->reemplazo_cache;
    int retardo_cache     = atoi(config->retardo_cache);
    conexion_memoria      = conexion;

    uint32_t codOp = EN_ESPERA;

    enviar_mensaje_codigo_operacion("DameDatossss", conexion_memoria, SOLICITUD_DATOS_PAGINAS);

    while(codOp != SOLICITUD_DATOS_PAGINAS) {
        // Recibe respuesta de la memoria: una linea de instruccion
        codOp = recibir_operacion(conexion_memoria);
    }
    // Se obtienen los datos necesarios para las traducciones
    recibir_datos_memoria(&cantNiveles, &cantEntradasTabla, &tamanioPagina, conexion_memoria);

    if(entradas_tlb > 0) {
        crear_TLB(entradas_tlb, reemplazo_tlb);
    } 
    
    if(entradas_cache > 0) {
        crear_cache_paginas(entradas_cache, reemplazo_cache, retardo_cache, tamanioPagina);
    }
}


/* Limpieza de TLB y caches */

void eliminar_entradas_tlb(uint32_t pid) {
    vaciar_entradas_tlb(pid);
}

void eliminar_entradas_cache(uint32_t pid, int conexion) {
    vaciar_entradas_cache(pid, conexion);
}

void eliminar_tabla_tlb() {
    vaciar_tlb();
}

void eliminar_tabla_cache() {
    vaciar_cache();
}

// Logs para debugging

void log_estado_tlb(uint32_t pid, const char* mensaje) {
    log_tlb(pid, mensaje);
}

void log_estado_cache(uint32_t pid, const char* mensaje) {
    log_cache(pid, mensaje);
}
