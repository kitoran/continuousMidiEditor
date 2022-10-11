#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H
#include <stddef.h>
template<typename T>
class DynamicArray
{
    size_t size{0};
    T* data{0};
};

#endif // DYNAMICARRAY_H
