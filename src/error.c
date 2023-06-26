#include "error.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern char *current_input;
extern char *current_file;

static void verror_at(int line_no, const char *loc, char *fmt, va_list va);

void error_log(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(1);
}

void verror_at(int line_no, const char *loc, char *fmt, va_list va)
{
	const char *line = loc;
	while (current_input < line && line[-1] != '\n')
		line--;
	const char *end = loc;
	while (*end != '\n')
	{
		end++;
	}

  int ident = fprintf(stderr, "%s:%d: ",current_file, line_no);
	fprintf(stderr, "%.*s\n", (int)(end - line), line);
	int pos = loc - line + ident;
	fprintf(stderr, "%*s", pos, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
}

void error_at(const char *loc, char *fmt, ...)
{
	int line_no = 1;
	for (char *p = current_input; p < loc; p++)
	{
		if (*p == '\n')
		{
			line_no++;
		}
	}
	va_list va;
	va_start(va, fmt);
	verror_at(line_no, loc, fmt, va);
	exit(1);
}

void error_tok(const token_t *tok, char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  verror_at(tok->line_no, tok->loc, fmt, va);
  exit(1);
}