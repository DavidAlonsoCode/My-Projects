#include <iostream>
#include <cassert>
#include <array>
#include <cmath>
using std::array;
using std::cin;
using std::cout;

struct Punto{ double x, y; };       // Puntos plano (x,y);   

struct Poligono{
    array<Punto, 100> puntos;       // Poligno esta formado por un conjunto de puntos
    unsigned size{};                  // Para determianr cuantos puntos fueron agregados en el array
};

double getDistancia(const Punto, const Punto);                //
unsigned cantDeLados(const Poligono&);                        //
double getPerimetro(const Poligono&);                         //
bool areNear(const double, const double, const double=0.0001);//
bool isEqual(const Punto, const Punto);                       //

Poligono& push(Poligono&, const Punto);                       //
Poligono& pop(Poligono&);                                     //
Poligono& pop(Poligono&, Punto&);                             //
Poligono& add(Poligono&, const Punto, unsigned);              //
Poligono& remove(Poligono&, const Punto);                     //
Poligono& set(Poligono&, const Punto, const Punto);           //
Punto top(const Poligono&);                                   //
Punto get(const Poligono&, unsigned);
unsigned getIndex(Poligono&, const Punto);                

int main(){
    Poligono p1{ {{ {0,0}, {3,0}, {0, 1.5}}}, 3}; //p1={0,0}{3,0}{0,1.5}

    // Probamos isEqual y areNear (areNear esta dentro de isEqual)
    assert(!isEqual({0, 0}, {3, 0}));
    assert(isEqual(p1.puntos.at(1), {3, 0}));     //p1={0,0}{3,0}{0,1.5}

    // Probamos getPerimetro
    assert(areNear(getPerimetro(p1), 7.8541));    //p1={0,0}{3,0}{0,1.5}
    assert(areNear(getPerimetro(p1), 7.8542));    //p1={0,0}{3,0}{0,1.5}
    assert(!areNear(getPerimetro(p1), 7.8543));   //p1={0,0}{3,0}{0,1.5}
    assert(!areNear(getPerimetro(p1), 7.8539));   //p1={0,0}{3,0}{0,1.5}
    // Probamos cantDeLados
    assert(cantDeLados(p1) == 3);                 //p1={0,0}{3,0}{0,1.5}
    assert(cantDeLados(p1) != 2);                 //p1={0,0}{3,0}{0,1.5}

    // Probamos top
    assert(isEqual(top(p1), {0, 1.5}));           //p1={0,0}{3,0}{0,1.5}
    assert(!isEqual(top(p1), {0, 1.4}));          //p1={0,0}{3,0}{0,1.5}

    // Probamos push
    push(p1, {-3.0, 1.5});                        //p1={0,0}{3,0}{0,1.5}{-3,1.5}
    assert(cantDeLados(p1) == 4);                 //p1={0,0}{3,0}{0,1.5}{-3,1.5}
    assert(isEqual(top(p1), {-3.0, 1.5}));        //p1={0,0}{3,0}{0,1.5}{-3,1.5}
    
    Poligono p2{};                                                      //p2 vacio
    push(push(push(push(p2, {0, 0}), {3,0}), {0, 1.5}), {-3.0, 1.5});   //p2={0,0}{3,0}{0,1.5}{-3,1.5}
    assert(cantDeLados(p2) == 4);                                       //p2={0,0}{3,0}{0,1.5}{-3,1.5}
    for(unsigned i{}; i < p1.size; i++)  //p1={0,0}{3,0}{0,1.5}{-3,1.5} //p2={0,0}{3,0}{0,1.5}{-3,1.5}
        assert(isEqual(p1.puntos.at(i), p2.puntos.at(i)));
    
    // Probamos pop
    pop(p1);                            //p1={0,0}{3,0}{0,1.5}
    assert(cantDeLados(p1) == 3);       //p1={0,0}{3,0}{0,1.5}
    assert(isEqual(top(p1), {0, 1.5})); //p1={0,0}{3,0}{0,1.5}

    // Corroboramos que funciona el pop, aunque la cant de lados es invalida para 1 punto
    pop(pop(p1));                       //p1={0,0}
    assert(cantDeLados(p1) == 1);       //p1={0,0}
    assert(isEqual(top(p1), {0,0}));    //p1={0,0}

    Punto removed;
    pop(p1, removed);                 //p1 vacio
    assert(cantDeLados(p1) == 0);     //p1 vacio
    assert(isEqual(removed, {0, 0})); //p1 vacio

    // Probamos add
    add(p2, {-3, 3.5}, 4);                 //p2={0,0}{3,0}{0,1.5}{-3,3.5}{-3,1.5}
    assert(cantDeLados(p2) == 5);          //p2={0,0}{3,0}{0,1.5}{-3,3.5}{-3,1.5}
    assert(isEqual(top(p2), {-3.0, 1.5})); //p2={0,0}{3,0}{0,1.5}{-3,3.5}{-3,1.5}

    add(p1, {-3, 3.5}, 1);                 //p1={-3,3.5}
    assert(cantDeLados(p1) == 1);          //p1={-3,3.5}
    assert(isEqual(top(p1), {-3, 3.5}));   //p1={-3,3.5}

    add(add(p1, {12, 20}, 2), {-1, -30}, 3); //p1={-3,3.5}{12,20}{-1,-30}
    assert(cantDeLados(p1) == 3);            //p1={-3,3.5}{12,20}{-1,-30}
    assert(isEqual(top(p1), {-1, -30}));     //p1={-3,3.5}{12,20}{-1,-30}
    
    // Probamos remove
    remove(p1, {12, 20});                   //p1={-3,3.5}{-1,-30}
    assert(cantDeLados(p1) == 2);           //p1={-3,3.5}{-1,-30}
    assert(isEqual(top(p1), {-1, -30}));    //p1={-3,3.5}{-1,-30}

    remove(remove(p2, {3, 0}), {0, 1.5});   //p2={0,0}{-3,3.5}{-3,1.5}
    assert(cantDeLados(p2) == 3);           //p2={0,0}{-3,3.5}{-3,1.5}
    assert(isEqual(top(p2), {-3.0, 1.5}));  //p2={0,0}{-3,3.5}{-3,1.5}

    // Probamos set
    set(p1, {-3, 3.5}, {2, 2});               //p1={2,2}{-1,-30}
    assert(isEqual(p1.puntos.at(0), {2, 2})); //p1={2,2}{-1,-30}

    set(set(p1, {2, 2}, {0, 0}), { -1, -30}, {20, 12}); //p1={0,0}{20,12}
    assert(isEqual(p1.puntos.at(0), {0, 0}));           //p1={0,0}{20,12}
    assert(isEqual(p1.puntos.at(1), {20, 12}));         //p1={0,0}{20,12}

    // Probamos get
    assert(isEqual(get(p2, 2), {-3, 3.5})); //p2={0,0}{-3,3.5}{-3,1.5}
    
    // Probamos getIndex
    assert(getIndex(p2,  {-3, 3.5}) == 2);  //p2={0,0}{-3,3.5}{-3,1.5}

    // solapados
}


