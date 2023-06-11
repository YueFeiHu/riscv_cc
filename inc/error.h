#ifndef __ERROR_H__
#define __ERROR_H__

#include <stdarg.h>

struct token;

void error(const char *fmt, ...);
void error_at(const char *loc, char *fmt, ...);
void error_tok(const struct token *tok, char *fmt, ...);

#endif