#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>

#define SHM_OBJ_NAME  "/forge_shm"
#define SHM_TOTAL_SIZE 622784
#define DATA_BUF_SIZE  622656

typedef struct {
    alignas(64) _Atomic uint64_t write_head;
    alignas(64) _Atomic uint64_t read_tail;
    uint8_t data_buffer[DATA_BUF_SIZE];
} forge_shm_layout_t;

int main(void) {
    printf("[INIT] Allocating fresh shared memory segment: %s...\n", SHM_OBJ_NAME);

    // Forcefully unlink any stale segments leftover in the system virtual table
    shm_unlink(SHM_OBJ_NAME);

    // CRITICAL: O_CREAT forces the Linux kernel to carve out a new memory allocation
    int shm_fd = shm_open(SHM_OBJ_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[FATAL] shm_open allocation failed");
        exit(EXIT_FAILURE);
    }

    // Allocate the memory region boundary size to exactly 622,784 bytes
    if (ftruncate(shm_fd, SHM_TOTAL_SIZE) == -1) {
        perror("[FATAL] ftruncate sizing failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Project into virtual address spaces to clear out garbage values
    forge_shm_layout_t *shm_layout = mmap(NULL, SHM_TOTAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_layout == MAP_FAILED) {
        perror("[FATAL] mmap configuration failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }

    // Initialize atomic pointers to zero states
    atomic_store_explicit(&shm_layout->write_head, 0, memory_order_relaxed);
    atomic_store_explicit(&shm_layout->read_tail, 0, memory_order_relaxed);

    printf("[INIT] Allocation successful! Memory layout initialized and live in RAM.\n");

    munmap(shm_layout, SHM_TOTAL_SIZE);
    close(shm_fd);
    return 0;
}
