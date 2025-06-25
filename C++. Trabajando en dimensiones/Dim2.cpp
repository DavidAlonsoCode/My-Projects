#include <iostream> 
#include<array>
#include<string>
#include <iomanip>
using std::array;
using std::string;
using std::setw;
using std::left;
using std::right;
// using std::setfill; Sirve para cambiar los espacios restantes del ancho "guardado" del set width por otro caracter que no sea "espacio"

array<array<int,12>, 3> dim2();
void printMonth(int, string);

int main(){
    array<array<int,12>, 3> resultados{dim2()};
    array<string, 12> months{"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

    // Se ordena por vendedor -> mes para mostrar importe.
    std::cout << left << setw(2) << '|' << setw(9) << "Vendedor" << setw(2) << '|' << setw(4) << ' ' << setw(8) << "Mes" << setw(2) << '|' << setw(8) << "Importe" << "|\n";
    for(int v{}; v<3; v++) {
        for (int m{}; m<12; m++)
            std::cout << left << setw(2) << '|'  << setw(3) << ' ' << setw(6) << v << setw(2) << '|' << setw(12) << months.at(m) << setw(2) << '|' << " $" << setw(4) << right << resultados.at(v).at(m) << "  |\n";
    }

    std::cout << "\n_____________________________________\n\n";

    // Se ordena por Mes-> vendedor para mostrar el importe. Se calcula el medio para imprmir meses
    std::cout << left << setw(2) << '|' << setw(4) << ' ' << setw(8) << "Mes" << setw(2) << '|' << setw(9) << "Vendedor" << setw(2) << '|' << setw(8) << "Importe" << "|\n";
    for(int i{}; i<12; i++){
        for(int v{}; v<3; v++){
            std::cout << left << setw(2) << '|';
            printMonth(12, months.at(i));
            std::cout << left << setw(2) << '|'  << setw(3) << ' ' << setw(6) << v << setw(2) << '|' << " $" << setw(4) << right << resultados.at(v).at(i) << "  |\n";
        }
    }
}

// El formato de entrada es un txt con formate importe mes vendedor y guardamos por vendedor cada mes.
array<array<int,12>, 3> dim2() {
    array<array<int,12>, 3> total{};

    for (int ventas, mes, vendedor; std::cin>>ventas>>mes>>vendedor;)
        total.at(vendedor).at(mes-1) += ventas;
    
    return total;
}

// Funcion pora poder imprimir los meses centrados, recibe el width para el setw y el mes a centrar.
void printMonth(int width, string month) {
    int leftPadding, rightPadding, slength;
    slength = month.length();
    leftPadding = (width - slength) / 2;
    rightPadding = leftPadding * 2 + slength == width ? leftPadding : leftPadding + 1;

    std::cout << right << setw(slength + leftPadding) << month << setw(rightPadding) << ' ';
}

    // Comentado para ver como funciona el for range nomas.
    /*std::cout << "-------------------\n";

    for(int v{}; v<3; v++) {
        for (auto res : resultados.at(v))
            std::cout << res << '\t' << v << '\n';
    }
    */