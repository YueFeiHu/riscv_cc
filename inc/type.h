#ifndef __TYPE_H__
#define __TYPE_H__

#include <stdbool.h>

typedef enum {
	TYPE_INT, 
	TYPE_CHAR, 
	TYPE_PTR, 
  TYPE_FUNC,
  TYPE_ARRAY,
  TYPE_STRUCT,
} TypeKind;

struct token;
struct struct_member;

typedef struct type {
  TypeKind kind;
  int type_sizeof;
  int align;

  struct token *name_token;
  struct type *base_type;
  struct type *func_ret_type;
  struct type *func_params;

  struct struct_member *mems;

  struct type *next;
  int array_len;
}type_t;

typedef struct struct_member{
  struct struct_member *next;
  type_t *ty;
  struct token *name;
  int offset;
}struct_member_t;

struct AST_node;

extern type_t *type_int;
extern type_t *type_char;

type_t *new_type(TypeKind kind, int size, int align);
type_t *type_ptr_create(type_t *base);
type_t *type_func_create(type_t *ret_type);
type_t *type_array_create(type_t *base, int len);
type_t *type_str_create(int len);
type_t *type_copy(type_t *ty);

bool is_integer(type_t *ty);
bool is_pointer(type_t *ty);
bool is_char(type_t *ty);
bool is_array(type_t *ty);
void type_add2node(struct AST_node *node);

#endif