#include <utils/hello.h>
#include "ciclo.h"          // importa
#include "mmu.h"            // importa   

#define TAM_NOMBRE_LOG 20
#define TAM_NOMBRE_PROCESO 20

int main(int argc, char* argv[]) {

    /* VARIABLES */

    //para log y config
    char nombre_log[TAM_NOMBRE_LOG];
    char nombre_proceso[TAM_NOMBRE_PROCESO];
    t_config* config;
    //para conexiones
    char* ip_kernel;
    char* ip_memoria;           //ips
    char* puerto_kernel_dis;
    char* puerto_kernel_int; 
    char* puerto_memoria;       //puertos
    char* id_cpu;               //id que se recibe por consola

    saludar("cpu");

    // Validar Argumentos de Línea de Comando
    if (argc < 2) { // Si no se proporciona un identificador, no podemos continuar
        fprintf(stderr, "Error: Se requiere un identificador para la CPU.\n");
        fprintf(stderr, "Uso: %s <identificador>\n", argv[0]);
        return EXIT_FAILURE; // Salir indicando un error
    }
    id_cpu = argv[1];


    /* INICIAR LOGGER Y CONFIG */

    //Iniciar logger
    snprintf(nombre_log, sizeof(nombre_log), "cpu-%s.log", id_cpu); //mete en "nombre_log" el string "cpu-<id_cpu>.log"
    snprintf(nombre_proceso, sizeof(nombre_log), "LOG-cpu-%s", id_cpu); //mete en "nombre_process" el string
    logger = iniciar_logger(nombre_log, nombre_proceso);
	
    log_info(logger, "Hola soy un log de cpu %s que sirve para ver si esta inicializado",id_cpu);
    //Iniciar config
    config = iniciar_config("./cpu.config");


    /* OBTENER LOS VALORES DEL CONFIG */

    //del KERNEL: 
    ip_kernel           = config_get_string_value(config, "IP_KERNEL");
    puerto_kernel_dis   = config_get_string_value(config,"PUERTO_KERNEL_DISPATCH");
    puerto_kernel_int   = config_get_string_value(config,"PUERTO_KERNEL_INTERRUPT");
    //de MEMORIA:
    ip_memoria          = config_get_string_value(config, "IP_MEMORIA");
    puerto_memoria      = config_get_string_value(config,"PUERTO_MEMORIA");
    //de MMU Y CACHE DE PAGINAS:
    valores_cache* configCache = malloc(sizeof(valores_cache));
    configCache->entradas_tlb    = config_get_string_value(config,"ENTRADAS_TLB");
    configCache->reemplazo_tlb   = config_get_string_value(config,"REEMPLAZO_TLB");
    configCache->entradas_cache  = config_get_string_value(config,"ENTRADAS_CACHE");
    configCache->reemplazo_cache = config_get_string_value(config,"REEMPLAZO_CACHE");
    configCache->retardo_cache   = config_get_string_value(config,"RETARDO_CACHE");
    

    /* CREAR LAS CONEXIONES */

    //kernel dispatch e interrupt, y memoria
    int fd_conexion_dispatch  = crear_conexion_segun("KERNEL Dispatch", ip_kernel, puerto_kernel_dis);
    int fd_conexion_interrupt = crear_conexion_segun("KERNEL Interrupt", ip_kernel, puerto_kernel_int);
    int fd_conexion_memoria   = crear_conexion_segun("MEMORIA", ip_memoria, puerto_memoria);


    /* HANDSHAKE INICIAL E INICIO DE MMU */

    // al servidor Kernel -> envia el id de la CPU
    enviar_mensaje_codigo_operacion(id_cpu,fd_conexion_dispatch, CPU);  // dispatch
    enviar_mensaje_codigo_operacion(id_cpu,fd_conexion_interrupt, CPU); // interrupt
    // al servidor Memoria -> envia el id de la CPU
    enviar_mensaje(id_cpu,fd_conexion_memoria);
    recibir_mensaje_desde_memoria(fd_conexion_memoria);
    // Iniciar modulo de MMU y cache
    iniciar_mmu(fd_conexion_memoria, configCache);
    log_info(logger, "CPU: %s - MMU iniciada correctamente", id_cpu); 


    /* CICLO FETCH-DECODE-EXECUTE-CHECKINTERRUPT */
    while(1) {
        // Se crea una estructura para guardar y manejar el proceso actual
        t_proceso_actual* proceso = recibir_paquete_proceso_cpu(fd_conexion_dispatch);
        proceso->fd_conexion_kernel  = fd_conexion_dispatch;
        proceso->fd_conexion_memoria = fd_conexion_memoria;

        // Setea la operacion en INICIO antes de correr el ciclo
        t_instruccion operacion = INICIO;
        
        while(operacion != INST_IO && operacion != INST_DUMP_MEMORY && operacion != INST_EXIT) {

            operacion = fetch_decode_execute(proceso);  // Obtiene la instruccion desde memoria, la decodifica y ejecuta
            uint32_t pidReady = check_interrupt(fd_conexion_interrupt);
            if(pidReady != -1) {
                log_info(logger, "## Llega interrupcion al puerto interrupt de la CPU %s", id_cpu);
                enviar_paquete_proceso_kernel_segun(fd_conexion_dispatch, pidReady, proceso->pc, REPLANIFICAR);
                break;  // Si hay interrupcion, se envia el proceso al kernel y se sale del ciclo
            }
            
            // >>> INICIO: AÑADIR LOGS AL EJECUTAR UNA INSTRUCCION <<<
            log_info(logger, "------------------------------------------------------");
            log_info(logger, "CPU recibe Proceso PID: %d. Estado de TLB y Cache:", proceso->pid);
            log_estado_tlb(proceso->pid, "Al ejecutar una instruccion");
            log_estado_cache(proceso->pid, "Al ejecutar una instruccion");
            log_info(logger, "------------------------------------------------------");
            // >>> FIN: AÑADIR LOGS AL EJECUTAR UNA INSTRUCCION <<<

            if(operacion != INST_GOTO) proceso->pc++; // GOTO no incrementa el pc
        }

        // Informa que se terminaron de ejecutar todas las instrucciones asociadas al proceso si la operacion es INST_EXIT
        if(operacion == INST_EXIT) {
            log_info(logger, "Se ejecutaron todas las instrucciones asociadas al proceso: %d", proceso->pid);
        } else {
            // Si no es INST_EXIT, pero se desaloja el proceso, se actualiza la memoria y se limpia la cache
            eliminar_entradas_tlb(proceso->pid); 
            eliminar_entradas_cache(proceso->pid, fd_conexion_memoria);

            log_info(logger, "Proceso: %d desalojado. Se espera un nuevo proceso a ejecutar", proceso->pid);
        }
        
        proceso->pid = 0; // Reinicia pid y pc para la proxima iteracion
        proceso->pc  = 0;
        
    }
    log_info(logger, "La CPU %s ha finalizado su ejecucion", id_cpu); // Mensaje de finalización de la CPU


    /* FINALIZACION DEL CPU */
    eliminar_tabla_tlb(); // Elimina la TLB al finalizar la cpu
    eliminar_tabla_cache(); // Elimina la TLB al finalizar la cpu
    free(configCache);
    close(fd_conexion_dispatch);
    close(fd_conexion_interrupt);
    terminar_programa(fd_conexion_memoria, logger, config);

    return 0;
}


