#include "token.h"
#include "parser.h"
#include "code_gen.h"
#include "token_stream.h"
#include "log.h"
#include <stdio.h>
#include <time.h>

// #define DEBUG 

struct function;
int main(int argc, char **argv)
{
	time_t now = time(NULL);
	struct tm *tm_now = gmtime(&now);
	char filename[64];
	strftime(filename, sizeof(filename), "../log/log_%Y-%m-%d_%H-%M-%S.log", tm_now);
	FILE *fp = fopen(filename, "a+");
	log_add_fp(fp, 0);

#ifndef DEBUG
	if (argc != 2)
	{
		fprintf(stderr, "%s", "argc != 2");
		return 1;
	}
	token_stream_t *ts = tokenize(argv[1]);
#else
	char *str = "{ x=3; y=5; return *(&y-1); }";
	token_stream_t *ts = tokenize(str);
#endif
	// token_stream_dump(ts);
	struct function *func = parse(ts);
	code_gen(func);

	return 0;
}