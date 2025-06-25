#ifndef UTILS_HELLO_H_
#define UTILS_HELLO_H_

#include<signal.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h>
#include<readline/readline.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>
#include <commons/temporal.h>
#include <sys/time.h>

typedef enum {
	MENSAJE, //0
	PAQUETE, // 1 
	USAR_IO, //2
	FINALIZACION_IO,
	PROCESO, //memoria 4
	PETICION_INSTRUCCION, //memoria
	PETICION_ESCRITURA,
	PETICION_LECTURA,
	DESTRUCCION_PROCESO, //memoria 8
	CPU,
	HANDSHAKE_IO,
	PROCESO_A_EJECUTAR, //9
	INTERRUPCION,
	INIT_PROC,
	IO,
	DUMP_MEMORY,
	EXIT,
	EN_ESPERA,
	LINEA_DE_INSTRUCCION, //17
	SOLICITUD_DATOS_PAGINAS,
	RESPUESTA_DATOS_PAGINAS,
	REPLANIFICAR,
	PETICION_DUMP, //memoria
	PETICION_MARCO,
	ACTUALIZAR_MEMORIA 
}op_code;

typedef struct {
	uint32_t pid;
	uint32_t pc;
} t_peticion_instruccion;

typedef struct {
	uint32_t size;  //tamaño del payload
	//int offset; //desplazamiento del payload
	void* stream; //payload
} t_buffer;

typedef struct
{
	uint32_t pid;
	uint32_t tiempo;
}t_usar_io;
//podemos si no enviar el pid al kernel
//cola de bloqueados 

//

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
    int* fd_conexion_ptr; // O considera usar 'int fd_conexion;' directamente
    char* servidor;
    char* path; // Nuevo campo que en memoria se usa para el path de instrucciones
} datos_cliente_t;

typedef struct {
    char* estado;                  // Nombre del estado: "READY", "EXEC", etc.
    uint32_t cantidadDeVecesQueEstuvo; // Veces que el proceso estuvo en ese estado
    uint64_t milisegundos;             // Tiempo total en milisegundos en ese estado
} metrica_estado_tiempo;

typedef struct {
    uint32_t pid;               // Identificador único
    uint32_t pc;                // Program Counter
    t_list* listaMetricas;	// Lista de MetricaEstadoTiempo*
	char* nombre_archivo;
	uint32_t tamanio_bytes; 
	uint64_t timestamp_inicio;
	int estimacionAnterior;
	uint64_t rafagaAnterior;
} pcb;

typedef struct {
	pthread_t *hilo;
	t_config *config;
	char *puerto;
	char *nombreServidor;
	char *tipoDePuerto;
} t_datos_thread_servidor;

typedef struct{ //esto debe ir en hello.h
    uint32_t pid;
	uint32_t tamanio_nombre_archivo;
    char* nombre_archivo;
    uint32_t tamanio_bytes_archivo;
} t_info_nuevo_proceso;


typedef struct{
	int fd_conexion;
	char* dispositivo;
	char estado; 
	uint32_t proceso_actual;
}dispositivo_io;

// typedef struct{
// 	char* dispositivo;
// 	lista_ pcb_proceso;
// }dispositivo_espera_bloqueados;


typedef struct{
	uint32_t tiempo;
	uint32_t pid;
	char* dispositivo;
	uint32_t tamanio_nombre_dispositivo;
	uint32_t pc;
}dispositivo_tiempo;

typedef struct{
	int dispatch;
	int interrupt; //es parecido al struct de arriba ,pero por ahora ya que puede tener diferentes valores
	char* identificador;
	char estado; //basicamente un estado de un char si es "O" ,significa ocupada , si es "L" libre
	pcb* procesoEnEjecucion;
}cpu_datos;

typedef struct{
	uint32_t pid;
	uint32_t pc;
}dump_datos;

//cpu1 , socket 43, socket | 

extern t_log* logger;

//////////////////////// INICIO SECCION RECIBIR ///////////
void recibir_mensaje(int,char*);
void recibir_mensaje_desde_cliente(int);
uint32_t recibir_operacion(int);
void* recibir_buffer(uint32_t*, int);
t_list* recibir_paquete(int);
//////////////////////// FIN SECCION RECIBIR //////////////

//////////////////////// INICIO SECCION ESPERAR ///////////////////////////////
int esperar_cliente(int);
uint32_t recibir_pid(int fd_conexion);

//////////////////////// FIN SECCION ESPERAR //////////////////////////////////

//////////////////////// INICIO SECCION ATENDER ///////////

void atenderConexion(void* datosServidorSinTipo);
//////////////////////// FIN SECCION ATENDER //////////////

//////////////////////// INICIO SECCION INICIAR ///////////////////////////////
t_config* iniciar_config(char* ruta);
t_log* iniciar_logger(char* nombreArchivo,char* nombreProceso);
int iniciar_servidor(char* puerto,char* nombreServidor);
//////////////////////// FIN SECCION INICIAR //////////////////////////////////
t_info_nuevo_proceso *recibir_nuevo_proceso(int socket_cliente); 
uint32_t des_serializar_buffer_nuevo_proceso(void* buffer, uint32_t size, t_info_nuevo_proceso* info_proceso);
//////////////////////// INICIO SECCION ENVIAR ////////////
void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void enviar_paquete_proceso_memoria(int conexion, uint32_t pid, char* nombreArchivo, uint32_t tamanio_bytes);
//////////////////////// FIN SECCION INICIAR //////////////

//////////////////////// INICIO SECCION CREAR /////////////////////////////////
int crear_conexion(char* ip, char* puerto);
t_paquete* crear_paquete(op_code codigoOperacion);
//////////////////////// FIN SECCION CREAR ////////////////////////////////////

void iterator(char* value);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
void iterator(char* value);
void leer_consola();
void llenarDatosParaThread(t_datos_thread_servidor* datos,t_config* config,char* puerto,char* nombreServidor,char* tipoDePuerto);
void enviar_mensaje_codigo_operacion(char *mensaje, int socket_cliente,op_code codigoOperacion);

void *serializar_paquete(t_paquete *, uint32_t );
/**
* @brief Imprime un saludo por consola
* @param quien Módulo desde donde se llama a la función
* @return No devuelve nada
*/

void saludar(char* quien);

uint32_t recibir_operacion_no_bloqueante(int socket_cliente);

#endif
