#include "function.h"
#include <stdlib.h>

function_t *function_create(struct AST_node *func_body, struct var_stream *local_vars)
{
    function_t *func = calloc(1, sizeof(function_t));
    func->func_body = func_body;
    func->local_vars = local_vars;
    return func;
}