#!/bin/bash
RISCV=/home/hyf/riscv
TEST_DIR=./tests
assert() {
  expected="$1"
  input="$2"

  ./rvcc "$input" > $TEST_DIR/tmp.s || exit
  # 编译rvcc产生的汇编文件
  $RISCV/bin/riscv64-unknown-linux-gnu-gcc -static -o $TEST_DIR/tmp $TEST_DIR/tmp.s
  $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot $TEST_DIR/tmp
  # $RISCV/bin/spike --isa=rv64gc $RISCV/riscv64-unknown-linux-gnu/bin/pk ./tmp
  # 获取程序返回值，存入 实际值
  actual="$?"
  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi

}

assert 0 0
assert 42 42

echo OK