# CC = /home/fluxbyt/riscv-compiled/bin/riscv64-unknown-elf-gcc
# DUMP = /home/fluxbyt/riscv-compiled/bin/riscv64-unknown-elf-objdump

CC = /home/fluxbyt/riscv-compiled_linux/bin/riscv64-unknown-linux-gnu-gcc
DUMP = /home/fluxbyt/riscv-compiled_linux/bin/riscv64-unknown-linux-gnu-objdump

ARM_CC = arm-linux-gnueabi-gcc

TOTAL_DATA_TO_SEND=32768
DEF_SIZE = $(shell expr $(TOTAL_DATA_TO_SEND) / 32)
CFLAGS = -Wall -static -DDEF_SIZE=$(DEF_SIZE)

all: test_send test_recv test

test: test.c
	$(CC) $(CFLAGS) -o test test.c

test_send: test_send.c
	$(CC) $(CFLAGS) -o test_send test_send.c

test_recv: test_recv.c
	$(CC) $(CFLAGS) -o test_recv test_recv.c

dump: test
	$(DUMP) -S test > test.dump


clean:
	rm -f test test_send test_recv

arm: test_send_arm test_recv_arm

test_send_arm: test_send.c
	$(ARM_CC) $(CFLAGS) -o test_send_arm test_send.c

test_recv_arm: test_recv.c
	$(ARM_CC) $(CFLAGS) -o test_recv_arm test_recv.c