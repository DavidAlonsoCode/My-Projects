#include "syscalls.h"

void syscall_init_proc(char*archivo, uint32_t tamanio_archivo){
  //falta la parte de elegir a quien poner en este caso si es fifo da bien pero los demas no 
   crear_proceso(archivo,tamanio_archivo);
    
}

//libera el pcb del proceso con el pid recibido 
void syscall_exit(uint32_t pid,int estado,t_list* lista){
    
    //cuando cpu lee la syscalls EXIT lo manda aca o cuando hay desconexion io o memoria da error en DUMP
    
    int conexion_memoria = crear_conexion(datos_planificador->ip_memoria, datos_planificador ->puerto_memoria);
    // 
    enviar_destruccion_proceso(pid,conexion_memoria);
    uint32_t codigo_operacion = recibir_operacion(conexion_memoria);
    char* respuesta;
    if(codigo_operacion == MENSAJE){
         uint32_t tamanio_buff;
         respuesta = recibir_buffer(&tamanio_buff,conexion_memoria);
    }
   
    if(strcmp(respuesta, "OK") == 0){
        //el proceso tambien podria estar el estado BLOCKE

        pcb* pcb_a_finalizar = sacar_proceso_de_lista_sin_mutex(pid,lista);
        actualizar_tiempo(pcb_a_finalizar ,estado);

        actualizar_metrica(pcb_a_finalizar,ESTADO_EXIT);
        actualizar_tiempo(pcb_a_finalizar,ESTADO_EXIT);
     //hago el log de metricas de estado 
        mostrar_metricas_finales(pcb_a_finalizar);
     //libero pcb del proceso recibido
        liberar_pcb(pcb_a_finalizar);
    }
    close(conexion_memoria);
    

}
void* hilo_syscall_dump(void* arg){
    uint32_t pid = *((uint32_t*)arg);
    free(arg);
    syscall_dump_memory(pid);
    return NULL;
}

void syscall_dump_memory(uint32_t pid){
  
    //se supone que este proceso hara su dump
    // antes de cumplir el tiempo limite de susp.blocked asi no pasa a susp.blocked
    int conexion_memoria = crear_conexion(datos_planificador->ip_memoria, datos_planificador ->puerto_memoria);
    if (conexion_memoria < 0) {
        	//esta n lista blocked? o esta en lista suspendedblocked

		pthread_mutex_lock(&MUTEX_BLOCKED);
		pcb* pcb_encontrado_blocked = obtener_pcb_lista_segun_pid_sin_mutex(listaBlocked,pid);
		
		bool estaEnBlocked = pcb_encontrado_blocked != NULL;

		if(estaEnBlocked){
			syscall_exit(pid,ESTADO_BLOCKED,listaBlocked);
		}

		pthread_mutex_unlock(&MUTEX_BLOCKED);

        if(!estaEnBlocked)
        {
            pthread_mutex_lock(&MUTEX_SUSPD_BLOCKED);
            pcb* pcb_encontrado_susp_blocked = obtener_pcb_lista_segun_pid_sin_mutex(listaSuspBlocked,pid);
            
            if(pcb_encontrado_susp_blocked != NULL)
            {

                syscall_exit(pid,ESTADO_SUSP_BLOCKED,listaSuspBlocked);
            }
            
            pthread_mutex_unlock(&MUTEX_SUSPD_BLOCKED);
        }


        sem_post(&sem_nuevo_proceso);

        return;
    }

    enviar_proceso_dump(conexion_memoria,pid);

   
    uint32_t codigo_operacion = recibir_operacion(conexion_memoria);
    char* respuesta;
     //ver si pongo otro codigo de operacion 
     if(codigo_operacion == MENSAJE)
     {
          uint32_t tamanio_buff;
          respuesta = recibir_buffer(&tamanio_buff,conexion_memoria);
         //terminamos la conexion sin importar si se pudo o no hacer dump
            close(conexion_memoria);
         if(strcmp(respuesta,"FIN_DUMP")==0){
            
            log_info(logger, "## %d finalizó DUMP MEMORY y pasa a READY",pid);
            proceso_blocked_a_ready(pid,true);
            
        }else{
        
        log_info(logger, "## %d no se logro hacer DUMP MEMORY",pid);
        pthread_mutex_lock(&MUTEX_BLOCKED);
		pcb* pcb_encontrado_blocked = obtener_pcb_lista_segun_pid_sin_mutex(listaBlocked,pid);
		
		bool estaEnBlocked = pcb_encontrado_blocked != NULL;

		if(estaEnBlocked){
			syscall_exit(pid,ESTADO_BLOCKED,listaBlocked);
		}

		pthread_mutex_unlock(&MUTEX_BLOCKED);

		if(!estaEnBlocked)
        {
            pthread_mutex_lock(&MUTEX_SUSPD_BLOCKED);
            pcb* pcb_encontrado_susp_blocked = obtener_pcb_lista_segun_pid_sin_mutex(listaSuspBlocked,pid);
            
            if(pcb_encontrado_susp_blocked != NULL)
            {
            syscall_exit(pid,ESTADO_SUSP_BLOCKED,listaSuspBlocked);

            }
            
            pthread_mutex_unlock(&MUTEX_SUSPD_BLOCKED);
		}

        sem_post(&sem_nuevo_proceso);
     }
     
    }
}


