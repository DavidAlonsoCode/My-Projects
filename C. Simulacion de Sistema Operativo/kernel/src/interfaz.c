#include "interfaz.h"

const char *ESTADOS_PROCESO[] = {"NEW", "READY", "EXEC", "BLOCKED", "EXIT", "SUSP.READY", "SUSP.BLOCKED"};
const int CANT_ESTADOS = 7;
const int ESTADO_NEW = 0;
const int ESTADO_READY = 1;
const int ESTADO_EXEC = 2;
const int ESTADO_BLOCKED = 3;
const int ESTADO_EXIT = 4;
const int ESTADO_SUSP_READY = 5;
const int ESTADO_SUSP_BLOCKED = 6;
t_list *listaNew;
t_list *listaReady;
t_list *listaExec;
t_list *listaBlocked;
t_list *listaSuspReady;
uint32_t pid = 0;
datos_para_planificador *datos_planificador;
pthread_mutex_t MUTEX_COLA_READY;
pthread_mutex_t MUTEX_NEW;
pthread_mutex_t MUTEX_EXEC;
pthread_mutex_t MUTEX_LISTA_ENTRADAS_SALIDAS;
pthread_mutex_t MUTEX_LISTA_CPUS;
pthread_mutex_t MUTEX_PID;
pthread_mutex_t MUTEX_BLOCKED;
pthread_mutex_t MUTEX_PROCESOS_ESPERA;
pthread_mutex_t MUTEX_SUSPD_READY;
sem_t sem_nuevo_proceso;
sem_t sem_ready;							  // Indica que hay un proceso listo
sem_t sem_cpu_libre;						  // se activa cuando una CPU termina y queda disponible
t_list *lista_dispositivos_io;				  // las io que tengo conectadas
t_list *lista_cpus;							  // cpu que tengo
t_list *lista_procesos_espera_entrada_salida; // estos procesos esperan por algun dispositivo
bool Ingresar_tiempo = true;

void planificador_corto_plazo()
{

	while (1)
		{
			// todo esto funciona con FIFO
			sem_wait(&sem_ready);
			sem_wait(&sem_cpu_libre); //

			pcb *proceso = obtener_pcb_lista(listaReady,&MUTEX_COLA_READY);
			
			remover_pcb_de_lista(proceso,listaReady,&MUTEX_COLA_READY);

			actualizar_tiempo(proceso, ESTADO_READY);
				
			cpu_datos *cpu = seleccionar_cpu_libre();

			cpu->procesoEnEjecucion = proceso;


			agregar_pcb_listaExec(proceso);

			enviar_paquete_proceso_cpu(cpu->dispatch, proceso->pid, proceso->pc);


			log_info(logger, "## (%d) Pasa de READY a EXEC", proceso->pid);
		}
	
}
		




cpu_datos *seleccionar_cpu_libre()
{
	pthread_mutex_lock(&MUTEX_LISTA_CPUS);
	for (int i = 0; i < list_size(lista_cpus); i++)
	{
		cpu_datos *cpu = list_get(lista_cpus, i);
		if (cpu->estado == 'L')
		{
			cpu->estado = 'O';
			pthread_mutex_unlock(&MUTEX_LISTA_CPUS);
			return cpu;
		}
	}
	pthread_mutex_unlock(&MUTEX_LISTA_CPUS);
	return NULL;
}

void planificador_largo_plazo()
{

	while (1)
	{
		sem_wait(&sem_nuevo_proceso);
		pcb* pcb_proceso ;
		int estado;
		t_list* listaCopia ;
		pthread_mutex_t* mutex;
		//antes verifrico en la lista de suspendidoReady
		if(hayElementosEnLista(listaSuspReady,&MUTEX_SUSPD_READY)){
			//saco de suspd.ready
			pcb_proceso = obtener_pcb_lista(listaSuspReady,&MUTEX_SUSPD_READY);
			estado = ESTADO_SUSP_READY;
			listaCopia = listaSuspReady;
			mutex = &MUTEX_SUSPD_READY;
		}else{
			//primero pregunto si memoria puede hcerlo , no lo saco todavia
			pcb_proceso = obtener_pcb_lista(listaNew,&MUTEX_NEW);
			estado = ESTADO_NEW;
			listaCopia = listaNew;
			mutex = &MUTEX_NEW;
		}

	
		char *memoriaConEspacio = validarConMemoria(pcb_proceso->nombre_archivo, pcb_proceso->tamanio_bytes, pcb_proceso->pid, datos_planificador->puerto_memoria, datos_planificador->ip_memoria);
		if (strcmp(memoriaConEspacio, "OK") == 0)
		{ // si devuelve 0 es que no hay espacio entonces no entra al if
			// printf("Llego el OK\n");
			
			remover_pcb_de_lista(pcb_proceso,listaCopia,mutex);

			actualizar_tiempo(pcb_proceso,estado);
			
			//ordenar por sjf 
			if(strcmp(datos_planificador->algoritmo_corto_plazo, "FIFO") != 0){
				pthread_mutex_lock(&MUTEX_COLA_READY);
				actualizar_metrica(pcb_proceso,ESTADO_READY);
				list_add_sorted(listaReady,pcb_proceso,comparacionRafagaMasChica);
				pthread_mutex_unlock(&MUTEX_COLA_READY);

 // si no hay cpu libre y el algoritmo es SRT
				
				if(hayElementosEnLista(lista_cpus,&MUTEX_LISTA_CPUS))
				{
				
				if(!cpuLibres() && strcmp(datos_planificador->algoritmo_corto_plazo, "SRT") == 0)
				{
					//ENTONCES verifico si los que estan ejecutando tienen menor prioridad


					pthread_mutex_lock(&MUTEX_LISTA_CPUS);

					for(int i= 0 ; i < list_size(lista_cpus);i++){

						cpu_datos * cpuAux = list_get(lista_cpus, i);

						//calculo la estimacion de lproceso ready	

						if(cpuAux->procesoEnEjecucion != NULL){

					
						uint64_t ahora = obtenerTiempoAhora();
						int diferencia_milisegundos = ahora - cpuAux->procesoEnEjecucion->timestamp_inicio;
						
						int estimacionMenosEjecutado = cpuAux->procesoEnEjecucion->estimacionAnterior - diferencia_milisegundos;


						int estimacionProcesoNuevo = estimacionSiguiente(pcb_proceso->rafagaAnterior,pcb_proceso->estimacionAnterior);


						if(estimacionProcesoNuevo < estimacionMenosEjecutado){
							lanzarInterrupcion(cpuAux,pcb_proceso);
						}
						}

					}
				
					pthread_mutex_unlock(&MUTEX_LISTA_CPUS);

				
				}
				}
			}
			else{
			agregar_pcb_a_lista(pcb_proceso,listaReady,&MUTEX_COLA_READY,ESTADO_READY);
			}

		

			log_info(logger, "## (%d) Pasa de NEW a READY", pcb_proceso->pid);

			sem_post(&sem_ready);
		}
		else
		{
			// en este else tengo que esperar finalizacion de otro proceso para poder
			// meterlo en memoria
			// Esto como es mock todavia no se va  a implementar ya que no tenemos la tabla de paginas y todo eso
		}
		free(memoriaConEspacio);
	}
}

