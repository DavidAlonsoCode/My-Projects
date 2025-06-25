#include "histograma.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct dataset datos;
const unsigned Max = 14;

void inicializar_dataset(struct dataset *datos) { //recibe la direccion en la que se ubica el struct
    datos->cantDeCategorias = Max; // (*datos).cantDeCategorias = Maximo; //con *datos entro al contenido de la estructura datos
    for (unsigned i = 0; i < Max; i++) {
        datos->longitudes[i] = 0;
    }
}

void crear_archivo_txt(const char* nombreArchivo) {
    FILE *archivoTest = fopen(nombreArchivo, "w");
    if (archivoTest == NULL) { //si el archivo es inaccesible, o no se peude abrir, retorna NULL -->
        perror("Error al crear el archivo"); //imprime un mensaje error a traves de stderr (estandar de errores)
        exit(EXIT_FAILURE); //exit_failure retorna "1" (error) al sistema operativo, exit_success un "0" (correctamente)
    }
    const char *contenido = "La poesia es un genero literario considerado como una manifestacion de la belleza o del sentimiento estetico por medio de la palabra en verso o en prosa";
    fprintf(archivoTest, "%s", contenido); //fprintf envia la cadena de texto al file (el cual es representado mediante una variable que tiene su direccion)
    fclose(archivoTest);
}

int main(void) {
    inicializar_dataset(&datos);

    // Test para verificar si el dataset se inicializa correctamente
    for (unsigned i = 0; i < datos.cantDeCategorias; i++) {
        assert(datos.longitudes[i] == 0);
    }

    // Test con un texto dentro de un archivo
    const char *nombreArchivo = "test.txt";
    crear_archivo_txt(nombreArchivo);

    // Abrir el archivo en modo lectura y redirigir la entrada estandar (stdin) a ese archivo.
    FILE *archivo = freopen(nombreArchivo, "r", stdin);
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    unsigned* resultado = contador(&datos); //<-- Aca es donde contador va a trabajar con el archivo abierto en modo "read"

    assert(resultado[0] == 2); //cantidad de palabras de longitud 1
    assert(resultado[1] == 9); //cantidad de palabras de longitud 2
    assert(resultado[2] == 3); //cantidad de palabras de longitud 3
    assert(resultado[3] == 1); //cantidad de palabras de longitud 4
    assert(resultado[4] == 3); //...
    assert(resultado[5] == 2);
    assert(resultado[6] == 2);
    assert(resultado[7] == 1);
    assert(resultado[8] == 1);
    assert(resultado[9] == 0);
    assert(resultado[10] == 2);
    assert(resultado[11] == 0);
    assert(resultado[12] == 1); //cantidad de palabras de longitud 13
    assert(resultado[13] == 0); //cantidad de palabras de longitud +13

    printf("Tests pasados correctamente");
    fclose(archivo);
}   
