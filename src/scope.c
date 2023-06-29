#define _POSIX_C_SOURCE 200809L

#include "scope.h"
#include "var.h"
#include "var_stream.h"
#include "type.h"
#include "token.h"
#include <stdlib.h>
#include <string.h>

scope_t *scope_create()
{
  scope_t *scp = calloc(1, sizeof(scope_t));
  return scp;
}

static var_t *var_create_null()
{
    return (var_t*)calloc(1, sizeof(var_t));
}

static void scope_var_stream_add(var_stream_t *var_stream, var_t *v)
{
    v->scope_var_next = NULL;

    if (var_stream->head == NULL)
    {
        var_stream->head = v;
        var_stream->tail = v;
    }
    else
    {
        v->scope_var_next = var_stream->head;
        var_stream->head = v;
    }
    var_stream->var_count++;
}

var_stream_t *scope_push_var(scope_t *scp, struct var *v)
{
  // var_scope_t *vs = calloc(1, sizeof(var_scope_t));
  // vs->scope_name = scp_name;
  // vs->var = v;
  // vs->next = (*scp)->var_scopes;
  // (*scp)->var_scopes = vs;
  // return vs;
  // var_t *tmp = var_create_null();
  // var_copy(tmp, v);
  scope_var_stream_add(scp->scope_vars, v);
}

void scope_push_tag(scope_t *scp, struct type *ty)
{
  tag_scope_t *tag = calloc(1, sizeof(tag_scope_t));
  tag->name = strndup(ty->name_token->loc, ty->name_token->len);
  tag->ty = ty;
  tag->next = scp->scope_tags;
  scp->scope_tags = tag;
}

void scope_enter(scope_t **scp)
{
  scope_t *s = calloc(1, sizeof(scope_t));
  s->scope_vars = var_stream_create();
  s->next = *scp;
  *scp = s;
}

void scope_leave(scope_t **scp)
{
  *scp = (*scp)->next; 
}