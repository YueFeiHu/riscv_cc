#include "token.h"
#include "error.h"
#include "parser.h"
#include "code_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *current_line;

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s", "argc != 2");
		return 1;
	}

	printf("	.text\n");
	printf("	.global main\n");
	printf("main:\n");
	char *p = argv[1];

	current_line = p;
	token_t *tok = tokenize();
	// dump_token(tok);
	AST_node_t *root = expr(&tok, tok);
	// dump_ast(root, 0);
	if (tok->kind != TK_EOF)
	{
		error_tok(tok, "extra token");
	}
	gen_expr(root);
	printf("	ret\n");
	return 0;
}