#ifndef FORGE_H
#define FORGE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <immintrin.h> 
#include <sched.h>
#include <sys/mman.h>

typedef enum { FORGE_INT, FORGE_STRING } ForgeType;

typedef struct {
    char delimiter;
    int expected_cols;
    int benchmark_only;
} ForgeConfig;

typedef struct {
    char *input_path;
    int threads;
    int benchmark_only;
} ForgeOps;

typedef struct {
    int thread_id;
    const char *map_base;
    size_t start_offset;
    size_t end_offset;
    ForgeConfig *config;
    size_t valid_rows;
} ForgeThreadTask;

void *forge_worker(void *arg);
void parse_arguments(int argc, char *argv[], ForgeOps *ops);

#endif
