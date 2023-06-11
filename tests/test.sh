#!/bin/bash
RISCV=/home/hyf/riscv
TEST_DIR=./tests
assert() {
  expected="$1"
  input="$2"

  ./rvcc "$input" > $TEST_DIR/tmp.s || exit
  $RISCV/bin/riscv64-unknown-linux-gnu-gcc -static -o $TEST_DIR/tmp $TEST_DIR/tmp.s
  $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot $TEST_DIR/tmp

  actual="$?"
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi

}

assert 6 '1+2+ 3'
assert 1 '1   -  1  + 1'

echo OK