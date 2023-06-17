#include "token_stream.h"
#include "token.h"
#include "error.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

char *current_line;

static bool start_with(const char *p, const char *str)
{
	return strncmp(p, str, strlen(str)) == 0;
}

static bool is_ident1(char c)
{
	return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

static bool is_ident2(char c)
{
	return is_ident1(c) || ('0' <= c && c <= '9');
}

static int read_punct(char *str)
{
	if (start_with(str, "==") || start_with(str, "!=") || 
			start_with(str, "<=") || start_with(str, ">="))
			return 2;
	return ispunct(*str) ? 1 : 0;
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
			cur = token_create(TK_NUM, start, p);
			cur->val = num;
			token_stream_add(ts, cur);
			continue;
		}
		if (is_ident1(*p))
		{
			char *start = p;
			do
			{
				++p;
			}while(is_ident2(*p));
			cur = token_create(TK_IDENT, start, p);
			token_stream_add(ts, cur);
			continue;
		}
		int punct_len = read_punct(p);
		if (punct_len)
		{
			cur = token_create(TK_PUNCT, p, p + punct_len);
			token_stream_add(ts, cur);
			p += punct_len;
			continue;
		}
		error_at(p, "invalid token: %c", *p);
	}
	cur = token_create(TK_EOF, p, p);
	token_stream_add(ts, cur);
	return ts;
}

token_stream_t *token_stream_create()
{
    token_stream_t *ts = calloc(1, sizeof(token_stream_t));
    return ts;
}

void token_stream_add(token_stream_t *ts, token_t *tok)
{
    if (ts->token_count == 0)
    {
        ts->tokens = tok;
        ts->cur_token_pos = tok;
    }
    else
    {
        ts->cur_token_pos->next = tok;
        ts->cur_token_pos = ts->cur_token_pos->next;
    }
    ts->token_count++;
}

token_t *token_stream_get(token_stream_t *ts)
{
    if (ts->token_count == 0)
    {
        return NULL;
    }
    token_t *tok = ts->tokens;
    return tok;
}

void token_stream_advance(token_stream_t *ts)
{
    if (ts->token_count != 0)
    {
        ts->tokens = ts->tokens->next;
        ts->token_count--;
    }
    else
    {
        error("token已经处理完了, 无法获取");
    }
}

void token_stream_dump(token_stream_t *ts)
{
    token_t *head = ts->tokens;
    while (head != NULL) {
        switch (head->kind) {
            case TK_IDENT:
                printf("IDENT: %.*s\n", head->len, head->loc);
                break;
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