#include <stdio.h>
#include <stdlib.h>

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
	printf("	li a0, %d\n", atoi(argv[1]));
	printf("	ret\n");
}