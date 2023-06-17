#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdbool.h>

typedef enum{
	TK_PUNCT,
	TK_NUM,
	TK_IDENT,
	TK_KEYWORD,
	TK_EOF,
}TokenKind;

typedef struct token token_t;

struct token{
	TokenKind kind;
	token_t *next;
	int val;
	const char *loc;
	int len;
};

token_t *token_create(TokenKind kind, const char *start, const char *end);

int token_get_val(const token_t *tok);

char *token_get_ident(const token_t *tok);

bool token_equal_str(const token_t *tok, const char *str);


#endif