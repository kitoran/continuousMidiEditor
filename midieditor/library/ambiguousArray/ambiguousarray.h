#ifndef AMBIGUOUSARRAY_H
#define AMBIGUOUSARRAY_H

#include <stdlib.h>

struct AmbiguousArray
{
    double* data;
    int sized;
//};
inline int sizec() {
    return sized/2;
}
    double d(int pos) {
        if(pos >= 0 && pos < sized) {
            return data[pos];
        } else {
            abort();
        }
    }
    std::complex<double> c(int pos) {
        if(pos >= 0 && pos < sized/2) {
            return ((std::complex<double>*)(data))[pos];
        } else {
            abort();
        }
    }
    AmbiguousArray slicec(int pos, int size) {
        if(pos >= 0 && pos+size < sized/2 && size >= 0) {
            return {data+pos/2, size*2};
        } else {
            abort();
        }
    }
    AmbiguousArray sliced(int pos, int size) {
        if(pos >= 0 && pos+size <= sized && size >= 0) {
            return {data+pos, size};
        } else {
            abort();
        }
    }
};

#endif // AMBIGUOUSARRAY_H
