#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include<sys/ipc.h>
#include <string.h>

uint64_t fifo_create(uint64_t id, uint64_t key) {
	uint64_t status;
	asm volatile
	(
	"fcreate %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (id), [y] "r" (key)
	);
    return status;
}

uint64_t fifo_conn(uint64_t id, uint64_t key) {
	uint64_t status;
	asm volatile
	(
	"fconn %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)	
	: [x] "r" (id), [y] "r" (key)
	);
    return status;
}

uint64_t fifo_send(uint64_t id, uint64_t key, uint64_t data[4]) {
    uint64_t status;

    asm volatile (
        "ld x28, 0(%[d])\n\t"   // Load data[0] into x28
        "ld x29, 8(%[d])\n\t"   // Load data[1] into x29
        "ld x30, 16(%[d])\n\t"  // Load data[2] into x30
        "ld x31, 24(%[d])\n\t"  // Load data[3] into x31
        "fsend %[z], %[x], %[y]\n\t"  // Execute fsend
        : [z] "=r" (status)
        : [x] "r" (id), [y] "r" (key), [d] "r" (data)
        : "x28", "x29", "x30", "x31", "memory" // Clobbers
    );

    return status;
}



uint64_t fifo_recv(uint64_t regnum, uint64_t timeout, uint64_t data[4]) {
    uint64_t status;

    asm volatile (
        "1:\n\t"
        "frecv %[z], %[x], %[y]\n\t"  // Perform frecv operation
        "li t0, -1\n\t"               // Load -1 into t0
        "beq %[z], t0, 2f\n\t"        // If status == -1, exit
        "beqz %[z], 2f\n\t"           // If status == 0, exit
        "j 1b\n\t"                    // Else, repeat frecv
        "2:\n\t"
        "sd x28, 0(%[d])\n\t"         // Store x28 into data[0]
        "sd x29, 8(%[d])\n\t"         // Store x29 into data[1]
        "sd x30, 16(%[d])\n\t"        // Store x30 into data[2]
        "sd x31, 24(%[d])\n\t"        // Store x31 into data[3]
        : [z] "=r" (status)
        : [x] "r" (regnum), [y] "r" (timeout), [d] "r" (data)
        : "x28", "x29", "x30", "x31", "t0", "memory" // Clobber list
    );

    return status;
}



uint64_t fifo_close(uint64_t core_num) {
	uint64_t status;
	asm volatile
	(
	"fclose %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (core_num), [y] "r" (0)
	);
    return status;
}

// clobber x28, x29, x30, x31
void clobber_registers() 
{
	asm volatile (
		"li x28, 0\n\t"
		"li x29, 0\n\t"
		"li x30, 0\n\t"
		"li x31, 0\n\t"
	);
}

void shm_test_send(){
    int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_SHARED | MAP_ANONYMOUS;
	uint8_t* send_data = (uint8_t*)malloc(DEF_SIZE * 4 * 8);
	uint8_t* recv_data = (uint8_t*)malloc(DEF_SIZE * 4 * 8);

	void *shm_ptr = mmap(NULL, DEF_SIZE * 4 * 8, PROT_NONE , visibility, -1, 0);


	if(shm_ptr == MAP_FAILED){
		perror("mmap");
		exit(1);
	}

	// get current time with nanoseconds
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	uint64_t start_ns = start_time.tv_sec * 1000000000 + start_time.tv_nsec;

	send_data[0] = (uint8_t)(start_ns & 0xFF);
	send_data[1] = (uint8_t)((start_ns >> 8) & 0xFF);
	send_data[2] = (uint8_t)((start_ns >> 16) & 0xFF);
	send_data[3] = (uint8_t)((start_ns >> 24) & 0xFF);
	send_data[4] = (uint8_t)((start_ns >> 32) & 0xFF);
	send_data[5] = (uint8_t)((start_ns >> 40) & 0xFF);
	send_data[6] = (uint8_t)((start_ns >> 48) & 0xFF);
	send_data[7] = (uint8_t)((start_ns >> 56) & 0xFF);
	
	// for(int i = 8; i < DEF_SIZE * 4 * 8; i++){
	// 	send_data[i] = i;
	// }

	memcpy(shm_ptr, send_data, DEF_SIZE * 4 * 8);

	memcpy(recv_data, shm_ptr, DEF_SIZE * 4 * 8);
	
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	uint64_t end_ns = end_time.tv_sec * 1000000000 + end_time.tv_nsec;
	double total_mb = (double)DEF_SIZE * 8 * 4 / 1024 / 1024;
	double local_delta_s = (double)(end_ns - start_ns)/1000000000;

	printf("SHM Send done, Start time: %ld, End time: %ld, Delta_ns: %ld, Delta_ms: %f, MB: %f, MB/s: %f\n", start_ns, end_ns, end_ns - start_ns, local_delta_s, total_mb, total_mb/local_delta_s);

	free(send_data);
}





void fifo_test_send(){
	int status;
	uint8_t* send_data = (uint8_t*)malloc(DEF_SIZE * 4 * 8);

	status = fifo_create(2,1234321);
	if(status != 0){
		printf("fcreate Status: %d\n", status);
		exit(1);
	}

    struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	uint64_t start_ns = start_time.tv_sec * 1000000000 + start_time.tv_nsec;

	send_data[0] = (uint8_t)(start_ns & 0xFF);
	send_data[1] = (uint8_t)((start_ns >> 8) & 0xFF);
	send_data[2] = (uint8_t)((start_ns >> 16) & 0xFF);
	send_data[3] = (uint8_t)((start_ns >> 24) & 0xFF);
	send_data[4] = (uint8_t)((start_ns >> 32) & 0xFF);
	send_data[5] = (uint8_t)((start_ns >> 40) & 0xFF);
	send_data[6] = (uint8_t)((start_ns >> 48) & 0xFF);
	send_data[7] = (uint8_t)((start_ns >> 56) & 0xFF);

	// for(int i = 8; i < DEF_SIZE * 4 * 8; i++){
	// 	send_data[i] = i;
	// }

	for(int i = 0; i < DEF_SIZE; i++){
		while(fifo_send(2, 1234321, (uint64_t*)(send_data + i*32)) != 0){}
	}

	free(send_data);
	// uint64_t start_ns = start_time.tv_sec * 1000000000 + start_time.tv_nsec;
	printf("FIFO Send done, Start time: %ld\n", start_ns);
}









int main() {
	// uint64_t buf[4];
	fifo_test_send();
	// shm_test_send();
	// while(fifo_send(2, 1234321, buf) != 0){}
	// printf("FIFO send done\n");
}
