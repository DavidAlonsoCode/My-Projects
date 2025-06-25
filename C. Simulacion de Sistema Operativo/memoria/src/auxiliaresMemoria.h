#ifndef AUXILIARES_MEMORIA_H
#define AUXILIARES_MEMORIA_H

#include <utils/hello.h>
#include <commons/memory.h>

#define SIN_PID  UINT32_MAX
// La entrada final, la que apunta a memoria de verdad
typedef struct {
    uint32_t nivel;
    t_list* entradas; //sera de 4 elementos por el config (xq tiene 4 entradas cada tabla)
    //estos elementos pueden ser de tipo t_entrada o t_entrada_ult_tabla
} t_tabla_de_pagina;

// Entradas para tablas de paginas que no son la ultima
typedef struct {
    t_tabla_de_pagina* ptrTabla;
    //uint8_t validez; //Ya no va: 0 si no es valida, 1 si es valida
} t_entrada;

// Entrada para la ultima tabla de paginas, que apunta a memoria real
typedef struct {
    int nroFrame;
    //uint8_t validez; //Ya no va: 0 si no es valida, 1 si es valida
} t_entrada_ult_tabla;

/*
Pag 1 -> entradas = [ptrProxTabla,ptrProxTabla,ptrProxTabla,ptrProxTabla] ¿Serian t_tabla_de_pagina* estos ptr?
Pag 2 -> entradas = [ptrProxTabla,ptrProxTabla,ptrProxTabla,ptrProxTabla] ¿Serian t_tabla_de_pagina* estos ptr?
...
Ultima pag -> entradas = [Frame,Frame,Frame,Frame]
*/

typedef struct {
    uint32_t pid; //PID del proceso que solicita la instruccion
    uint32_t direccion; //Direccion fisica donde se va a leer
    uint32_t tamanio; //Tamanio de la lectura
} t_recepcion_lectura;

typedef struct {
    uint32_t pid; //PID del proceso que solicita la escritura
    uint32_t direccion; //Direccion fisica donde se va a escribir
    uint32_t tamanio; //Tamanio de la escritura
    char* datos; //Datos a escribir, puede ser un string o un numero
} t_recepcion_escritura;

typedef struct {
    uint32_t cant_accesos_a_TTDPP; //TTDPP = tablas de paginas
    uint32_t cant_instruccs_solicitadas;
    uint32_t cant_bajadas_swap;
    uint32_t cant_subidas_memoria;
    uint32_t cant_lecturas_memoria;
    uint32_t cant_escrituras_memoria;
} t_metricas_proceso;

typedef struct {
    uint32_t tamanio_proceso;
    t_list* instrucciones;
    t_tabla_de_pagina* tabla_primer_nvl;
    t_metricas_proceso* metricas;
} t_value_proceso;

typedef struct { // Elementos de la lista que gestiona frames
    bool libre;
    uint32_t pid_proceso;
    uint32_t nro_pagina;
    // uso, modificado, etc.
} t_info_frame;

typedef struct {
    uint32_t pid;
    t_list* entradas_por_nivel;
} t_peticion_marco;

typedef struct {
    char* puertoEscucha;
    uint32_t tamanio_memoria;
    uint32_t tamanio_pagina;
    uint32_t entradas_por_tabla;
    uint32_t cant_niveles;
    uint32_t retardo_memoria;
    char* pathSwapfile;
    uint32_t retardo_swap;
    char* dump_path;
    char* pathInstrucciones; // Path donde se encuentran las instrucciones de los procesos
} memoria_configs;

//GENERICOS
void* atender_cliente(void* datos_cliente);
char* guardar_proceso(t_info_nuevo_proceso* info_proceso);
char* buscar_instruccion(uint32_t pid, uint32_t pc);
char* destruirProceso(char* pid);
void leer_lineas_archivo(FILE* archivo, t_list* lista_instrucciones);
t_value_proceso* llenar_contenido_proceso(t_info_nuevo_proceso* info_proceso, t_list* instrucciones, t_tabla_de_pagina* tabla_global_L1);
t_tabla_de_pagina* crear_tabla_de_nivel(uint32_t nivelTabla);
void guardar_configs(t_config* config);
void destruir_lista_frames(t_list* lista_frames);
void enviar_info_configs_a_mmu(int conexion);

int encontrar_primer_frame_libre_en_lista();
uint32_t contar_frames_libres_en_lista();
void liberar_frame_en_lista(int nro_frame);
void liberar_frames_de_proceso(uint32_t pid);
void asignar_frames_al_proceso(t_value_proceso* value_proceso, int paginas_necesarias,int pid_proceso);
int encontrar_y_asignar_primer_frame_libre(uint32_t pid_proceso, uint32_t pagina);
void destruir_tablas_recursivamente(t_tabla_de_pagina* tabla);
void liberar_frames_de_proceso(uint32_t pid);

//Relacionados a DUMP
char* realizar_dump(uint32_t pid_a_dump);
int obtener_cantidad_paginas(uint32_t pid);
void* inicio_frame(int indice_frame);

//RECIBIR COSAS
uint32_t recibir_proceso_dump(int conexion_kernel);
t_peticion_instruccion* recibir_peticion_instruccion(int socket_cliente);
t_recepcion_lectura* recibir_instruccion_lectura(int socket_cliente);
t_recepcion_escritura* recibir_instruccion_escritura(int socket_cliente);

//DES_SERIALIZAR COSAS
//uint32_t des_serializar_buffer_nuevo_proceso(void* buffer, uint32_t size, t_info_nuevo_proceso* info_proceso);
uint32_t des_serializar_buffer_peticion_instruccion(void* buffer, uint32_t size, t_peticion_instruccion* info_peticion_instruccion);

//Variables globales
extern t_dictionary* dicc_pids_con_instrucciones; //su key es el pid y el valor es un puntero a la estructura pid_instrucciones_t
extern void* memoria_usuario;
extern memoria_configs* configs; // Estructura que contiene la configuracion de memoria
extern t_list* lista_frames; //lista de frames (reemplaza bitmap)

// Semaforos y mutex
extern pthread_mutex_t mutex_diccionario_instrucciones;
extern pthread_mutex_t mutex_frames;
extern pthread_mutex_t mutex_memoria_usuario;
extern pthread_mutex_t mutex_frames_y_diccionario; //protege a ambos cuando se tienen que utilizar juntos

//t_tabla_de_pagina* crear_tablas_de_paginas(); ANTES
//t_tabla_de_pagina* crear_estructura_de_paginacion_global(); ANTES
//void asignar_frames_requeridos(t_tabla_de_pagina* tabla_L1, int paginas_necesarias, int pid_proceso); ANTES

t_tabla_de_pagina* crear_arbol_de_paginacion_completo();
void asignar_frames_a_proceso(t_value_proceso* value_proceso, int paginas_necesarias, uint32_t pid);
int encontrar_y_asignar_primer_frame_libre(uint32_t pid, uint32_t nro_pagina);
t_peticion_marco* recibir_peticion_marco(int socket_cliente);
uint32_t buscar_frame_para_pagina(t_peticion_marco* peticion);
void enviar_frame_a_cpu(int socket_cpu, uint32_t nro_frame);
char* leer_direccion(t_recepcion_lectura* recepcionL);
void atender_peticion_escritura(t_recepcion_escritura* recepcion_E);

#endif