#include "parser.h"
#include <stdio.h>
int depth;

static void push();
static void pop(char *reg);

static void push()
{
  printf("  addi sp, sp, -8\n");
  printf("  sd a0, 0(sp)\n");
  depth++;
}

static void pop(char *reg)
{
  printf("  ld %s, 0(sp)\n", reg);
  printf("  addi sp, sp, 8\n");
  depth--;
}

void gen_expr(AST_node_t *root)
{
  if (root->kind == AST_NODE_NUM)
  {
    printf("  li a0,   %d\n", root->val);
    return;
  }
  else if (root->kind == AST_NODE_NEG)
  {
    gen_expr(root->left);
    printf("  neg a0, a0\n");
    return;
  }

  gen_expr(root->right);
  push();
  gen_expr(root->left);
  pop("a1");

  switch (root->kind)
  {
  case AST_NODE_ADD:
    printf("  add a0, a0, a1\n");
    break;
  case AST_NODE_SUB:
    printf("  sub a0, a0, a1\n");
    break;
  case AST_NODE_MUL:
    printf("  mul a0, a0, a1\n");
    break;
  case AST_NODE_DIV:
    printf("  div a0, a0, a1\n");
    break;
  case AST_NODE_EQ:
  case AST_NODE_NE:
    printf("  xor a0, a0, a1\n");
    if (root->kind == AST_NODE_EQ)
    {
      printf("  seqz a0, a0\n");
    }
    else
    {
      printf("  snez a0, a0\n");
    }
    return;
  case AST_NODE_LT:
    printf("  slt a0, a0, a1\n");
    return;
  case AST_NODE_LE:
    printf("  slt a0, a1, a0\n");
    printf("  xori a0, a0, 1\n");
    return;
  default:
    break;
  }
}

void gen_stmt(AST_node_t *root)
{
  if (root->kind == AST_NODE_EPXR_STMT)
  {
    gen_expr(root->left);
  }
}

void code_gen(AST_node_t *root)
{
  printf("	.text\n");
	printf("	.global main\n");
	printf("main:\n");
  while(root)
  {
    gen_stmt(root);
    root = root->stmt_list_node;
  }
	printf("	ret\n");
}