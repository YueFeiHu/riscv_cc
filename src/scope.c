#include "scope.h"
#include "var.h"
#include "var_stream.h"
#include <stdlib.h>

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

var_stream_t* scope_enter(scope_t **scp)
{
  scope_t *s = calloc(1, sizeof(scope_t));
  s->scope_vars = var_stream_create();
  s->next = *scp;
  *scp = s;
  return s->scope_vars;
}

void scope_leave(scope_t **scp)
{
  *scp = (*scp)->next; 
}