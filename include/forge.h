#ifndef FORGE_H
#define FORGE_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// --- ENGINE DEFAULTS ---
#define DEFAULT_THREADS 8
#define SCRIBE_BUFFER_SIZE (1024 * 1024 * 60)
#define TOKEN_CAPACITY 1000000

// --- SCHEMA ENFORCEMENT ---
typedef enum { FORGE_INT, FORGE_STRING } ForgeType;

typedef struct {
    char delimiter;
    int expected_cols;
    ForgeType types[16];
    int required[16];
    int benchmark_only;
} ForgeConfig;

// --- OPERATIONAL INTERFACE ---
typedef struct {
    char *input_path;
    char *output_path;
    int threads;
    int benchmark_only;
    int validate_only; // Fixed: Added to resolve 'no member named' error
} ForgeOps;

// --- DATA STRUCTURES ---
typedef struct {
    char *data;
    size_t length;
} ForgeToken;

typedef struct {
    ForgeToken *tokens;
} ForgeArena;

// --- MULTITHREADED EXECUTION UNIT ---
typedef struct {
    int thread_id;
    const char *map_base;
    size_t start_offset;
    size_t end_offset;
    ForgeConfig *config;
    char *output_buffer;
    size_t output_size;
    // --- INTEGRITY TELEMETRY ---
    size_t valid_rows;
    size_t error_count;
} ForgeThreadTask;

// --- FUNCTION PROTOTYPES ---
void *forge_worker(void *arg);
int forge_fast_itoa(size_t val, char *buf);
void serialize_row(char *buf, size_t *ptr, size_t row_id, ForgeArena *arena, ForgeConfig *cfg);
void parse_arguments(int argc, char *argv[], ForgeOps *ops);

#endif
