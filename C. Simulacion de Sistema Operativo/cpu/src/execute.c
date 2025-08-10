#include "ciclo.h"         // implementa
#include "cache.h"         // importa

/* Funciones auxiliares para READ y WRITE */

void enviar_solicitud_escritura(int conexion, uint32_t direccion, char* datos, uint32_t pid){

	t_paquete *paquete = malloc(sizeof(t_paquete));
    uint32_t tam_datos = strlen(datos) + 1; // +1 para el terminador \0

	paquete->codigo_operacion = (uint32_t)PETICION_ESCRITURA;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t)*3 + tam_datos; // +1 para el terminador \0

	paquete->buffer->stream = malloc(paquete->buffer->size);

    //	op_code | size buffer | pid | direccion | size datos | datos

    int desplazamiento = 0;
    memcpy(paquete->buffer->stream, &pid, sizeof(uint32_t));
    
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &direccion, sizeof(uint32_t));

    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &tam_datos, sizeof(uint32_t));
	
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, datos, tam_datos);

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void enviar_solicitud_lectura(int conexion, uint32_t direccion, uint32_t tamanio, uint32_t pid) {
    
    t_paquete *paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = (uint32_t)PETICION_LECTURA;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(uint32_t)*3; // +1 para el terminador \0

    paquete->buffer->stream = malloc(paquete->buffer->size);

    //	op_code | size buffer | pid | direccion | tamanio

    int desplazamiento = 0;
    memcpy(paquete->buffer->stream, &pid, sizeof(uint32_t));
    
    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &direccion, sizeof(uint32_t));

    desplazamiento += sizeof(uint32_t);
    memcpy(paquete->buffer->stream + desplazamiento, &tamanio, sizeof(uint32_t));

    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}

void enviar_syscall_io(int conexion, uint32_t pid, uint32_t pc, char* dispositivo, uint32_t tiempo) {
    //aca ejecutamos la consulta enviamos el mensaje a io
    t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)IO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t)*4
                            +(strlen(dispositivo) + 1);

	paquete->buffer->stream = malloc(paquete->buffer->size);

//	op_code | size buffer | pid | pc | tiempo | tamanio_nombre_dispositivo | nombre_dispositivo

	uint32_t tamanio_nombre_dispositivo = strlen(dispositivo) + 1;
    
    int offset = 0;
    memcpy(paquete->buffer->stream , &pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &tiempo, sizeof(uint32_t));
    offset += sizeof(uint32_t);


    memcpy(paquete->buffer->stream + offset, &tamanio_nombre_dispositivo, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, dispositivo, tamanio_nombre_dispositivo);

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);

}

void enviar_init_proc(int conexion, uint32_t pid, char* archivo, uint32_t tamanio){
    int offset = 0;
    
    uint32_t longitud_archivo = string_length(archivo)+1;

    //agrego pid | pc | tamanionombrearchivo | nombre | tamaniobytesarchivo
    //porque se envia el PC?
    t_buffer* buff = crear_buffer_segun(sizeof(uint32_t)*3 + longitud_archivo);
    void* inicio = buff->stream;
    aniadir_int_a_buffer(buff, pid, &offset);
    
    aniadir_string_a_buffer(buff, longitud_archivo, archivo, &offset);
    aniadir_int_a_buffer(buff, tamanio, &offset);

    buff->stream = inicio;

    empaquetar_y_enviar(INIT_PROC, buff, conexion);

   // liberar_buffer(buff);
}

void enviar_exit(int conexion, uint32_t pid) {
    t_buffer* buff = crear_buffer_segun(sizeof(uint32_t));
    void* inicio = buff->stream;
    int offset = 0;

    aniadir_int_a_buffer(buff, pid, &offset);
    
    buff->stream = inicio;

    empaquetar_y_enviar(EXIT, buff, conexion);

    // liberar_buffer(buff);
}
void enviar_dump_a_kernel(int conexion, uint32_t pid, uint32_t pc, uint32_t codOp) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    
    paquete->codigo_operacion = codOp;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(uint32_t) * 2; 

    paquete->buffer->stream = malloc(paquete->buffer->size);

    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &pid, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));


    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}



