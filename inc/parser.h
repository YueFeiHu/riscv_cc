#ifndef __PARSER_H__
#define __PARSER_H__

#include "var.h"

typedef enum
{
	AST_NODE_ADD,
	AST_NODE_SUB,
	AST_NODE_MUL,
	AST_NODE_DIV,
	AST_NODE_NUM,
	AST_NODE_NEG,
	AST_NODE_NE,
	AST_NODE_LT,
	AST_NODE_LE,
	AST_NODE_EQ,
	AST_NODE_VAR,
	AST_NODE_ASSIGN,
	AST_NODE_EPXR_STMT,
	AST_NODE_RETURN,
	AST_NODE_BLOCK,
	AST_NODE_IF,
	AST_NODE_FOR,
} AST_node_kind;

typedef struct AST_node AST_node_t;

struct token;

struct AST_node
{
	AST_node_kind kind;
	struct token *end_tok;

	AST_node_t *left;
	AST_node_t *right;
	AST_node_t *stmt_list_node;
	AST_node_t *block_body;

	AST_node_t *if_condition;
	AST_node_t *then_stmts;
	AST_node_t *else_stmts;

	AST_node_t *init_condition;
	AST_node_t *inc_condition;

	var_t *var;
	int val;
};

struct token_stream;
struct function;

void dump_ast(AST_node_t *root, int indent);
struct function *parse(struct token_stream *ts);

#endif