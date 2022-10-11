#ifndef XBACKEND_H
#define XBACKEND_H

#ifdef SDL
#include <SDL2/SDL_events.h>
//union SDL_Event;//struct SDL_Event;
typedef SDL_Event Event;
struct SDL_Window;
typedef struct SDL_Window GuiWindow;
enum {
    MotionEvent = SDL_MOUSEMOTION,

};
#else
#define TimerEvent LASTEvent+1
#include <X11/Xlib.h>
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
typedef Window GuiWindow;
typedef struct Painter {
    Drawable drawable;
    GC gc;
} Painter;
typedef XEvent Event;
#define GET_X(a) ((a).xbutton.x)
#define GET_Y(a) ((a).xbutton.y)
//#define IS_BUTTON_RELEASE(a) ((a).type == ButtonRelease)
//#define IS_BUTTON_PRESS(a) ((a).type == ButtonPress)
//#define IS_KEY_PRESS(a) ((a).type == KeyPress)
//#define IS_MOTION(a) ((a).type == MotionNotify)
//#define IS_TIMER(a) ((a).type == TimerEvent)
#define GET_KEYSYM(a) (XLookupKeysym(&a.xkey, 0))
typedef enum GuiKeySym {
    right = XK_Right,
    enter = XK_Return,
    left = XK_Left,
    backspace = XK_BackSpace
} GuiKeySym;
#endif

typedef struct Painter Painter;
typedef struct Size Size;

GuiWindow guiMakeWindow();
Painter guiMakePainter(GuiWindow w);
void guiMoveResizeWindow(GuiWindow win, int x, int y, int w, int h);
void guiShowWindow(GuiWindow w);
void guiHideWindow(GuiWindow w);
void guiClearWindow(GuiWindow w);
#endif // XBACKEND_H
