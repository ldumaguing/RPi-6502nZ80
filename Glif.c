#include <stdio.h>
#include <wchar.h>
#include <locale.h>

int main() {
    setlocale(LC_CTYPE, "");
    for (int i=5793; i<=5880; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    for (int i=8593; i<=8597; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    for (int i=8602; i<=8616; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    for (int i=8619; i<=8703; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    for (int i=8853; i<=8865; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");
    
    for (int i=8865; i<=10000; i++) {
        wprintf(L"%lc", (wchar_t)i);
    }
    wprintf(L"\n");

}
