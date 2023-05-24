#include "bbmalloc.h"

int main(int argc, char* argv[]) {
    int *a1[15];
    for (int i = 0; i != 15; i++) {
        a1[i] = bbmalloc(100000*sizeof(int));
        *(a1[i]) = i;
    }
    printHeap();
    return 0;
}
