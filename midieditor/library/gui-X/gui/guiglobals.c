#include "gui.h"
#include "guiglobals.h"
int maxDigitWidth = 0;
int maxDigitHeight = 0;
bool redraw = false;
struct context context = {
    NULL,
    0,
    {0},
    0,
    0
};
Size rootWindowSize;
