#include "token.h"
#include "parser.h"
#include "code_gen.h"
#include "token_stream.h"
#include "log.h"
#include "error.h"
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <errno.h>
#include <string.h>

// 目标文件的路径
static char *output_file;

// 输入文件的路径
static char *input;

// 输出程序的使用说明
static void usage(int status) {
  fprintf(stderr, "rvcc [ -o <path> ] <file>\n");
  exit(status);
}

// 解析传入程序的参数
static void parse_args(int argc, char **argv) {
  // 遍历所有传入程序的参数
  for (int i = 1; i < argc; i++) {
    // 如果存在help，则直接显示用法说明
    if (!strcmp(argv[i], "--help"))
      usage(0);

    // 解析-o XXX的参数
    if (!strcmp(argv[i], "-o")) {
      // 不存在目标文件则报错
      if (!argv[++i])
        usage(1);
      // 目标文件的路径
      output_file = argv[i];
      continue;
    }

    // 解析-oXXX的参数
    if (!strncmp(argv[i], "-o", 2)) {
      // 目标文件的路径
      output_file = argv[i] + 2;
      continue;
    }

    // 解析为-的参数
    if (argv[i][0] == '-' && argv[i][1] != '\0')
      error_log("unknown argument: %s", argv[i]);

    // 其他情况则匹配为输入文件
    input = argv[i];
  }

  // 不存在输入文件时报错
  if (!input)
    error_log("no input files");
}

// 打开需要写入的文件
static FILE *open_file(char *path) {
  if (!path || strcmp(path, "-") == 0)
    return stdout;

  // 以写入模式打开文件
  FILE *out = fopen(path, "w");
  if (!out)
    error_log("cannot open output file: %s: %s", path, strerror(errno));
  return out;
}

struct function;
int main(int argc, char **argv)
{
	// time_t now = time(NULL);
	// struct tm *tm_now = gmtime(&now);
	// char filename[64];
	// strftime(filename, sizeof(filename), "../log/log_%Y-%m-%d_%H-%M-%S.log", tm_now);
	// FILE *fp = fopen(filename, "a+");
	// log_add_fp(fp, 0);

#ifndef DEBUG
	parse_args(argc, argv);
	// 没有从文件中读取，先这样吧
	token_stream_t *ts = tokenize_file(input);
#else
	char *str = "int main() { return sub_char(7, 3, 3); } int sub_char(char a, char b, char c) { return a-b-c; }";
	char *str2 = "int main() { return ({ int i=0; while(i<10) i=i+1; i; }); }";
	token_stream_t *ts = tokenize_file(argv[1]);
	token_stream_dump(ts);
#endif
	struct function *func = parse(ts);
	FILE *out = open_file(output_file);
	code_gen(func, out);

	return 0;
}