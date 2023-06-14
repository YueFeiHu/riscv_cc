#include "token_stream.h"
#include "error.h"
#include <stdio.h>

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