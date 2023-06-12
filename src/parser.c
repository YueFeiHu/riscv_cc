#include "parser.h"
#include "token.h"
#include "error.h"
#include <stdlib.h>

static AST_node_t *new_AST_node(AST_node_kind kind);
static AST_node_t *new_binary(AST_node_kind kind, AST_node_t *left, AST_node_t *right);
static AST_node_t *new_num_node(int val);

AST_node_t *expr(token_t **token_list, token_t *tok);
static AST_node_t *mul(token_t **token_list, token_t *tok);
static AST_node_t *primary(token_t **token_list, token_t *tok);

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

AST_node_t *expr(token_t **token_list, token_t *tok)
{
  AST_node_t *node = mul(&tok, tok);
  while (true)
  {
    if (equal(tok, "+"))
    {
      node = new_binary(AST_NODE_ADD, node, mul(&tok, tok->next));
      continue; 
    }

    if (equal(tok, "-"))
    {
      node = new_binary(AST_NODE_SUB, node, mul(&tok, tok->next));
      continue; 
    }
    *token_list = tok;
    return node;
  }
  return NULL;
}

AST_node_t *mul(token_t **token_list, token_t *tok)
{
  AST_node_t *node = primary(&tok, tok);
  while (true)
  {
    if (equal(tok, "*"))
    {
      node = new_binary(AST_NODE_MUL, node, primary(&tok, tok->next));
      continue; 
    }

    if (equal(tok, "/"))
    {
      node = new_binary(AST_NODE_DIV, node, primary(&tok, tok->next));
      continue; 
    }
    *token_list = tok;
    return node;
  }
  return NULL;
}

AST_node_t *primary(token_t **token_list, token_t *tok)
{
  if (equal(tok, "("))
  {
    AST_node_t *tree_node = expr(&tok, tok->next);
    *token_list = skip(tok, ")");
    return tree_node;
  }
  if (tok->kind == TK_NUM)
  {
    AST_node_t *num_node = new_num_node(tok->val);
    *token_list = tok->next;
    return num_node;
  }
  error_tok(tok, "expect an expression");
  return NULL;
}

void dump_ast(AST_node_t *root, int depth){
  if (root == NULL) {
    return;
  }
  // 打印右子树
  dump_ast(root->right, depth + 1);
  
  // 打印左子树
  dump_ast(root->left, depth + 1);
  // 输出当前节点
  for (int i = 0; i < depth; i++) {
    printf("  "); // 每个缩进对应两个空格
  }
  switch (root->kind) {
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

