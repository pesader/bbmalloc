#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define MULTIPLIER 10
#define MIN_SIZE 1
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


typedef enum {
  AVAILABLE, TOO_SMALL, UNAVAILABLE,
} MemStatus;


typedef struct MemMetadata {
    size_t size;
    MemStatus status;
    struct MemMetadata* prev;
    struct MemMetadata* next;
    char memBegin[1];
} MemMetadata;


static MemMetadata* head = NULL;


void printMemBlocks(MemMetadata* head) {
    MemMetadata* current;
    for (current = head; current != NULL; current = current->next){
        printf("metadata at: %p size: %ld \n", current, current->size);
    }
}


MemMetadata* requestMoreMemory(MemMetadata* lastChunk, size_t size){
    void* brkPoint = sbrk(0);

    size_t requestedMemory = MULTIPLIER * (size + sizeof(MemMetadata));
    assert(sbrk(requestedMemory) != (void*) -1 && "Could not alloc more memory");

    MemMetadata* newChunk = brkPoint;
    newChunk->size = requestedMemory - sizeof(MemMetadata);
    newChunk->status = AVAILABLE;
    newChunk->prev = lastChunk;
    newChunk->next = NULL;

    // first call to bbmalloc
    if (lastChunk == NULL)
        lastChunk = newChunk;
    else
        lastChunk->next = newChunk;

    return lastChunk;
}


int isLastChunk(MemMetadata* chunk){
    return (chunk != NULL) && (chunk->next == NULL);
}


void splitChunk(MemMetadata* chunk, size_t size){
    assert(chunk != NULL && "Cannot split a NULL chunk");

    MemMetadata* newChunk = (MemMetadata*) chunk->memBegin + size;
    newChunk->size = chunk->size - (size + sizeof(MemMetadata));
    newChunk->status = chunk->size < MIN_SIZE ? TOO_SMALL : AVAILABLE;
    newChunk->prev = chunk;
    newChunk->next = NULL;

    chunk->size = size;
    chunk->next = newChunk;
}


MemMetadata* findChunk(MemMetadata* head, size_t size){
    MemMetadata* current;
    for (current = head; current != NULL; current = current->next){
        if (current->size >= size + sizeof(MemMetadata) && current-> status == AVAILABLE){
            splitChunk(current, size);
            current->status = UNAVAILABLE;
            break; // first fit
        }
        // if the last chunk is not large enough to store the size requested by
        // the user, request more memory. This will add a new chunk to the
        // linked list, which will be allocated in the next iteration
        if (isLastChunk(current)) requestMoreMemory(current, size);
    }
    return current;
}


void* bbmalloc(size_t size) {
    MemMetadata* chunk;

    // first call to bbmalloc
    if (head == NULL) {
        chunk = requestMoreMemory(head, size);
        splitChunk(chunk, size);
    }
    // next calls to bbmalloc
    else chunk = findChunk(head, size);

    return chunk;
}


int main(int argc, char* argv[]) {
    for (int i = 0; i != 1000; i++){
        int* a = bbmalloc(sizeof(int));
        *a = 10;
        printf("%p->%d\n", a, *a);
        printf("%p\n", sbrk(0));
    }
    return 0;
}
