#include <iostream>
#include <cassert>
#include "Punto.h"
#include "FuncionesAuxiliares.h"
#include "PoligonoEnlazado.h"
using std::cin;
using std::cout;

// Prueba de Poligono Enlazado

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
