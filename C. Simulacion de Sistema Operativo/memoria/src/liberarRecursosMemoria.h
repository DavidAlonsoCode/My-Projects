#ifndef LIBERAR_RECURSOS_MEMORIA_H
#define LIBERAR_RECURSOS_MEMORIA_H
#include <utils/hello.h>
#include <auxiliaresMemoria.h>

void destruir_diccionario_instrucciones();
void liberar_instrucciones(char* key, t_list* instrucciones);
void destruir_lista_frames(t_list* lista_frames);

#endif