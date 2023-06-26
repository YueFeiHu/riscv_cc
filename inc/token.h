#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdbool.h>

typedef enum{
	TK_PUNCT,
	TK_NUM,
	TK_IDENT,
	TK_STR,
	TK_KEYWORD,
	TK_EOF,
}TokenKind;

typedef struct token token_t;

struct token{
	TokenKind kind;
	token_t *next;
	int val;
	const char *loc;
	char *str; // string literal 包括 '\0'
	int len;
	int line_no;
};

token_t *token_create(TokenKind kind, const char *start, const char *end);

int token_get_val(const token_t *tok);

char *token_get_ident(const token_t *tok);

bool token_equal_str(const token_t *tok, const char *str);

bool token_is_keyword(const token_t *tok);

void token_add_line_no(token_t *tok);
#endif