pcb *crear_pcb(uint32_t pid, char *nombre_archivo, uint32_t tamanio_bytes)
{
	pcb *nuevo_pcb = malloc(sizeof(pcb));
	nuevo_pcb->pid = pid;
	nuevo_pcb->pc = 0;
	nuevo_pcb->listaMetricas = list_create();
	nuevo_pcb->tamanio_bytes = tamanio_bytes;
	nuevo_pcb->nombre_archivo = nombre_archivo;
	nuevo_pcb->estimacionAnterior = datos_planificador->estimadorInicial;
	nuevo_pcb->rafagaAnterior = 0;
	nuevo_pcb->timestamp_inicio = 0;//Acordarse de que si usamos esta funcion , este cmapo debe tener algun valor ya que lo usamos unicamente para encontrar la diferencia.
	for (int i = 0; i < CANT_ESTADOS; i++)
	{
		metrica_estado_tiempo *metrica = malloc(sizeof(metrica_estado_tiempo));
		metrica->estado = strdup(ESTADOS_PROCESO[i]);
		metrica->cantidadDeVecesQueEstuvo = 0;
		metrica->milisegundos = 0;
		list_add(nuevo_pcb->listaMetricas, metrica);
	}

	return nuevo_pcb;
}

void iniciar_semaforos()
{
	pthread_mutex_init(&MUTEX_COLA_READY, NULL);
	pthread_mutex_init(&MUTEX_NEW, NULL);
	pthread_mutex_init(&MUTEX_EXEC, NULL);
	pthread_mutex_init(&MUTEX_LISTA_ENTRADAS_SALIDAS, NULL);
	pthread_mutex_init(&MUTEX_LISTA_CPUS, NULL);
	pthread_mutex_init(&MUTEX_PID, NULL);
	pthread_mutex_init(&MUTEX_BLOCKED, NULL);
	pthread_mutex_init(&MUTEX_PROCESOS_ESPERA, NULL);
	sem_init(&sem_nuevo_proceso, 0, 0);
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_cpu_libre, 0, 0);
}

void finalizar_semaforos()
{
	pthread_mutex_destroy(&MUTEX_COLA_READY);
	pthread_mutex_destroy(&MUTEX_NEW);
	pthread_mutex_destroy(&MUTEX_EXEC);
	pthread_mutex_destroy(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	pthread_mutex_destroy(&MUTEX_LISTA_CPUS);
	pthread_mutex_destroy(&MUTEX_PID);
	pthread_mutex_destroy(&MUTEX_BLOCKED);
	pthread_mutex_destroy(&MUTEX_PROCESOS_ESPERA);
	sem_destroy(&sem_nuevo_proceso);
	sem_destroy(&sem_ready);
	sem_destroy(&sem_cpu_libre);
}

char *recibir_mensaje_desde_cliente_memoria_kernel(int socket_cliente)
{
	uint32_t size;
	char *buffer = recibir_buffer(&size, socket_cliente); // liberar
	return buffer;
}

char *validarConMemoria(char *nombreArchivo, uint32_t tamanioBytes, uint32_t pid, char *puerto_memoria, char *ip_memoria)
{

	int fd_conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

	enviar_paquete_proceso_memoria(fd_conexion_memoria, pid, nombreArchivo, tamanioBytes);
	// recibir_mensaje_confirmacion_memoria();
	int codOperacion = recibir_operacion(fd_conexion_memoria);
	char *valor_retorno_memoria;
	if (codOperacion == MENSAJE)
	{
		valor_retorno_memoria = recibir_mensaje_desde_cliente_memoria_kernel(fd_conexion_memoria);
	}

	close(fd_conexion_memoria);

	return valor_retorno_memoria;
}

void llenarDatosParaThread(t_datos_thread_servidor *datos, t_config *config, char *puerto, char *nombreServidor, char *tipoDePuerto)
{
	datos->config = config;
	datos->puerto = puerto;
	datos->nombreServidor = nombreServidor;
	datos->tipoDePuerto = tipoDePuerto;
}
// incrementa uno el contador del estado recibido en el pcb que se paso
void actualizar_metrica(pcb *pcb, int estado)
{
	metrica_estado_tiempo *elem = list_get(pcb->listaMetricas, estado);
	elem->cantidadDeVecesQueEstuvo += 1;
	pcb->timestamp_inicio = obtenerTiempoAhora();
}

void actualizar_tiempo(pcb *pcb, int estado)
{
	metrica_estado_tiempo *elem = list_get(pcb->listaMetricas, estado);
	uint64_t ahora = obtenerTiempoAhora();
	uint64_t diferencia_milisegundos = ahora - pcb->timestamp_inicio;
	elem->milisegundos += diferencia_milisegundos;
}

uint64_t obtenerTiempoAhora()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

//	actualizar_tiempo(p, ESTADO_READY);




void agregar_pcb_listaExec(pcb *pcb_proceso)
{
	pthread_mutex_lock(&MUTEX_EXEC);
	actualizar_metrica(pcb_proceso, ESTADO_EXEC);
	list_add(listaExec, pcb_proceso);
	pthread_mutex_unlock(&MUTEX_EXEC);
}


void agregar_pcb_a_lista(pcb *pcb_proceso,t_list* lista,pthread_mutex_t * mutex,uint32_t estado)
{
	pthread_mutex_lock(mutex);
	actualizar_metrica(pcb_proceso, estado);
	list_add(lista, pcb_proceso);
	pthread_mutex_unlock(mutex);
}

//no tiene mutex porque lo invoco cuando la llamo 
void push_cola_pids_procesos_espera(t_queue *pids, uint32_t pid)
{
	queue_push(pids, pid);
}

uint32_t pop_cola_pids_procesos_espera(t_queue *pids)
{
	pthread_mutex_lock(&MUTEX_PROCESOS_ESPERA);
	uint32_t pid = queue_pop(pids);
	pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);
	return pid;
}



