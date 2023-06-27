#include "parser.h"
#include "function.h"
#include "error.h"
#include "var.h"
#include "ast_node.h"
#include "type.h"
#include "var_stream.h"
#include "token.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

extern var_stream_t *global_vars;
static FILE *output_file;
static int depth;
static int label_i;
static function_t *current_func;
static char *func_call_args_reg[] = {"a0", "a1", "a2", "a3", "a4", "a5"};

static void push();
static void pop(char *reg);
static void load(type_t *ty);
static void store(type_t *ty);
static void gen_expr(AST_node_t *root);
static int align(int n, int align_num);
static void assign_var_offset(function_t *prog);
static void gen_stmt(AST_node_t *root);
static void pre_gen_function();
static void emit_data(var_stream_t* vs);
static void emit_text(function_t * prog);

// 输出字符串并换行
static void print_ln(char *fmt, ...) {
  va_list va;
  va_start(va, fmt);
  vfprintf(output_file, fmt, va);
  va_end(va);
  fprintf(output_file, "\n");
}

static int label_index(void)
{
	static int cnt = 1;
	return cnt++;
}

static void push()
{
	print_ln("	# 压栈, 将a0的值存入栈顶");
	print_ln("	addi sp, sp, -8");
	print_ln("	sd a0, 0(sp)");
	depth++;
}

static void pop(char *reg)
{
	print_ln("	# 弹栈，将栈顶的值存入%s", reg);
	print_ln("	ld %s, 0(sp)", reg);
	print_ln("	addi sp, sp, 8");
	depth--;
}

static void load(type_t *ty)
{
	if (ty->kind == TYPE_ARRAY)
	{
		return;
	}
  print_ln("	# 读取a0中存放的地址, 得到的值存入a0");
	if (ty->type_sizeof == 1)
	{
		print_ln("	lb a0, 0(a0)");
	}
	else
	{
  	print_ln("	ld a0, 0(a0)");
	}
}

static void store(type_t *ty)
{
	pop("a1");
	print_ln("	# 将a0的值, 写入到a1中存放的地址");
  if (ty->type_sizeof == 1)
	{
		print_ln("	sb a0, 0(a1)");
	}
	else
	{
  	print_ln("	sd a0, 0(a1)");
	}
}

static int align(int n, int align_num)
{
	return (n + align_num - 1) / align_num * align_num;
}

// 这里local变量与节点中变量的关系尚未厘清
// 解法大概是一个var需要两个next指针
static void assign_var_offset(function_t *prog)
{
	for (function_t *fn = prog; fn; fn = fn->next_function)
	{
		int offset = 0;
		var_t *var = fn->local_vars->head;
		for (; var; var = var->next)
		{
			offset += var->type->type_sizeof;
			offset = align(offset, var->type->align);
			var->offset = -offset;
		}
		fn->stack_size = align(offset, 16);
	}
	for (function_t *fn = prog; fn; fn = fn->next_function)
	{
		int offset = 0;
		var_t *var = fn->func_params->head;
		for (; var; var = var->next)
		{
			offset += var->type->type_sizeof;
			offset = align(offset, var->type->align);
			var->offset = -offset;
		}
	}
}

static void gen_var_addr(AST_node_t *node)
{
	if (node->kind == AST_NODE_VAR)
	{
		if (node->var->is_global)
		{
			print_ln("	# 获取全局变量%.*s的地址", node->var->name_len, node->var->name);
      print_ln("	la a0, %.*s", node->var->name_len, node->var->name);
		}
		else
		{
			print_ln("	# 获取变量%.*s的栈内地址为%d(fp)", node->var->name_len, node->var->name, node->var->offset);
			print_ln("	addi a0, fp, %d", node->var->offset);
		}
		return;
	}
	else if (node->kind == AST_NODE_DEREF)
	{
		gen_expr(node->left);
		return;
	}
	else if (node->kind == AST_NODE_COMMA)
	{
		gen_expr(node->left);
		gen_var_addr(node->right);
		return;
	}
	else if (node->kind == AST_NODE_MEMBER)
	{
		gen_var_addr(node->left);
		print_ln("	# 计算成员变量的地址偏移量");
    print_ln("	li t0, %d", node->mem->offset);
    print_ln("	add a0, a0, t0");
    return;
	}
	error_log("not an lvalue");
}

