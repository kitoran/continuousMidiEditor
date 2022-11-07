#ifndef MISC_H
#define MISC_H

typedef unsigned int u32;
typedef int i32;
typedef long long int i64;
typedef short int i16;
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define ELEMS(a) (sizeof(a)/sizeof(*(a)))
#define FOR_STATIC_ARRAY(element, array) for(typeof(&array[0]) element = array; element < array+ELEMS(array); element++)
#define FOR(index, iters) for(int index = 0; index < iters; index++)
//#define FOR_STB_ARRAY(element, array) for(typeof(array) element = array; element < array+arrlen(array); element++)

#define DEBUG_PRINT(a, f) fprintf(stderr, #a " = " f "\n", (a));
//#ifndef tau
//#define tau 6.283185307179586
//#endif
#define STATIC(a,b,c) static a b; { static bool init = false;\
    if(!init) {init = true; b = (c);} }

#define STRU(type, ...) ({type ___r = {__VA_ARGS__}; ___r; })

#define DEFER_MERGE(a,b) a##b
#define DEFER_VARNAME(a) DEFER_MERGE(defer_scopevar_, a)
#define DEFER_FUNCNAME(a) DEFER_MERGE(defer_scopefunc_, a)
#define DEFER(BLOCK) void DEFER_FUNCNAME(__LINE__)(int *a) BLOCK; __attribute__((cleanup(DEFER_FUNCNAME(__LINE__)))) int DEFER_VARNAME(__LINE__)

#endif // MISC_H
