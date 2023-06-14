#include "token.h"
#include "error.h"
#include "token_stream.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

char *current_line;

static token_t *new_token(TokenKind kind, const char *start, const char *end);
static bool start_with(const char *p, const char *str);
static int read_punct(const char *str);

bool start_with(const char *p, const char *str)
{
	return strncmp(p, str, strlen(str)) == 0;
}

int read_punct(const char *str)
{
	if (start_with(str, "==") || start_with(str, "!=") || 
			start_with(str, "<=") || start_with(str, ">="))
			return 2;
	return ispunct(*str) ? 1 : 0;
}

bool equal(const token_t *tok, const char *str)
{
	return memcmp(tok->loc, str, tok->len) == 0 && str[tok->len] == '\0';
}

token_t *skip(const token_t *tok, const char *str)
{
	if (!equal(tok, str))
	{
		error_tok(tok, "expect '%s'", str);
	}
	return tok->next;
}

int get_token_val(const token_t *tok)
{
	if (tok->kind != TK_NUM)
	{
		error_tok(tok, "expect a number");
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

token_stream_t *tokenize(char *p)
{
	current_line = p;
	token_stream_t *ts = token_stream_create();

	token_t *cur;
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
			cur = new_token(TK_NUM, start, p);
			cur->val = num;
			token_stream_add(ts, cur);
			continue;
		}

		int punct_len = read_punct(p);
		if (punct_len)
		{
			cur = new_token(TK_PUNCT, p, p + punct_len);
			token_stream_add(ts, cur);
			p += punct_len;
			continue;
		}
		error_at(p, "invalid token: %c", *p);
	}
	cur = new_token(TK_EOF, p, p);
	token_stream_add(ts, cur);
	return ts;
}

void dump_token(token_t *head) {
    while (head != NULL) {
        switch (head->kind) {
            case TK_PUNCT:
                printf("PUNCT: %.*s\n", head->len, head->loc);
                break;
            case TK_NUM:
                printf("NUM: %d\n", head->val);
                break;
            case TK_EOF:
                printf("EOF\n");
                break;
            default:
                printf("Unknown token kind.\n");
                break;
        }
        head = head->next;
    }
}
