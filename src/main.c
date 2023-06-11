#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	token_t *tok = tokenize(p);

	while (tok)
	{
		if (tok->kind == TK_NUM)
		{
			printf("	li a0, %d\n", get_token_val(tok));
		}
		else if (tok->kind == TK_PUNCT)
		{
			if (equal(tok, "+"))
			{
				tok = tok->next;
				printf("	addi a0, a0, %d\n", get_token_val(tok));
			}
			else if (equal(tok, "-"))
			{
				tok = tok->next;
				printf("	addi a0, a0, -%d\n", get_token_val(tok));
			}
			else
			{
				fprintf(stderr, "expect + or -\n");
			}
		}
		else if (tok->kind == TK_EOF)
		{
			break;
		}
		tok = tok->next;
	}
	printf("	ret\n");
	return 0;
}