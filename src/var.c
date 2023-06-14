#include "var.h"
#include <stdlib.h>
#include <string.h>

var_t* var_create(const char *name, int name_len, int offset)
{
    var_t* new_var = (var_t*) malloc(sizeof(var_t));
    new_var->name = (char*) malloc(name_len + 1);
    strncpy(new_var->name, name, name_len + 1);
    new_var->offset = offset;
    new_var->next = NULL;

    return new_var;
}


bool var_equal(var_t *lhs, var_t* rhs)
{
    if (strcmp(lhs->name, rhs->name) == 0 && lhs->offset == rhs->offset) {
        return true;
    }

    return false;
}

void var_free(var_t *v)
{
    free(v->name);
    free(v);
}