#ifndef __TOKEN_STREAM_H__
#define __TOKEN_STREAM_H__

#include <stdlib.h>
#include <stdbool.h>

struct token;

typedef struct token_stream
{
    struct token *tokens;
    int token_count;
    struct token *cur_token_pos;
}token_stream_t;

token_stream_t *tokenize(char *p);

token_stream_t *token_stream_create();

void token_stream_add(token_stream_t *ts, struct token *tok);

struct token *token_stream_get(token_stream_t *ts);

void token_stream_advance(token_stream_t *ts);

void token_stream_dump(token_stream_t *ts);

bool token_stream_consume(token_stream_t *ts, char *str);

struct token* token_stream_peek_next(token_stream_t *ts);

token_stream_t *tokenize_file(char *file_path);

#endif