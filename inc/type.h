#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdbool.h>

typedef enum {
	TYPE_INT, 
	TYPE_PTR, 
} TypeKind;

struct token;
typedef struct type {
  TypeKind kind;
  struct type *base;
  struct token *name_token;
}type_t;

struct AST_node;

extern type_t *type_int;

type_t *type_ptr_create(type_t *base);
bool is_integer(type_t *ty);
bool is_pointer(type_t *ty);
void type_add2node(struct AST_node *node);

#endif