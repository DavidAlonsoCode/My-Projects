#include <utils/hello.h>
#include <auxiliaresMemoria.h>
#include <liberarRecursosMemoria.h>

t_dictionary* dicc_pids_con_instrucciones;
void* memoria_usuario;
memoria_configs* configs;
t_list* lista_frames;

pthread_mutex_t mutex_diccionario_instrucciones;
pthread_mutex_t mutex_frames_y_diccionario;
pthread_mutex_t mutex_frames;
pthread_mutex_t mutex_memoria_usuario;

int main(int argc, char* argv[]){
    saludar("memoria");

	//Inicializo el diccionario para guardar los pids y las instrucciones
	dicc_pids_con_instrucciones = dictionary_create();
	
	// Inicializo el mutex
	pthread_mutex_init(&mutex_diccionario_instrucciones, NULL);
	pthread_mutex_init(&mutex_frames_y_diccionario, NULL);
	pthread_mutex_init(&mutex_frames, NULL);
	pthread_mutex_init(&mutex_memoria_usuario, NULL);

	// Inicializo logger
    logger = iniciar_logger("memoria.log","MEMORIA_LOGGER");
	log_info(logger, "Hola soy el server de memoria");
    
	//Inicializo config
	t_config* config;
    config = iniciar_config("./memoria.config");
	
    //Obtengo valores del config (sin LOG_LEVEL)
	guardar_configs(config);

	// inicio memoria de usuario
	memoria_usuario = calloc(1, configs->tamanio_memoria);
	uint32_t cantidad_frames = configs->tamanio_memoria / configs->tamanio_pagina; //tamaño frame = tamanio_pagina
	log_info(logger, "Memoria de usuario inicializada con tamaño de %d bytes y %d frames de %d bytes", configs->tamanio_memoria, cantidad_frames, configs->tamanio_pagina);

	// Creo bitmap para gestionar los frames
	lista_frames = list_create();
	for(int i = 0; i < cantidad_frames; i++) {
		t_info_frame* info_frame = malloc(sizeof(t_info_frame));
		info_frame->libre = true;
		info_frame->pid_proceso = SIN_PID;
		info_frame->nro_pagina = 0;
		list_add(lista_frames, info_frame);
	}

	// Levanto sevidor
	int server_fd = iniciar_servidor(configs->puertoEscucha,"memoria");
	log_info(logger, "Servidor memoria listo para recibir al cliente");

	while (1) {
		pthread_t thread;
		int *fd_conexion_ptr = malloc(sizeof(int));
		*fd_conexion_ptr = esperar_cliente(server_fd); //hace accept
	
		// Creamos tipo de datos para que al atender cliente ademas de tener la conexion sepa que servidor es
		datos_cliente_t* datosCliente = malloc(sizeof(datos_cliente_t));
		datosCliente->fd_conexion_ptr=fd_conexion_ptr;
		datosCliente->servidor="MEMORIA";
		datosCliente->path = NULL; // ESTO YA NO ES NECESARIO, por eso "NULL"
		// free para fd_conexion_ptr en "enviar_mensaje", y para servidor en "recibir_mensaje" (en atender_cliente)
		
		// Creamos el hilo para atender al cliente
		pthread_create(&thread,NULL,(void*) atender_cliente,(void*) datosCliente);
		pthread_detach(thread); // Significa: La memoria asociada al hilo puede ser liberada cuando el hilo termina
	}

	log_destroy(logger);
	config_destroy(config);
	destruir_diccionario_instrucciones();
	pthread_mutex_destroy(&mutex_diccionario_instrucciones);
	pthread_mutex_destroy(&mutex_frames);
    destruir_lista_frames(lista_frames);
    free(memoria_usuario);
    free(configs);

	return EXIT_SUCCESS;
}

void guardar_configs(t_config* config) {
	// Reservamos memoria para la estructura de configuración
	configs = malloc(sizeof(memoria_configs));
	
	// Asignamos los valores desde el config a la estructura global
	configs->puertoEscucha = config_get_string_value(config, "PUERTO_ESCUCHA");
	configs->tamanio_memoria = config_get_int_value(config, "TAM_MEMORIA");
	configs->tamanio_pagina = config_get_int_value(config, "TAM_PAGINA");
	configs->entradas_por_tabla = config_get_int_value(config, "ENTRADAS_POR_TABLA");
	configs->cant_niveles = config_get_int_value(config, "CANTIDAD_NIVELES");
	configs->retardo_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	configs->pathSwapfile = config_get_string_value(config, "PATH_SWAPFILE");
	configs->retardo_swap = config_get_int_value(config, "RETARDO_SWAP");
	configs->dump_path = config_get_string_value(config, "DUMP_PATH");
	configs->pathInstrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
}

