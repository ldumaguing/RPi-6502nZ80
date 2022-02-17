#include <stdio.h>
// A normal function with an int parameter
// and void return type
void fun ( int a ) {
    printf ( "Value of a is %d\n", a );
}

typedef void FUNC();

FUNC NOP_ea, LDA_a9;
FUNC *OpCodes[255];

void NOP_ea() {
    printf ( "NOP ea\n" );
}

void LDA_a9() {
    printf ( "LDA a9\n" );
}

int main() {
// fun_ptr is a pointer to function fun()
    void ( *fun_ptr ) ( int ) = &fun;
    ( *fun_ptr ) ( 10 );

    OpCodes[0xea] = &NOP_ea;
    OpCodes[0xea]();


    //(X)();


    //OpCodes[0xa9] = &LDA_a9;
    //(*OpCodes[0xea])();
    //(*OpCodes[0xa9])();



    return 0;
}
