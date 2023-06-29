#ifndef __SCOPE_H__
#define __SCOPE_H__

struct var;
struct var_stream;
struct type;

typedef struct var_scope
{
  struct var_scope *next;
  char *scope_name;
  struct var *var;
}var_scope_t;

typedef struct tag_scope
{
  struct tag_scope *next;
  char *name;
  struct type *ty;
}tag_scope_t;

typedef struct scope
{
  struct scope *next;       // 指向上一级的域
  // var_scope_t *var_scopes;  // 指向当前域内的变量
  struct var_stream* scope_vars;
  tag_scope_t* scope_tags;
}scope_t;

scope_t *scope_create();
struct var_stream *scope_push_var(scope_t *scp, struct var *v);
void scope_push_tag(scope_t *scp, struct type *ty);
void scope_enter(scope_t **scp);
void scope_leave(scope_t **scp);

#endif