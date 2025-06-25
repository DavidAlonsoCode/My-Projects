#ifndef INTERFAZ_IO_H
#define INTERFAZ_IO_H

#include "utils/usar_io.h"
t_usar_io* recibir_usar_io(int conexion);
void comenzar_uso(t_usar_io* datos);
void finalizar_uso(t_usar_io* datos,int conexion);



#endif