void enviar_paquete_proceso_cpu(int conexion, uint32_t pid, uint32_t pc)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)PROCESO_A_EJECUTAR;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t) * 2;

	paquete->buffer->stream = malloc(paquete->buffer->size);

	//	op_code | size buffer | pid | pc

	int offset = 0;
	memcpy(paquete->buffer->stream, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + offset, &pc, sizeof(uint32_t));

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void atenderConexion(void *datosServidorSinTipo)
{
	t_datos_thread_servidor *datosServidor = (t_datos_thread_servidor *)datosServidorSinTipo; // Cast a t_datos_thread_servidor
	t_config *config = datosServidor->config;
	char *puerto = datosServidor->puerto;
	char *nombreServidor = datosServidor->nombreServidor;
	char *tipoDePuerto = datosServidor->tipoDePuerto; // "ESCUCHA_IO", "DISPATCH(CPU)", "INTERRUPT(CPU)"

	char *puerto_escucha = config_get_string_value(config, puerto);
	int server_fd = iniciar_servidor(puerto_escucha, nombreServidor);
	if (strcmp(puerto, "PUERTO_ESCUCHA_IO") == 0)
	{
		esperar_multiples_conexiones_io(server_fd, tipoDePuerto);
	}
	else
	{
		esperar_multiples_conexiones_cpu(server_fd, tipoDePuerto);
	}
}

void esperar_multiples_conexiones_io(int server_fd, char *servidor)
{
	while (1)
	{
		pthread_t thread;
		int *fd_conexion_ptr = malloc(sizeof(int));
		if (!fd_conexion_ptr)
		{
			log_error(logger, "No se pudo reservar memoria para el nuevo cliente");
			continue;
		}

		*fd_conexion_ptr = accept(server_fd, NULL, NULL);
		if (*fd_conexion_ptr == -1)
		{
			log_error(logger, "Fallo al aceptar cliente");
			free(fd_conexion_ptr);
			continue;
		}

		log_info(logger, "Nuevo cliente \"%d\" conectado al servidor \"%s\"", *fd_conexion_ptr, servidor);

		datos_cliente_t *datos = malloc(sizeof(datos_cliente_t));
		datos->fd_conexion_ptr = fd_conexion_ptr;
		datos->servidor = servidor; // cuidado si es string local
		datos->path = NULL;			// Inicializo en NULL, se puede cambiar luego

		pthread_create(&thread, NULL, atender_cliente_io, (void *)datos);
		pthread_detach(thread); // No necesitamos esperar a que termine el hilo
	}
}

void esperar_multiples_conexiones_cpu(int server_fd, char *servidor)
{
	while (1)
	{
		pthread_t thread;
		int *fd_conexion_ptr = malloc(sizeof(int));
		if (!fd_conexion_ptr)
		{
			log_error(logger, "No se pudo reservar memoria para el nuevo cliente");
			continue;
		}

		*fd_conexion_ptr = accept(server_fd, NULL, NULL);
		if (*fd_conexion_ptr == -1)
		{
			log_error(logger, "Fallo al aceptar cliente");
			free(fd_conexion_ptr);
			continue;
		}

		log_info(logger, "Nuevo cliente \"%d\" conectado al servidor \"%s\"", *fd_conexion_ptr, servidor);

		datos_cliente_t *datos = malloc(sizeof(datos_cliente_t));
		datos->fd_conexion_ptr = fd_conexion_ptr;
		datos->servidor = servidor; // cuidado si es string local
		datos->path = NULL;			// Inicializo en NULL, se puede cambiar luego

		pthread_create(&thread, NULL, atender_cliente_cpu, (void *)datos);
		pthread_detach(thread); // No necesitamos esperar a que termine el hilo
	}
}

