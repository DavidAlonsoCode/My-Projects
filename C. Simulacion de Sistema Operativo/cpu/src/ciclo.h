#ifndef CPU_H
#define CPU_H

#include "utils/hello.h"
#include "stdbool.h"

// Lista de operaciones
typedef enum {
    INICIO,
    INST_NOOP,
    INST_WRITE,
    INST_READ,
    INST_GOTO,
    INST_IO,
    INST_INIT_PROC,
    INST_DUMP_MEMORY,    
    INST_EXIT
} t_instruccion;

// Estructura para guardar los datos del proceso recibido
typedef struct {
    uint32_t pid;     
    uint32_t pc;
    int fd_conexion_kernel;
    int fd_conexion_memoria;   
} t_proceso_actual;

void recibir_mensaje_desde_memoria(int fd_conexion);

void enviar_solicitud_a_memoria(int conexion,uint32_t pid,uint32_t pc);

int crear_conexion_segun(char* tipo, char* ip, char* puerto);

t_proceso_actual* recibir_paquete_proceso_cpu(int conexion);

t_instruccion fetch_decode_execute(t_proceso_actual* proceso);

uint32_t check_interrupt(int conexion);

void enviar_dump_a_kernel(int conexion, uint32_t pid, uint32_t pc, uint32_t codOp);

void enviar_paquete_proceso_kernel_segun(int conexion, uint32_t pid, uint32_t pc, uint32_t codOp);

char* fetch(t_proceso_actual* proceso);

void ejecutar_write(t_proceso_actual* proceso, uint32_t direccion, char* datos);

char* ejecutar_read(t_proceso_actual* proceso, uint32_t direccion, uint32_t tamanio);

void ejecutar_goto(t_proceso_actual* proceso, uint32_t valor); 

void ejecutar_io(t_proceso_actual* proceso, char* dispositivo, uint32_t tiempo);

void ejecutar_init_proc(t_proceso_actual* proceso, char* archivo, uint32_t tamanio);

void ejecutar_dump_memory(t_proceso_actual* proceso);

void ejecutar_exit(t_proceso_actual* proceso);

#endif