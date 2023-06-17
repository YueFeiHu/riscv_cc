#ifndef __AST_NODE_H__
#define __AST_NODE_H__

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
	AST_NODE_ADDR,
	AST_NODE_DEREF,
	AST_NODE_VAR,
	AST_NODE_ASSIGN,
	AST_NODE_EPXR_STMT,
	AST_NODE_RETURN,
	AST_NODE_BLOCK,
	AST_NODE_IF,
	AST_NODE_FOR,
	AST_NODE_FUNC_CALL,
} AST_node_kind;

struct token;
struct var;
struct type;
typedef struct AST_node AST_node_t;

struct AST_node
{
	AST_node_kind kind;
	struct token *end_tok;
	struct type *data_type;

	AST_node_t *left;
	AST_node_t *right;
	AST_node_t *stmt_list_next;
	AST_node_t *block_body;

	AST_node_t *if_condition;
	AST_node_t *then_stmts;
	AST_node_t *else_stmts;

	AST_node_t *init_condition;
	AST_node_t *inc_condition;

	char *func_call_name;
	AST_node_t *func_call_args;

	struct var *var;
	int val;
};

AST_node_t *new_AST_node(AST_node_kind kind, struct token *tok);
AST_node_t *new_binary(AST_node_kind kind, AST_node_t *left, AST_node_t *right, struct token *tok);
AST_node_t *new_unary(AST_node_kind kind, AST_node_t *left, struct token *tok);
AST_node_t *new_num_node(int val, struct token *tok);
AST_node_t *new_var_node(struct var *var, struct token *tok);

#endif