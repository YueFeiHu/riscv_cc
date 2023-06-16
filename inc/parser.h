#ifndef __PARSER_H__
#define __PARSER_H__

struct token_stream;
struct function;
struct AST_node;

void dump_ast(struct AST_node *root, int indent);
struct function *parse(struct token_stream *ts);

#endif