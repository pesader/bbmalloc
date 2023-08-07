<h1><img align="center" height="60" src="assets/logo.png"> bbmalloc</h1>

`bbmalloc` is a custom implementation of malloc (and free), optimized for big buffers.

## Performance

| Total blocks | Volatile blocks | bbmalloc (ms) | GNU malloc (ms) |
|------------------|------------------------|---------------|-----------------|
| 512 | 100 | 2.686  | 3.309 |
| 512 | 400 | 4.120  | 4.954 |
| 800 | 200 | 7.737  | 5.904 |
| 800 | 600 | 10.189 | 7.662 |

## How it works

The program calls `sbrk` to request memory to the operating system. The requested memory is then split into chunks as needed. Each of these chunks are stored in a doubly linked list, so they know their neighbouring chunks (previous and next). They also know their own size and their own status (available or unavailable). This is all represented the struct shown below:

```c
typedef struct MemMetadata {
    unsigned int size;
    MemStatus status;
    struct MemMetadata* prev;
    struct MemMetadata* next;
} MemMetadata;
```

The optimization for big buffers comes into play when a memory allocation takes almost an entire chunk. If that happens, instead of splitting the chunk (and adding a most likely useless small chunk to the linked list) we simply allocate the entire thing. This keeps the linked list shorter and faster to traverse.

## Limitations

We haven't given any thought whatsoever to multithreaded usage. GNU malloc does that brilliantly though, so it's definetly a much better memory allocator for any large scale application.

## References

- [MallocInternals](https://sourceware.org/glibc/wiki/MallocInternals), by D. J. Delorie, Carlos O'Donnel, and Florian Weimer
- [A Memory Allocator](https://gee.cs.oswego.edu/dl/html/malloc.html), by Doug Lea
- [The GNU Allocator](https://www.gnu.org/software/libc/manual/html_node/The-GNU-Allocator.html), by the Free Software Foundation
- [Custom Memory Management Implementation](https://github.com/miguelperes/custom-malloc), by Miguel Péres
