#include "error.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

extern char *current_line;

static void verror_at(const char *loc, char *fmt, va_list va);

void error_log(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(1);
}

void verror_at(const char *loc, char *fmt, va_list va)
{
  fprintf(stderr, "%s\n", current_line);
	int pos = loc - current_line;
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
}

void error_at(const char *loc, char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	verror_at(loc, fmt, va);
	exit(1);
}

void error_tok(const token_t *tok, char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  verror_at(tok->loc, fmt, va);
  exit(1);
}