#include "token.h"
#include "parser.h"
#include "code_gen.h"
#include "token_stream.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "%s", "argc != 2");
		return 1;
	}

	token_stream_t *ts = tokenize(argv[1]);
	// token_stream_dump(ts);
	AST_node_t *root = parse(ts);
	code_gen(root);

	return 0;
}