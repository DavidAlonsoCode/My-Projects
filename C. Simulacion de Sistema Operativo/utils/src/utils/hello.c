#include <utils/hello.h>

void saludar(char *quien)
{
	printf("Hola desde %s!!\n", quien);
}

t_log *logger;

/////////////////////////////////// INICIO SECCION INICIAR ///////////////////////////////////////
int iniciar_servidor(char *puerto, char *nombreServidor)
{
	int socket_servidor;

	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creo el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
							 servinfo->ai_socktype,
							 servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");
	log_info(logger, "Servidor %s listo para recibir clientes...", nombreServidor);

	return socket_servidor;
}

t_log *iniciar_logger(char *nombreArchivo, char *nombreProceso)
{

	t_log *nuevo_logger = log_create(nombreArchivo, nombreProceso, 0, LOG_LEVEL_INFO);

	if (nuevo_logger == NULL)
	{
		perror("Se produjo un error");
		exit(EXIT_FAILURE);
	}

	// log_info(logger, "Logger iniciado correctamente");
	return nuevo_logger;
}

t_config *iniciar_config(char *ruta)
{

	t_config *nuevo_config = config_create(ruta);

	if (nuevo_config == NULL)
	{
		perror("No se encontró el archivo en el path especificado");
		exit(EXIT_FAILURE);
	}
	// log_info(logger, "Config iniciado correctamente");

	return nuevo_config;
}

/////////////////////////////////// FIN SECCION INICIAR ///////////////////////////////////////

/////////////////////////////////// INICIO SECCION RECIBIR ///////////////////////////////////////
uint32_t recibir_operacion(int socket_cliente)
{
	uint32_t cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

uint32_t recibir_operacion_no_bloqueante(int socket_cliente)
{
	uint32_t cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(uint32_t), MSG_DONTWAIT) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(uint32_t *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(uint32_t), MSG_WAITALL); 
	//recv devuelve nro de bytes recibidos, y en el seg param recibe los bytes indicados en el 3er parametro
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje_desde_cliente(int socket_cliente)
{
	uint32_t size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Se recibio mensaje del servidor %s", buffer);
	free(buffer);
}

void recibir_mensaje(int socket_cliente, char *nombreServidor)
{
	uint32_t size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "## %s Conectado a %s - FD del socket: %d", buffer,nombreServidor,socket_cliente);
	free(buffer);
}




t_list *recibir_paquete(int socket_cliente)
{
	uint32_t size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

/////////////////////////////////// FIN SECCION RECIBIR ///////////////////////////////////////

/////////////////////////////////// INICIO SECCION ENVIAR ///////////////////////////////////////
void enviar_mensaje(char *mensaje, int socket_cliente)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	uint32_t bytes = paquete->buffer->size + 2 * sizeof(uint32_t);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}


void enviar_mensaje_codigo_operacion(char *mensaje, int socket_cliente,op_code codigoOperacion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)codigoOperacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	uint32_t bytes = paquete->buffer->size + 2 * sizeof(uint32_t);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	uint32_t bytes = paquete->buffer->size + 2 * sizeof(uint32_t);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void enviar_paquete_proceso_memoria(int conexion,uint32_t pid, char* nombreArchivo,uint32_t tamanio_bytes)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

    uint32_t largo_nombre = strlen(nombreArchivo) + 1;

	paquete->codigo_operacion = PROCESO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t)*3 + largo_nombre;  
	
//   op_code | size bufer | pid | strlen nombre_archivo | nombre_archivo | tamanio_bytes

    paquete->buffer->stream = malloc(paquete->buffer->size);

    int offset = 0;
    memcpy(paquete->buffer->stream , &pid, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, &largo_nombre, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    memcpy(paquete->buffer->stream + offset, nombreArchivo, largo_nombre);
    offset += largo_nombre;

    memcpy(paquete->buffer->stream + offset, &tamanio_bytes, sizeof(uint32_t));

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}


/////////////////////////////////// FINALIZA SECCION ENVIAR ///////////////////////////////////////

