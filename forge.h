#ifndef FORGE_H
#define FORGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    char delimiter;
    int expected_cols;
} ForgeConfig;

typedef struct {
    int thread_id;
    const char *map_base;
    size_t start_offset;
    size_t end_offset;
    size_t valid_rows;
    size_t corrupt_bytes; // Tracks semantic violations (non-digits)
    ForgeConfig *config;
} ForgeThreadTask;

// Function prototype for the worker
void *forge_worker(void *arg);

#endif
