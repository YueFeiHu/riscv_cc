#! /bin/bash

rm arith.exe arith.i out.s
make
 ~/riscv/bin/riscv64-unknown-linux-gnu-gcc -o- -E -P -C tests/string.c > arith.i
 ./rvcc -o out.s arith.i
 ~/riscv/bin/riscv64-unknown-linux-gnu-gcc -static -o arith.exe out.s -xc tests/common
 ~/riscv/bin/qemu-riscv64 -L ~/riscv/sysroot arith.exe