void *atender_cliente_io(void *datosSinTipo)
{
	datos_cliente_t *datos = (datos_cliente_t *)datosSinTipo;
	int fd_conexion = *(datos->fd_conexion_ptr);
	char *servidor = datos->servidor;
	char *pathInstrucciones = datos->path; // path de instrucciones (lo necesita memoria)
	// (*datos).fd_conexion_ptr == datos->fd_conexion_ptr   (RECORDANDO UN POCO LA SINTAXIS)

	free(datos->fd_conexion_ptr);
	free(datos); // Liberamos memoria reservada

	t_list *lista;

	while (1) // habria que agregar una condicion de salida para cortar el programa manualmente
	{
		uint32_t cod_op = recibir_operacion(fd_conexion);

		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(fd_conexion, servidor);
			char *mensaje = "Hola desde servidor";
			enviar_mensaje(mensaje, fd_conexion);
			break;
		case HANDSHAKE_IO:
			// NOMBRE IO

			dispositivo_io *d_io = recibir_io(fd_conexion);

			agregar_io(d_io);
			break;
		case FINALIZACION_IO:
			// si finalizo entonces recibo solo el codigo unicamente
			// aca tengo que verificar que mis dos listas
			dispositivo_io *dispositivio_io = encuentro_dispositivo_io(fd_conexion);
			log_info(logger, "## %d finalizó IO y pasa a READY", dispositivio_io->proceso_actual);
			proceso_blocked_a_ready(dispositivio_io->proceso_actual);
			
			dispositivio_io->estado = 'L';
			dispositivio_io->proceso_actual = 0;
			// Aca entonces deberiamos ver que IO quedan pendiente y asignarselas
			// si es que hay procesos en espera hacer algo si no , lo dejamos asi ya no sirve de nada ahcer algo
			asignar_a_un_proceso(d_io);

			break;
		case -1:
			log_warning(logger, "Cliente desconectado: fd=%d", fd_conexion);
			// ACA En caso de que la io se desoencte pero estaba ejecutando un proceso
			dispositivo_io *dispositivoEncontrado = encuentro_dispositivo_io(fd_conexion);

			if (dispositivoEncontrado->estado == 'O')
			{
				syscall_exit(dispositivoEncontrado->proceso_actual);
			}

			free(dispositivoEncontrado);
			close(fd_conexion); // Cerramos conexión
			return NULL;
		default:
			log_warning(logger, "Operación desconocida recibida del cliente: fd=%d", fd_conexion);
			break;
		}
	}

	return NULL;
}

void *atender_cliente_cpu(void *datosSinTipo)
{
	datos_cliente_t *datos = (datos_cliente_t *)datosSinTipo;
	int fd_conexion = *(datos->fd_conexion_ptr);
	char *servidor = datos->servidor;
	//char *pathInstrucciones = datos->path; // path de instrucciones (lo necesita memoria)
	// (*datos).fd_conexion_ptr == datos->fd_conexion_ptr   (RECORDANDO UN POCO LA SINTAXIS)

	free(datos->fd_conexion_ptr);
	free(datos); // Liberamos memoria reservada

	//t_list *lista;

	while (1) // habria que agregar una condicion de salida para cortar el programa manualmente
	{
		uint32_t cod_op = recibir_operacion(fd_conexion);

		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(fd_conexion, servidor);
			char *mensaje = "Hola desde servidor";
			enviar_mensaje(mensaje, fd_conexion);
			break;

		case CPU:
			cpu_datos *cpu = recibir_cpu(fd_conexion);

			pthread_mutex_lock(&MUTEX_LISTA_CPUS);
			cpu_datos *cpu_existente = buscar_cpu_por_id(cpu->identificador);

			if (cpu_existente == NULL)
			{
				// No estaba, la creamos y agregamos
				cpu_datos *nueva_cpu = malloc(sizeof(cpu_datos));
				nueva_cpu->identificador = strdup(cpu->identificador);
				nueva_cpu->dispatch = 0;
				nueva_cpu->interrupt = 0;
				nueva_cpu->estado = 'L';
				nueva_cpu->procesoEnEjecucion = NULL;
				if (strcmp(servidor, "DISPATCH(CPU)") == 0)
					nueva_cpu->dispatch = fd_conexion;
				else
					nueva_cpu->interrupt = fd_conexion;

				list_add(lista_cpus, nueva_cpu);

				if (nueva_cpu->dispatch != 0 && nueva_cpu->interrupt != 0)
				{
					sem_post(&sem_cpu_libre);
				}
			}
			else
			{
				// Ya existe, soSSlo completamos el otro socket
				if (strcmp(servidor, "DISPATCH(CPU)") == 0)
					cpu_existente->dispatch = fd_conexion;
				else
					cpu_existente->interrupt = fd_conexion;

				if (cpu_existente->dispatch != 0 && cpu_existente->interrupt != 0)
				{	
					sem_post(&sem_cpu_libre);
				}
			}
			pthread_mutex_unlock(&MUTEX_LISTA_CPUS);
			log_info(logger, "CPU %s conectada por %s", cpu->identificador, servidor);

			break;
		case EXIT:
			pcb* pcbNuevo = NULL;
			// debo cambiarle el estado a cpu
			cambiar_estado_cpu(fd_conexion, 'L',pcbNuevo);
			// recibo de cpu el pid
			uint32_t pid_a_finalizar = recibir_pid(fd_conexion);
			// log_info(logger, "## (%d)- Finaliza el proceso",pid_a_finalizar);
			syscall_exit(pid_a_finalizar);
			// signal cpu liberada
			sem_post(&sem_cpu_libre);
			break;
		case IO:
			dispositivo_tiempo *dt = recibir_tiempo_dispositivo(fd_conexion);
			log_info(logger, "## %d - Solicitó syscall: IO", dt->pid);
			//la cpu paso de ocupada a libre
			cambiar_estado_cpu(fd_conexion,'L',pcbNuevo);
			
			// aca se libera la CPU entonces la puedo usar podria mandar una señal
			sem_post(&sem_cpu_libre);
			// actualizo pc
			actualizar_pc_en_lista(dt->pid,dt->pc,&MUTEX_EXEC,listaExec);



			syscall_io(dt->dispositivo, dt->tiempo, dt->pid);

			break;
		case INIT_PROC:
			// recibidos datos para realizar init_proc
			t_info_nuevo_proceso *proceso = recibir_nuevo_proceso(fd_conexion);
			// el pid recibido es el del proceso padre
			log_info(logger, "## %d-Solicito syscall: INIT_PROC", proceso->pid);
			syscall_init_proc(proceso->nombre_archivo, proceso->tamanio_bytes_archivo);
			// el sem_post esta en init_proc
			
			break;
		case DUMP_MEMORY:
			//necesito recibir el pid y el pc para el contexto del proceso
			dump_datos *proceso_dump = recibir_datos_dump(fd_conexion);
			log_info(logger, "## %d-Solicito syscall: DUMP_MEMORY", proceso_dump->pid);
			//la cpu paso de ocupada a libre  REALMENTE ESTA LIBRE??
			cambiar_estado_cpu(fd_conexion,'L',NULL);
			
			// aca se libera la CPU entonces manda una señal de cpu libre
			sem_post(&sem_cpu_libre);
			// actualizo pc
			actualizar_pc_en_lista(proceso_dump->pid,proceso_dump->pc,&MUTEX_EXEC,listaExec);
			syscall_dump_memory(proceso_dump->pid);

			break;	
		case REPLANIFICAR:
				//sacar lista ready
				
				uint32_t pid_a_replanificar = recibir_pid(fd_conexion);


				cpu_datos *cpu_a_replanificar = buscar_cpu_por_conexion(fd_conexion);

				pcb *procesoEnEjecucion = sacar_proceso_de_lista(cpu_a_replanificar->procesoEnEjecucion->pid, &MUTEX_EXEC, listaExec);

				uint64_t ahora = obtenerTiempoAhora();
				int diferencia_milisegundos_a = ahora - procesoEnEjecucion->timestamp_inicio;
				//LA ESTIMACION cambia , pero la rafaga sigue siendo la misma
				procesoEnEjecucion->estimacionAnterior -= diferencia_milisegundos_a;


				actualizar_tiempo(procesoEnEjecucion, ESTADO_EXEC);
				
				//SACO EL PROCESO EN EJECUCION LO METO A READY

				pthread_mutex_lock(&MUTEX_COLA_READY);
				actualizar_metrica(procesoEnEjecucion,ESTADO_READY);
				list_add_sorted(listaReady,procesoEnEjecucion,comparacionRafagaMasChica);
				pthread_mutex_unlock(&MUTEX_COLA_READY);

				cpu_a_replanificar->procesoEnEjecucion = proceso;

			//proceso que tiene MAS prioridad que el que ejecuta lo saco de ready
				pcb *procesoAEjecutar = sacar_proceso_de_lista(pid_a_replanificar, &MUTEX_COLA_READY, listaReady);
				actualizar_tiempo(procesoAEjecutar, ESTADO_READY);
				// lo meto a ejecutar
				agregar_pcb_listaExec(procesoAEjecutar);

				enviar_paquete_proceso_cpu(cpu_a_replanificar->dispatch, procesoAEjecutar->pid, procesoAEjecutar->pc);

		break;
		case -1:
			log_warning(logger, "Cliente desconectado: fd=%d", fd_conexion);
			close(fd_conexion); // Cerramos conexión
			return NULL;
		default:
			log_warning(logger, "Operación desconocida recibida del cliente: fd=%d", fd_conexion);
			break;
		}
	}

	return NULL;
}

