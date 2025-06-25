#include <iostream>
#include <cassert>
#include <array>
#include <cmath>
using std::cin;
using std::cout;

struct Punto{ double x, y; };       // Puntos plano (x,y);   

struct Node{
    Punto punto;
    Node* next{nullptr};
};

struct Poligono{
    Node* primerNodo{nullptr};
    unsigned size{};
};

double getDistancia(const Punto, const Punto);                //
unsigned cantDeLados(const Poligono&);                        //
double getPerimetro(const Poligono&);                         //
bool areNear(const double, const double, const double=0.0001);//
bool isEqual(const Punto, const Punto);                       //

Poligono& push(Poligono&, const Punto);                     //
Poligono& pop(Poligono&);//
Poligono& pop(Poligono&, Punto&);//
Poligono& add(Poligono&, const Punto, unsigned); //
Poligono& removePorPunto(Poligono&, const Punto);
Poligono& removePorPosicion(Poligono&, unsigned);
Poligono& set(Poligono&, const Punto, const Punto);
Punto top(const Poligono&);      //
Punto first(const Poligono&);
Punto get(const Poligono&, unsigned);
unsigned getIndex(Poligono&, const Punto);

int main(){
    Poligono p1{new Node{{0, 0}, new Node{{3, 0}, new Node{0, 1.5}}}, 3};

    // Probamos isEqual y areNear (areNear esta dentro de isEqual)
    assert(!isEqual({0, 0}, {3, 0}));
    assert(isEqual(p1.primerNodo->next->punto, {3, 0}));

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

    Poligono p2;                                                        //p2 vacio   
    push(push(push(push(p2, {0, 0}), {3,0}), {0, 1.5}), {-3.0, 1.5});   //p2={0,0}{3,0}{0,1.5}{-3,1.5}
    assert(cantDeLados(p2) == 4);                                       //p2={0,0}{3,0}{0,1.5}{-3,1.5}

    for(auto uno{p1.primerNodo}, dos{p2.primerNodo}; uno and dos ; uno=uno->next,dos=dos->next)
        assert(isEqual(uno->punto, dos->punto));

    // Probamos pop 
    pop(p1);                               //p1={0,0}{3,0}{0,1.5}
    assert(cantDeLados(p1) == 3);          //p1={0,0}{3,0}{0,1.5}
    assert(isEqual(top(p1), {0, 1.5}));    //p1={0,0}{3,0}{0,1.5}

    //corroboramos que funciona el pop, aunque la cant de lados es invalida para 1 punto (no cumple la precondicion)
    pop(pop(p1));                          //p1={0,0}
    assert(cantDeLados(p1) == 1);          //p1={0,0}
    assert(isEqual(top(p1), {0,0}));       //p1={0,0}

    Punto removed;
    pop(p1, removed);                      //p1 vacio
    assert(cantDeLados(p1) == 0);          //p1 vacio
    assert(isEqual(removed, {0, 0}));      //p1 vacio

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

    // Probamos removePorPunto
    removePorPunto(p1, {12, 20});                   //p1={-3,3.5}{-1,-30}
    assert(cantDeLados(p1) == 2);           //p1={-3,3.5}{-1,-30}
    assert(isEqual(top(p1), {-1,-30}));    //p1={-3,3.5}{-1,-30}

    removePorPunto(removePorPunto(p2, {3, 0}), {0, 1.5});   //p2={0,0}{-3,3.5}{-3,1.5}
    assert(cantDeLados(p2) == 3);           //p2={0,0}{-3,3.5}{-3,1.5}
    assert(isEqual(top(p2), {-3.0, 1.5}));  //p2={0,0}{-3,3.5}{-3,1.5}

    // Probamos set
    set(p1, {-3, 3.5}, {2, 2});                    //p1={2,2}{-1,-30}
    assert(isEqual(p1.primerNodo->punto, {2, 2})); //p1={2,2}{-1,-30}

    set(set(p1, {2, 2}, {0, 0}), { -1, -30}, {20, 12});      //p1={0,0}{20,12}
    assert(isEqual(p1.primerNodo->punto, {0, 0}));           //p1={0,0}{20,12}
    assert(isEqual(p1.primerNodo->next->punto, {20, 12}));   //p1={0,0}{20,12}

    // Probamos get
    assert(isEqual(get(p2, 2), {-3, 3.5})); //p2={0,0}{-3,3.5}{-3,1.5}

    // Probamos getIndex
    assert(getIndex(p2,  {-3, 3.5}) == 2);  //p2={0,0}{-3,3.5}{-3,1.5}

    // Probamos removePorPosicion
    removePorPosicion(p2,2);  //p2={0,0}{-3,1.5}
    assert(getIndex(p2,  {-3, 3.5}) != 2);
    assert(isEqual(get(p2,2),{-3,1.5}));

    //Probamos first
    assert(isEqual(first(p2),{0,0}));
    removePorPosicion(p2,1);  //p2={-3,1.5}
    assert(isEqual(first(p2),{-3,1.5}));
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

    auto actual {p.primerNodo};
    for(; actual->next; actual = actual->next){
        perimetro += getDistancia(actual->punto, actual->next->punto);
    }
    perimetro +=  getDistancia(actual->punto, p.primerNodo->punto);
    return perimetro;
}

double getDistancia(const Punto a,const Punto b){
    return hypot((b.x - a.x), (b.y - a.y));
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
