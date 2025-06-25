#include "Punto.h"
#include "FuncionesAuxiliares.h"
#include <array>
using std::array;

// Tipo de Dato Poligono Contiguo

// Conjunto de Valores
struct Poligono{
    array<Punto, 100> puntos;
    unsigned size{};
};

// Conjunto de Operaciones (declaracion)
unsigned cantDeLados(const Poligono&);
double getPerimetro(const Poligono&);
Poligono& push(Poligono&, const Punto);
Poligono& pop(Poligono&);
Poligono& pop(Poligono&, Punto&);
Poligono& add(Poligono&, const Punto, unsigned);
Poligono& remove(Poligono&, const Punto);
Poligono& set(Poligono&, const Punto, const Punto);
Punto top(const Poligono&);
Punto first(const Poligono&);
Punto get(const Poligono&, unsigned);
unsigned getIndex(Poligono&, const Punto);                

// Conjunto de Operaciones (implementacion)
unsigned cantDeLados(const Poligono& p){
    return p.size;
}

double getPerimetro(const Poligono& p){
    double perimetro {};

    for(unsigned i{0}; i < p.size - 1; i++)  
        perimetro += getDistancia(p.puntos.at(i), p.puntos.at(i + 1));
    
    perimetro += getDistancia(p.puntos.at((p.size) - 1), p.puntos.at(0)); 
    
    return perimetro;
}

Poligono& push(Poligono& p, const Punto nuevoPunto){
    if(p.size < p.puntos.size()) {
        p.puntos.at(p.size) = nuevoPunto;
        p.size++;
    }

    return p;
}

Poligono& pop(Poligono& p){
    if(p.size > 0){
        p.size--;
    }

    return p;
}

Poligono& pop(Poligono& p, Punto& puntoRemoved){
    if(p.size > 0){
        p.size--;
        puntoRemoved = p.puntos.at(p.size);
    }

    return p;
}

Poligono& add(Poligono& p, const Punto punto, unsigned pos){ 
    pos--;
    for(unsigned i{p.size}; i > pos; i--)
        p.puntos.at(i) = p.puntos.at(i - 1);
        
    
    if (pos <= p.size) {
        p.puntos.at(pos) = punto;
        p.size++;
    }

    return p;
}

Punto top(const Poligono& p) {
    return p.puntos.at(p.size - 1);
}

Punto first(const Poligono& p){
    return p.puntos.at(0);
}

Poligono& remove(Poligono& p, const Punto puntoToRemove){
    bool flag = false;
    
    for(unsigned i{}; i < p.size; i++){
        if (flag) p.puntos.at(i - 1) = p.puntos.at(i);
        if(isEqual(p.puntos.at(i), puntoToRemove) && !flag) 
            flag = true;
    }

    if(flag) p.size--;

    return p;
}

Punto get(const Poligono& p, unsigned pos){
    return p.puntos.at(pos - 1);
}  

Poligono& set(Poligono& p, const Punto oldPunto, const Punto newPunto){
    for(unsigned i{}; i < p.size; i++){
        if (isEqual(p.puntos.at(i), oldPunto)) 
            p.puntos.at(i) = newPunto; 
    }

    return p;
}  

unsigned getIndex(Poligono& p, const Punto punto){
    for(unsigned i{}; i < p.size; i++){
        if (isEqual(p.puntos.at(i), punto)) 
            return i + 1;
    }

    return 0;
}

