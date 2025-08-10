#include "interfaz_io.h"
#include "utils/hello.h"

int main(int argc, char* argv[]) {
    saludar("io");

	int fd_conexion_kernel;
	char* ip;
	char* puerto;
	char* dispositivo_io_conectado = argv[1];
	char* nivel_log;
	char* rutaConfig = argv[2];

	t_config* config = iniciar_config(rutaConfig);

	ip = config_get_string_value(config, "IP_KERNEL");
	puerto = config_get_string_value(config, "PUERTO_KERNEL");	
	nivel_log = config_get_string_value(config,"LOG_LEVEL");
	
	logger = iniciar_logger("io.log", "IO_LOGGER",nivel_log);

	
	log_info(logger, "El logger se inició correctamente");


	log_info(logger, "Dispositivo conectado: %s", dispositivo_io_conectado);
	
	// Creamos una conexión hacia el servidor
	fd_conexion_kernel = crear_conexion(ip, puerto);
	
	// Enviamos al servidor un mensaje con el dispositivo que se conecto
	enviar_mensaje_codigo_operacion(dispositivo_io_conectado, fd_conexion_kernel,HANDSHAKE_IO);
	t_usar_io* datos_a_usar; 
	while(1){
		
		uint32_t cod_operacion = recibir_operacion(fd_conexion_kernel);

		switch (cod_operacion){
			case USAR_IO:
				datos_a_usar = recibir_usar_io(fd_conexion_kernel);
				comenzar_uso(datos_a_usar);
				finalizar_uso(datos_a_usar,fd_conexion_kernel);
				datos_a_usar = NULL;
			break;
			case MENSAJE:
				recibir_mensaje_desde_cliente(fd_conexion_kernel);
			break;
		}
    	
	}


	terminar_programa(fd_conexion_kernel, logger, config);
}
