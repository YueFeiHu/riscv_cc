
// int assert(int expected, int actual, char *code)
// {
//   return actual;
// }

int main() { 
  // assert(55, ({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; }), "({ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; j; })");
  return  ({ int a=3; int z=5; a+z; });
}