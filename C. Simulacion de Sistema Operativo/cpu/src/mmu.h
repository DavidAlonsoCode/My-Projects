#ifndef MMU_H
#define MMU_H

#include "utils/hello.h"

typedef struct {
    char* entradas_tlb;
    char* reemplazo_tlb;
    char* entradas_cache;
    char* reemplazo_cache;
    char* retardo_cache;
} valores_cache;

void iniciar_mmu(int fd_conexion, valores_cache* configuracion);

void eliminar_entradas_tlb(uint32_t pid);

void eliminar_entradas_cache(uint32_t pid);

void eliminar_tabla_tlb();

void eliminar_tabla_cache();

#endif