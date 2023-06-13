#include "token.h"
#include "parser.h"
#include "code_gen.h"
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s", "argc != 2");
		return 1;
	}

	token_t *tok = tokenize(argv[1]);
	AST_node_t *root = parse(tok);
	code_gen(root);

	return 0;
}