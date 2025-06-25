#ifndef FUNCIONES_AUXILIARES_INCLUIDA
#define FUNCIONES_AUXILIARES_INCLUIDA

// Conjunto de Operaciones (declaracion)
double Abs(double);
bool areNear(const double, const double, const double = 0.0001);

// Conjunto de Operaciones (implementacion)
double Abs(double x){
    return (x>0) ? x : -x;
}

bool areNear(const double a, const double b, const double delta){
    return Abs(a-b) < delta;
}

#endif