static void gen_expr(AST_node_t *root)
{
	print_ln("	.loc 1 %d", root->end_tok->line_no);
	switch (root->kind)
	{
	case AST_NODE_NUM:
		print_ln("	# 将%d加载到a0中", root->val);
		print_ln("	li a0,   %d", root->val);
		return;
	case AST_NODE_NEG:
		gen_expr(root->left);
		print_ln("	# 对a0值进行取反");
		print_ln("	neg a0, a0");
		return;
	case AST_NODE_VAR:
	case AST_NODE_MEMBER:
		gen_var_addr(root);
		load(root->data_type);
		return;
	case AST_NODE_ASSIGN:
		gen_var_addr(root->left);
		push();
		gen_expr(root->right);
		store(root->data_type);	
		return;
	case AST_NODE_ADDR:
		gen_var_addr(root->left);
		return;
	case AST_NODE_DEREF:
		gen_expr(root->left);
		load(root->data_type);
		return;
	case AST_NODE_STMT_EXPR:
		for (AST_node_t *cur = root->block_body; cur; cur = cur->stmt_list_next)
		{
			gen_stmt(cur);
		}
		return;
	case AST_NODE_COMMA:
		gen_expr(root->left);
		gen_var_addr(root->right);
		return;
	case AST_NODE_FUNC_CALL:
	{
		int num_args = 0;
		for (AST_node_t *arg_node = root->func_call_args;
				 arg_node;
				 arg_node = arg_node->stmt_list_next)
		{
			gen_expr(arg_node);
			push();
			num_args++;
		}
		for (int i = 0; i < num_args; i++)
		{
			pop(func_call_args_reg[i]);
		}
		print_ln("  # 调用函数%s", root->func_call_name);
		print_ln("	call %s", root->func_call_name);
		return;
	}
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
		print_ln("	# a0+a1, 结果写入a0");
		print_ln("	add a0, a0, a1");
		return;
	case AST_NODE_SUB:
		print_ln("	# a0-a1, 结果写入a0");
		print_ln("	sub a0, a0, a1");
		return;
	case AST_NODE_MUL:
		print_ln("	# a0*a1,结果写入a0");
		print_ln("	mul a0, a0, a1");
		return;
	case AST_NODE_DIV:
		print_ln("	# a0/a1, 结果写入a0");
		print_ln("	div a0, a0, a1");
		return;
	case AST_NODE_EQ:
	case AST_NODE_NE:
		print_ln("	# 判断是否a0%sa1", root->kind == AST_NODE_EQ ? "=" : "≠");
		print_ln("	xor a0, a0, a1");
		if (root->kind == AST_NODE_EQ)
		{
			print_ln("	seqz a0, a0");
		}
		else
		{
			print_ln("	snez a0, a0");
		}
		return;
	case AST_NODE_LT:
		print_ln("	# 判断a0<a1");
		print_ln("	slt a0, a0, a1");
		return;
	case AST_NODE_LE:
		print_ln("	# 判断是否a0≤a1");
		print_ln("	slt a0, a1, a0");
		print_ln("	xori a0, a0, 1");
		return;
	default:
		break;
	}
}

static void gen_stmt(AST_node_t *root)
{
	print_ln("	.loc 1 %d", root->end_tok->line_no);
	AST_node_t *cur_node;
	switch (root->kind)
	{
	case AST_NODE_IF:
	{
		label_i = label_index();
		print_ln("# =====分支语句%d==============", label_i);
		print_ln("# Cond表达式%d", label_i);
		gen_expr(root->if_condition);
		print_ln("  # 若a0为0, 则跳转到分支%d的.L.else.%d段", label_i, label_i);
		print_ln("	beqz a0, .L.else.%d", label_i);
		print_ln("# Then语句%d", label_i);
		gen_stmt(root->then_stmts);
		print_ln("  # 跳转到分支%d的.L.end.%d段", label_i, label_i);
		print_ln("	j .L.end.%d", label_i);
		print_ln("# Else语句%d", label_i);
		print_ln("# 分支%d的.L.else.%d段标签", label_i, label_i);
		print_ln(".L.else.%d:", label_i);
		if (root->else_stmts)
		{
			gen_stmt(root->else_stmts);
		}
		print_ln("# 分支%d的.L.end.%d段标签", label_i, label_i);
		print_ln(".L.end.%d:", label_i);
		return;
	}
	case AST_NODE_FOR:
	{
		label_i = label_index();
		print_ln("# =====循环语句%d===============", label_i);
		if (root->init_condition)
		{
			gen_stmt(root->init_condition);
		}
		print_ln("# 循环%d的.L.begin.%d段标签", label_i, label_i);
		print_ln(".L.begin.%d:", label_i);
		print_ln("# Cond表达式%d", label_i);
		if (root->if_condition)
		{
			gen_expr(root->if_condition);
			print_ln("	# 若a0为0, 则跳转到循环%d的.L.end.%d段", label_i, label_i);
			print_ln("	beqz a0, .L.end.%d", label_i);
		}
		print_ln("# Then语句%d", label_i);
		gen_stmt(root->then_stmts);
		if (root->inc_condition)
		{
			print_ln("# Inc语句%d", label_i);
			gen_expr(root->inc_condition);
		}
		print_ln("  # 跳转到循环%d的.L.begin.%d段", label_i, label_i);
		print_ln("	j .L.begin.%d", label_i);
		print_ln("# 循环%d的.L.end.%d段标签", label_i, label_i);
		print_ln(".L.end.%d:", label_i);
		return;
	}
	case AST_NODE_EPXR_STMT:
		gen_expr(root->left);
		return;
	case AST_NODE_RETURN:
		print_ln("# 返回语句");
		gen_expr(root->left);
		print_ln("	# 跳转到.L.return.%s段", current_func->func_name);
		print_ln("	j .L.return.%s", current_func->func_name);
		return;
	case AST_NODE_BLOCK:
		cur_node = root->block_body;
		while (cur_node)
		{
			gen_stmt(cur_node);
			cur_node = cur_node->stmt_list_next;
		}
		return;
	default:
		break;
	}
	error_tok(root->end_tok, "invalid statement");
}

