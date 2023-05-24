#include "bbmalloc.h"
#include <time.h>
#include <stdlib.h>

#define NUM_DATA_BLOCKS 512
#define BUFFER_SIZE 1000000
#define VOLATILE_DATA_BLOCKS 100

void randomIdxVector(int* vec, int size, int randomMax){
    for (int i = 0; i < size; ++i) vec[i] = -1;
    for (int i = 0; i < size; ++i) {
        int foundValidIdx = 0;
        int r;
        while (!foundValidIdx) {
            r = rand()%randomMax;

            for (int j = 0; j < size; ++j) {
                if (vec[j] == r) {
                    r = -1;
                    break;
                }
            }
            if (r != -1) {
                vec[i] = r;
                foundValidIdx = 1;
            }
        }
    }
}

int main(int argc, char* argv[]) {

    int randomIdxs[NUM_DATA_BLOCKS];
    randomIdxVector(randomIdxs, NUM_DATA_BLOCKS, NUM_DATA_BLOCKS);

    int randomVol[VOLATILE_DATA_BLOCKS];
    int randomValues[VOLATILE_DATA_BLOCKS];
    randomIdxVector(randomVol, VOLATILE_DATA_BLOCKS, NUM_DATA_BLOCKS);
    randomIdxVector(randomValues, VOLATILE_DATA_BLOCKS, NUM_DATA_BLOCKS);

    int *dataPointers[NUM_DATA_BLOCKS];

    clock_t t;
    t = clock();

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i) {
        dataPointers[i] = bbmalloc(i*BUFFER_SIZE*sizeof(int));
        dataPointers[i][0] = i;
    }

    for (int i = 0; i < VOLATILE_DATA_BLOCKS; ++i) {
        bbfree(dataPointers[randomVol[i]]);
    }

    for (int i = 0; i < VOLATILE_DATA_BLOCKS; ++i) {
        dataPointers[randomVol[i]] = bbmalloc(randomValues[i]*BUFFER_SIZE*sizeof(int));
        dataPointers[randomVol[i]][0] = randomValues[i];
    }

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i) {
        bbfree(dataPointers[randomIdxs[i]]);
    }

    t = clock() - t;
    double timeTaken = 1000*((double)t)/CLOCKS_PER_SEC;
    printf("bbmalloc time taken: %lf ms\n", timeTaken);


    int *dataPointersMalloc[NUM_DATA_BLOCKS];

    t = clock();

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i) {
        dataPointersMalloc[i] = malloc(i*BUFFER_SIZE*sizeof(int));
        dataPointersMalloc[i][0] = i;
    }

    for (int i = 0; i < VOLATILE_DATA_BLOCKS; ++i) {
        free(dataPointersMalloc[randomVol[i]]);
    }

    for (int i = 0; i < VOLATILE_DATA_BLOCKS; ++i) {
        dataPointersMalloc[randomVol[i]] = malloc(randomValues[i]*BUFFER_SIZE*sizeof(int));
        dataPointersMalloc[randomVol[i]][0] = randomValues[i];
    }

    for (int i = 0; i < NUM_DATA_BLOCKS; ++i) {
        free(dataPointersMalloc[randomIdxs[i]]);
    }

    t = clock() - t;
    timeTaken = 1000*((double)t)/CLOCKS_PER_SEC;
    printf("libc malloc time taken: %lf ms\n", timeTaken);



    return 0;
}
