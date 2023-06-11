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
	long num = strtol(p, &p, 10);
	printf("	li a0, %ld\n", num);
	while (*p)
	{
		if (*p == '+')
		{
			p = p + 1;
			printf("addi a0, a0, %ld\n", strtol(p, &p, 10));
			continue;
		}
		if (*p == '-')
		{
			p = p + 1;
			printf("addi a0, a0, -%ld\n", strtol(p, &p, 10));
			continue;
		}
		fprintf(stderr, "unexpected character: '%c'\n", *p);
		return 1;
	}
	printf("	ret\n");
	return 0;
}