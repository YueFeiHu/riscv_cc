#include "token.h"
#include "error.h"
#include "token_stream.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>



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

token_t *token_create(TokenKind kind, const char *start,const char *end)
{
	token_t *tok = calloc(1, sizeof(token_t));
	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;
	return tok;
}



