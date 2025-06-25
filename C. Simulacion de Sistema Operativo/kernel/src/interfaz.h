#ifndef INTERFAZ_H_
#define INTERFAZ_H_


#include "utils/hello.h"
#include "syscalls.h"

extern const char* ESTADOS_PROCESO[];
extern const int CANT_ESTADOS;
extern const int ESTADO_NEW;
extern const int ESTADO_READY;
extern const int ESTADO_EXEC;
extern const int ESTADO_BLOCKED;
extern const int ESTADO_EXIT;
extern const int ESTADO_SUSP_READY;
extern const int ESTADO_SUSP_BLOCKED;
extern t_queue * colaNew;
extern t_queue * colaReady;
extern t_list * listaExec;
extern t_list * listaBlocked;
extern t_list * listaSuspReady;
extern uint32_t pid;
extern pthread_mutex_t MUTEX_COLA_READY;
extern pthread_mutex_t MUTEX_COLA_NEW;
extern pthread_mutex_t MUTEX_EXEC;
extern pthread_mutex_t MUTEX_LISTA_CPUS;
extern pthread_mutex_t MUTEX_PID;
extern pthread_mutex_t MUTEX_BLOCKED;
extern pthread_mutex_t MUTEX_COLA_PIDS_PROCESOS_ESPERA;
extern sem_t sem_nuevo_proceso;
extern sem_t sem_ready;
extern sem_t sem_cpu_libre; 
extern t_list* lista_dispositivos_io;
extern t_list* lista_cpus;
extern t_list* lista_procesos_espera_entrada_salida;
extern bool Ingresar_tiempo;
typedef struct {
	char* algoritmo_largo_plazo;
	char* algoritmo_corto_plazo;
	char * ip_memoria;
	char* puerto_memoria;
	double alfa;
	uint64_t estimadorInicial;
}datos_para_planificador;
extern datos_para_planificador*datos_planificador; 


typedef struct {
	char* dispositivo;
	uint32_t tiempo;
	t_queue* pids; //tendra los pids que esperan a tal dispositivo
} procesos_espera_entrada_salida;

typedef struct{
	uint32_t pid;
	int conexion_memoria;
}espera_dump_t;
//en caso de que se quede en espera y bloque el hilo de ese cpu


void llenarDatosParaThread(t_datos_thread_servidor* datos,t_config* config,char* puerto,char* nombreServidor,char* tipoDePuerto);


pcb* crear_pcb(uint32_t ,char*,uint32_t);


void planificador_largo_plazo();
void planificador_corto_plazo();

void iniciar_semaforos();
void finalizar_semaforos();
cpu_datos* seleccionar_cpu_libre();
dispositivo_io* seleccionar_io_libre(char* dispositivo);

void enviar_proceso_a_cpu(cpu_datos* cpu,pcb* proceso);
void marcar_cpu_ocupada(cpu_datos* cpu);

char* validarConMemoria(char* ,uint32_t ,uint32_t ,char* ,char* );
void actualizar_metrica(pcb* pcb,int estado);


void agregar_pcb_listaExec(pcb* pcb_proceso);


pcb* sacar_proceso_de_lista(uint32_t pid,pthread_mutex_t* mutex,t_list* listaASacar);



void enviar_paquete_proceso_cpu(int conexion,uint32_t pid,uint32_t pc);

char* recibir_mensaje_desde_cliente_memoria_kernel(int);

void *atender_cliente_kernel(void *datosSinTipo);


void atenderConexion(void* datosServidorSinTipo);

void cambiar_estado_cpu(int conexion,char nuevo_estado,pcb*);

cpu_datos* buscar_cpu_por_conexion(int conexion);

void agregar_io(dispositivo_io* );

void esperar_multiples_conexiones_io(int server_fd, char *servidor);

void esperar_multiples_conexiones_cpu(int , char *);

dump_datos* recibir_datos_dump(int conexion_cpu);

dispositivo_io* recibir_io(int);

dispositivo_tiempo* recibir_tiempo_dispositivo(int fd_conexion);

void *atender_cliente_io(void *datosSinTipo);

cpu_datos* recibir_cpu(int);

cpu_datos* buscar_cpu_por_id(char* id_cpu);

uint32_t obtener_pid();

//dispositivo_tiempo* recibir_tiempo_dispositivo(int fd_conexion);

void syscall_io(char* dispositivo,uint32_t tiempo,uint32_t pid);

void bloqueo_proceso(uint32_t pid);

void enviar_tiempo_sleep_io(uint32_t pid,uint32_t tiempo,int fd_conexion);

void *atender_cliente_cpu(void *datosSinTipo);


uint64_t obtenerTiempoAhora();


void actualizar_io(int );

dispositivo_io* encuentro_dispositivo_io(int );

procesos_espera_entrada_salida*  encontrar_proceso_en_espera(char* );

void asignar_a_un_proceso(dispositivo_io* );

void push_cola_pids_procesos_espera(t_queue* pids,uint32_t pid);

dispositivo_io* encontrar_io_por_fd_conexion(int fd_conexion);

void proceso_blocked_a_ready(uint32_t pid);

void actualizar_pc_en_lista(uint32_t pid, uint32_t nuevo_pc, pthread_mutex_t* mutex, t_list* lista);

void inicializo_datos();

void esperar_enter();

pcb * obtener_pcb_lista(t_list *, pthread_mutex_t * mutex);

void remover_pcb_de_lista(pcb* proceso,t_list* lista,pthread_mutex_t* mutex);


void  crear_proceso(char* proceso_pseudocodigo,uint32_t tamanio_proceso);



bool hayProcesosEnEspera(t_queue * pids);

void libero_procesos_en_espera(procesos_espera_entrada_salida* procesoEntrada);

uint32_t cantidadPids(t_queue * pids);

bool hayElementosEnLista(t_list* lista, pthread_mutex_t* mutex);


bool comparacionEsMasChicoTamanioBytes(void* a, void* b);

void remover_proceso_entrada_salida(procesos_espera_entrada_salida* proceso,t_list* lista,pthread_mutex_t* mutex);

int estimacionSiguiente(int tiempoEjecutado,int estimacionAnterior);

void actualizar_tiempo(pcb *pcb, int estado);

bool comparacionRafagaMasChica(void* a, void* b);

bool estaLibre(void* ptr) ;

bool cpuLibres();

void lanzarInterrupcion(cpu_datos* cpu,pcb* proceso);

void agregar_pcb_a_lista(pcb *pcb_proceso,t_list* lista,pthread_mutex_t * mutex,uint32_t estado);


#endif
