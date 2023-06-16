#include "ast_node.h"
#include "token.h"
#include "var.h"
#include <stdlib.h>

AST_node_t *new_AST_node(AST_node_kind kind, token_t *tok)
{
	AST_node_t *node = calloc(1, sizeof(AST_node_t));
	node->kind = kind;
	node->end_tok = tok; 
	return node;
}

AST_node_t *new_binary(AST_node_kind kind, AST_node_t *left, AST_node_t *right, token_t *tok)
{
	AST_node_t *node = new_AST_node(kind, tok);
	node->left = left;
	node->right = right;
	return node;
}

AST_node_t *new_num_node(int val, token_t *tok)
{
	AST_node_t *node = new_AST_node(AST_NODE_NUM, tok);
	node->val = val;
	return node;
}

AST_node_t *new_unary(AST_node_kind kind, AST_node_t *left, token_t *tok)
{
	AST_node_t *node = new_AST_node(kind, tok);
	node->left = left;
	return node;
}

AST_node_t *new_var_node(var_t *var, token_t *tok)
{
	AST_node_t *node = new_AST_node(AST_NODE_VAR, tok);
	node->var = var;
	return node;
}