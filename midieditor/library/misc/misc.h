#ifndef MISC_H
#define MISC_H

typedef unsigned int u32;
typedef long long int i64;
typedef short int i16;
#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))
#define ELEMS(a) (sizeof(a)/sizeof(*(a)))
#define FOR_STATIC_ARRAY(element, array) for(typeof(&array[0]) element = array; element < array+ELEMS(array); element++)
//#define FOR_STB_ARRAY(element, array) for(typeof(array) element = array; element < array+arrlen(array); element++)

#define DEBUG_PRINT(a, f) fprintf(stderr, #a " = " f "\n", (a));
//#ifndef tau
//#define tau 6.283185307179586
//#endif
#define STATIC(a,b,c) static a b; { static bool init = false;\
    if(!init) {init = true; b = (c);} }

#define STRU(type, ...) ({type ___r = {__VA_ARGS__}; ___r; })
#endif // MISC_H
