#include <iostream>
#include <array>
using std::array;

array<int,12> dim1();

int main (){
    array<int,12> resultados{dim1()}; 
    for(int i{}; i<12; i++)
        std::cout << resultados.at(i) << '\n';
    }

array<int,12> dim1(){
    array<int,12> total{};
    for (int importe, mes; std::cin>>importe>>mes;)
        total.at(mes-1) += importe;
    return total;
}