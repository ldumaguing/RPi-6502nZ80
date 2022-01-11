#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int main() {
    setlocale ( LC_CTYPE, "" );
    wchar_t star = 0x1f600;
    wprintf ( L"%lc\n", star );
    wprintf ( L"%lc\n", 4096 );
    wprintf ( L"%lc\n", 0x41 );
}