void emit_data(var_stream_t *vs)
{
	var_t *var = vs->head;
	while (var)
	{
		print_ln("  # 数据段标签");
    print_ln("  .data");
		if (var->init_data)
		{
			print_ln("%s:", var->name);
			for (int i = 0; i < var->type->type_sizeof; i++)
			{
				char c = var->init_data[i];
				if (isprint(c))
				{
          print_ln("	.byte %d\t# 字符：%c", c, c);
				}
				else
				{
          print_ln("	.byte %d", c);
				}
			}
		}
		else
		{
			print_ln("  .globl %.*s",var->name_len, var->name);
			print_ln("  # 全局变量%.*s",var->name_len, var->name);
			print_ln("%.*s:", var->name_len, var->name);
			print_ln("  # 零填充%d位", var->type->type_sizeof);
			print_ln("  .zero %d", var->type->type_sizeof);
		}

		var = var->next;
	}
}

void emit_text(function_t *prog)
{
	for (function_t *fn = prog; fn; fn = fn->next_function)
	{
		print_ln("	# 定义全局%s段", fn->func_name);
		print_ln("	.global %s", fn->func_name);
		print_ln("	.text");
		print_ln("# =====%s段开始===============", fn->func_name);
		print_ln("# %s段标签, 也是程序入口段", fn->func_name);
		print_ln("%s:", fn->func_name);
		// 栈布局
		//-------------------------------// sp
		//              fp
		//-------------------------------// fp = sp-8
		//             变量
		//-------------------------------// sp = sp-8-StackSize
		//           表达式计算
		//-------------------------------//

		// Prologue, 前言
		// 将ra寄存器压栈,保存ra的值
		print_ln("	# 将ra寄存器压栈,保存ra的值");
		print_ln("	addi sp, sp, -16");
		print_ln("	sd ra, 8(sp)");
		// 将fp压入栈中，保存fp的值
		print_ln("	# 将fp压栈, fp属于“被调用者保存”的寄存器, 需要恢复原值");
		print_ln("	sd fp, 0(sp)");
		// 将sp写入fp
		print_ln("	# 将sp的值写入fp");
		print_ln("	mv fp, sp");

		print_ln("	# sp腾出StackSize大小的栈空间");
		print_ln("	addi sp, sp, -%d", fn->stack_size);

		print_ln("# =====%s段主体===============", fn->func_name);
		int reg_i = 0;
		var_stream_t *args_vars = fn->func_params;
		var_t *var = args_vars->head;
		while (var != NULL)
		{
			print_ln("	# 将%s寄存器的值存入%.*s的栈地址", 
								func_call_args_reg[reg_i], var->name_len +1, var->name);
			if (var->type->type_sizeof == 1)
			{
      	print_ln("	sb %s, %d(fp)", func_call_args_reg[reg_i++], var->offset);
			}
			else
			{
      	print_ln("	sd %s, %d(fp)", func_call_args_reg[reg_i++], var->offset);
			}
			var = var->next;
		}
		current_func = fn;
		gen_stmt(fn->func_body);
		// Epilogue，后语
		print_ln("# =====%s段结束===============",fn->func_name);
		print_ln("# return段标签");
		print_ln(".L.return.%s:", fn->func_name);
		// 将fp的值改写回sp
		print_ln("	# 将fp的值写回sp");
		print_ln("	mv sp, fp");
		// 将最早fp保存的值弹栈，恢复fp。
		print_ln("	# 将最早fp保存的值弹栈, 恢复fp和sp");
		print_ln("	ld fp, 0(sp)");
		// 将ra寄存器弹栈,恢复ra的值
		print_ln("	# 将ra寄存器弹栈,恢复ra的值");
		print_ln("	ld ra, 8(sp)");
		print_ln("	addi sp, sp, 16");
		print_ln("	ret");
	}
}

void code_gen(function_t *prog, FILE *out)
{
	output_file = out;
	assign_var_offset(prog);
	emit_data(global_vars);
	emit_text(prog);
}