/*------------------------------ Implementacion funciones ------------------------------ */

bool isEqual(const Punto a, const Punto b) { 
    return areNear(a.x, b.x) and areNear(a.y,b.y); 
}

bool areNear(const double a, const double b, const double delta){ 
    return (a - delta) <= b and (a + delta) >= b;
}

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

/* 
    Para calcular distancia hago PuntoDestino(b).PuntoOrigen(a) para generar un vector,
    luego calculo su modulo. Entonces para calcular el vector hago
    b-a osea (b.x,b.y)-(a.x,a.y), y me queda el vector (b.x-a.x,b.y-a.y)
    Y despues calculo el modulo de ese vector
*/
double getDistancia(const Punto a,const Punto b){
    return hypot((b.x - a.x), (b.y - a.y));
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

/*  
    La posicion indica el punto del poligono, abstrayendose de si es o no un arreglo, 
    entonces posicion 0 no existe, va de 1 a 100
*/

Poligono& add(Poligono& p, const Punto punto, unsigned pos){ 
    pos--;                                  // Como trabajamos con arreglo, hacemos coincidir la posicion con el arreglo.
    for(unsigned i{p.size}; i > pos; i--)
        p.puntos.at(i) = p.puntos.at(i - 1);
        
    
    if (pos <= p.size) { //en caso que pos==p.size, seria como un push
        p.puntos.at(pos) = punto;
        p.size++;
    }

    return p;
}

Punto top(const Poligono& p) {
    return p.puntos.at(p.size - 1);
}

Poligono& remove(Poligono& p, const Punto puntoToRemove){
    bool flag = false;
    
    for(unsigned i{}; i < p.size; i++){
        if (flag) p.puntos.at(i - 1) = p.puntos.at(i);                  // Desde el punto a borrar al final hay que mover todo una posicion para abajo (ya que saco un elemento)
        if(isEqual(p.puntos.at(i), puntoToRemove) && !flag) 
            flag = true;                                                // encuentro el punto a borrar
    }

    if(flag) p.size--;                                                  // Si encontre un elemento, al finalzar todo borro la posicion extra que me queda, no lo hago antes por el for

    return p;
}

Punto get(const Poligono& p, unsigned pos){
    return p.puntos.at(pos - 1); //pos-1 porque el usuario trabaja con "naturales" y nosotros con "naturales con el cero"
}  

Poligono& set(Poligono& p, const Punto oldPunto, const Punto newPunto){
    for(unsigned i{}; i < p.size; i++){
        if (isEqual(p.puntos.at(i), oldPunto)) 
            p.puntos.at(i) = newPunto; 
    }

    return p;
}  

// Posicion del poligono, no el arreglo.
unsigned getIndex(Poligono& p, const Punto punto){
    for(unsigned i{}; i < p.size; i++){
        if (isEqual(p.puntos.at(i), punto)) 
            return i + 1;
    }

    return 0; // como 0 nunca es una posicion del poligono, ya que es de 1 a 100 puntos
}

