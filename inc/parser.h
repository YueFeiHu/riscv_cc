#ifndef __PARSER_H__
#define __PARSER_H__

typedef enum{
  AST_NODE_ADD,
  AST_NODE_SUB,
  AST_NODE_MUL,
  AST_NODE_DIV,
  AST_NODE_NUM,
  AST_NODE_NEG,
  AST_NODE_NE,
  AST_NODE_LT,
  AST_NODE_LE,
  AST_NODE_EQ,
  AST_NODE_EPXR_STMT,
} AST_node_kind;

typedef struct AST_node AST_node_t;

struct AST_node{
  AST_node_kind kind;
  AST_node_t *left;
  AST_node_t *right;
  AST_node_t *stmt_list_node;
  int val;
};

struct token_stream;

void dump_ast(AST_node_t *root,int indent);
AST_node_t *parse(struct token_stream *ts);

#endif