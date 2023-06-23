SRC_DIR := ./src
INC_DIR := ./inc
BIN_DIR := ./bin
TESTS_DIR := ./tests

THIRD_PARTY_DIR := ./thirdparty

CC := gcc
CFLAGS := -std=c11 -g -fno-common -I${INC_DIR} -I${THIRD_PARTY_DIR}/log.c/src

SRC_FILES := $(wildcard ${SRC_DIR}/*.c)
SRC_FILES += $(wildcard ${THIRD_PARTY_DIR}/log.c/src/*.c)

OBJ_FILES := $(patsubst ${SRC_DIR}/%.c, ${BIN_DIR}/%.o, ${SRC_FILES})

.DEFAULT_GOAL := all

all : rvcc

rvcc : ${OBJ_FILES}
	${CC} -o $@ $(CFLAGS) $^

${BIN_DIR}/%.o : ${SRC_DIR}/%.c
	${CC} ${CFLAGS} -c -o $@ $<

.PHONY : clean test

test : rvcc
	bash ${TESTS_DIR}/test.sh
	# bash ${TESTS_DIR}/test-driver.sh
clean:
	rm -rf rvcc ${BIN_DIR}/*.o ${TESTS_DIR}/tmp* ./core ${TESTS_DIR}/*.s ${TESTS_DIR}/*.exe