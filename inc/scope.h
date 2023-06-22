#ifndef __SCOPE_H__
#define __SCOPE_H__

struct var;

typedef struct var_scope
{
  struct var_scope *next;
  char *scope_name;
  struct var *var;
}var_scope_t;

typedef struct scope
{
  struct scope *next;       // 指向上一级的域
  var_scope_t *var_scopes;  // 指向当前域内的变量
}scope_t;
scope_t *scope_create();
var_scope_t *scope_push_var(scope_t **scp, char *scp_name, struct var *v);
void scope_enter(scope_t **scp);
void scope_leave(scope_t **scp);

#endif