#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#define MULTIPLIER 10

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

static MemMetadata *head = NULL;

void printMemBlocks(MemMetadata* head) {
    MemMetadata* current;
    for (current = head; current != NULL; current = current->next){
        printf("metadata at: %p size: %ld \n", current, current->size);
    }
}

void* bbmalloc(size_t size) {
    // get beginning of heap
    if (head == NULL) {
        void* brkPoint = sbrk(0);
        assert(sbrk(MULTIPLIER * (size + sizeof(MemMetadata))) != (void*) -1 && "Could not alloc more memory");

        MemMetadata *ptr = brkPoint;
        ptr->size = size;
        ptr->status = UNAVAILABLE;
        ptr->prev = NULL;
        ptr->next = NULL;

        MemMetadata *emptySpace;
        emptySpace->size = (MULTIPLIER - 1) * (size + sizeof(MemMetadata)) - sizeof(MemMetadata);
        emptySpace->status = AVAILABLE;
        emptySpace->prev = ptr;
        emptySpace->next = NULL;

        ptr->next = emptySpace;
    }


    return brkPoint;
}

int main(int argc, char* argv[]) {
    int* a = bbmalloc(sizeof(int));
    *a = 10;

    printf("%p->%d\n", a, *a);
    printf("%p\n", sbrk(0));

    return 0;
}
