#ifndef MEMORY_GUARD_H
#define MEMORY_GUARD_H

#include <stdlib.h>

typedef struct {
    void *ptr;
    size_t size;
    const char *file;
    int line;
    int is_freed;
} alloc_record_t;

typedef struct {
    alloc_record_t *records;
    int count;
    int capacity;
    int total_allocations;
    int total_frees;
    size_t peak_memory;
    size_t current_memory;
} memory_tracker_t;

#define SAFE_MALLOC(size) tracked_malloc(size, __FILE__, __LINE__)
#define SAFE_FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)
#define SAFE_REALLOC(ptr, size) tracked_realloc(ptr, size, __FILE__, __LINE__)

void* tracked_malloc(size_t size, const char *file, int line);
void tracked_free(void *ptr, const char *file, int line);
void* tracked_realloc(void *ptr, size_t size, const char *file, int line);
void print_memory_report();
int detect_memory_leaks();

#endif // MEMORY_GUARD_H
