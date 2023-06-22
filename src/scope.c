#include "scope.h"
#include "var.h"
#include <stdlib.h>

scope_t *scope_create()
{
  scope_t *scp = calloc(1, sizeof(scope_t));
  return scp;
}
var_scope_t *scope_push_var(scope_t **scp, char *scp_name, struct var *v)
{
  var_scope_t *vs = calloc(1, sizeof(var_scope_t));
  vs->scope_name = scp_name;
  vs->var = v;
  vs->next = (*scp)->var_scopes;
  (*scp)->var_scopes = vs;
  return vs;
}

void scope_enter(scope_t **scp)
{
  scope_t *s = calloc(1, sizeof(scope_t));
  s->next = *scp;
  *scp = s;
}

void scope_leave(scope_t **scp)
{
  *scp = (*scp)->next; 
}