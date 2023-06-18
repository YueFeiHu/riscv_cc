#define _POSIX_C_SOURCE 200809L

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
#include <string.h>

#define EQUAL_SKIP(ts, str)                                \
	if (token_equal_str(token_stream_get(ts), str))          \
	{                                                        \
		token_stream_advance(ts);                              \
	}                                                        \
	else                                                     \
	{                                                        \
		error_tok(token_stream_get(ts), "expect \'" str "\'"); \
	}

var_stream_t *vs;

// program = functionDefinition*
// functionDefinition = declspec declarator "{" compoundStmt*
// declspec = "int"
// declarator = "*"* ident typeSuffix
// typeSuffix = "(" funcParams | "[" num "]" typeSuffix | ε
// funcParams = (param ("," param)*)? ")"
// param = declspec declarator

// compoundStmt = (declaration | stmt)* "}"
// declaration =
//    declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"

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
// primary = "(" expr ")" | ident func-args? | num
// funcall = ident "(" (assign ("," assign)*)? ")"
static function_t *function_define(token_stream_t *ts);
static type_t *type_suffix(token_stream_t *ts, type_t *ty);
static AST_node_t *compound_stmt(token_stream_t *ts);
static AST_node_t *declaration(token_stream_t *ts);
static type_t *declspec(token_stream_t *ts);
static type_t *declarator(token_stream_t *ts, type_t *type);
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
static AST_node_t *func_call(token_stream_t *ts);

// functionDefinition = declspec declarator "{" compoundStmt*
static function_t *function_define(token_stream_t *ts)
{
	vs = var_stream_create();
	type_t *ty = declspec(ts);
	ty = declarator(ts, ty);
	function_t *func = calloc(1, sizeof(function_t));
	func->func_name = token_get_ident(ty->name_token);
	func->func_params = vs;
	// vs = var_stream_create();
	EQUAL_SKIP(ts, "{");
	func->func_body = compound_stmt(ts);
	func->local_vars = vs;

	return func;
}

// funcParams = (param ("," param)*)? ")"
// param = declspec declarator
static type_t *func_params(token_stream_t *ts, type_t *ty)
{
	token_t *tok = token_stream_get(ts);
	type_t head;
	type_t *cur = &head;
	while (!token_equal_str(tok, ")"))
	{
		if (cur != &head)
		{
			EQUAL_SKIP(ts, ",");
		}
		type_t *base_type = declspec(ts);
		type_t *real_type = declarator(ts, base_type);
		var_stream_add_tail(vs, var_create(token_get_ident(real_type->name_token),
																			 real_type->name_token->len, real_type));
		tok = token_stream_get(ts);
		cur->next = real_type;
		cur = cur->next;
	}
	token_stream_advance(ts);
	ty = type_func_create(ty);
	ty->func_params = head.next;
	return ty;
}

// typeSuffix = "(" funcParams | "[" num "]" typeSuffix | ε
static type_t *type_suffix(token_stream_t *ts, type_t *ty)
{
	token_t *tok = token_stream_get(ts);
	if (token_equal_str(tok, "("))
	{
		token_stream_advance(ts);
		tok = token_stream_get(ts);
		return func_params(ts, ty);
	}
	if (token_equal_str(tok, "["))
	{
		token_stream_advance(ts);
		int array_size = token_get_val(token_stream_get(ts));
		token_stream_advance(ts);
		EQUAL_SKIP(ts, "]");
		ty = type_suffix(ts, ty);
		return type_array_create(ty, array_size);
	}
	return ty;
}

// compoundStmt = (declaration | stmt)* "}"
static AST_node_t *compound_stmt(token_stream_t *ts)
{
	AST_node_t head;
	AST_node_t *cur = &head;
	token_t *tok = token_stream_get(ts);
	while (!token_equal_str(tok, "}"))
	{
		if (token_equal_str(tok, "int"))
		{
			cur->stmt_list_next = declaration(ts);
		}
		else
		{
			cur->stmt_list_next = stmt(ts);
		}
		cur = cur->stmt_list_next;
		tok = token_stream_get(ts);
	}
	token_stream_advance(ts);
	AST_node_t *node = new_AST_node(AST_NODE_BLOCK, tok);
	node->block_body = head.stmt_list_next;
	type_add2node(node);
	return node;
}

