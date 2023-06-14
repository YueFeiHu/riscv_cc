#ifndef __TOKEN_STREAM_H__
#define __TOKEN_STREAM_H__

#include "token.h"
#include <stdlib.h>

typedef struct token_stream
{
    token_t *tokens;
    int token_count;
    token_t *cur_token_pos;
}token_stream_t;

token_stream_t *token_stream_create();

void token_stream_add(token_stream_t *ts, token_t *tok);

token_t *token_stream_get(token_stream_t *ts);

void token_stream_advance(token_stream_t *ts);

void token_stream_dump(token_stream_t *ts);
#endif