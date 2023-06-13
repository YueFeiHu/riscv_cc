#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdbool.h>

typedef enum{
	TK_PUNCT,
	TK_NUM,
	TK_EOF
}TokenKind;

typedef struct token token_t;

struct token{
	TokenKind kind;
	token_t *next;
	int val;
	const char *loc;
	int len;
};

token_t *skip(const token_t *tok, const char *str);
int get_token_val(const token_t *tok);
bool equal(const token_t *tok, const char *str);
token_t *tokenize(char *p);
void dump_token(token_t *head);

#endif