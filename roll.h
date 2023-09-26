#ifndef ROLL_H
#define ROLL_H
#include <stdbool.h>
struct Painter;

struct Size;
void roll(struct Painter* p, struct Size size, bool moreEvents);

#endif // ROLL_H
