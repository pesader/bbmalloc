#ifndef BBMALLOC_BBMALLOC_H
#define BBMALLOC_BBMALLOC_H
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#define MULTIPLIER 10
#define MIN_SIZE 1

void* bbmalloc(size_t size);
void bbfree(void* ptr);
void printHeap();


#endif //BBMALLOC_BBMALLOC_H
