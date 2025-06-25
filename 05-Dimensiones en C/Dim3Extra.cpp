#include<iostream>
#include<array>
#include<string>
using std::array;
using std::string;

enum Regiones {Norte, Sur, Este, Oeste};

struct MesYVentas{
    int mes;
    int ventas;
};

array<array<array<int,12>,3>,4>dim3();
string getMonth(int);
string getEmployeer(int);
string getRegion(Regiones);
void mostrarVentasPorVendedor(array<array<array<int,12>,3>,4> resultados, int empleado);
void mostrarTotales(array<array<array<int,12>,3>,4> resultados);
MesYVentas getMaxMonth(array<array<array<int,12>,3>,4>resultados);
MesYVentas getMinMonth(array<array<array<int,12>,3>,4>resultados);
double getPromYear(array<array<array<int,12>,3>,4>resultados);


int main(){
    // Array principal de dimension 3
    array<array<array<int,12>,3>,4>resultados{dim3()};

    // Muestra las ventas de cada vendedor, mes, region e importe
    mostrarVentasPorVendedor(resultados,0);

    // Muestra la ventas totales de cada vendedor
    mostrarTotales(resultados);

    // Muestra el mes que mas se vendio y cuanto
    MesYVentas maxMes = getMaxMonth(resultados);
    std::cout << "\n\n*** Mes que mas se vendio: " << getMonth(maxMes.mes)
              << ". Con un total de: $" << maxMes.ventas << '\n';

    // Muestra el mes que menos se vendio y cuanto
    MesYVentas minMes = getMinMonth(resultados);
    std::cout << "\n*** Mes que menos se vendio: " << getMonth(minMes.mes)
              << ". Con un total de: $" << minMes.ventas << '\n';

    // Muestra el promedio de ventas anuales
    std::cout << "\n*** Promedio de ventas anuales: $" << getPromYear(resultados);

}

array<array<array<int,12>,3>,4>dim3(){
    array<array<array<int,12>,3>,4>Dim3{};
    for (int imp,mes,vend,reg;std::cin>>imp>>mes>>vend>>reg;)
        Dim3.at(reg).at(vend).at(mes-1) += imp;
    return Dim3;
}

string getMonth(int mes) {
    switch(mes){
        case 1: return " Enero\t";
        case 2: return " Febrero";
        case 3: return " Marzo\t";
        case 4: return " Abril\t";
        case 5: return " Mayo\t";
        case 6: return " Junio\t";
        case 7: return " Julio\t";
        case 8: return " Agosto\t";
        case 9: return "Septiembre";
        case 10: return " Octubre";
        case 11: return "Noviembre";
        case 12: return "Diciembre";
        default: return "---";
    }
}

string getEmployeer(int nombre){
    switch (nombre) {
        case 0: return "Juan";
        case 1: return "Cami";
        case 2: return "Pepe";
        default: return "---";
    }
}

string getRegion(Regiones region) {
    switch(region) {
        case Norte: return "Norte";
        case Sur: return "Sur ";
        case Este: return "Este";
        case Oeste: return "Oeste";
        default: return "---";
    }
}

void mostrarVentasPorVendedor(array<array<array<int,12>,3>,4> resultados, int empleado){


    std::cout << "\t\t\t\t\t*** Ventas por Vendedor *** \n\n";
    std::cout << "|\t  Vendedor\t\t|\t\t  Mes\t\t\t|   Region\t|\t   Importe\t\t|\n";


    for(int v{};v<3;v++){
        for (int m{};m<12;m++)
            for(Regiones r : {Norte, Sur, Este, Oeste})
                std::cout << "|\t\t" << getEmployeer(v) << "\t\t|"
                          << "\t\t" << getMonth(m+1) << "\t\t|"
                          << "\t" << getRegion(r) << "\t|\t"
                          << "\t$" << resultados.at(r).at(v).at(m) << "\t\t\t|" << '\n';


        std::cout << "-----------------------------------------------------------------------------\n";}
}

void mostrarTotales(array<array<array<int,12>,3>,4> resultados) {
    std::cout << "\n\n\t\t\t\t *** Total de Ventas *** \n\n";
    std::cout << "|\t  Empleado\t\t|\t\tTotal\t\t\t|\n";


    for(int v{}; v<3; v++) {
        int totalEmpleado{};
        for(int m{}; m<12; m++) {
            for(Regiones r : {Norte, Sur, Este, Oeste}) {
                totalEmpleado += resultados.at(r).at(v).at(m);
            }
        }
        std::cout << "|\t\t" << getEmployeer(v) << "\t\t|"
                  << "\t\t$" << totalEmpleado << "\t\t\t|" << '\n';
        std::cout << "---------------------------------------------\n";
    }
}

MesYVentas getMaxMonth(array<array<array<int,12>,3>,4>resultados){
    int maxImp{};
    int maxMes{};
    for(int m{};m<12;m++){
        int totalImp{};
        for(int v{};v<3;v++){
            for(Regiones r: {Norte,Sur,Este,Oeste}){
                totalImp += resultados.at(r).at(v).at(m);
            }
        }
        if(totalImp > maxImp){
            maxImp = totalImp;
            maxMes = m+1;
        }
    }
    return {maxMes,maxImp};
}

MesYVentas getMinMonth(array<array<array<int,12>,3>,4>resultados){
    int minImp{9999999};
    int minMes{};
    for(int m{};m<12;m++){
        int totalImp{};
        for(int v{};v<3;v++){
            for(Regiones r: {Norte,Sur,Este,Oeste}){
                totalImp += resultados.at(r).at(v).at(m);
            }
        }
        if(totalImp < minImp){
            minImp = totalImp;
            minMes = m+1;
        }
    }
    return {minMes,minImp};
}

double getPromYear(array<array<array<int,12>,3>,4>resultados){
    double totalImp{};
    for (int m{}; m < 12; m++)
        for (int v{}; v < 3; v++)
            for (Regiones r : {Norte, Sur, Este, Oeste}) {
                totalImp += resultados.at(r).at(v).at(m);
            }
    return totalImp / 3;
}