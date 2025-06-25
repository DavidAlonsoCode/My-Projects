#include <stdio.h>
#include "ciclo.h"         // implementa

// Esta funcion es similar a la que se encuentra en interfaz.c de kernel
// La unica diferencia es el codigo de operacion
void enviar_solicitud_a_memoria(int conexion,uint32_t pid,uint32_t pc){
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)PETICION_INSTRUCCION;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t)*2;

	paquete->buffer->stream = malloc(paquete->buffer->size);

//	op_code | size buffer | pid | pc

    int offset = 0;
    memcpy(paquete->buffer->stream , &pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));
	
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

// Esta etapa solicita la instruccion a memoria y la recibe como cadena de caracteres
char* fetch(t_proceso_actual* proceso) {
    uint32_t cadenaSize;
    uint32_t pid         = proceso->pid;                    // Proceso a ejecutar
    uint32_t pc          = proceso->pc;                     // Contador
    int conexion_memoria = proceso->fd_conexion_memoria; // Conexion a memoria

    uint32_t codOp = EN_ESPERA; // Inicializa el codigo de operacion

    // Envia la solicitud a memoria a traves de un paquete -> Por cada solicitud SIEMPRE SE ENVIA PID Y PC
    enviar_solicitud_a_memoria(conexion_memoria, pid, pc);

    while(codOp != LINEA_DE_INSTRUCCION) {
        // Recibe respuesta de la memoria: una linea de instruccion
        codOp = recibir_operacion(conexion_memoria);
    }

    char* instruccion = recibir_buffer(&cadenaSize, conexion_memoria);
    log_info(logger, "## PID: %d - FETCH - Program Counter: %d", pid, pc);

	return instruccion;
}


