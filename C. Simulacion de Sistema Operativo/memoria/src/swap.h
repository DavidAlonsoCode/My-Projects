#ifndef SWAP_H
#define SWAP_H

#include "auxiliaresMemoria.h"

void iniciar_swap();
char* sacar_de_swap(uint32_t pid_a_sacar);
int obtener_frame_asignado(t_tabla_de_pagina* tabla_L1, int pagina_logica);
void liberar_bloque(int indice);
int encontrar_pagina_en_swap(uint32_t pid_a_sacar);
int bloques_swap_libres();
void pasar_a_swap(uint32_t proceso);
void agregar_a_lista_bloques(int cantidad_bloques);
int encontrar_asignar_bloque_libre(uint32_t pid);
void escribir_en_swap(uint32_t proceso);
char* leer_frame(int nro_frame);
bool esta_en_swap(uint32_t pid);
void liberar_bloques_swap_de_pid(uint32_t pid);

#endif