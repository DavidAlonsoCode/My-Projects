#ifndef PUNTO_INCLUIDO
#define PUNTO_INCLUIDO

// Tipo de Dato Punto
#include<math.h>                    // Importa (depende de)
#include"FuncionesAuxiliares.h"

// Conjunto de Valores
struct Punto{
    double x,y;
};

// Conjunto de Operaciones (declaracion)
double getDistancia(const Punto, const Punto);
bool isEqual(const Punto, const Punto);

// Conjunto de Operaciones (implementacion)
double getDistancia(const Punto a,const Punto b){
    return hypot((b.x - a.x), (b.y - a.y));
}
bool isEqual(const Punto a, const Punto b) {
    return areNear(a.x, b.x) and areNear(a.y,b.y);
}

#endif