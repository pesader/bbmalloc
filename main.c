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
    unsigned int size;
    MemStatus status;
    struct MemMetadata* prev;
    struct MemMetadata* next;
} MemMetadata;


static MemMetadata* globalHead = NULL;


void printMemBlocks(MemMetadata* head) {
    int count = 0;
    printf("\nPRINTING MEM BLOCKS:\n");
    for (MemMetadata* current = head; current != NULL; current = current->next){
        printf("Chunk %d:\n", count);
        printf("metadata at: %p\n", current);
        printf("size: %d\n", current->size);
        printf("MemStatus: %d\n", current->status);
        printf("Next: %p\n\n", current->next);
        count++;
    }
}

void splitChunk(MemMetadata* chunk, size_t size){
    assert(chunk != NULL && "Cannot split a NULL chunk");

    MemMetadata* newChunk = (void *) chunk + sizeof(MemMetadata) + size;
    newChunk->size = chunk->size - (size + sizeof(MemMetadata));
    newChunk->status = chunk->size < MIN_SIZE ? TOO_SMALL : AVAILABLE;
    newChunk->prev = chunk;
    newChunk->next = chunk->next;

    if (chunk->next != NULL) chunk->next->prev = newChunk;

    chunk->status = UNAVAILABLE;
    chunk->size = size;
    chunk->next = newChunk;

}

MemMetadata* requestMoreMemory(MemMetadata* lastChunk, size_t size){
    void* brkPoint = sbrk(0);

    size_t requestedMemory = MULTIPLIER * (size + sizeof(MemMetadata));
    assert(sbrk(requestedMemory) != (void*) -1 && "Could not alloc more memory");

    if (lastChunk != NULL && lastChunk->size == 0) lastChunk = lastChunk->prev;
    MemMetadata* newChunk = brkPoint;
    newChunk->size = requestedMemory - sizeof(MemMetadata);
    newChunk->status = AVAILABLE;
    newChunk->prev = lastChunk;
    newChunk->next = NULL;

    if (lastChunk != NULL){
        lastChunk->next = newChunk;
        lastChunk->status = UNAVAILABLE;
    }

    splitChunk(newChunk, size);


    return newChunk;
}

int isLastChunk(MemMetadata* chunk){
    return (chunk != NULL) && (chunk->next == NULL);
}

MemMetadata* findChunk(MemMetadata* head, size_t size){
    MemMetadata* current;
    for (current = head; current != NULL; current = current->next){
        if (current-> status == AVAILABLE && current->size >= size + sizeof(MemMetadata)){
            splitChunk(current, size);
            break; // first fit
        }
        // if the last chunk is not large enough to store the size requested by
        // the user, request more memory. This will add a new chunk to the
        // linked list, which will be allocated in the next iteration
        if (isLastChunk(current)) current = requestMoreMemory(current, size);
    }
    return current;
}

void* bbmalloc(size_t size) {
    MemMetadata* chunk;

    // first call to bbmalloc
    if (globalHead == NULL) {
        globalHead = requestMoreMemory(globalHead, size);
        chunk = globalHead;
    }
        // next calls to bbmalloc
    else chunk = findChunk(globalHead, size);

    return (void *) chunk + sizeof(MemMetadata);
}

void mergeChunkPrev(MemMetadata* chunk){
    assert(chunk != NULL && "Cannot merge a NULL chunk");

    if (chunk->prev != NULL && chunk->prev->status != UNAVAILABLE) {

        // increment the size of the merged chunk
        chunk->prev->size += (chunk->size + sizeof(MemMetadata));

        // remove the freed chunk from the linked list
        MemMetadata* temp = chunk->next;
        chunk->prev->next = temp;
        if (chunk->next != NULL)
            chunk->next->prev = chunk->prev;
    }
}

void mergeChunkNext(MemMetadata* chunk){
    assert(chunk != NULL && "Cannot merge a NULL chunk");

    if (chunk->next != NULL && chunk->next->status != UNAVAILABLE) {

        // increment the size of the merged chunk
        chunk->size += (chunk->next->size + sizeof(MemMetadata));

        // remove the freed chunk from the linked list
        MemMetadata* temp = chunk->next;
        chunk->next = chunk->next->next;
        if (temp->next != NULL)
            temp->next->prev = chunk;
    }
}

void bbfree(void* ptr){
    assert(ptr >= (void*) globalHead && ptr <= sbrk(0) &&
           "Attempted to free memory outside of allocated region");

    // pointers point to the beginning of the data itself, so the related
    // metadata is directly behind it hence the pointer arithmetic below
    MemMetadata* chunkToFree = ptr - sizeof(MemMetadata);
    chunkToFree->status = AVAILABLE;

    // merge adjacent chunks that are either available or too small
    mergeChunkNext(chunkToFree);
    mergeChunkPrev(chunkToFree);
}


int main(int argc, char* argv[]) {
//    int *a1[12];
//    for (int i = 0; i != 12; i++) {
//        a1[i] = bbmalloc(sizeof(int));
//        *(a1[i]) = i;
////        printf("a\n");
////        printf("sbrk: %p\n", sbrk(0));
////        printf("head: %p\n", globalHead);
//
//    }
//    printMemBlocks(globalHead);

    int *a = (int *) bbmalloc(3*sizeof(int));
    a[0] = 1;
    a[1] = 8;
    a[2] = 3;
    int *b = (int *) bbmalloc(sizeof(int));

//    printMemBlocks(globalHead);

    double* k = (double *) bbmalloc(sizeof(double));
    printMemBlocks(globalHead);

    bbfree(a);
    bbfree(b);

    printMemBlocks(globalHead);

    bbfree(k);
    printf("\n");
    printMemBlocks(globalHead);




    return 0;
}
