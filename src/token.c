#define _POSIX_C_SOURCE 200809L

#include "token.h"
#include "error.h"
#include "token_stream.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

static char *keywords[] = {"return", "if", "else", "for", "while", "int", "sizeof", "char"};
extern char *current_input;
bool token_equal_str(const token_t *tok, const char *str)
{
	return memcmp(tok->loc, str, tok->len) == 0 && str[tok->len] == '\0';
}

int token_get_val(const token_t *tok)
{
	if (tok->kind != TK_NUM)
	{
		error_tok(tok, "expect a number");
	}
	return tok->val;
}

char *token_get_ident(const token_t *tok)
{
	if (tok->kind != TK_IDENT)
	{
		error_tok(tok, "expected an identifier");
	}
	return strndup(tok->loc, tok->len);
}
token_t *token_create(TokenKind kind, const char *start,const char *end)
{
	token_t *tok = calloc(1, sizeof(token_t));
	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;
	return tok;
}

bool token_is_keyword(const token_t *tok)
{
	for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++)
	{
		if (token_equal_str(tok, keywords[i]))
		{
			return true;
		}
	}
	return false;
}

void token_add_line_no(token_t *tok)
{
	char *p = current_input;
	int n = 1;
	do
	{
		if (p == tok->loc)
		{
			tok->line_no = n;
			tok = tok->next;
		}
		if (*p == '\n')
		{
			n++;
		}
	}while (*p++);
}