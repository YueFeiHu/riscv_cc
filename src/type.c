#include "type.h"
#include "ast_node.h"
#include <stdlib.h>

type_t *type_int = &(type_t){TYPE_INT, NULL};


bool is_integer(type_t *ty)
{
    return ty->kind == TYPE_INT;
}
bool is_pointer(type_t *ty)
{
    return ty->kind == TYPE_PTR;
}
void type_add2node(AST_node_t *node)
{
    if (node == NULL || node->data_type != NULL)
    {
        return;
    }
    type_add2node(node->left);
    type_add2node(node->right);
    type_add2node(node->init_condition);
    type_add2node(node->if_condition);
    type_add2node(node->inc_condition);
    type_add2node(node->then_stmts);
    type_add2node(node->else_stmts);

    for (AST_node_t *cur = node->block_body; cur ;cur = cur->stmt_list_next)
    {
        type_add2node(cur);
    }

    switch (node->kind)
    {
    case AST_NODE_ADD:
    case AST_NODE_SUB:
    case AST_NODE_MUL:
    case AST_NODE_DIV:
    case AST_NODE_NEG:
    case AST_NODE_ASSIGN:
        node->data_type = node->left->data_type;
        return;
    
    case AST_NODE_EQ:
    case AST_NODE_NE:
    case AST_NODE_LT:
    case AST_NODE_LE:
    case AST_NODE_VAR:
    case AST_NODE_NUM:
    case AST_NODE_FUNC_CALL:
        node->data_type = type_int;
        return;
    case AST_NODE_ADDR:
        node->data_type = type_ptr_create(node->left->data_type);
        return;
    case AST_NODE_DEREF:
        if (node->left->data_type->kind == TYPE_PTR)
        {
            node->data_type = node->left->data_type->base;
        }
        else
        {
            node->data_type = type_int;
        }
        return;
    default:
        break;
    }
}

type_t *type_ptr_create(type_t *base)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->kind = TYPE_PTR;
    ty->base = base;
    return ty;
}

type_t *type_func_create(type_t *ret_type)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->kind = TYPE_FUNC;
    ty->base = ret_type;
    return ty;
}