void cambiar_estado_cpu(int conexion, char nuevo_estado,pcb* pcbNuevo)
{
	pthread_mutex_lock(&MUTEX_LISTA_CPUS);
	cpu_datos *cpu_a_cambiar = buscar_cpu_por_conexion(conexion);
	cpu_a_cambiar->procesoEnEjecucion = pcbNuevo;
	cpu_a_cambiar->estado = nuevo_estado;
	pthread_mutex_unlock(&MUTEX_LISTA_CPUS);
}

cpu_datos *buscar_cpu_por_conexion(int conexion)
{

	bool cpu_contiene_conexion(void *ptr)
	{
		cpu_datos *cpu = (cpu_datos *)ptr;
		return (cpu->dispatch == conexion);
	}

	return list_find(lista_cpus, cpu_contiene_conexion);
}

void agregar_io(dispositivo_io *d_io)
{
	pthread_mutex_lock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	list_add(lista_dispositivos_io, d_io);
	pthread_mutex_unlock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
}

dispositivo_io *recibir_io(int socket_cliente)
{

	dispositivo_io *d_io = malloc(sizeof(dispositivo_io)); // LIBERAR

	uint32_t size;

	char *buffer = recibir_buffer(&size, socket_cliente);
	d_io->dispositivo = buffer;
	d_io->fd_conexion = socket_cliente;
	d_io->estado = 'L';
	d_io->proceso_actual = 0;

	return d_io;
}

cpu_datos *recibir_cpu(int socket_cliente)
{

	cpu_datos *cpu = malloc(sizeof(cpu_datos)); // LIBERAR

	uint32_t size;

	char *buffer = recibir_buffer(&size, socket_cliente);
	cpu->identificador = buffer;
	cpu->interrupt = socket_cliente;
	cpu->dispatch = socket_cliente;

	return cpu;
}

cpu_datos *buscar_cpu_por_id(char *id_cpu)
{
	for (int i = 0; i < list_size(lista_cpus); i++)
	{
		cpu_datos *cpu = list_get(lista_cpus, i);
		if (strcmp(cpu->identificador, id_cpu) == 0)
		{
			return cpu;
		}
	}
	return NULL;
}

