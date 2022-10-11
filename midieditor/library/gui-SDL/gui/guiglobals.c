#include "gui.h"
//Display * xdisplay = 0;
//XFontStruct *xFontStruct;
TTF_Font sdlFontSet;
Window rootWindow = 0;
int sdlDepth = 0;
int maxDigitWidth = 0;
int maxDigitHeight = 0;
bool redraw = false;
struct context {
    void* active;
    int pos;
} context;
Size rootWindowSize;
