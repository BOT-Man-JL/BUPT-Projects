#include "server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int main (int argc, TCHAR* argv[], TCHAR* envp[]) {
    startUp ();
    return 0;
}

void clear (char*a, int size) {
    int i = 0;
    while (i < size) {
        a[i] = '\0';
        i++;
    }
}
