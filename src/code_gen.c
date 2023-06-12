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
  default:
    break;
  }
}
