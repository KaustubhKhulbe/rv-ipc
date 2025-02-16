#! /bin/bash



 build/RISCV/gem5.fast --outdir=m5out/ configs/deprecated/example/se.py --cpu-type=RiscvTimingSimpleCPU --num-cpus=2 \
    --redirects /lib=/home/fluxbyt/riscv-compiled/lib \
    --cpu-clock=4GHz \
    --cacheline_size=64 \
    --caches \
    --l1i_size=8kB \
    --l1d_size=8kB \
    --l2cache \
    --l2_size=4MB \
    --l2_assoc=16 \
    --cmd="ipc_bin/test_send;ipc_bin/test_recv"