// declaration =
//    declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
AST_node_t *declaration(token_stream_t *ts)
{
	type_t *base_type = declspec(ts);
	AST_node_t head = {};
	AST_node_t *cur = &head;
	int i = 0;
	token_t *tok = token_stream_get(ts);
	while (!token_equal_str(tok, ";"))
	{
		if (i++ > 0)
		{
			EQUAL_SKIP(ts, ",");
			tok = token_stream_get(ts);
		}
		type_t *ty = declarator(ts, base_type);
		var_t *var = var_create(token_get_ident(ty->name_token), ty->name_token->len, ty);
		var_stream_add(vs, var);
		tok = token_stream_get(ts);
		if (!token_equal_str(tok, "="))
		{
			continue;
		}
		AST_node_t *left = new_var_node(var, ty->name_token);
		token_stream_advance(ts);
		AST_node_t *right = assign(ts);
		AST_node_t *node = new_binary(AST_NODE_ASSIGN, left, right, tok);
		cur->stmt_list_next = new_unary(AST_NODE_EPXR_STMT, node, tok);
		cur = cur->stmt_list_next;
		tok = token_stream_get(ts);
	}
	AST_node_t *node = new_AST_node(AST_NODE_BLOCK, tok);
	node->block_body = head.stmt_list_next;
	token_stream_advance(ts);
	return node;
}

// declspec = "int"
type_t *declspec(token_stream_t *ts)
{
	EQUAL_SKIP(ts, "int");
	return type_int;
}

