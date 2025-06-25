#ifndef SYSCALLS_H
#define SYSCALLS_H
#include "interfaz.h" // la necesito si quiero usar el pid global
#include "utils/usar_io.h"

void syscall_init_proc(char*archivo, uint32_t tamanio_archivo);
void syscall_exit(uint32_t pid_a_finalizar);
void syscall_dump_memory(uint32_t pid);
void* espera_respuesta_dump(void* datos_espera_dump);
void enviar_proceso_dump(int conexion_memoria,uint32_t pid);
void mostrar_metricas_finales(pcb* pcb_finalizado);
void destruir_metrica(void* metrica);
void liberar_pcb(pcb* proceso_a_finalizar);
void enviar_destruccion_proceso(uint32_t pid_string,int conexion_memoria);



#endif