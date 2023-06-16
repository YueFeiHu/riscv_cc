#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdbool.h>

typedef enum {
	TY_INT, 
	TY_PTR, 
} TypeKind;

typedef struct type {
  TypeKind Kind;
  struct Type *Base;
}type_t;

struct AST_node;

extern type_t *TyInt;

bool is_integer(type_t *ty);

void type_add2node(struct AST_node *node);

#endif