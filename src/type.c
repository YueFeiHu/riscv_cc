#include "type.h"
#include "ast_node.h"
#include "error.h"
#include "var.h"
#include <stdlib.h>

type_t *type_int = &(type_t){TYPE_INT, 8};
type_t *type_char = &(type_t){TYPE_CHAR, 1};


bool is_integer(type_t *ty)
{
    if (ty)
    {
        return ty->kind == TYPE_INT;
    }
    return false;
}

bool is_char(type_t *ty)
{
    if (ty)
    {
        return ty->kind == TYPE_CHAR;
    }
    return false;
}

bool is_pointer(type_t *ty)
{
    if (ty)
    {
        return ty->kind == TYPE_PTR;
    }
    return false;
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

    for (AST_node_t *cur = node->block_body; cur ; cur = cur->stmt_list_next)
    {
        type_add2node(cur);
    }

    for (AST_node_t *cur = node->func_call_args; cur; cur = cur->stmt_list_next)
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
        node->data_type = node->left->data_type;
        return;

    case AST_NODE_ASSIGN:
        if (node->left->data_type->kind == TYPE_ARRAY)
        {
            error_tok(node->left->end_tok, "not an lvalue");
        }
        node->data_type = node->left->data_type;
        return;

    case AST_NODE_EQ:
    case AST_NODE_NE:
    case AST_NODE_LT:
    case AST_NODE_LE:
    case AST_NODE_NUM:
    case AST_NODE_FUNC_CALL:
        node->data_type = type_int;
        return;
        
    case AST_NODE_VAR:
        node->data_type = node->var->type;
        return;
    case AST_NODE_COMMA:
        node->data_type = node->right->data_type;
        return;
    case AST_NODE_ADDR:{
        type_t *ty = node->left->data_type;
        if (ty->kind == TYPE_ARRAY)
        {
            node->data_type = type_ptr_create(ty->base_type);
        }
        else
        {
            node->data_type = type_ptr_create(ty);
        }
        return;
    }
    case AST_NODE_DEREF:
        if (!node->left->data_type->base_type)
        {
            error_tok(node->end_tok, "invalid pointer dereference");
        }
        else
        {
            node->data_type = node->left->data_type->base_type;
        }
        return;
    case AST_NODE_EPXR_STMT:
        if (node->block_body)
        {
            AST_node_t *stmt = node->block_body;
            while (stmt->stmt_list_next)
            {
                stmt = stmt->stmt_list_next;
            }
            if (stmt->kind == AST_NODE_EPXR_STMT)
            {
                node->data_type = stmt->left->data_type;
            }
            return;
        }
        // error_tok(node->end_tok, "statement expression returning void is not supported");
        return;
    default:
        break;
    }
}

type_t *type_ptr_create(type_t *base_type)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->type_sizeof = 8;
    ty->kind = TYPE_PTR;
    ty->base_type = base_type;
    return ty;
}

type_t *type_func_create(type_t *ret_type)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->kind = TYPE_FUNC;
    ty->base_type = ret_type;
    return ty;
}

type_t *type_array_create(type_t *base_type, int len)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->kind = TYPE_ARRAY;
    ty->base_type = base_type;
    ty->array_len = len;
    ty->type_sizeof = base_type->type_sizeof * len;
    return ty;
}

type_t *type_str_create(int len)
{
    type_t *ty = calloc(1, sizeof(type_t));
    ty->kind = TYPE_ARRAY;
    ty->base_type = type_char;
    ty->array_len = len;
    ty->type_sizeof = len;
    return ty;
}

type_t *type_copy(type_t *ty)
{
    type_t *ret = calloc(1, sizeof(type_t));
    *ret = *ty;
    return ret;
}