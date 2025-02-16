#include <stdio.h>
#include <stdint.h>

uint32_t fifo_create(uint32_t core_num) {
	uint32_t status;
	asm volatile
	(
	"fcreate %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (core_num), [y] "r" (0)
	);
}

uint32_t fifo_conn(uint32_t core_num) {
	uint32_t status;
	asm volatile
	(
	"fconn %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (core_num), [y] "r" (0)
	);
}

uint32_t fifo_send(uint32_t regnum, uint32_t timeout) {
	uint32_t status;
	asm volatile
	(
	"fsend %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (regnum), [y] "r" (timeout)
	);
}

uint32_t fifo_recv(uint32_t regnum, uint32_t timeout) {
	uint32_t status;
	asm volatile
	(
	"frecv %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (regnum), [y] "r" (timeout)
	);
}

uint32_t fifo_close(uint32_t core_num) {
	uint32_t status;
	asm volatile
	(
	"fclose %[z], %[x], %[y]\n\t"
	: [z] "=r" (status)
	: [x] "r" (core_num), [y] "r" (0)
	);
}

int main() {
	printf("Tests");
	fifo_create(0);
	fifo_conn(0);
	fifo_send(0, 0);
	fifo_recv(0, 0);
	fifo_close(0);
}

