#ifndef MISC_H
#define MISC_H

typedef unsigned int u32;
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define ELEMS(a) (sizeof(a)/sizeof(*(a)))
#define DEBUG_PRINT(a, f) fprintf(stderr, #a " = " f "\n", (a));
#define tau 6.283185307179586
#define STATIC(a,b,c) static a b; { static bool init = false;\
    if(!init) {init = true; b = (c);} }

#define STRU(type, ...) ({type ___r = {__VA_ARGS__}; ___r; })
#endif // MISC_H
