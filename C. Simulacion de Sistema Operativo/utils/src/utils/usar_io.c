
#include "usar_io.h"

//lo va a usar kernel e io
t_buffer* usar_io_serializar(t_usar_io* usar_io){
    t_buffer *buffer = crear_buffer_segun(sizeof(uint32_t) * 2 );
    //lo que recibe crear buffer es el tamanio de lo que va en el stream
    int offset = 0;
    void* inicio = buffer->stream;
    //aniadimos al buffer los datos de usar_io
     aniadir_int_a_buffer(buffer,usar_io->pid,&offset);
     aniadir_int_a_buffer(buffer, usar_io->tiempo,&offset);

    buffer->stream = inicio;//ponerlo en otro lugar

    return buffer;
    //este buffer despues lo vamos a empaquetar y tambien debemos liberarlo
}

//podria haber una funcion de calcular tamanio segun el tipo de operacion este caso usar_io

// esto lo utilizar io cuando recibe el pid y el tiempo de usleep que le manda kernel
t_usar_io* usar_io_deserializar(t_buffer* buffer_usar_io) {
    t_usar_io* usar_io = malloc(sizeof(t_usar_io));
    int offset = 0;
    usar_io->pid = buffer_leer_int(buffer_usar_io,&offset);//cambiar nombre
    usar_io->tiempo = buffer_leer_int(buffer_usar_io,&offset);
    
    return usar_io;
}
//tambien en el TAD se pondran las funciones de liberar memoria
void liberar_usar_io(t_usar_io* datos){
    if(datos!=NULL){
        free(datos);
    }
}

//no deberia estar aca despues lo muevo
// mejoracion de serializacion
//esto va a estar en cada modulo segun sus operaciones
//kernel a io
// void enviar_operacion(int codigo_op,void* dato_a_enviar,int conexion)
// serializar usar_io(mete los datos de las estructuras a un buffer)
// empaquetar()
//enviar paquete()
//
// donde se serializa segun el tipo de dato a enviar 
// se empaqueta y se envia el paquete


//**SERIALIZAR**
//crea un bufer listo para llenar segun un tipo de dato
t_buffer* crear_buffer_segun(uint32_t tamanio_a_enviar)
{
    t_buffer* buffer = malloc(sizeof(t_buffer));

    buffer->size = tamanio_a_enviar;
    //buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
    return buffer;

}

void aniadir_int_a_buffer(t_buffer* buffer,uint32_t dato,int*valor){

    memcpy(buffer->stream + *valor, &dato, sizeof(uint32_t));
    *valor += sizeof(uint32_t);
}

void aniadir_string_a_buffer(t_buffer* buffer,uint32_t longitud_s,char*string,int*valor){
    memcpy(buffer->stream + *valor, &longitud_s, sizeof(uint32_t));
    *valor += sizeof(uint32_t);
    memcpy(buffer->stream + *valor,string, longitud_s );
    *valor+=longitud_s;
}


//calcula el tamanio del payload segun el dato que es
//es mejor que este en el tad de la operacion int tamanio_a_enviar(void* dato_a_enviar)

//empaquetar lo que hace es meter en a_enviar  el header con el payload
//el header es el codigo_op y el payload es todo el contenido del buffer 
//al final lo envia
void empaquetar_y_enviar(uint32_t codigo_op,t_buffer* buffer,int un_socket){
    t_paquete* paquete = malloc(sizeof(t_paquete));

    paquete->codigo_operacion = codigo_op; // Podemos usar una constante por operación
    paquete->buffer = buffer; // Nuestro buffer de antes.

    void* a_enviar = malloc(buffer->size + sizeof(uint32_t)*2);
    int offset = 0;

    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    
	
	//enviamos el paquete
	send(un_socket, a_enviar, paquete->buffer->size +  sizeof(int)*2 ,0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
	eliminar_paquete(paquete);

}



//**DESEREALIZAR**

t_buffer* leer_buffer(int un_socket) {
    t_buffer* buffer = malloc(sizeof(t_buffer));
    //int offset = 0;

    // Recibir tamaño del stream
    recv(un_socket, &(buffer->size), sizeof(uint32_t), 0);

    // Reservar memoria para el contenido
    buffer->stream = malloc(buffer->size);

    // Recibir el contenido
    recv(un_socket, buffer->stream, buffer->size, 0);

    return buffer;
}

int buffer_leer_int(t_buffer*buffer,int*valor){
    uint32_t dato_int;
    // Deserializamos los campos que tenemos en el buffer
    memcpy(&dato_int, buffer->stream + (*valor), sizeof(uint32_t));
    *valor += sizeof(uint32_t);
    return dato_int;
}

void liberar_buffer(t_buffer* buff){
	free(buff->stream);
	free(buff);
}