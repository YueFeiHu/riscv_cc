#define _POSIX_C_SOURCE 200809L

#include "str_util.h"
#include <stdio.h>
#include <stdarg.h>

// 格式化后返回字符串
char *format(char *fmt, ...) {
  char *buf;
  size_t buf_len;
  // 将字符串对应的内存作为I/O流
  FILE *out = open_memstream(&buf, &buf_len);

  va_list va;
  va_start(va, fmt);
  // 向流中写入数据
  vfprintf(out, fmt, va);
  va_end(va);

  fclose(out);
  return buf;
}