uint32_t obtener_pid()
{
	pthread_mutex_lock(&MUTEX_PID);
	uint32_t nuevo_pid = pid++;
	pthread_mutex_unlock(&MUTEX_PID);
	return nuevo_pid;
}
dump_datos* recibir_datos_dump(int conexion_cpu){
	dump_datos* datos = malloc(sizeof(dump_datos));
	uint32_t size_datos;
	uint32_t desplazamiento = 0;
	
	void* buffer = recibir_buffer(&size_datos, conexion_cpu);
	//debo recibir el pid y el pc
	memcpy(&(datos->pid), buffer + desplazamiento, sizeof(uint32_t)); // recibo el pid
	desplazamiento += sizeof(uint32_t);

	memcpy(&(datos->pc), buffer + desplazamiento, sizeof(uint32_t)); // recibo el pc
	desplazamiento += sizeof(uint32_t);

	free(buffer);
	return datos;


	
}

dispositivo_tiempo *recibir_tiempo_dispositivo(int fd_conexion)
{

	dispositivo_tiempo *dt = malloc(sizeof(dispositivo_tiempo)); // LIBERAR
	uint32_t size;

	uint32_t desplazamiento = 0;

	void *buffer = recibir_buffer(&size, fd_conexion);

	memcpy(&(dt->pid), buffer + desplazamiento, sizeof(uint32_t)); // recibo el pid
	desplazamiento += sizeof(uint32_t);

	memcpy(&(dt->pc), buffer + desplazamiento, sizeof(uint32_t)); // recibo el pc
	desplazamiento += sizeof(uint32_t);

	memcpy(&(dt->tiempo), buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(dt->tamanio_nombre_dispositivo), buffer + desplazamiento, sizeof(uint32_t)); // recibo el tamanio del nombre del dispositivo
	desplazamiento += sizeof(uint32_t);

	dt->dispositivo = malloc(dt->tamanio_nombre_dispositivo);						  // DEBO LIBERAR DESPUES <-------------------
	memcpy(dt->dispositivo, buffer + desplazamiento, dt->tamanio_nombre_dispositivo); // recibo el nombre del archivo
	desplazamiento += dt->tamanio_nombre_dispositivo;

	free(buffer);
	return dt;
}

void syscall_io(char *dispositivo, uint32_t tiempo, uint32_t pid)
{

	dispositivo_io *dispositivoIO = seleccionar_io_libre(dispositivo);

	if (dispositivoIO == NULL)
	{
		// COMO NO ENCONTRE NINGUN DISPOSITIVO SALGO
		syscall_exit(pid);
	}
	else
	{
		bloqueo_proceso(pid);
		log_info(logger, "## %d - Bloqueado por IO: %s", pid, dispositivo);
		if (dispositivoIO->estado == 'L')
		{
			dispositivoIO->estado = 'O';
			enviar_tiempo_sleep_io(pid, tiempo, dispositivoIO->fd_conexion);
			dispositivoIO->proceso_actual = pid;
		}
		else
		{
			// aca como tengo la lista de cuales esperan mi proceso las entradas
			// aca creo un dato nuevo para guardar en esa estructura lista_procesos_espera_entrada_salida

			// que pasa si ya esta creado? entonces deberia primero verificar eso
			procesos_espera_entrada_salida *procesosEnEspera = encontrar_proceso_en_espera(dispositivo);

			if (procesosEnEspera != NULL)
			{
				pthread_mutex_lock(&MUTEX_PROCESOS_ESPERA);
				push_cola_pids_procesos_espera(procesosEnEspera->pids, pid);
				pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);
			}
			else
			{
				pthread_mutex_lock(&MUTEX_PROCESOS_ESPERA);
				procesos_espera_entrada_salida *procesoEnEsperaNuevo = malloc(sizeof(procesos_espera_entrada_salida));
				procesoEnEsperaNuevo->tiempo = tiempo;
				procesoEnEsperaNuevo->pids = queue_create();
				push_cola_pids_procesos_espera(procesoEnEsperaNuevo->pids, pid);
				procesoEnEsperaNuevo->dispositivo = dispositivo;

				list_add(lista_procesos_espera_entrada_salida, procesoEnEsperaNuevo); // liberar
				pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);
			}
		}
	}
}

dispositivo_io *seleccionar_io_libre(char *dispositivo)
{
	pthread_mutex_lock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	dispositivo_io *dispositivoEncontrado = NULL;

	for (int i = 0; i < list_size(lista_dispositivos_io); i++)
	{
		dispositivo_io *io = list_get(lista_dispositivos_io, i);
		if (strcmp(io->dispositivo, dispositivo) == 0)
		{ // usa strcmp para comparar strings
			if (io->estado == 'L')
			{
				pthread_mutex_unlock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
				return io; // si está libre, lo devuelves de inmediato
			}
			else
			{
				dispositivoEncontrado = io; // guarda el ocupado por si no hay libre
			}
		}
	}

	pthread_mutex_unlock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	return dispositivoEncontrado; // puede ser NULL si no encontró ninguno
}

void bloqueo_proceso(uint32_t pid)
{

	pcb *pcb_proceso = sacar_proceso_de_lista(pid, &MUTEX_EXEC, listaExec);
	// poner metrica log
	actualizar_tiempo(pcb_proceso, ESTADO_EXEC);


	metrica_estado_tiempo *elem = list_get(pcb_proceso->listaMetricas, ESTADO_EXEC);
	uint64_t ahora = obtenerTiempoAhora();
	uint64_t diferencia_milisegundos = ahora - pcb_proceso->timestamp_inicio;
	elem->milisegundos += diferencia_milisegundos;

	//si lo bloqueo tengo que actualizar su rafaga anterior y su estimacionanterior
	if(strcmp(datos_planificador->algoritmo_corto_plazo, "SRT") == 0)
	{
		pcb_proceso->estimacionAnterior = estimacionSiguiente(pcb_proceso->rafagaAnterior,pcb_proceso->estimacionAnterior);
		pcb_proceso->rafagaAnterior = diferencia_milisegundos;
	}

	agregar_pcb_a_lista(pcb_proceso,listaBlocked,&MUTEX_BLOCKED,ESTADO_BLOCKED);

	log_info(logger, "## %d Pasa del estado EXEC al estado BLOCKED", pid);
}

