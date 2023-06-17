#ifndef __VAR_H__
#define __VAR_H__

#include <stdbool.h>

struct type;
typedef struct var
{
    char *name;
    int name_len;
    int offset;
    struct type *type;
    struct var *next;
}var_t;

var_t* var_create(const char *name, int name_len, struct type *ty);

bool var_equal(var_t *lhs, var_t* rhs);

void var_free(var_t *v);

#endif