// declarator = "*"* ident typeSuffix
type_t *declarator(token_stream_t *ts, type_t *type)
{
	while (token_stream_consume(ts, "*"))
	{
		type = type_ptr_create(type);
	}
	token_t *tok = token_stream_get(ts);
	token_stream_advance(ts);
	if (tok->kind != TK_IDENT)
	{
		error_tok(tok, "expect a variable name");
	}
	type = type_suffix(ts, type);
	type->name_token = tok;
	return type;
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
	if (token_equal_str(tok, "return"))
	{
		token_stream_advance(ts);
		node = new_unary(AST_NODE_RETURN, expr(ts), tok);
		EQUAL_SKIP(ts, ";");
		return node;
	}
	else if (token_equal_str(tok, "while"))
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
	else if (token_equal_str(tok, "for"))
	{
		token_stream_advance(ts);
		node = new_AST_node(AST_NODE_FOR, tok);
		EQUAL_SKIP(ts, "(");
		node->init_condition = expr_stmt(ts);
		tok = token_stream_get(ts);
		if (!token_equal_str(tok, ";"))
		{
			node->if_condition = expr(ts);
		}
		EQUAL_SKIP(ts, ";");
		tok = token_stream_get(ts);
		if (!token_equal_str(tok, ")"))
		{
			node->inc_condition = expr(ts);
		}
		EQUAL_SKIP(ts, ")");
		node->then_stmts = stmt(ts);
		return node;
	}
	else if (token_equal_str(tok, "if"))
	{
		token_stream_advance(ts);
		node = new_AST_node(AST_NODE_IF, tok);
		EQUAL_SKIP(ts, "(");
		node->if_condition = expr(ts);
		EQUAL_SKIP(ts, ")");
		node->then_stmts = stmt(ts);
		tok = token_stream_get(ts);
		if (token_equal_str(tok, "else"))
		{
			token_stream_advance(ts);
			node->else_stmts = stmt(ts);
		}
		return node;
	}
	else if (token_equal_str(tok, "{"))
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
	if (token_equal_str(tok, ";"))
	{
		token_stream_advance(ts);
		return new_AST_node(AST_NODE_BLOCK, tok);
	}

	node = new_unary(AST_NODE_EPXR_STMT, expr(ts), tok);
	tok = token_stream_get(ts);
	if (token_equal_str(tok, ";"))
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

	if (token_equal_str(tok, "="))
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
		if (token_equal_str(tok, "=="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_EQ, node, relational(ts), tok);
			continue;
		}
		if (token_equal_str(tok, "!="))
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
		if (token_equal_str(tok, "<"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LT, node, add(ts), tok);
			continue;
		}

		if (token_equal_str(tok, "<="))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LE, node, add(ts), tok);
			continue;
		}

		if (token_equal_str(tok, ">"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_LT, add(ts), node, tok);
			continue;
		}

		if (token_equal_str(tok, ">="))
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
		if (token_equal_str(tok, "+"))
		{
			token_stream_advance(ts);
			node = ptr_add(node, mul(ts), tok);
			continue;
		}

		if (token_equal_str(tok, "-"))
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
	right = new_binary(AST_NODE_MUL, right, 
					new_num_node(left->data_type->base_type->type_sizeof, tok), tok);
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
		return new_binary(AST_NODE_DIV, node, 
					 new_num_node(left->data_type->base_type->type_sizeof, tok), tok);
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
		if (token_equal_str(tok, "*"))
		{
			token_stream_advance(ts);
			node = new_binary(AST_NODE_MUL, node, unary(ts), tok);
			continue;
		}

		if (token_equal_str(tok, "/"))
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
	if (token_equal_str(tok, "+"))
	{
		token_stream_advance(ts);
		return unary(ts);
	}

	if (token_equal_str(tok, "-"))
	{
		token_stream_advance(ts);
		return new_unary(AST_NODE_NEG, unary(ts), tok);
	}

	if (token_equal_str(tok, "*"))
	{
		token_stream_advance(ts);
		AST_node_t *node = new_unary(AST_NODE_DEREF, unary(ts), tok);
		return node;
	}

	if (token_equal_str(tok, "&"))
	{
		token_stream_advance(ts);
		return new_unary(AST_NODE_ADDR, unary(ts), tok);
	}
	return primary(ts);
}
// primary = "(" expr ")" | ident args? | num
// args = "(" ")"
AST_node_t *primary(token_stream_t *ts)
{
	token_t *tok = token_stream_get(ts);
	if (token_equal_str(tok, "("))
	{
		token_stream_advance(ts);
		AST_node_t *tree_node = expr(ts);
		tok = token_stream_get(ts);
		if (token_equal_str(tok, ")"))
		{
			token_stream_advance(ts);
		}
		return tree_node;
	}

	if (tok->kind == TK_IDENT)
	{
		if (token_equal_str(token_stream_peek_next(ts), "("))
		{
			return func_call(ts);
		}
		var_t *var = var_stream_find(vs, tok->loc);
		if (!var)
		{
			var = var_create(tok->loc, tok->len, type_int);
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

static AST_node_t *func_call(token_stream_t *ts)
{
	token_t *func_name_tok = token_stream_get(ts);
	token_stream_advance(ts);
	token_stream_advance(ts);
	token_t *end_tok = token_stream_get(ts);

	AST_node_t head = {};
	AST_node_t *cur = &head;

	while (!token_equal_str(end_tok, ")"))
	{
		if (cur != &head)
		{
			EQUAL_SKIP(ts, ",");
			end_tok = token_stream_get(ts);
		}
		cur->stmt_list_next = assign(ts);
		cur = cur->stmt_list_next;
		end_tok = token_stream_get(ts);
	}
	EQUAL_SKIP(ts, ")");
	AST_node_t *node = new_AST_node(AST_NODE_FUNC_CALL, func_name_tok);
	node->func_call_name = strndup(func_name_tok->loc, func_name_tok->len);
	node->func_call_args = head.stmt_list_next;
	return node;
}

// program = "{" compoundStmt
function_t *parse(token_stream_t *ts)
{
	function_t head;
	function_t *cur = &head;
	token_t *tok = token_stream_get(ts);
	while (tok->kind != TK_EOF)
	{
		cur->next_function = function_define(ts);
		cur = cur->next_function;
		tok = token_stream_get(ts);
	}
	return head.next_function;
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
