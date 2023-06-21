#include "token.h"
#include "parser.h"
#include "code_gen.h"
#include "token_stream.h"
#include "log.h"
#include <stdio.h>
#include <time.h>

struct function;
int main(int argc, char **argv)
{
	// time_t now = time(NULL);
	// struct tm *tm_now = gmtime(&now);
	// char filename[64];
	// strftime(filename, sizeof(filename), "../log/log_%Y-%m-%d_%H-%M-%S.log", tm_now);
	// FILE *fp = fopen(filename, "a+");
	// log_add_fp(fp, 0);

#ifndef DEBUG
	if (argc != 2)
	{
		fprintf(stderr, "%s", "argc != 2");
		return 1;
	}
	token_stream_t *ts = tokenize(argv[1]);
#else
	char *str = "int x; int y; int main() { x=3; y=4; return x+y; }";
	char *str2 = "int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }";
	token_stream_t *ts = tokenize(str2);
	token_stream_dump(ts);
#endif
	struct function *func = parse(ts);
	code_gen(func);

	return 0;
}