#include "parser.h"
#include "token.h"
#include "error.h"
#include "token_stream.h"
#include "var_stream.h"
#include "function.h"
#include <stdlib.h>
#include <stdio.h>

#define EQUAL_SKIP(ts, tok, str) \
	if (equal(tok, str))\
	{\
		token_stream_advance(ts);\
	}\
	else\
	{\
		error_tok(tok, "expect \'"str"\'");\
	}

var_stream_t *vs;

static AST_node_t *new_AST_node(AST_node_kind kind);
static AST_node_t *new_binary(AST_node_kind kind, AST_node_t *left, AST_node_t *right);
static AST_node_t *new_unary(AST_node_kind kind, AST_node_t *left);
static AST_node_t *new_num_node(int val);
static AST_node_t *new_var_node(var_t *var);

AST_node_t *new_AST_node(AST_node_kind kind)
{
	AST_node_t *node = calloc(1, sizeof(AST_node_t));
	node->kind = kind;
	return node;
}

AST_node_t *new_binary(AST_node_kind kind, AST_node_t *left, AST_node_t *right)
{
	AST_node_t *node = new_AST_node(kind);
	node->left = left;
	node->right = right;
	return node;
}

AST_node_t *new_num_node(int val)
{
	AST_node_t *node = new_AST_node(AST_NODE_NUM);
	node->val = val;
	return node;
}

static AST_node_t *new_unary(AST_node_kind kind, AST_node_t *left)
{
	AST_node_t *node = new_AST_node(kind);
	node->left = left;
	return node;
}

AST_node_t *new_var_node(var_t *var)
{
	AST_node_t *node = new_AST_node(AST_NODE_VAR);
	node->var = var;
	return node;
}

// program = "{" compoundStmt
// compoundStmt = stmt* "}"
// stmt = "return" expr ";" | "{" compoundStmt | exprStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-") unary | primary
// primary = "(" expr ")" | ident | num

static AST_node_t *compound_stmt(token_stream_t *ts);
static AST_node_t *stmt(token_stream_t *ts);
static AST_node_t *expr_stmt(token_stream_t *ts);
static AST_node_t *expr(token_stream_t *ts);
static AST_node_t *assign(token_stream_t *ts);
static AST_node_t *equality(token_stream_t *ts);
static AST_node_t *relational(token_stream_t *ts);
static AST_node_t *add(token_stream_t *ts);
static AST_node_t *mul(token_stream_t *ts);
static AST_node_t *unary(token_stream_t *ts);
static AST_node_t *primary(token_stream_t *ts);

// compoundStmt = stmt* "}"
static AST_node_t *compound_stmt(token_stream_t *ts)
{
	AST_node_t head;
	AST_node_t *cur = &head;
	token_t *tok = token_stream_get(ts);
	while (!equal(tok, "}"))
	{
		cur->stmt_list_node = stmt(ts);
		cur = cur->stmt_list_node;
		tok = token_stream_get(ts);
	}
	token_stream_advance(ts);
	AST_node_t *node = new_AST_node(AST_NODE_BLOCK);
	node->block_body = head.stmt_list_node;
	return node;
}

// stmt = "return" expr ";" | "{" compoundStmt | exprStmt
AST_node_t *stmt(token_stream_t *ts)
{
	AST_node_t *node;
	token_t *tok = token_stream_get(ts);
	if (equal(tok, "return"))
	{
		token_stream_advance(ts);
		node = new_unary(AST_NODE_RETURN, expr(ts));
		tok = token_stream_get(ts);
		if (equal(tok, ";"))
		{
			token_stream_advance(ts);
		}
		else
		{
			error_tok(tok, "function[stmt] expect ';'");
		}
		return node;
	}
	else if (equal(tok, "{"))
	{
		token_stream_advance(ts);
		return compound_stmt(ts);
	}
	return expr_stmt(ts);
}

AST_node_t *expr_stmt(token_stream_t *ts)
{
	token_t *tok;
	AST_node_t *node;
	tok = token_stream_get(ts);
	if (equal(tok, ";"))
	{
		token_stream_advance(ts);
		return new_AST_node(AST_NODE_BLOCK);
	}

	node = new_unary(AST_NODE_EPXR_STMT, expr(ts));
	tok = token_stream_get(ts);
	if (equal(tok, ";"))
	{
		token_stream_advance(ts);
	}
	else
	{
		error_tok(tok, "fucntion[expr_stmt] expect ';'");
	}
	return node;
}

AST_node_t *expr(token_stream_t *ts)
{
	return assign(ts);
}

