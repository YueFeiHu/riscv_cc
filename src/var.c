#include "var.h"
#include "str_util.h"
#include "scope.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern scope_t *scp;

static char *new_unique_name()
{
    static int id = 0;
    return format(".L..%d", id++);
}

var_t* var_create(const char *name, int name_len, struct type* ty)
{
    var_t* new_var = (var_t*) malloc(sizeof(var_t));
    new_var->name_len = name_len;
    new_var->name = (char*) malloc(name_len);
    strncpy(new_var->name, name, name_len);
    new_var->type = ty;
    new_var->offset = 0;
    new_var->next = NULL;
    scope_push_var(&scp, new_var->name, new_var);
    return new_var;
}


bool var_equal(var_t *lhs, var_t* rhs)
{
    if (strcmp(lhs->name, rhs->name) == 0 && lhs->offset == rhs->offset) {
        return true;
    }

    return false;
}

// 创建了一个字符串节点，名称是系统生成的，将会被加入到全局变量列表中
// 内容，也就是字符串内容为str
var_t *var_str_create(char *str, struct type *ty)
{
    char *unique_name = new_unique_name();
    var_t *var = var_create(unique_name, strlen(unique_name), ty);
    var->init_data = str;
    var->is_global = true;
    return var;
}
void var_free(var_t *v)
{
    free(v->name);
    free(v);
}