void enviar_paquete_proceso_kernel_segun(int conexion, uint32_t pid, uint32_t pc, uint32_t codOp) {
    t_paquete *paquete = malloc(sizeof(t_paquete));
    
    paquete->codigo_operacion = codOp;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = sizeof(uint32_t) * 2; 

    paquete->buffer->stream = malloc(paquete->buffer->size);

    int offset = 0;
    memcpy(paquete->buffer->stream + offset, &pid, sizeof(uint32_t));
    offset+=sizeof(uint32_t);
    memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));
    
    enviar_paquete(paquete, conexion);
    eliminar_paquete(paquete);
}


/* Funciones principales */

void ejecutar_write(t_proceso_actual* proceso, uint32_t direccion, char* datos) {
    uint32_t cadenaSize;
    uint32_t pid = proceso->pid;
    int conexion_memoria = proceso->fd_conexion_memoria; // Se obtiene el fd de conexion de memoria

    enviar_solicitud_escritura(conexion_memoria, direccion, datos, pid);

    uint32_t codOp = EN_ESPERA;
    
    while(codOp != MENSAJE) {
        codOp = recibir_operacion(conexion_memoria);
    }
    char* respuesta = recibir_buffer(&cadenaSize, conexion_memoria);

    free(respuesta); // Liberar el buffer recibido
}

char* ejecutar_read(t_proceso_actual* proceso, uint32_t direccion, uint32_t tamanio) {
    uint32_t cadenaSize;
    uint32_t pid = proceso->pid;
    int conexion_memoria = proceso->fd_conexion_memoria; // Se obtiene el fd de conexion de memoria

    enviar_solicitud_lectura(conexion_memoria, direccion, tamanio, pid);

    char* respuesta;
    uint32_t codOp = EN_ESPERA;
    
    while(codOp != MENSAJE) {
        codOp = recibir_operacion(conexion_memoria);
    }
    respuesta = recibir_buffer(&cadenaSize, conexion_memoria);

    printf("## PID: %d - READ - Datos leidos: %s\n", pid, respuesta);
    return respuesta; // Retorna la cadena leida desde memoria
}

void ejecutar_goto(t_proceso_actual* proceso, uint32_t valor) {
    // Modificar valor del pc 
    proceso->pc = valor;
}

void ejecutar_io(t_proceso_actual* proceso, char* dispositivo, uint32_t tiempo) {
    uint32_t pid = proceso->pid;
    uint32_t pc = proceso->pc; 
    pc++;
    int conexion_kernel = proceso->fd_conexion_kernel; // Se obtiene el fd de conexion del kernel
    
    enviar_syscall_io(conexion_kernel, pid, pc, dispositivo, tiempo);
}

void ejecutar_init_proc(t_proceso_actual* proceso, char* archivo, uint32_t tamanio) {
    uint32_t pid = proceso->pid;
    int conexion_kernel = proceso->fd_conexion_kernel; // Se obtiene el fd de conexion del kernel

    enviar_init_proc(conexion_kernel, pid, archivo, tamanio);
}

void ejecutar_dump_memory(t_proceso_actual* proceso) {
    uint32_t pid = proceso->pid;
    uint32_t pc = proceso->pc;
    pc++;
    int conexion_kernel = proceso->fd_conexion_kernel; // Se obtiene el fd de conexion del kernel

    enviar_dump_a_kernel(conexion_kernel, pid, pc, DUMP_MEMORY);
}

void ejecutar_exit(t_proceso_actual* proceso) {
    uint32_t pid = proceso->pid;
    int conexion_kernel = proceso->fd_conexion_kernel; // Se obtiene el fd de conexion del kernel

    enviar_exit(conexion_kernel, pid);
}



// --------------------------------------

/* para IO */

// Estructura a utilizar -> enviar_paquete_proceso_memoria o empaquetar_y_enviar

// codop = IO | sizebuffer | pid | tiempo | tamanio_nombre_dispositivo | nombre_dispositivo
// socket_dispatch


/* para INIT_PROC */

// Estructura a utilizar -> enviar_paquete_proceso_memoria o empaquetar_y_enviar

// codOp = INIT_PROC | sizebuffer | pid | tamanio_archivo | tamanio_nombre_archivo | nombre_archivo
// socket_dispatch


/* para EXIT */

// Estructura a utilizar -> enviar_mensaje

// codOp = EXIT | sizebuffer | pid 
