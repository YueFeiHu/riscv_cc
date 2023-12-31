#include "var_stream.h"
#include <stdlib.h>
#include <string.h>

var_stream_t *var_stream_create()
{
    var_stream_t *new_var_stream = (var_stream_t *)malloc(sizeof(var_stream_t));
    new_var_stream->head = NULL;
    new_var_stream->tail = NULL;
    new_var_stream->var_count = 0;

    return new_var_stream;
}

void var_stream_add(var_stream_t *var_stream, var_t *v)
{
    v->next = NULL;

    if (var_stream->head == NULL)
    {
        var_stream->head = v;
        var_stream->tail = v;
    }
    else
    {
        v->next = var_stream->head;
        var_stream->head = v;
    }
    var_stream->var_count++;
}

void var_stream_add_tail(var_stream_t *var_stream, var_t *v)
{
    // v->next = NULL;

    if (var_stream->head == NULL)
    {
        var_stream->head = v;
        var_stream->tail = v;
    }
    else
    {
        var_stream->tail->next = v;
        var_stream->tail = v;
    }
    var_stream->var_count++;
}

void var_stream_remove(var_stream_t *var_stream, var_t *v)
{
    var_t *curr_var = var_stream->head;
    var_t *prev_var = NULL;

    while (curr_var != NULL)
    {
        if (curr_var == v)
        {
            if (prev_var == NULL)
            {
                var_stream->head = curr_var->next;
            }
            else
            {
                prev_var->next = curr_var->next;
            }

            if (var_stream->tail == curr_var)
            {
                var_stream->tail = prev_var;
            }

            var_stream->var_count--;
            free(curr_var->name);
            free(curr_var);
            break;
        }

        prev_var = curr_var;
        curr_var = curr_var->next;
    }
}

void var_stream_update(var_stream_t *var_stream, var_t *v)
{
    // 根据需求进行修改 offset，略
}

var_t *var_stream_find(var_stream_t *var_stream, const char *str)
{
    var_t *curr_var = var_stream->head;

    while (curr_var != NULL)
    {
        if (strncmp(curr_var->name, str, curr_var->name_len) == 0)
        {
            return curr_var;
        }

        curr_var = curr_var->next;
    }

    return NULL;
}

void var_stream_copy(var_stream_t *a, var_stream_t *b) {
    if (a == NULL || b == NULL) {
        return;
    }

    var_t *current_b = b->head;

    while (current_b != NULL) {
        // 创建一个新的var_t并复制current_b的数据到新的变量中
        var_t *new_var = (var_t*)malloc(sizeof(var_t));
        memcpy(new_var, current_b, sizeof(var_t));
        new_var->next = NULL;

        // 将复制得到的新变量添加到a的变量流中
        var_stream_add(a, new_var);

        current_b = current_b->next;
    }
}