#include "parser.h"
#include "token.h"
#include "error.h"
#include "ast_node.h"
#include "token_stream.h"
#include "var_stream.h"
#include "function.h"
#include "type.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>

#define EQUAL_SKIP(ts, str) \
	if (equal(token_stream_get(ts), str))\
	{\
		token_stream_advance(ts);\
	}\
	else\
	{\
		error_tok(token_stream_get(ts), "expect \'"str"\'");\
	}

var_stream_t *vs;


// program = "{" compoundStmt
// compoundStmt = stmt* "}"
// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "{" compoundStmt
//        | exprStmt
// exprStmt = expr? ";"
// expr = assign
// assign = equality ("=" assign)?
// equality = relational ("==" relational | "!=" relational)*
// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
// add = mul ("+" mul | "-" mul)*
// mul = unary ("*" unary | "/" unary)*
// unary = ("+" | "-" | "*" | "&") unary | primary
// primary = "(" expr ")" | ident | num
static AST_node_t *compound_stmt(token_stream_t *ts);
static AST_node_t *stmt(token_stream_t *ts);
static AST_node_t *expr_stmt(token_stream_t *ts);
static AST_node_t *expr(token_stream_t *ts);
static AST_node_t *assign(token_stream_t *ts);
static AST_node_t *equality(token_stream_t *ts);
static AST_node_t *relational(token_stream_t *ts);
static AST_node_t *add(token_stream_t *ts);
static AST_node_t *ptr_add(AST_node_t *left, AST_node_t *right, token_t *tok);
static AST_node_t *ptr_sub(AST_node_t *left, AST_node_t *right, token_t *tok);
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
		cur->stmt_list_next = stmt(ts);
		cur = cur->stmt_list_next;
		tok = token_stream_get(ts);
	}
	token_stream_advance(ts);
	AST_node_t *node = new_AST_node(AST_NODE_BLOCK, tok);
	node->block_body = head.stmt_list_next;
	type_add2node(node);
	return node;
}

