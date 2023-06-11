CC := gcc
CFLAGS := -std=c11 -g -fno-common

SRC_DIR := ./src
INC_DIR := ./inc
BIN_DIR := ./bin
TESTS_DIR := ./tests

.DEFAULT_GOAL := all

all : rvcc

rvcc : ${BIN_DIR}/main.o
	${CC} -o $@ $(CFLAGS) $^

${BIN_DIR}/%.o : ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

.PHONY : clean test

test : rvcc
	bash ${TESTS_DIR}/test.sh
clean:
	rm -rf rvcc ${BIN_DIR}/*.o ${TESTS_DIR}/tmp*