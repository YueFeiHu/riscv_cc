#ifndef __VAR_STREAM_H__
#define __VAR_STREAM_H__

#include "var.h"

typedef struct var_stream
{
    var_t *head;
    var_t *tail;
    int var_count;
    struct var_stream *next;
}var_stream_t;

var_stream_t* var_stream_create();

void var_stream_add(var_stream_t *var_stream, var_t *v);

void var_stream_add_tail(var_stream_t *var_stream, var_t *v);

void var_stream_remove(var_stream_t *var_stream, var_t *v);

void var_stream_update(var_stream_t *var_stream, var_t *v);

var_t* var_stream_find(var_stream_t *var_stream, const char *str);

void var_stream_copy(var_stream_t *a, var_stream_t *b);
#endif