#include "parser.h"
#include "function.h"
#include "error.h"
#include "var.h"
#include "var_stream.h"
#include <stdio.h>
static int depth;

static void push();
static void pop(char *reg);

static int label_index(void) {
  static int cnt = 1;
  return cnt++;
}
static int label_i;

void push()
{
	printf("	addi sp, sp, -8\n");
	printf("	sd a0, 0(sp)\n");
	depth++;
}

void pop(char *reg)
{
	printf("	ld %s, 0(sp)\n", reg);
	printf("	addi sp, sp, 8\n");
	depth--;
}

static int align(int n, int align_num)
{
	return (n + align_num - 1) / align_num * align_num;
}

static void assign_var_offset(function_t *prog)
{
	int offset = 0;
	var_t *var = prog->local_vars->head;
	for (; var; var = var->next)
	{
		offset += 8;
		var->offset = -offset;
	}
	prog->stack_size = align(offset, 16);
}

static void gen_var_addr(AST_node_t *node)
{
	if (node->kind == AST_NODE_VAR)
	{
		printf("	addi a0, fp, %d\n", node->var->offset);
		return;
	}
	error("not an lvalue");
}

static void gen_expr(AST_node_t *root)
{
	switch(root->kind)
	{
		case AST_NODE_NUM:
			printf("	li a0,   %d\n", root->val);
			return;
		case AST_NODE_NEG:
			gen_expr(root->left);
			printf("	neg a0, a0\n");
			return;
		case AST_NODE_VAR:
			gen_var_addr(root);
			printf("	ld a0, 0(a0)\n");
			return;
		case AST_NODE_ASSIGN:
			gen_var_addr(root->left);
			push();
			gen_expr(root->right);
			pop("a1");
			printf("	sd a0, 0(a1)\n");
			return;
		default:
			break;
	}

	gen_expr(root->right);
	push();
	gen_expr(root->left);
	pop("a1");

	switch (root->kind)
	{
	case AST_NODE_ADD:
		printf("	add a0, a0, a1\n");
		break;
	case AST_NODE_SUB:
		printf("	sub a0, a0, a1\n");
		break;
	case AST_NODE_MUL:
		printf("	mul a0, a0, a1\n");
		break;
	case AST_NODE_DIV:
		printf("	div a0, a0, a1\n");
		break;
	case AST_NODE_EQ:
	case AST_NODE_NE:
		printf("	xor a0, a0, a1\n");
		if (root->kind == AST_NODE_EQ)
		{
			printf("	seqz a0, a0\n");
		}
		else
		{
			printf("	snez a0, a0\n");
		}
		return;
	case AST_NODE_LT:
		printf("	slt a0, a0, a1\n");
		return;
	case AST_NODE_LE:
		printf("	slt a0, a1, a0\n");
		printf("	xori a0, a0, 1\n");
		return;
	default:
		break;
	}
}

static void gen_stmt(AST_node_t *root)
{
	AST_node_t *cur_node;
	switch (root->kind)
	{
	case AST_NODE_IF:{
		label_i = label_index();
		gen_expr(root->if_condition);
		printf("	beqz a0, .L.else.%d\n",label_i);
		gen_stmt(root->then_stmts);
		printf("	j .L.end.%d\n", label_i);
		printf(".L.else.%d:\n",label_i);
		if (root->else_stmts)
		{
			gen_stmt(root->else_stmts);
		}
		printf(".L.end.%d:\n", label_i);
		return;
	}
	case AST_NODE_EPXR_STMT:
		gen_expr(root->left);
		return;
	case AST_NODE_RETURN:
		gen_expr(root->left);
		printf("	j .L.return\n");
		return;
	case AST_NODE_BLOCK:
		cur_node = root->block_body;
		while (cur_node)
		{
			gen_stmt(cur_node);
			cur_node = cur_node->stmt_list_node;
		}
		return;
	default:
		break;
	}
}

void code_gen(function_t *prog)
{
	assign_var_offset(prog);
	printf("	.text\n");
	printf("	.global main\n");
	printf("main:\n");
	// 栈布局
	//-------------------------------// sp
	//              fp
	//-------------------------------// fp = sp-8
	//             变量
	//-------------------------------// sp = sp-8-StackSize
	//           表达式计算
	//-------------------------------//

	// Prologue, 前言
	// 将fp压入栈中，保存fp的值
	printf("	addi sp, sp, -8\n");
	printf("	sd fp, 0(sp)\n");
	// 将sp写入fp
	printf("	mv fp, sp\n");

	printf("	addi sp, sp, -%d\n", prog->stack_size);
	AST_node_t *root = prog->func_body;
	while (root)
	{
		gen_stmt(root);
		root = root->stmt_list_node;
	}
	// Epilogue，后语
	printf(".L.return:\n");
	// 将fp的值改写回sp
	printf("	mv sp, fp\n");
	// 将最早fp保存的值弹栈，恢复fp。
	printf("	ld fp, 0(sp)\n");
	printf("	addi sp, sp, 8\n");
	printf("	ret\n");
}