pcb *sacar_proceso_de_lista(uint32_t pid, pthread_mutex_t* mutex, t_list *listaASacar)
{
	pthread_mutex_lock(mutex);
	for (int i = 0; i < list_size(listaASacar); i++)
	{
		pcb *proceso = list_get(listaASacar, i);
		if (proceso->pid == pid)
		{
			list_remove(listaASacar, i);
			pthread_mutex_unlock(mutex);
			return proceso;
		}
	}
	pthread_mutex_unlock(mutex);
	return NULL;
}

void enviar_tiempo_sleep_io(uint32_t pid, uint32_t tiempo, int fd_conexion)
{

	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)USAR_IO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t) * 2;

	paquete->buffer->stream = malloc(paquete->buffer->size);

	//	op_code | size buffer | pid | tiempo

	int offset = 0;
	memcpy(paquete->buffer->stream, &pid, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(paquete->buffer->stream + offset, &tiempo, sizeof(uint32_t));

	enviar_paquete(paquete, fd_conexion);
	eliminar_paquete(paquete);
}

dispositivo_io *encuentro_dispositivo_io(int fd_conexion)
{
	pthread_mutex_lock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	for (int i = 0; i < list_size(lista_dispositivos_io); i++)
	{
		dispositivo_io *io = list_get(lista_dispositivos_io, i);
		if (io->fd_conexion == fd_conexion)
		{
			pthread_mutex_unlock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
			return io;
		}
	}
	pthread_mutex_unlock(&MUTEX_LISTA_ENTRADAS_SALIDAS);
	return NULL;
}

void asignar_a_un_proceso(dispositivo_io *d_io)
{

	// buscamos si tiene algun registro creado cuando se espero poor el
	procesos_espera_entrada_salida *procesoEntrada = encontrar_proceso_en_espera(d_io->dispositivo);
	// si hay entonces hay que verificar si hay procesos todavia
	if (procesoEntrada != NULL)
	{	
		uint32_t pids = cantidadPids(procesoEntrada->pids);
		if(pids > 0)
		{
			uint32_t pid = pop_cola_pids_procesos_espera(procesoEntrada->pids);
			// ejecuto IO ya que la cpu mando procesos pero estan en espera.
			//como removi uno y si no queda ninguno entonces puedo eliminar esa estructura
			if(pids - 1 == 0){
				remover_proceso_entrada_salida(procesoEntrada,lista_procesos_espera_entrada_salida,&MUTEX_PROCESOS_ESPERA);
				libero_procesos_en_espera(procesoEntrada);
			}

			d_io->estado = 'O';
			enviar_tiempo_sleep_io(pid, procesoEntrada->tiempo, d_io->fd_conexion);
			d_io->proceso_actual = pid;
		}
		else{
			remover_proceso_entrada_salida(procesoEntrada,lista_procesos_espera_entrada_salida,&MUTEX_PROCESOS_ESPERA);
			libero_procesos_en_espera(procesoEntrada);
		}
	}
}

procesos_espera_entrada_salida *encontrar_proceso_en_espera(char *dispositivo)
{
	pthread_mutex_lock(&MUTEX_PROCESOS_ESPERA);
	for (int i = 0; i < list_size(lista_cpus); i++)
	{
		procesos_espera_entrada_salida *procesosEnEspera = list_get(lista_cpus, i);
		if (procesosEnEspera->dispositivo == dispositivo)
		{
			pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);
			return procesosEnEspera;
		}
	}
	pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);
	return NULL;
}

void proceso_blocked_a_ready(uint32_t pid)
{

	// remove de la lista con ese pid
	pcb *proceso = sacar_proceso_de_lista(pid, &MUTEX_BLOCKED, listaBlocked);
	// actualizo el tiempo

	actualizar_tiempo(proceso, ESTADO_BLOCKED);
	// lo meto en ready
	agregar_pcb_a_lista(proceso,listaReady,&MUTEX_COLA_READY,ESTADO_READY);

	sem_post(&sem_ready);
	log_info(logger, "## %d Pasa del estado BLOCKED al estado READY", pid);
}


void actualizar_pc_en_lista(uint32_t pid, uint32_t nuevo_pc, pthread_mutex_t* mutex, t_list* lista)
{
    pthread_mutex_lock(mutex);

    for (int i = 0; i < list_size(lista); i++)
    {
        pcb* proceso = list_get(lista, i);
        if (proceso->pid == pid)
        {
            proceso->pc = nuevo_pc;
            break; // Ya lo encontramos y actualizamos
        }
    }

    pthread_mutex_unlock(mutex);
}


void iniciar_colas_y_listas(){
	
    listaReady = list_create();
    listaSuspReady = list_create();
	listaNew = list_create();
    lista_dispositivos_io = list_create();
    lista_cpus = list_create();
    lista_procesos_espera_entrada_salida = list_create();
    listaExec = list_create();
    listaBlocked = list_create();
}


void inicializo_datos(){

  iniciar_colas_y_listas();
  iniciar_semaforos();

}



void esperar_enter()
{
	char* leido;

	// La primera te la dejo de yapa
	leido = readline("> ");

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(leido != NULL && strcmp(leido, "") != 0){
		leido = readline("> ");
	}

	free(leido);

	// ¡No te olvides de liberar las lineas antes de regresar!

}


