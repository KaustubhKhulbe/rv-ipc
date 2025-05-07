#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define SHM_NAME "/shm_benchmark"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <size_in_bytes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the size parameter
    size_t shm_size = atoi(argv[1]);
    if (shm_size <= 0) {
        fprintf(stderr, "Error: Size must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    int shm_fd;
    char *shm_base;
    struct timespec start, end;

    // Open the shared memory object
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Map the shared memory into the address space
    shm_base = mmap(NULL, shm_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Pin this process to CPU 1
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(1, &mask);
    if (sched_setaffinity(getpid(), sizeof(mask), &mask) == -1) {
        perror("sched_setaffinity");
        exit(EXIT_FAILURE);
    }

    // Read data from shared memory
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < shm_size; i++) {
        if (shm_base[i] != 'A') {
            fprintf(stderr, "Reader: Data mismatch at index %zu\n", i);
            exit(EXIT_FAILURE);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time
    long elapsed_ns = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    long start_ns = start.tv_sec * 1000000000 + start.tv_nsec;
    long end_ns = end.tv_sec * 1000000000 + end.tv_nsec;
    long delta = end_ns - start_ns;
    printf("%ld\n", delta);


    // Clean up
    munmap(shm_base, shm_size);
    close(shm_fd);
    shm_unlink(SHM_NAME); // Remove the shared memory object

    return 0;
}