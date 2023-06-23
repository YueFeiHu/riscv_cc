#ifndef __CODE_GEN_H__
#define __CODE_GEN_H__
#include <stdio.h>

struct AST_node;
struct function;
void code_gen(struct function *func, FILE* fp);

#endif