pcb* obtener_pcb_lista(t_list* lista, pthread_mutex_t * mutex){

	pthread_mutex_lock(mutex);
		
	if (list_size(lista) > 0 )
	{
		//busco el primero ya que es el siguiente que sigue 
		pcb *proceso = list_get(lista, 0);
		pthread_mutex_unlock(mutex);
		return proceso;
	}
		
	pthread_mutex_unlock(mutex);
	return NULL;

}




pcb* obtener_pcb_lista_segun_pid(t_list* lista, pthread_mutex_t * mutex,uint32_t pid){

	pthread_mutex_lock(mutex);
		
	if (list_size(lista) > 0 )
	{

		for(int i = 0;i < list_size(lista);i++)
		{
			
		pcb* proceso = list_get(lista, i);
        if (proceso->pid == pid)
        {
            
			pthread_mutex_unlock(mutex);
			return proceso;
        }

		}
	
	}
		
	pthread_mutex_unlock(mutex);
	return NULL;

}

//esta funcion sirve para remover el pcb cuando ya lo obtuvimos de alguna forma.
void remover_pcb_de_lista(pcb* proceso,t_list* lista,pthread_mutex_t* mutex){
		pthread_mutex_lock(mutex);
		list_remove_element(lista,proceso);
		pthread_mutex_unlock(mutex);
}

void remover_proceso_entrada_salida(procesos_espera_entrada_salida* proceso,t_list* lista,pthread_mutex_t* mutex){
		pthread_mutex_lock(mutex);
		list_remove_element(lista,proceso);
		pthread_mutex_unlock(mutex);
}




void crear_proceso(char* proceso_pseudocodigo,uint32_t tamanio_proceso){

	uint32_t nuevo_pid = obtener_pid();
    pcb *pcb_inicial = crear_pcb(nuevo_pid,proceso_pseudocodigo,tamanio_proceso);
	
	if (strcmp(datos_planificador->algoritmo_largo_plazo, "PMCP") == 0){
		actualizar_metrica(pcb_inicial,ESTADO_NEW);
		pthread_mutex_lock(&MUTEX_NEW);
		list_add_sorted(listaNew,pcb_inicial,comparacionEsMasChicoTamanioBytes);
		pthread_mutex_unlock(&MUTEX_NEW);
	}
	else{
		agregar_pcb_a_lista(pcb_inicial,listaNew,&MUTEX_NEW,ESTADO_NEW);
	}
	
    sem_post(&sem_nuevo_proceso); 
}




bool hayElementosEnLista(t_list* lista, pthread_mutex_t* mutex){
		pthread_mutex_lock(mutex);
		bool hayElementos = list_size(lista);
		pthread_mutex_unlock(mutex);

		return hayElementos;
}


uint32_t cantidadPids(t_queue * pids){

	pthread_mutex_lock(&MUTEX_PROCESOS_ESPERA);
	uint32_t cantidadProcesosEnEspera = queue_size(pids);
	pthread_mutex_unlock(&MUTEX_PROCESOS_ESPERA);

	return cantidadProcesosEnEspera;
}


void libero_procesos_en_espera(procesos_espera_entrada_salida* procesoEntrada){
	queue_destroy(procesoEntrada->pids);
	free(procesoEntrada->dispositivo);
	free(procesoEntrada);
}


bool comparacionEsMasChicoTamanioBytes(void* a, void* b) {
    pcb* proceso_a = (pcb*) a;
    pcb* proceso_b = (pcb*) b;
    return proceso_a->tamanio_bytes <= proceso_b->tamanio_bytes;
}


//Est(n+1) =  R(n) + (1-) Est(n) ;     [0,1]


								//proceso3 //proceso2
								//proceso2   //proceso3
bool comparacionRafagaMasChica(void* a, void* b) {
    pcb* proceso_a = (pcb*) a;
    pcb* proceso_b = (pcb*) b;
	

	// uint64_t ahora = obtenerTiempoAhora();
	// int diferencia_milisegundos_a = ahora - proceso_a->timestamp_inicio;
	// int diferencia_milisegundos_b = ahora - proceso_b->timestamp_inicio;

	
	int estimacion_a = estimacionSiguiente(proceso_a->rafagaAnterior,proceso_a->estimacionAnterior);
	int estimacion_b = estimacionSiguiente(proceso_b->rafagaAnterior,proceso_b->estimacionAnterior);

	log_info(logger, "Estimacion de proceso nombre %s y estimacion: %d   y su rafaga anterior era : %ld", proceso_a->nombre_archivo,estimacion_a,proceso_a->rafagaAnterior);
	log_info(logger, "Estimacion de proceso nombre %s y estimacion: %d  y su rafaga anterior era: %ld", proceso_b->nombre_archivo,estimacion_b,proceso_b->rafagaAnterior);

	// -1   -2 
    return estimacion_a < estimacion_b;
}


int estimacionSiguiente(int tiempoEjecutado,int estimacionAnterior){
	return   (datos_planificador->alfa)*tiempoEjecutado + (1 - datos_planificador->alfa)*estimacionAnterior;
}




bool estaLibre(void* ptr) {
	cpu_datos* cpu = (cpu_datos*) ptr;
	return cpu->estado == 'L';
}



bool cpuLibres(){
	bool hayLibres ;
	pthread_mutex_lock(&MUTEX_LISTA_CPUS);
	hayLibres = list_any_satisfy(lista_cpus,estaLibre) ;
	pthread_mutex_unlock(&MUTEX_LISTA_CPUS);

	return hayLibres;
}



void lanzarInterrupcion(cpu_datos* cpu,pcb* proceso){

	//aca envio un mensaje
	

	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)INTERRUPCION;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;

	paquete->buffer->stream = malloc(paquete->buffer->size);



	memcpy(paquete->buffer->stream, &(proceso->pid), sizeof(uint32_t));

	//	op_code | size buffer | pid


	enviar_paquete(paquete, cpu->interrupt);
	eliminar_paquete(paquete);

}