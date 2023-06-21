#ifndef __VAR_H__
#define __VAR_H__

#include <stdbool.h>

struct type;
typedef struct var
{
    char *name;         // 变量名
    struct type *type;  // 变量类型
    int name_len;       // 变量名长度
    int offset;         // 局部变量 fp的偏移量
    bool is_global;     // 是否是全局变量

    char *init_data;    // 全局变量
    struct var *next;
}var_t;

var_t *var_create(const char *name, int name_len, struct type *ty);

var_t *var_str_create(char *str, struct type *ty);

bool var_equal(var_t *lhs, var_t* rhs);

void var_free(var_t *v);

#endif