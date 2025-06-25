/* Div : N -> N

Div(dividend, divisor) = 
            0                                   dividend < divisor,
    1+ Div(dividend- divisor, divisor)          eoc

*/

#include <iostream>
#include <cassert>

unsigned Div(unsigned dividend, unsigned divisor);


int main() {
    assert( Div(16, 2)==8);
    assert( Div(20, 1)==20);
    assert( Div(0, 3)==0);
    assert( Div(1, 2)==0);
    assert( Div(27, 2)==13);
    assert( Div(13, 13)==1);
    assert( Div(10, 3)==3);
    assert( Div(100, 8)==12);

}


unsigned Div(unsigned dividend, unsigned divisor){
    return (dividend < divisor) ? 0: 1+ Div(dividend- divisor, divisor);
}

/* otra versión de Div fue 
bool Div(unsigned dividend, unsigned divisor) {
    return (divisor == 0 || dividend < divisor) ? false : (dividend == divisor ? true : Div(dividend - divisor, divisor));
}
*/
