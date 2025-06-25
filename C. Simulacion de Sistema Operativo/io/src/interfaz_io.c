#include "interfaz_io.h"

t_usar_io* recibir_usar_io(int conexion){
    
	t_usar_io* dt = malloc(sizeof(t_usar_io)); //LIBERAR
	uint32_t size;

	uint32_t desplazamiento = 0;

	void *buffer = recibir_buffer(&size, conexion); 

	memcpy(&(dt->pid), buffer + desplazamiento, sizeof(uint32_t)); //recibo el pid
	desplazamiento += sizeof(uint32_t);
	memcpy(&(dt->tiempo), buffer + desplazamiento, sizeof(uint32_t)); 
	desplazamiento += sizeof(uint32_t);

	free(buffer);
	return dt;
}

void comenzar_uso(t_usar_io* datos){

	usleep(datos->tiempo * 1000);
    log_info(logger, "PID:%d - INICIO DE IO - Tiempo: %d", datos->pid,datos->tiempo);

}

void finalizar_uso(t_usar_io* datos,int conexion){
    int codigo = FINALIZACION_IO;

    send(conexion,&codigo,sizeof(uint32_t),0);

    log_info(logger, "PID:%d - FIN DE IO", datos->pid);

    liberar_usar_io(datos);

}
// void desconectar_io(int signal){
//     extern int fd_conexion_kernel;
//     int codigo = DESCONEXION_IO;
//     //avisar a kernel 
//     send(fd_conexion_kernel,&codigo,sizeof(int),0);
//     close(fd_conexion_kernel);
        
//     exit(0);
// }