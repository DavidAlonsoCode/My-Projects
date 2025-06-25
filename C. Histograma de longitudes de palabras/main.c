#include "histograma.h"
#include "Graficador.h"
#include <stdio.h>

struct dataset datos;
const unsigned Max = 14;

void inicializar_dataset(struct dataset *datos) { //recibe la direccion en la que se ubica el struct
    datos->cantDeCategorias = Max; // (*datos).cantDeCategorias = Maximo; //con *datos entro al contenido de la estructura datos
    for (unsigned i = 0; i < Max; i++) {
        datos->longitudes[i] = 0;
    }
}

int main(void) {
    inicializar_dataset(&datos);
    dibujar_histograma(contador(&datos)); // Llamar a la función del graficador
}
