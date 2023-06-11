#include "token.h"
#include <stdarg.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static void error(const char *fmt, ...);
static token_t *skip(const token_t *tok, const char *str);
static token_t *new_token(TokenKind kind, const char *start, const char *end);

void error(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
	exit(1);
}

bool equal(const token_t *tok, const char *str)
{
	return memcmp(tok->loc, str, tok->len) == 0 && str[tok->len] == '\0';
}

token_t *skip(const token_t *tok, const char *str)
{
	if (!equal(tok, str))
	{
		error("expect '%s'", str);
	}
	return tok->next;
}

int get_token_val(const token_t *tok)
{
	if (tok->kind != TK_NUM)
	{
		error("expect a number");
	}
	return tok->val;
}

token_t *new_token(TokenKind kind, const char *start, const char *end)
{
	token_t *tok = calloc(1, sizeof(token_t));
	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;
	return tok;
}

token_t *tokenize(char *p)
{
	token_t tok = {};
	token_t *cur = &tok;
	long num;

	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue;
		}

		if (isdigit(*p))
		{
			const char *start = p;
			num = strtol(p, &p, 10);
			cur->next = new_token(TK_NUM, start, p);
			cur = cur->next;
			cur->val = num;
		}
		else if (*p == '+' || *p == '-')
		{
			cur->next = new_token(TK_PUNCT, p, p + 1);
			p++;
			cur = cur->next;
		}
		else
		{
			error("invalid token: %c", *p);
		}
	}
	cur->next = new_token(TK_EOF, p, p);
	return tok.next;
}
