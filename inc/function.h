#ifndef __FUCNTION_H__
#define __FUNCTION_H__

struct AST_node;
struct var_stream;

typedef struct function
{
    char *func_name;
    struct function *next_function;
    struct AST_node *func_body;
    struct var_stream *local_vars;
    struct var_stream *func_params;
    int stack_size;
}function_t;

function_t *function_create(struct AST_node *func_body, struct var_stream *local_vars);

#endif