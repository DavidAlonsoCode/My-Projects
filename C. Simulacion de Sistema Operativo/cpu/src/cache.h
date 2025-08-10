#ifndef DECODE_H
#define DECODE_H

#include "utils/usar_io.h"
#include "utils/hello.h"
#include <stdbool.h>

/* extern uint32_t cant_entradas_tlb;
extern char* algoritmo_reemplazo_tlb;
extern uint32_t num_pag_solicitado_tlb; 

extern void* memoria_cache;

extern uint32_t cant_entradas_cache;
extern uint32_t retardo_cache;
extern uint32_t tamanio_pagina;
extern uint32_t num_pag_solicitado_cache;
extern char* algoritmo_reemplazo_cache; */

// Estructura principal para TLB
typedef struct {
    t_list* entradas;
} t_tabla_entradas;  

typedef struct {
    uint32_t pagina;
    uint32_t frame;
    uint64_t tiempo_acceso; // Para usarlo con el algoritmo LRU
}t_entrada_tlb;

typedef struct {
    uint32_t pid;
    uint32_t pagina;
    uint32_t direccion_fisica;
    char* contenido;    // Este puntero guarda los datos que se reciben, pero el contenido se copia aparte en la cache
    uint8_t bit_referencia; // Clock
    uint8_t bit_modificado; // Clock-M
}t_entrada_cache;

uint32_t traducir_a_direccion_fisica(uint32_t direccion_logica, uint32_t pid);

// TLB

void crear_TLB(int cantEntradas, char* algoritmo);

t_entrada_tlb* verificar_en_tlb(uint32_t num_pagina);

void agregar_a_tlb(uint32_t pagina, uint32_t frame);

void vaciar_entradas_tlb(uint32_t pid);

void vaciar_tlb();

void vaciar_entradas_cache(uint32_t pid);

void vaciar_cache();

// Cache de paginas

void crear_cache_paginas(int cantEntradas, char* algoritmo, int retardo, uint32_t tamanioPagina, int conexion_memoria);

t_entrada_cache* verificar_en_cache(uint32_t direccion);

void ejecutar_write_en_cache(t_entrada_cache* entrada_cache, char* datos);

char* ejecutar_read_en_cache(t_entrada_cache* entrada_cache, uint32_t tamanio);

void agregar_a_cache(uint32_t direccion_fisica, char* datos, uint32_t pid, char accion);

void enviar_solicitud_escritura(int conexion, uint32_t direccion, char* datos, uint32_t pid); // necesario para actualizar la memoria

// Logs para debugging

void log_tlb(uint32_t pid, const char* momento);

void log_cache(uint32_t pid, const char* momento);

#endif