// stmt = "return" expr ";"
//        | "if" "(" expr ")" stmt ("else" stmt)?
//        | "for" "(" exprStmt expr? ";" expr? ")" stmt
//        | "while" "(" expr ")" stmt
//        | "{" compoundStmt
//        | exprStmt
AST_node_t *stmt(token_stream_t *ts)
{
	AST_node_t *node;
	token_t *tok = token_stream_get(ts);
	if (equal(tok, "return"))
	{
		token_stream_advance(ts);
		node = new_unary(AST_NODE_RETURN, expr(ts), tok);
		EQUAL_SKIP(ts, ";");
		return node;
	}
	else if (equal(tok, "while"))
	{
		token_stream_advance(ts);
		node = new_AST_node(AST_NODE_FOR, tok);
		EQUAL_SKIP(ts, "(");
		node->if_condition = expr(ts);
		EQUAL_SKIP(ts, ")");
		tok = token_stream_get(ts);
		node->then_stmts = stmt(ts);
		return node;
	}
	else if (equal(tok, "for"))
	{
		token_stream_advance(ts);
		node = new_AST_node(AST_NODE_FOR, tok);
		EQUAL_SKIP(ts, "(");
		node->init_condition = expr_stmt(ts);
		tok = token_stream_get(ts);
		if (!equal(tok, ";"))
		{
			node->if_condition = expr(ts);
		}
		EQUAL_SKIP(ts, ";");
		tok = token_stream_get(ts);
		if (!equal(tok, ")"))
		{
			node->inc_condition = expr(ts);
		}
		EQUAL_SKIP(ts, ")");
		node->then_stmts = stmt(ts);
		return node;
	}
	else if (equal(tok, "if"))
	{
		token_stream_advance(ts);
		node = new_AST_node(AST_NODE_IF, tok);
		EQUAL_SKIP(ts, "(");
		node->if_condition = expr(ts);
		EQUAL_SKIP(ts, ")");
		node->then_stmts = stmt(ts);
		tok = token_stream_get(ts);
		if (equal(tok, "else"))
		{
			token_stream_advance(ts);
			node->else_stmts = stmt(ts);
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
		return new_AST_node(AST_NODE_BLOCK, tok);
	}

	node = new_unary(AST_NODE_EPXR_STMT, expr(ts), tok);
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

// assign = equality ("=" assign)?
static AST_node_t *assign(token_stream_t *ts) 
{
	token_t *tok;
	AST_node_t *node = equality(ts);
	tok = token_stream_get(ts);

	if (equal(tok, "="))
	{
		token_stream_advance(ts);
		return new_binary(AST_NODE_ASSIGN, node, assign(ts), tok);
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
			node = new_binary(AST_NODE_EQ, node, relational(ts), tok);
			continue;
		}
		if (equal(tok, "!="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_NE, node, relational(ts), tok);
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
			node = new_binary(AST_NODE_LT, node, add(ts), tok);
			continue;
		}

		if (equal(tok, "<="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LE, node, add(ts), tok);
			continue;
		}

		if (equal(tok, ">"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LT, add(ts), node, tok);
			continue;
		}

		if (equal(tok, ">="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LE, add(ts), node, tok);
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
			node = ptr_add(node, mul(ts), tok);
			continue;
		}

		if (equal(tok, "-"))
		{
			token_stream_advance(ts);
			node = ptr_sub(node, mul(ts), tok);
			continue;
		}
		return node;
	}
	return NULL;
}

AST_node_t *ptr_add(AST_node_t *left, AST_node_t *right, token_t *tok)
{
	type_add2node(left);
	type_add2node(right);
	if (is_integer(left->data_type) && is_integer(right->data_type))
	{
		return new_binary(AST_NODE_ADD, left, right, tok);
	}
	if (is_pointer(left->data_type) && is_pointer(right->data_type))
	{
		error_tok(tok, "can not ptr + ptr");
	}
	if (is_integer(left->data_type) && is_pointer(right->data_type))
	{
		AST_node_t *temp = left;
		left = right;
		right = left;
	}
	right = new_binary(AST_NODE_MUL, right, new_num_node(8, tok), tok);
	// type_add2node(right);
	return new_binary(AST_NODE_ADD, left, right, tok);
}

AST_node_t *ptr_sub(AST_node_t *left, AST_node_t *right, token_t *tok)
{
	type_add2node(left);
	type_add2node(right);
	AST_node_t *node;
	if (is_integer(left->data_type) && is_integer(right->data_type))
	{
		return new_binary(AST_NODE_SUB, left, right, tok);
	}

	if (is_pointer(left->data_type) && is_integer(right->data_type))
	{
		right = new_binary(AST_NODE_MUL, right, new_num_node(8, tok), tok);
		type_add2node(right);
		AST_node_t *node = new_binary(AST_NODE_SUB, left, right, tok);
		node->data_type = left->data_type;
		return node;
	}

	if (is_pointer(left->data_type) && is_pointer(right->data_type))
	{
		node = new_binary(AST_NODE_SUB, left, right, tok);
		node->data_type = type_int;
		return new_binary(AST_NODE_DIV, node, new_num_node(8, tok), tok);
	}

	error_tok(tok, "num - ptr error");
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
			node = new_binary(AST_NODE_MUL, node, unary(ts), tok);
			continue;
		}

		if (equal(tok, "/"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_DIV, node, unary(ts), tok);
			continue;
		}
		
		return node;
	}
	return NULL;
}
// unary = ("+" | "-" | "*" | "&") unary | primary
AST_node_t *unary(token_stream_t *ts)
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
		return new_unary(AST_NODE_NEG, unary(ts), tok);
	}

	if (equal(tok, "*"))
	{
		token_stream_advance(ts);
		return new_unary(AST_NODE_DEREF, unary(ts), tok);
	}

	if (equal(tok, "&"))
	{
		token_stream_advance(ts);
		return new_unary(AST_NODE_ADDR, unary(ts), tok);
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
		AST_node_t *var_node = new_var_node(var, tok);
		token_stream_advance(ts);
		return var_node;
	}

	if (tok->kind == TK_NUM)
	{
		AST_node_t *num_node = new_num_node(tok->val, tok);
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
	EQUAL_SKIP(ts, "{");
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
