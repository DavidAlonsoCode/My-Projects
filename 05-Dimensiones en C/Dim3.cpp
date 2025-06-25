#include<iostream>
#include<array>
#include<string>
using std::array;
using std::string;

array<array<array<int,12>,3>,4>dim3();
string getMonth(int);
void mostrarVentasPorMes(array<array<array<int,12>,3>,4> resultados);
void mostrarVentasPorVendedor(array<array<array<int,12>,3>,4> resultados);

int main(){
    array<array<array<int,12>,3>,4>resultados{dim3()};

    // Ordena | Vendedor | Mes | Region | Importe
    mostrarVentasPorMes(resultados);

    // Ordena | Mes | Vendedor | Region | Importe
    mostrarVentasPorVendedor(resultados);

    return 0;
}

array<array<array<int,12>,3>,4>dim3(){
    array<array<array<int,12>,3>,4>Dim3{};
    for (int imp,mes,vend,reg;std::cin>>imp>>mes>>vend>>reg;)
        Dim3.at(reg).at(vend).at(mes-1) += imp;
    return Dim3;
}

string getMonth(int mes) {
    switch(mes){
        case 1: return "  Enero\t";
        case 2: return " Febrero";
        case 3: return "  Marzo\t";
        case 4: return "  Abril\t";
        case 5: return "  Mayo\t";
        case 6: return "  Junio\t";
        case 7: return "  Julio\t";
        case 8: return " Agosto\t";
        case 9: return "Septiembre";
        case 10: return " Octubre";
        case 11: return "Noviembre";
        case 12: return "Diciembre";
        default: return "---";
    }
}

void mostrarVentasPorMes(array<array<array<int,12>,3>,4> resultados){
    std::cout << "\n\n\t\t\t\t *** Ventas ordenadas por Mes *** \n\n";
    std::cout << "|\t   Mes\t\t|\t Vendedor\t|\t  Region\t|\t  Importe\t\t|\n";

    for(int m{};m<12;m++){
        for (int v{};v<3;v++)
            for(int r{};r<4;r++)
                std::cout << "|\t"   << getMonth(m+1) << "\t|"
                          << "\t\t"  << v << "\t\t|"
                          << "\t\t"  << r << "\t\t|"
                          << "\t\t$" << resultados.at(r).at(v).at(m) << "\t\t\t|" << '\n';

        std::cout << "---------------------------------------------------------------------\n";}
}

void mostrarVentasPorVendedor(array<array<array<int,12>,3>,4> resultados){
    std::cout << "\t\t\t\t\t*** Ventas ordenadas por Vendedor *** \n\n";
    std::cout << "|\t Vendedor\t|\t\t   Mes\t\t\t|\t  Region\t|\t  Importe\t\t|\n";

    for(int v{};v<3;v++){
        for (int m{};m<12;m++)
            for(int r{};r<4;r++)
                std::cout << "|\t\t"   << v << "\t\t|"
                          << "\t\t"  << getMonth(m+1) << "\t\t|"
                          << "\t\t"  << r << "\t\t|"
                          << "\t\t$" << resultados.at(r).at(v).at(m) << "\t\t\t|" << '\n';
        std::cout << "-----------------------------------------------------------------------------\n";}
}
