#include "Punto.h"
#include "FuncionesAuxiliares.h"

// Tipo de Dato Poligono Enlazado

// Conjunto de Valores
struct Node{ 
    Punto punto;
    Node* next{nullptr};               
};

struct Poligono{
    Node* primerNodo{nullptr};
    unsigned size{};
};

// Conjunto de Operaciones (declaracion)
unsigned cantDeLados(const Poligono&);
double getPerimetro(const Poligono&);
Poligono& push(Poligono&, const Punto);
Poligono& pop(Poligono&);
Poligono& pop(Poligono&, Punto&);
Poligono& add(Poligono&, const Punto, unsigned);
Poligono& removePorPunto(Poligono&, const Punto);
Poligono& removePorPosicion(Poligono&, unsigned);
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

    auto actual {p.primerNodo};
    for(; actual->next; actual = actual->next){
        perimetro += getDistancia(actual->punto, actual->next->punto);
    }
    perimetro +=  getDistancia(actual->punto, p.primerNodo->punto);
    return perimetro;
}

Poligono& push(Poligono& p, const Punto puntoToAdd){
    if(p.primerNodo) {
        Node* actual{p.primerNodo};
        for (; actual->next; actual = actual->next);
        actual->next = new Node;
        actual->next->punto = puntoToAdd;
    }
    else {
        p.primerNodo = new Node;
        p.primerNodo->punto = puntoToAdd;
    }
    p.size++;

    return p;
}

Poligono& pop(Poligono& p){
    if (p.primerNodo->next) {
        Node* actual = p.primerNodo;

        for (; actual->next->next; actual = actual->next);

        delete actual->next;
        actual->next = nullptr;
    } else{
        delete p.primerNodo;
        p.primerNodo = nullptr;
    }


    p.size--;

    return p;
}

Poligono& pop(Poligono& p, Punto& puntoRemoved){
    if (p.primerNodo->next){
        Node* actual{p.primerNodo};

        for (; actual->next->next; actual = actual->next);

        puntoRemoved = actual->next->punto;
        delete actual->next;
        actual->next = nullptr;
    } else {
        puntoRemoved = p.primerNodo->punto;
        delete p.primerNodo;
        p.primerNodo = nullptr;
    }
    p.size--;
    return p;
}

/*
    La posicion indica el punto del poligono, abstrayendose de si es o no un arreglo, 
    entonces posicion 0 no existe, va de 1 a 100
*/

Poligono& add(Poligono& p, const Punto punto, unsigned pos){
    Node* actual {p.primerNodo};

    // Pos es de 1 a 100 entonces empezamos el for en 1;
    for(unsigned i{1}; i < pos - 1; i++){
        actual = actual->next;
    }

    if (p.primerNodo) //si no está vacio            
        actual->next = new Node{punto, actual->next};
/*3->4->5*/

    else
        p.primerNodo = new Node{punto};

    p.size++;

    return p;
}

Punto top(const Poligono& p) {
    auto actual {p.primerNodo};

    for(;actual->next;actual = actual->next);

    return actual->punto;
}

Punto first(const Poligono& p){
    return p.primerNodo->punto;
}

Poligono& removePorPunto(Poligono& p, const Punto puntoToRemove){
    if (isEqual(p.primerNodo->punto, puntoToRemove)) { //p1={-3,3.5}{12,20}{-1,-30}{null}
        auto nodeToRemove{p.primerNodo};
        p.primerNodo = nodeToRemove->next;
        p.size--;
        delete nodeToRemove;
        return p;
    }

    // Si no se encuentra en el primer nodo, se recorre.
    for(auto actual {p.primerNodo}; actual; actual = actual ->next){
        if(isEqual(actual->next->punto, puntoToRemove)) {
            auto nodeToRemove = actual->next;
            actual->next = nodeToRemove->next;
            delete nodeToRemove;
            p.size--;
            break;
        }
    }
    return p;
}

Poligono& removePorPosicion(Poligono& p, unsigned pos){
    if (pos == 1){
        auto nodeToRemove {p.primerNodo};
        p.primerNodo = nodeToRemove->next;
        p.size--;
        delete nodeToRemove;
        return p;
    }
    auto actual{p.primerNodo};
    for(unsigned i = 1; i < pos - 1 && actual->next; i++) {
        actual = actual->next;
    }
    if (actual->next) {
        auto nodeToRemove = actual->next;
        actual->next = nodeToRemove->next;
        p.size--;
        delete nodeToRemove;
        return p;
    }
}

Punto get(const Poligono& p, unsigned pos){
    auto actual{p.primerNodo};

    for(unsigned i{1}; i < pos; i++){
        actual = actual->next;
    }

    return actual->punto;
}

Poligono& set(Poligono& p, Punto oldPunto, Punto newPunto){
    for(auto actual {p.primerNodo}; actual; actual = actual ->next){
        if(isEqual(actual->punto, oldPunto)) {
            actual->punto = newPunto;
            break;
        }
    }
    return p;
}

unsigned getIndex(Poligono& p, const Punto punto){
    unsigned i{1};

    for(auto actual {p.primerNodo}; actual; actual = actual ->next, i++){
        if(isEqual(actual->punto, punto)) {
            return i;
        }
    }

    return 0;  // como 0 nunca es una posicion del poligono, ya que es de 1 a 100 puntos
}