static AST_node_t *assign(token_stream_t *ts)
{
	token_t *tok;
	AST_node_t *node = equality(ts);
	tok = token_stream_get(ts);

	if (equal(tok, "="))
	{
		token_stream_advance(ts);
		return new_binary(AST_NODE_ASSIGN, node, assign(ts));
	}
	return node;
}

AST_node_t *equality(token_stream_t *ts)
{
	AST_node_t *node = relational(ts);
	token_t *tok;
	while (true)
	{
		tok = token_stream_get(ts);
		if (equal(tok, "=="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_EQ, node, relational(ts));
			continue;
		}
		if (equal(tok, "!="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_NE, node, relational(ts));
			continue;
		}
		return node;
	}
	return NULL;
}

AST_node_t *relational(token_stream_t *ts)
{
	AST_node_t *node = add(ts);
	token_t *tok;
	while (true)
	{
		tok = token_stream_get(ts);
		if (equal(tok, "<"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LT, node, add(ts));
			continue;
		}

		if (equal(tok, "<="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LE, node, add(ts));
			continue;
		}

		if (equal(tok, ">"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LT, add(ts), node);
			continue;
		}

		if (equal(tok, ">="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LE, add(ts), node);
			continue;
		}
		return node;
	}
	return NULL;
}

AST_node_t *add(token_stream_t *ts)
{
	AST_node_t *node = mul(ts);
	token_t *tok;
	while (true)
	{
		tok = token_stream_get(ts);
		if (equal(tok, "+"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_ADD, node, mul(ts));
			continue;
		}

		if (equal(tok, "-"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_SUB, node, mul(ts));
			continue;
		}
		return node;
	}
	return NULL;
}

AST_node_t *mul(token_stream_t *ts)
{
	AST_node_t *node = unary(ts);
	token_t *tok;
	while (true)
	{
		tok = token_stream_get(ts);
		if (equal(tok, "*"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_MUL, node, unary(ts));
			continue;
		}

		if (equal(tok, "/"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_DIV, node, unary(ts));
			continue;
		}
		return node;
	}
	return NULL;
}
// unary = ("+" | "-") unary | primary
static AST_node_t *unary(token_stream_t *ts)
{
	token_t *tok = token_stream_get(ts);
	if (equal(tok, "+"))
	{
		token_stream_advance(ts);
		return unary(ts);
	}
	if (equal(tok, "-"))
	{
		token_stream_advance(ts);
		return new_unary(AST_NODE_NEG, unary(ts));
	}
	return primary(ts);
}

AST_node_t *primary(token_stream_t *ts)
{
	token_t *tok = token_stream_get(ts);
	if (equal(tok, "("))
	{
		token_stream_advance(ts);
		AST_node_t *tree_node = expr(ts);
		tok = token_stream_get(ts);
		if (equal(tok, ")"))
		{
			token_stream_advance(ts);
		}
		return tree_node;
	}

	if (tok->kind == TK_IDENT)
	{
		var_t *var = var_stream_find(vs, tok->loc);
		if (!var)
		{
			var = var_create(tok->loc, tok->len, 0);
			var_stream_add(vs, var);
		}
		AST_node_t *var_node = new_var_node(var);
		token_stream_advance(ts);
		return var_node;
	}

	if (tok->kind == TK_NUM)
	{
		AST_node_t *num_node = new_num_node(tok->val);
		token_stream_advance(ts);
		return num_node;
	}
	error_tok(tok, "expect an expression");
	return NULL;
}

// program = "{" compoundStmt
function_t *parse(token_stream_t *ts)
{
	vs = var_stream_create();
	// dump_ast(root, 0);
	token_t *tok = token_stream_get(ts);
	if (equal(tok, "{"))
	{
		token_stream_advance(ts);
	}
	else
	{
		error_tok(tok, "expect {");
	}
	return function_create(compound_stmt(ts), vs);
}

void dump_ast(AST_node_t *root, int depth)
{
	if (root == NULL)
	{
		return;
	}
	// 打印右子树
	dump_ast(root->right, depth + 1);

	// 打印左子树
	dump_ast(root->left, depth + 1);
	// 输出当前节点
	for (int i = 0; i < depth; i++)
	{
		printf("  "); // 每个缩进对应两个空格
	}
	switch (root->kind)
	{
	case AST_NODE_ADD:
		printf("+\n");
		break;
	case AST_NODE_SUB:
		printf("-\n");
		break;
	case AST_NODE_MUL:
		printf("*\n");
		break;
	case AST_NODE_DIV:
		printf("/\n");
		break;
	case AST_NODE_NUM:
		printf("%d\n", root->val);
		break;
	default:
		printf("Unknown node kind.\n");
		return;
	}
}
