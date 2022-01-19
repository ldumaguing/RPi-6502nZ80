#include <stdio.h>
#include <stdlib.h>

// **************************************************************************************
int main ( void ) {
    unsigned X = 234;  // EA
    unsigned m = 1;
    int ByteData[8];

    for ( int i=0; i<8; i++ ) {
        ByteData[i] = ( X & m ) ? 1 : 0;
        m = m << 1;
    }

    for ( int i=0; i<8; i++ ) {
        printf ( "%d\n", ByteData[i] );
    }

    return 0;
}

