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
    // new_var->next = NULL;
    scope_push_var(scp, new_var);
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

void var_copy(var_t *a, var_t *b) {
    if (a == NULL || b == NULL) {
        return;
    }

    a->name_len = b->name_len;

    free(a->name);
    a->name = (char*)malloc((a->name_len + 1) * sizeof(char));
    strncpy(a->name, b->name, a->name_len);
    a->name[a->name_len] = '\0';

    a->type = b->type;
    a->offset = b->offset;
    a->is_global = b->is_global;

    free(a->init_data);
    if (b->init_data != NULL) {
        int init_data_len = strlen(b->init_data);
        a->init_data = (char*)malloc((init_data_len + 1) * sizeof(char));
        strncpy(a->init_data, b->init_data, init_data_len);
        a->init_data[init_data_len] = '\0';
    } else {
        a->init_data = NULL;
    }

    // 复制next指针
    // ...

    // 如果需要复制其他成员变量，请在此处添加复制代码
    // ...
}