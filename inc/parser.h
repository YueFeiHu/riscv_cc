#ifndef __PARSER_H__
#define __PARSER_H__

typedef enum{
  AST_NODE_ADD,
  AST_NODE_SUB,
  AST_NODE_MUL,
  AST_NODE_DIV,
  AST_NODE_NUM,
}AST_node_kind;

typedef struct AST_node AST_node_t;

struct AST_node{
  AST_node_kind kind;
  AST_node_t *left;
  AST_node_t *right;
  int val;
};

struct token;

AST_node_t *expr(struct token **token_list, struct token *tok);
void dump_ast(AST_node_t *root,int indent);
#endif