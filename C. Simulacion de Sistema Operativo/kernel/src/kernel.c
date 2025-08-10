#include<interfaz.h>


int main(int argc, char *argv[])
{
    printf("Hola ingresa un enter para comenzar la planificacion de largo plazo\n");
    // Validar argumentos
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <archivo_pseudocodigo> <tamanio_proceso> <ruta_config>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Obtener argumentos iniciales
    char *proceso_pseudocodigo = argv[1];
    uint32_t tamanio_proceso = atoi(argv[2]);
    char* rutaConfig = argv[3];


    inicializo_datos();
  
    // Inicializar logger y config
    //./kernel.config
    t_config* ip_config = iniciar_config("./ip.config");
    t_config* config = iniciar_config(rutaConfig);

    char* ip_memoria = config_get_string_value(ip_config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(ip_config, "PUERTO_MEMORIA");
    char *corto_plazo = config_get_string_value(config, "ALGORITMO_CORTO_PLAZO");
    char *largo_plazo = config_get_string_value(config,"ALGORITMO_INGRESO_A_READY");
    double alfa = 	config_get_double_value (config,"ALFA");
    uint64_t estimadorInicial = config_get_int_value(config,"ESTIMACION_INICIAL");
    double tiempo_suspension = 	config_get_double_value (config,"TIEMPO_SUSPENSION");
    char* logLevel = config_get_string_value(config,"LOG_LEVEL");
    
    logger = iniciar_logger("kernel.log", "KERNEL_LOGGER",logLevel);

        // Conexión con Memoria
    int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
    enviar_mensaje("KERNEL", fd_conexion_memoria);
    int codOperacion = recibir_operacion(fd_conexion_memoria);
    if (codOperacion == MENSAJE) {
        recibir_mensaje_desde_cliente(fd_conexion_memoria);
    }

    // Inicializo datos a planificador largo plazo
    datos_planificador = malloc(sizeof(datos_para_planificador));
    datos_planificador->algoritmo_largo_plazo = largo_plazo;
    datos_planificador->algoritmo_corto_plazo = corto_plazo;
    datos_planificador->ip_memoria = ip_memoria;
    datos_planificador->puerto_memoria = puerto_memoria;
    datos_planificador->alfa = alfa;
    datos_planificador->estimadorInicial = estimadorInicial;
    datos_planificador->tiempo_suspension = tiempo_suspension;
    datos_planificador->nombreArchivo = proceso_pseudocodigo;
    
    // Crear proceso inicial
    pthread_mutex_lock(&MUTEX_NEW);

    crear_proceso(proceso_pseudocodigo,tamanio_proceso);
    
    pthread_mutex_unlock(&MUTEX_NEW);


    // Crear hilos de planificación
    pthread_t hiloPlanificadorLargoPlazo;
    pthread_t hiloPlanificadorCortoPlazo;
    pthread_create(&hiloPlanificadorLargoPlazo, NULL, (void *)planificador_largo_plazo,NULL);
    pthread_create(&hiloPlanificadorCortoPlazo, NULL, (void *)planificador_corto_plazo, NULL);
    pthread_detach(hiloPlanificadorLargoPlazo);
    pthread_detach(hiloPlanificadorCortoPlazo);


       // Crear hilos para recibir conexiones
    pthread_t hiloServidorIO;
    pthread_t hiloServidorCPUDispatch;
    pthread_t hiloServidorCPUInterrupt;

    t_datos_thread_servidor *datosIO = malloc(sizeof(t_datos_thread_servidor));
    t_datos_thread_servidor *datosDispatch = malloc(sizeof(t_datos_thread_servidor));
    t_datos_thread_servidor *datosInterrupt = malloc(sizeof(t_datos_thread_servidor));

    llenarDatosParaThread(datosIO, config, "PUERTO_ESCUCHA_IO", "io", "ESCUCHA_IO");
    llenarDatosParaThread(datosDispatch, config, "PUERTO_ESCUCHA_DISPATCH", "dispatch", "DISPATCH(CPU)");
    llenarDatosParaThread(datosInterrupt, config, "PUERTO_ESCUCHA_INTERRUPT", "interrupt", "INTERRUPT(CPU)");

    pthread_create(&hiloServidorIO, NULL, (void *)atenderConexion, (void *)datosIO);
    pthread_create(&hiloServidorCPUDispatch, NULL, (void *)atenderConexion, (void *)datosDispatch);
    pthread_create(&hiloServidorCPUInterrupt, NULL, (void *)atenderConexion, (void *)datosInterrupt);

    esperar_enter();
    sem_post(&sem_inicio_planificador);
    // Esperar hilos de conexión (los otros ya están detached)
    pthread_join(hiloServidorIO, NULL);
    pthread_join(hiloServidorCPUDispatch, NULL);
    pthread_join(hiloServidorCPUInterrupt, NULL);

    // Limpiar recursos
    free(datosIO);
    free(datosDispatch);
    free(datosInterrupt);
    free(datos_planificador);
    config_destroy(ip_config);
    terminar_programa(fd_conexion_memoria, logger, config);
    finalizar_semaforos();
  

    return EXIT_SUCCESS;
}






