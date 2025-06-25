#include <iostream>

int dim0();
//Declaro la función que me dara el valor de la suma de los importes del txt

int main (){
    std::cout<<dim0();
    //imprimo el resultado de la funcion
}

int dim0(){
    int sum{};                         //declaro un sumador inicializado en 0 donde se guardara el valor del total de los importes
    for (int imp; std::cin>>imp;)      //declaro una varible donde va a estar el valor que ingrese por linea del txt hasta que no haya valores enteros
        sum+=imp;                      //voy sumando los importes y guardadandolos en el sumador
    return sum;                        //devuelvo el total de la suma
}
