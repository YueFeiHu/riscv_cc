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
  printf("	# 压栈, 将a0的值存入栈顶\n");
	printf("	addi sp, sp, -8\n");
	printf("	sd a0, 0(sp)\n");
	depth++;
}

void pop(char *reg)
{
  printf("	# 弹栈，将栈顶的值存入%s\n", reg);
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
		printf("	# 获取变量%s的栈内地址为%d(fp)\n", node->var->name,
           node->var->offset);
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
    	printf("	# 将%d加载到a0中\n", root->val);
			printf("	li a0,   %d\n", root->val);
			return;
		case AST_NODE_NEG:
			gen_expr(root->left);
    	printf("	# 对a0值进行取反\n");
			printf("	neg a0, a0\n");
			return;
		case AST_NODE_VAR:
			gen_var_addr(root);
    	printf("	# 读取a0中存放的地址, 得到的值存入a0\n");
			printf("	ld a0, 0(a0)\n");
			return;
		case AST_NODE_ASSIGN:
			gen_var_addr(root->left);
			push();
			gen_expr(root->right);
			pop("a1");
    	printf("	# 将a0的值, 写入到a1中存放的地址\n");
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
    printf("	# a0+a1, 结果写入a0\n");
		printf("	add a0, a0, a1\n");
		break;
	case AST_NODE_SUB:
    printf("	# a0-a1, 结果写入a0\n");
		printf("	sub a0, a0, a1\n");
		break;
	case AST_NODE_MUL:
    printf("	# a0*a1,结果写入a0\n");
		printf("	mul a0, a0, a1\n");
		break;
	case AST_NODE_DIV:
    printf("	# a0/a1, 结果写入a0\n");
		printf("	div a0, a0, a1\n");
		break;
	case AST_NODE_EQ:
	case AST_NODE_NE:
    printf("	# 判断是否a0%sa1\n", root->kind == AST_NODE_EQ ? "=" : "≠");
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
    printf("	# 判断a0<a1\n");
		printf("	slt a0, a0, a1\n");
		return;
	case AST_NODE_LE:
    printf("	# 判断是否a0≤a1\n");
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
    printf("\n# =====分支语句%d==============\n", label_i);
    printf("\n# Cond表达式%d\n", label_i);
		gen_expr(root->if_condition);
    printf("  # 若a0为0, 则跳转到分支%d的.L.else.%d段\n", label_i, label_i);
		printf("	beqz a0, .L.else.%d\n",label_i);
    printf("\n# Then语句%d\n", label_i);
		gen_stmt(root->then_stmts);
    printf("  # 跳转到分支%d的.L.end.%d段\n", label_i, label_i);
		printf("	j .L.end.%d\n", label_i);
    printf("\n# Else语句%d\n", label_i);
    printf("# 分支%d的.L.else.%d段标签\n", label_i, label_i);
		printf(".L.else.%d:\n",label_i);
		if (root->else_stmts)
		{
			gen_stmt(root->else_stmts);
		}
    printf("\n# 分支%d的.L.end.%d段标签\n", label_i, label_i);
		printf(".L.end.%d:\n", label_i);
		return;
	}
	case AST_NODE_FOR:{
		label_i = label_index();
    printf("\n# =====循环语句%d===============\n", label_i);
		if (root->init_condition)
		{
			gen_stmt(root->init_condition);
		}
    printf("\n# 循环%d的.L.begin.%d段标签\n", label_i, label_i);
		printf(".L.begin.%d:\n", label_i);
    printf("# Cond表达式%d\n", label_i);
		if (root->if_condition)
		{
			gen_expr(root->if_condition);
      printf("	# 若a0为0, 则跳转到循环%d的.L.end.%d段\n", label_i, label_i);
			printf("	beqz a0, .L.end.%d\n", label_i);
		}
    printf("\n# Then语句%d\n", label_i);
		gen_stmt(root->then_stmts);
		if (root->inc_condition)
		{
      printf("\n# Inc语句%d\n", label_i);
			gen_expr(root->inc_condition);
		}
    printf("  # 跳转到循环%d的.L.begin.%d段\n", label_i, label_i);
		printf("	j .L.begin.%d\n", label_i);
    printf("\n# 循环%d的.L.end.%d段标签\n", label_i, label_i);
		printf(".L.end.%d:\n", label_i);
		return;
	}
	case AST_NODE_EPXR_STMT:
		gen_expr(root->left);
		return;
	case AST_NODE_RETURN:
    printf("# 返回语句\n");
		gen_expr(root->left);
    printf("	# 跳转到.L.return段\n");
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
  printf("	# 定义全局main段\n");
	printf("	.global main\n");
  printf("\n# =====程序开始===============\n");
  printf("# main段标签, 也是程序入口段\n");
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
  printf("	# 将fp压栈, fp属于“被调用者保存”的寄存器, 需要恢复原值\n");
	printf("	addi sp, sp, -8\n");
	printf("	sd fp, 0(sp)\n");
	// 将sp写入fp
  printf("	# 将sp的值写入fp\n");
	printf("	mv fp, sp\n");

  printf("	# sp腾出StackSize大小的栈空间\n");
	printf("	addi sp, sp, -%d\n", prog->stack_size);
	AST_node_t *root = prog->func_body;
  printf("\n# =====程序主体===============\n");
	while (root)
	{
		gen_stmt(root);
		root = root->stmt_list_node;
	}
	// Epilogue，后语
  printf("\n# =====程序结束===============\n");
  printf("# return段标签\n");
	printf(".L.return:\n");
	// 将fp的值改写回sp
  printf("	# 将fp的值写回sp\n");
	printf("	mv sp, fp\n");
	// 将最早fp保存的值弹栈，恢复fp。
  printf("	# 将最早fp保存的值弹栈, 恢复fp和sp\n");
	printf("	ld fp, 0(sp)\n");
	printf("	addi sp, sp, 8\n");
  printf("	# 返回a0值给系统调用\n");
	printf("	ret\n");
}