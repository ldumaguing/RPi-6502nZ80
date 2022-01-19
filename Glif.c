#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int main() {
    setlocale(LC_CTYPE, "");
    for (int i=5793; i<=5880; i++){
    wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    
}

