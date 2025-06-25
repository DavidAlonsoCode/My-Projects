/* Busca el MCD entre 2 numeros
MCD:RxR-->N

*A/B = C + R
*B/R = C1 + R1
*R/R1 = C2 + R2
*...
*RN-3/RN-2 = CN-1 + RN-1
*RN-2/RN-1 = CN + RN=0 --> Si resto=0, el MCD es el resto anterior (RN-1)
*/
#include <cassert>

int Absoluto(int);
int MCD(int,int);
unsigned MCD2(int,int);

int main(){

    assert(MCD(720,224) == 16);  //POSITIVOS
    assert(MCD(-300,-168) == 12);//NEGATIVOS
    assert(MCD(162,-48) == 6);   //POSITIVOS Y NEGATIVOS
    
    assert(MCD(0,6) == 6);       //CASO LIMITE
    assert(MCD(-5,0) == 5);      //CASO LIMITE
    assert(MCD(0,0) == 0);       //En realidad el MCD entre 0 y 0, es cualquier numero, ya que todos lo dividen
    assert(MCD(100,100) == 100);

    assert(MCD(97,89) == 1);     //COPRIMOS POSITIVOS
    assert(MCD(-435,-73) == 1);  //COPRIMOS NEGATIVOS
    assert(MCD(-525,124) == 1);  //COPRIMOS NEGATIVOS Y POSITIVOS
    
}


/* 
Usamos int como retorno de la funcion pese a que siempre van a ser numeros naturales ya que la entrada son enteros
y el valor maximo de la imagen esta determinada por el maximo valor de la entrada por lo que utilizar unsigned para tener
un mayor rango de valores no es necesario y podemos usar int ahorrandonos problemas de casteo, etc
*/
int MCD(int a, int b){ return b == 0 ? Absoluto(a) : MCD(b, a%b); }

/* 
Primera implementacion donde se respeta el dominio e imagen (R -> N) (int -> unsigned) por lo que tuvimos que castear las
entradas int a unsgined para su correcta implementacion
*/
unsigned MCD2(int a, int b){ 
    
    unsigned A = static_cast<unsigned>(Absoluto(a));
    unsigned B = static_cast<unsigned>(Absoluto(b));
    
    return B==0? A : MCD(B,A%B);
    //Si b es cero, devuelve "a", que es el anteúltimo resto entre a y b
    //Si b no es cero, reemplaza a "a" por "b" y a "b" por el resto de 'a' y 'b'
}

// Función auxiliar para hacer el módulo ya que la salida siempre es positiva en el MCD
int Absoluto(int x){
    return x<0 ? -x : x;
}
