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

# [13] 支持{...}
assert 3 '{ {1; {2;} return 3;} }'

# [14] 支持空语句
assert 5 '{ ;;; return 5; }'

# [15] 支持if语句
assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'

# [1] 返回指定数值
assert 0 '{ return 0; }'
assert 42 '{ return 42; }'

# [2] 支持+ -运算符
assert 34 '{ return 12-34+56; }'

# [3] 支持空格
assert 41 '{ return  12 + 34 - 5 ; }'

# [5] 支持* / ()运算符
assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 17 '{ return 1-8/(2*2)+3*6; }'

# [6] 支持一元运算的+ -
assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'
assert 48 '{ return ------12*+++++----++++++++++4; }'

# [7] 支持条件运算符
assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'
assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'
assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'
assert 1 '{ return 5==2+3; }'
assert 0 '{ return 6==4+3; }'
assert 1 '{ return 0*9+5*2==4+4*(6/3)-2; }'

# [9] 支持;分割语句
assert 3 '{ 1; 2;return 3; }'
assert 12 '{ 12+23;12+99/3;return 78-66; }'

# [10] 支持单字母变量
assert 3 '{ a=3;return a; }'
assert 8 '{ a=3; z=5;return a+z; }'
assert 6 '{ a=b=3;return a+b; }'
assert 5 '{ a=3;b=4;a=1;return a+b; }'

# [11] 支持多字母变量
assert 3 '{ foo=3;return foo; }'
assert 74 '{ foo2=70; bar4=4;return foo2+bar4; }'

# [12] 支持return
assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'



echo OK