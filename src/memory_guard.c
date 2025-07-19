/*
 * FT-DFRP: Memory Safety and Tracking System
 * 
 * Dual Licensed:
 * 1. AGPL-3.0 for research/academic use: https://www.gnu.org/licenses/agpl-3.0.html
 * 2. Commercial license available - contact michael.doran.808@gmail.com for terms
 * 
 * Copyright (C) 2025 Michael Doran
 */

#include "memory_guard.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 10000

static memory_tracker_t tracker = {
    .records = NULL,
    .count = 0,
    .capacity = MAX_RECORDS,
    .total_allocations = 0,
    .total_frees = 0,
    .peak_memory = 0,
    .current_memory = 0
};

void* tracked_malloc(size_t size, const char *file, int line) {
    if (!tracker.records) tracker.records = malloc(MAX_RECORDS * sizeof(alloc_record_t));

    void *ptr = malloc(size);
    if (!ptr) return NULL;

    tracker.records[tracker.count++] = (alloc_record_t){ ptr, size, file, line, 0 };
    tracker.total_allocations++;
    tracker.current_memory += size;
    if (tracker.current_memory > tracker.peak_memory) tracker.peak_memory = tracker.current_memory;

    return ptr;
}

void* tracked_realloc(void *ptr, size_t size, const char *file, int line) {
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr) return NULL;

    for (int i = 0; i < tracker.count; i++) {
        if (tracker.records[i].ptr == ptr && !tracker.records[i].is_freed) {
            tracker.current_memory -= tracker.records[i].size;
            tracker.current_memory += size;
            tracker.records[i] = (alloc_record_t){ new_ptr, size, file, line, 0 };
            return new_ptr;
        }
    }

    return new_ptr;
}

void tracked_free(void *ptr, const char *file, int line) {
    for (int i = 0; i < tracker.count; i++) {
        if (tracker.records[i].ptr == ptr && !tracker.records[i].is_freed) {
            tracker.records[i].is_freed = 1;
            tracker.current_memory -= tracker.records[i].size;
            tracker.total_frees++;
            free(ptr);
            return;
        }
    }
    fprintf(stderr, "[MEMORY GUARD] Warning: attempt to free unknown pointer %p (%s:%d)\n", ptr, file, line);
}

void print_memory_report() {
    printf("[MEMORY REPORT]\nTotal allocations: %d\nTotal frees: %d\nPeak memory: %zu bytes\nCurrent memory: %zu bytes\n",
           tracker.total_allocations, tracker.total_frees, tracker.peak_memory, tracker.current_memory);
}

int detect_memory_leaks() {
    int leaks = 0;
    for (int i = 0; i < tracker.count; i++) {
        if (!tracker.records[i].is_freed) {
            printf("[LEAK] %p of %zu bytes (allocated at %s:%d)\n",
                   tracker.records[i].ptr, tracker.records[i].size, tracker.records[i].file, tracker.records[i].line);
            leaks++;
        }
    }
    return leaks;
}
