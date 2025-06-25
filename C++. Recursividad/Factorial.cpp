/*
Fact(n): N -> N / { 1		        n < 2
					n*Fact(n - 1)	e.o.c }
*/
#include<cassert>
// #include<iostream>


unsigned Fact(unsigned);

int main(){
	
	assert(Fact(0)  == 1);
	assert(Fact(1)  == 1);
	assert(Fact(2)  == 2);
	assert(Fact(5)  == 120);
	assert(Fact(10) == 3628800);

	// std::cout << Fact(13); 
	// 1.932.053.504 es el resultado que se imprime en consola, cuando deberia dar 6.227.020.800 que es el factorial de 13.
}

unsigned Fact(unsigned n){
	return n < 2 ? 1 : n*Fact(n-1);
}
