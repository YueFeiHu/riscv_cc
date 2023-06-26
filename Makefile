SRC_DIR := ./src
INC_DIR := ./inc
BIN_DIR := ./bin
TESTS_DIR := ./tests
RISCV:= ~/riscv
THIRD_PARTY_DIR := ./thirdparty

CC := gcc
CFLAGS := -std=c11 -g -fno-common -I${INC_DIR} -I${THIRD_PARTY_DIR}/log.c/src

SRC_FILES := $(wildcard ${SRC_DIR}/*.c)
SRC_FILES += $(wildcard ${THIRD_PARTY_DIR}/log.c/src/*.c)
OBJ_FILES := $(patsubst ${SRC_DIR}/%.c, ${BIN_DIR}/%.o, ${SRC_FILES})

TEST_SRCS=$(wildcard tests/*.c)
# test/文件夹的c测试文件编译出的可执行文件
TESTS=$(TEST_SRCS:.c=.exe)

.DEFAULT_GOAL := all

all : rvcc

rvcc : ${OBJ_FILES}
	${CC} -o $@ $(CFLAGS) $^

${BIN_DIR}/%.o : ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

# 测试标签，运行测试
tests/%.exe: rvcc tests/%.c
# $(CC) -o- -E -P -C tests/$*.c | ./rvcc -o tests/$*.s -
	$(RISCV)/bin/riscv64-unknown-linux-gnu-gcc -o- -E -P -C tests/$*.c | ./rvcc -o tests/$*.s -
# $(CC) -o $@ tests/$*.s -xc tests/common
	$(RISCV)/bin/riscv64-unknown-linux-gnu-gcc -static -o $@ tests/$*.s -xc tests/common

test: $(TESTS)
# for i in $^; do echo $$i; ./$$i || exit 1; echo; done
	for i in $^; do echo $$i; $(RISCV)/bin/qemu-riscv64 -L $(RISCV)/sysroot ./$$i || exit 1; echo; done
#	for i in $^; do echo $$i; $(RISCV)/bin/spike --isa=rv64gc $(RISCV)/riscv64-unknown-linux-gnu/bin/pk ./$$i || exit 1; echo; done
	tests/driver.sh

# 清理标签，清理所有非源代码文件
clean:
	rm -rf rvcc tmp* $(TESTS) test/*.s test/*.exe
	find * -type f '(' -name '*~' -o -name '*.o' -o -name '*.s' ')' -exec rm {} ';'

# 伪目标，没有实际的依赖文件
.PHONY: test clean
# .PHONY : clean test

# test : rvcc
# 	bash ${TESTS_DIR}/test.sh
# bash ${TESTS_DIR}/test-driver.sh
# clean:
# 	rm -rf rvcc ${BIN_DIR}/*.o ${TESTS_DIR}/tmp* ./core ${TESTS_DIR}/*.s ${TESTS_DIR}/*.exe
