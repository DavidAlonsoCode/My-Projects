/*
Fib (n): N -> N / { n               n < 2
					F(n-1) + F(n-2)	    e.o.c }
            
*/

#include<cassert>
// #include<iostream>

/* 
    El parametro de entrada a la función representa el término correspondiente a la sucesion de fibonacci.
    El valor de retorno de la función representa el número correspondiente a dicho término.
*/
unsigned Fib (unsigned);

int main() {

    assert(Fib(0u) == 0u);
    assert(Fib(1u) == 1u);
    assert(Fib(2u) == 1u);
    assert(Fib(3u) == 2u);
    assert(Fib(4u) == 3u);
    assert(Fib(5u) == 5u);
    assert(Fib(11u) == 89u);
    assert(Fib(14u) == 377u);
    assert(Fib(50u) == 3996334433u);
    assert(Fib(51u) == 3185141890u);

    // std::cout << Fib(48);

    //   512.559.680  (48)
    // 3.483.774.753  (49)
    // 3.996.334.433  (50) tardo como 1 minuto (48) + (49) suman correctamente el de (50)
    // 3.185.141.890  (51) Da el resultado de Fib(49) + Fib(50) - 2^32 (4294967296) cuando deberia dar 7.480.109.186 que es el Fibonacci de 51 
};

unsigned Fib (unsigned n) {
    return n < 2 ? n : Fib(n-1) + Fib(n-2);
}