int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se acepto una conexion nueva, cuyo FD es %d", socket_cliente);

	return socket_cliente;
}


uint32_t recibir_pid(int fd_conexion){

	 uint32_t size;

	 uint32_t pid = 0 ;//malloc(sizeof(uint32_t)); //LIBERAR

	 uint32_t desplazamiento = 0;

	 void *buffer = recibir_buffer(&size, fd_conexion); 

	 memcpy(&pid, buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
	 desplazamiento += sizeof(uint32_t);

	 free(buffer);
	 return pid;
	
}

void *serializar_paquete(t_paquete *paquete, uint32_t bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}
// esta funcion solo envia texto
void paquete(int conexion)
{
	char *leido;
	t_paquete *paquete = crear_paquete(PAQUETE);

	// Leemos y agregamos las lineas al paquete
	leido = readline("> ");
	while (strcmp(leido, "\0") != 0)
	{ // o strcmp(leido, "") != 0
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
		leido = readline("> ");
	}

	free(leido);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void terminar_programa(int conexion, t_log *logger, t_config *config)
{
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}

int crear_conexion(char *ip, char *puerto) // para cliente
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	return socket_cliente;
}

void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(op_code codigoOperacion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = (uint32_t)codigoOperacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}




t_info_nuevo_proceso *recibir_nuevo_proceso(int socket_cliente) 
{											
	/****************************************************************************************************************************
	Aca, en un principio, memoria va a recibir datos para crear un proceso, por eso se llama recibir_nuevo_proceso
	Solo espera su PID, nombre del archivo pseudocodigo y tamanio del archivo, pero para recibir el nombre, debo tener antes su tamanio...
	... entonces agrego esa variable de tipo "int" al TAD "t_info_nuevo_proceso". 
	
	Previamente en el while se recibió el cod_op, por lo que ya se que es un proceso
	Conclusion: X ahora pido TAMANIO DE BUFFER, luego PID, tamanio de nombre del archivo, luego nombre del archivo y luego el tamanio del archivo
	*****************************************************************************************************************************/
	t_info_nuevo_proceso* info_proceso = malloc(sizeof(t_info_nuevo_proceso)); //LIBERAR MEMORIA DESPUES <-----------------
	uint32_t size;
	void *buffer = recibir_buffer(&size, socket_cliente); //recibo el tamaño en size y el buffer en buffer

	//des_serializo el buffer que contiene el nuevo proceso
	uint32_t desplazamiento = 0;
	desplazamiento = des_serializar_buffer_nuevo_proceso(buffer, size, info_proceso);
	
	if(desplazamiento != size){
		log_error(logger, "Error al recibir el proceso, tamanios no coinciden %d ,size %d",desplazamiento,size);
		free(buffer);
		free(info_proceso->nombre_archivo);
		free(info_proceso);
		return NULL;
	}

	free(buffer);
	return info_proceso;
}

uint32_t des_serializar_buffer_nuevo_proceso(void* buffer, uint32_t size, t_info_nuevo_proceso* info_proceso)
{
	int desplazamiento = 0;
	memcpy(&(info_proceso->pid), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
	desplazamiento += sizeof(uint32_t);
	memcpy(&(info_proceso->tamanio_nombre_archivo), buffer + desplazamiento, sizeof(uint32_t)); //recibo el tamanio del nombre del archivo
	desplazamiento += sizeof(uint32_t);
	info_proceso->nombre_archivo = malloc(info_proceso->tamanio_nombre_archivo); //DEBO LIBERAR DESPUES <-------------------
	memcpy(info_proceso->nombre_archivo, buffer + desplazamiento, info_proceso->tamanio_nombre_archivo); //recibo el nombre del archivo
	desplazamiento += info_proceso->tamanio_nombre_archivo;
	memcpy(&(info_proceso->tamanio_bytes_archivo), buffer + desplazamiento, sizeof(uint32_t)); //recibo el tamanio del archivo
	desplazamiento += sizeof(uint32_t);

	return desplazamiento; //devuelvo el desplazamiento para saber si se recibio todo el buffer (comparando con el size)
}
