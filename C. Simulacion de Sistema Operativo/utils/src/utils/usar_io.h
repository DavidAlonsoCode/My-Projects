#ifndef USAR_IO_H
#define USAR_IO_H

#include "hello.h"

t_buffer* usar_io_serializar(t_usar_io* usar_io);
t_usar_io* usar_io_deserializar(t_buffer* buffer_usar_io);
void liberar_usar_io(t_usar_io* datos);
t_buffer* crear_buffer_segun(uint32_t tamanio_a_enviar);
void aniadir_int_a_buffer(t_buffer* buffer,uint32_t dato,int*valor);
void aniadir_string_a_buffer(t_buffer* buffer,uint32_t longitud_s,char*string,int*valor);
void empaquetar_y_enviar(uint32_t codigo_op,t_buffer* buffer,int un_socket);
t_buffer* leer_buffer(int un_socket);
int buffer_leer_int(t_buffer*buffer,int*valor); 
void liberar_buffer(t_buffer* buff);

#endif