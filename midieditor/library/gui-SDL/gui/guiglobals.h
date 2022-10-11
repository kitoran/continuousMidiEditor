#include "gui.h"
//extern Display * xdisplay;
//XFontStruct *xFontStruct;
extern TTF_Font* sdlFontSet;
extern SDL_Window rootWindow;
extern int sdlDepth;
extern int maxDigitWidth;
extern int maxDigitHeight;
extern bool redraw;
struct context {
    void* active;
    int pos;
};
extern struct context context;
extern Size rootWindowSize;