void enviar_proceso_dump(int conexion_memoria,uint32_t pid){
    t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)PETICION_DUMP;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t);

    paquete->buffer->stream = malloc(paquete->buffer->size);

    // codigo op| size buffer (tamanio pid)|pid
	memcpy(paquete->buffer->stream, &pid, sizeof(uint32_t));

    enviar_paquete(paquete,conexion_memoria);

    eliminar_paquete(paquete);

}


void mostrar_metricas_finales(pcb* pcb_finalizado){
    char* metricas = string_new ();
   
    for(int i = 0; i< CANT_ESTADOS; i++){
     metrica_estado_tiempo* aux_metricas = list_get(pcb_finalizado->listaMetricas, i);

     char* aux = string_from_format("%s %d %ld ,",
                                   aux_metricas->estado,
                                   aux_metricas->cantidadDeVecesQueEstuvo,
                                   aux_metricas->milisegundos);

     string_append(&metricas, aux);  

    free(aux);  
    }
    
    log_info(logger,"## (%d)- Metricas de estado: %s",pcb_finalizado->pid, metricas);
    free(metricas);
}
void destruir_metrica(void* metrica) {
    metrica_estado_tiempo* metrica_cas = (metrica_estado_tiempo*)metrica;
    free(metrica_cas->estado); //cada estado fue strdup
    free(metrica);  // Cada métrica fue malloc'd
}

void liberar_pcb(pcb* proceso_a_finalizar){
    
    if (proceso_a_finalizar == NULL) return;

    // Liberar cada métrica de la lista y luego la lista misma
    if (proceso_a_finalizar->listaMetricas != NULL) {
        list_destroy_and_destroy_elements(proceso_a_finalizar->listaMetricas, destruir_metrica);
    }
    //no liberamos el nombre del archivo ya que no fue reservada memoria y daba error
    log_info(logger,"## %d - Finaliza el proceso",proceso_a_finalizar->pid);
    // Finalmente, liberar el PCB
    free(proceso_a_finalizar);

    
}    


void uint32_to_str(uint32_t num, char* buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%u", num);
}

void enviar_destruccion_proceso(uint32_t pid ,int conexion_memoria){
    // t_buffer *buffer = crear_buffer_segun(sizeof(uint32_t) * 2 );
    // //lo que recibe crear buffer es el tamanio de lo que va en el stream
    // int offset = 0;
    // void* inicio = buffer->stream;
    // //aniadimos al buffer el pid
    //  aniadir_int_a_buffer(buffer,pid,&offset);

    // buffer->stream = inicio;//ponerlo en otro lugar
    // empaquetar_y_enviar(DESTRUCCION_PROCESO,buffer,conexion_memoria);

    //liberar_buffer(buffer);

    t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = (uint32_t)DESTRUCCION_PROCESO;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = sizeof(uint32_t);  
	
//   op_code | size bufer | pid 

    paquete->buffer->stream = malloc(paquete->buffer->size);

    memcpy(paquete->buffer->stream , &pid, sizeof(uint32_t));

	enviar_paquete(paquete, conexion_memoria);
	eliminar_paquete(paquete);
}

    



//    si kernel recibe que una syscall io o sjf con desalojo o dumpmemoria 
//debe mandarle la interrupcion por la conexion  interrup ?