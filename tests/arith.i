/* Copyright (C) 1991-2021 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
int main() {
  // [20] 支持一元& *运算符
  assert(3, ({ int x=3; *&x; }), "({ int x=3; *&x; })");
  assert(3, ({ int x=3; int *y=&x; int **z=&y; **z; }), "({ int x=3; int *y=&x; int **z=&y; **z; })");
  assert(5, ({ int x=3; int y=5; *(&x+1); }), "({ int x=3; int y=5; *(&x+1); })");
  assert(3, ({ int x=3; int y=5; *(&y-1); }), "({ int x=3; int y=5; *(&y-1); })");
  assert(5, ({ int x=3; int y=5; *(&x-(-1)); }), "({ int x=3; int y=5; *(&x-(-1)); })");
  assert(5, ({ int x=3; int *y=&x; *y=5; x; }), "({ int x=3; int *y=&x; *y=5; x; })");
  assert(7, ({ int x=3; int y=5; *(&x+1)=7; y; }), "({ int x=3; int y=5; *(&x+1)=7; y; })");
  // ASSERT(7, ({ int x=3; int y=5; *(&y-2+1)=7; x; }));
  // ASSERT(5, ({ int x=3; (&x+2)-&x+3; }));
  // ASSERT(8, ({ int x, y; x=3; y=5; x+y; }));
  // ASSERT(8, ({ int x=3, y=5; x+y; }));
  // // [27] 支持一维数组
  // ASSERT(3, ({ int x[2]; int *y=&x; *y=3; *x; }));
  // ASSERT(3, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *x; }));
  // ASSERT(4, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+1); }));
  // ASSERT(5, ({ int x[3]; *x=3; *(x+1)=4; *(x+2)=5; *(x+2); }));
  // // [28] 支持多维数组
  // ASSERT(0, ({ int x[2][3]; int *y=x; *y=0; **x; }));
  // ASSERT(1, ({ int x[2][3]; int *y=x; *(y+1)=1; *(*x+1); }));
  // ASSERT(2, ({ int x[2][3]; int *y=x; *(y+2)=2; *(*x+2); }));
  // ASSERT(3, ({ int x[2][3]; int *y=x; *(y+3)=3; **(x+1); }));
  // ASSERT(4, ({ int x[2][3]; int *y=x; *(y+4)=4; *(*(x+1)+1); }));
  // ASSERT(5, ({ int x[2][3]; int *y=x; *(y+5)=5; *(*(x+1)+2); }));
  // ASSERT(3, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *x; }));
  // ASSERT(4, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+1); }));
  // ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }));
  // ASSERT(5, ({ int x[3]; *x=3; x[1]=4; x[2]=5; *(x+2); }));
  // ASSERT(5, ({ int x[3]; *x=3; x[1]=4; 2[x]=5; *(x+2); }));
  // // [29] 支持 [] 操作符
  // ASSERT(0, ({ int x[2][3]; int *y=x; y[0]=0; x[0][0]; }));
  // ASSERT(1, ({ int x[2][3]; int *y=x; y[1]=1; x[0][1]; }));
  // ASSERT(2, ({ int x[2][3]; int *y=x; y[2]=2; x[0][2]; }));
  // ASSERT(3, ({ int x[2][3]; int *y=x; y[3]=3; x[1][0]; }));
  // ASSERT(4, ({ int x[2][3]; int *y=x; y[4]=4; x[1][1]; }));
  // ASSERT(5, ({ int x[2][3]; int *y=x; y[5]=5; x[1][2]; }));
  printf("OK\n");
  return 0;
}
