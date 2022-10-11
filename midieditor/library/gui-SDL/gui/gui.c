#include "gui.h"
#include "guiglobals.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
//#define XK_LATIN1
//#define XK_MISCELLANY
#include <SDL2/SDL_keycode.h>
#include <sys/select.h>
#include <stdlib.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

Point defaultGetPos() {
    fprintf(stderr, "getPos not set");
    abort();
}
void defaultFeedbackSize(Size s) {
    (void)s;
}

Point (*getPos)() = defaultGetPos;
void (*feedbackSize)(Size) = defaultFeedbackSize;

void guiDrawLine(Painter *a, int b, int c, int d, int e)
{
    SDL_RenderDrawLine(a->rend, b, c, d, e);
}

void guiDrawRectangle(Painter *a, int b, int c, int d, int e)
{
    SDL_Rect r = {b,c,d,e};
    SDL_RenderDrawRect(a->rend, &r);
}
void guiFillRectangle(Painter *a, int b, int c, int d, int e)
{
    SDL_Rect r = {b,c,d,e};
//    fprintf(stderr, "filling rect (%d, %d) %dx%d\n", b, c, d,e);
    SDL_RenderFillRect(a->rend, &r);
}

void guiSetForeground(Painter *a, unsigned long b)
{
    SDL_Color c = {b >> 16, b >> 8, b >> 0, b >> 24};
    SDL_SetRenderDrawColor(a->rend, c.r,c.g,c.b,c.a);
    a->textColor=c;
}

void guiDrawTextZT(Painter *a, int b, int c, char *d)
{
// do caching
    SDL_Surface * surface = TTF_RenderUTF8_Solid(sdlFontSet, d, a->textColor);
    SDL_Texture * texture = SDL_CreateTextureFromSurface(a->rend, surface);
    SDL_Rect rect = {b,c,surface->w,surface->h};
    SDL_RenderCopy(a->rend, texture, 0, &rect);
//    xdisplay, a->drawable, sdlFontSet, a->rend, b, c, d, e);
}

void guiSetSize(SDL_Window *win, uint w, uint h)
{
    SDL_SetWindowSize(win, w, h);
}

void guiLabel(Painter* p, char *text, int len)
{
    Point pos = getPos();
    SDL_Rect overallInk;
//    SDL_Rect overallLog;

    TTF_SizeUTF8(sdlFontSet, text, &overallInk.w, &overallInk.h);//, &overallLog);
    Size size = {overallInk.w + 10,
                overallInk.h + 10};

//    XDrawRectangle(xdisplay, p->window, p->gc, pos.x, pos.y,
//                   size.width, size.height);
    if(sdlEvent.type != SDL_MOUSEMOTION) {
//        SDL_SetRenderDrawColor(p->rend, 255,255,255,255);
        SDL_Color c = {255,255,255,255};
        p->textColor = c;
//        fprintf(stderr, "printing label %s %d %d\n",
//                text, pos.x+5 + overallLog.x, pos.y+5 - overallLog.y);
        guiDrawTextZT(p, pos.x+5,pos.y+5, text);
        //p->drawable, sdlFontSet, p->rend,
        //             + overallLog.x,  - overallLog.y, text);
    }
    feedbackSize(size);
}
void guiLabelZT(Painter* p, char *text) {
    guiLabel(p, text, strlen(text));
}
bool guiButton(Painter *p, char* text, int len)
{
//    if(sdlEvent.xany.window != p->drawable) return false;
    Point pos = getPos();
    SDL_Rect overallInk;
//    XRectangle overallLog;

    TTF_SizeUTF8(sdlFontSet, text, &overallInk.w, &overallInk.h);
    Size size = {overallInk.w + 10,
                overallInk.h + 10};
    if(sdlEvent.type != SDL_MOUSEMOTION) {
        guiSetForeground(p, 0xff555555);
//        XSetBackground(xdisplay, l->gc, 0xffff5555);
//        fprintf(stderr, "filling rect (%d, %d) %dx%d\n", pos.x, pos.y,
//                size.width, size.height);
        guiFillRectangle(p, pos.x, pos.y,size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        guiDrawTextZT(p,pos.x+5 /*+ overallLog.x*/, pos.y+5 /*- overallLog.y*/, text);
    }
//    XPutImage(xdisplay, l->window, l->gc, l->x+5, l->y+5, width, height);
    bool res = false;
    if(sdlEvent.type == SDL_MOUSEBUTTONUP) {
        int mx = sdlEvent.button.x;
        int my = sdlEvent.button.y;
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            res = true;
        }
    }
//    p->x += width+5;
//    p->maxHeight = MAX(p->maxHeight, height);
    feedbackSize(size);
    return res;
}
bool guiButtonZT(Painter* p, char *text) {
    return guiButton(p, text, strlen(text));
}
bool guiToolButton(Painter *p, SDL_Texture *i, bool *consume) {
//    if(consume) *consume = false;
//    if(sdlEvent.xany.window != p->drawable) return false;
    Point pos = getPos();
    Size size;/* = {i->w,
                 i->h};*/
    SDL_QueryTexture(i,0,0,&size.width,&size.height);
    if(sdlEvent.type != SDL_MOUSEMOTION) {
        SDL_Rect r = {pos.x, pos.y,
                      size.width, size.height};
        SDL_RenderCopy(p->rend, i,
                  0,&r );
    }
    bool res = false;
    if(sdlEvent.type == ButtonRelease
            || sdlEvent.type == ButtonPress) {
        int mx = sdlEvent.xbutton.x;
        int my = sdlEvent.xbutton.y;
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            if(sdlEvent.type == ButtonRelease) {
                res = true;
            }
            if(consume) *consume = true;
        }
    }
    feedbackSize(size);
    return res;
}

bool guiNumberEdit(Painter *p, int digits, int *number, bool *consume) {
//    if(consume) *consume = false;
//    fprintf(stderr, "%d", *consume);
    Point pos = getPos();
    static bool cursor = false;

    Size size = {maxDigitWidth*digits + 10,
                maxDigitHeight + 10};

    int numberOfDigits = 0;
    {
        int curNumber = *number;
        while(curNumber) {
            numberOfDigits++;
            curNumber/=10;
        }
        if(*number <= 0) {
            numberOfDigits++;
        }
    }
    bool res = false;

    if(sdlEvent.type == KeyPress && context.active == number) {
        if(consume) *consume = true;
        KeySym sym = XLookupKeysym(&sdlEvent.xkey, 0);
        fprintf(stderr, "%c %c! \n", (int)sym, SDLK_RIGHT);
        if(sym == SDLK_RIGHT) {
            if(context.pos < numberOfDigits) {
                context.pos++;
            }
            cursor = true;
        } else if(sym == SDLK_LEFT) {
            if(context.pos > 0) {
                context.pos--;
            }
            cursor = true;
        } else if(sym >= '0' && sym <= '9' || sym == SDLK_BACKSPACE) {
            if(context.pos == 0 && *number < 0) {
                goto keyPressBreak;
            }
            int valueExponent = numberOfDigits - context.pos;
            int value = 1;
            for(int i = 0; i < valueExponent; i++) {
                value*=10;
            }
            if(sym == SDLK_BACKSPACE) {
                if(context.pos == 0) {
                    goto keyPressBreak;
                } else if(context.pos == 1  && *number < 0) {
                    *number = -*number;
                    res = true;
                } else {
                    *number = (*number)/value/10*value + (*number)%value;
                    res = true;
                }
                if(context.pos > 0) {
                    context.pos--;
                }
            } else {
                if(*number == 0) context.pos--;
                bool neg = *number < 0;
                if(neg) *number = -*number;
                *number = (*number)/value*value*10 +
                 (sym-'0')*value + (*number)%value;
                if(neg) *number = -*number;
                context.pos++;
                res = true;
            }
        } else if(sym == '-') {
            if(context.pos == 0 && *number > 0) {
                *number = -*number;
                res = true;
            }
        }
    }
    keyPressBreak:

    if(sdlEvent.type == ButtonPress || sdlEvent.type == ButtonRelease) {
        int mx = sdlEvent.xbutton.x;
        int my = sdlEvent.xbutton.y;
//        fprintf(stderr, "%d", *consume);
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            if(consume) *consume = true;
            fprintf(stderr, "fwefwefw!\n");
            context.active = number;
//            fprintf(stderr, "%d", *consume);
            int newPos = (mx - pos.x - 5)/maxDigitWidth;
            context.pos = newPos > numberOfDigits? numberOfDigits:
                          newPos > 0 ? newPos :
                          0;
        } else if(context.active == number) {
            context.active = 0;
        }
    }
    if(sdlEvent.type != SDL_MOUSEMOTION) {
        guiSetForeground(p, 0xff333333);
//        fprintf(stderr, "filling rect (%d, %d) %dx%d\n", pos.x, pos.y,
//                size.width, size.height);
        guiFillRectangle(p, pos.x, pos.y,size.width, size.height);
//        fprintf(stderr, "%d", *consume);
        if(context.active == number) {
            guiSetForeground(p, 0xff0000ff);
        } else {
            guiSetForeground(p, 0xffffffff);
        }
        XDrawRectangle(xdisplay, p->drawable, p->rend, pos.x, pos.y,
                       size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        char string[20];
        int len = snprintf(string, 20, "%d", *number);
        guiDrawTextZT(p,pos.x+5 /*+ maxDigitWidth*digits*/,pos.y+5 + maxDigitHeight, string);
        if(cursor && context.active == number) {
            XRectangle overallInk;
            XRectangle overallLog;

            Xutf8TextExtents(sdlFontSet, string, context.pos, &overallInk, &overallLog);
            guiSetForeground(p, 0xffffffff);
            XDrawLine(xdisplay, p->drawable, p->rend,
                      pos.x + 5 + overallInk.width, pos.y + 5,
                      pos.x + 5 + overallInk.width, pos.y + 5 + maxDigitHeight);
//            fprintf(stderr, "%d", *consume);

        }
        if(sdlEvent.type == TimerEvent) {
            cursor = !cursor;
        }
  }
    feedbackSize(size);
//    fprintf(stderr, "%d", *consume);

    return res;
}

//extern const char* appName;
void guiStartDrawing(const char* appName) {

    xdisplay = XOpenDisplay(NULL);
    assert(xdisplay);

    char **missingList;
    int missingCount;
    char *defString;
    sdlFontSet =
        XCreateFontSet(xdisplay,  "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
                       &missingList, &missingCount, &defString);
    if (sdlFontSet == NULL) {
        fprintf(stderr, "Failed to create fontset\n");
        abort();
    }
    XFreeStringList(missingList);


    for(char d = '0'; d < '9'; d++) {
        XRectangle overallInk;
        XRectangle overallLog;
        Xutf8TextExtents(sdlFontSet, &d, 1, &overallInk, &overallLog);
        if(maxDigitWidth < overallLog.width) {
            maxDigitWidth = overallLog.width;
        }
        if(maxDigitHeight < overallLog.height) {
            maxDigitHeight = overallLog.height;
        }
    }


    int screen = DefaultScreen(xdisplay);
    int blackColor = BlackPixel(xdisplay, screen );
    int whiteColor = WhitePixel(xdisplay, screen );
    sdlDepth = DefaultDepth(xdisplay, screen);

    int numberOfDepths;
    int* depths = XListDepths(xdisplay, screen, &numberOfDepths);
    printf("Depth %d\n", sdlDepth);
    // Create the window
    rootWindow = XCreateSimpleWindow(xdisplay , DefaultRootWindow(xdisplay ), 0, 0,
                                        700, 700, 0, blackColor, blackColor);
    XSetWindowAttributes ats = {
     None, blackColor, CopyFromParent, whiteColor,
        ForgetGravity, NorthWestGravity, NotUseful,
        -1, 0, False, 0, 0, False, CopyFromParent, None
    };
    // We want to get MapNotify events
    XSelectInput(xdisplay, rootWindow, StructureNotifyMask | ButtonPressMask
                 | ExposureMask | KeyPressMask
                 | ButtonMotionMask | ButtonReleaseMask);

    XStoreName(xdisplay, rootWindow, appName);
    // "Map" the window (that is, make it appear on the screen)
    XMapWindow(xdisplay , rootWindow);

    // Create a "Graphics Context"
    GC gc = XCreateGC(xdisplay , rootWindow, 0, NULL);
//    xFontStruct = XLoadQueryFont(xdisplay, "fixed");
    // Tell the GC we draw using the white color
    XSetForeground(xdisplay , gc, whiteColor);
    // Wait for the MapNotify event
    for(;;) {
        XEvent e;
        XNextEvent(xdisplay, &e);
        if (e.type == MapNotify)
            break;
    }

    XFlush(xdisplay);

}

static int wait_fd(int fd, double seconds)
{
    struct timeval tv;
    fd_set in_fds;
    FD_ZERO(&in_fds);
    FD_SET(fd, &in_fds);
    tv.tv_sec = trunc(seconds);
    tv.tv_usec = (seconds - trunc(seconds))*1000000;
    return select(fd+1, &in_fds, 0, 0, &tv);
}

void guiNextEvent()
{
    if(redraw) {
        redraw = false;
        sdlEvent.type = Expose;
        return;
    }

    if (XPending(xdisplay) || wait_fd(ConnectionNumber(xdisplay),0.530)) {
       XNextEvent(xdisplay, &sdlEvent);
       if(sdlEvent.xany.window != rootWindow) {
            fprintf(stderr, "got wrong event %ud %ud\n", sdlEvent.type, sdlEvent.xany.window );
           abort();
       }

        if(sdlEvent.type == ConfigureNotify) {
            Size newSize = {
                sdlEvent.xconfigure.width,
                sdlEvent.xconfigure.height};
            rootWindowSize = newSize;
//            fprintf(stderr, "reconfigure  %d x %d\n!!",
//                    newSize.width,
//                                          newSize.height);;
        }
       return;
    } else {
        sdlEvent.type = TimerEvent;
    }
}

Size guiGetSize()
{
    return rootWindowSize;
    Window w;
    int x, y;
    Size s;
    uint bw, d;
    XGetGeometry(xdisplay, rootWindow, &w,
                 &x, &y,
                 &s.width, &s.height,
                 &bw, &d);
    return s;
}


void guiRedraw()
{
    redraw = true;
}


void guiDoubleEdit(Painter *p, int digits, double *number)
{
    Point pos = getPos();
    static bool cursor = false;

    Size size = {maxDigitWidth*digits + 10,
                maxDigitHeight + 10};

    int numberOfDigits = 5;

    if(sdlEvent.type == ButtonPress) {
        int mx = sdlEvent.xbutton.x;
        int my = sdlEvent.xbutton.y;
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            fprintf(stderr, "fwefwefw!\n");
            context.active = number;
            int newPos = (mx - pos.x - 5)/maxDigitWidth;
            context.pos = newPos > numberOfDigits? numberOfDigits:
                          newPos > 0 ? newPos :
                          0;
        } else if(context.active == number) {
            context.active = 0;
        }
    }
//    if(xEvent.type == Expose || ) {
        guiSetForeground(p, 0xff333333);
//        fprintf(stderr, "filling rect (%d, %d) %dx%d\n", pos.x, pos.y,
//                size.width, size.height);
        guiFillRectangle(p, pos.x, pos.y, size.width, size.height);
        if(context.active == number) {
            guiSetForeground(p, 0xff0000ff);
        } else {
            guiSetForeground(p, 0xffffffff);
        }
        XDrawRectangle(xdisplay, p->drawable, p->rend, pos.x, pos.y,
                       size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        char string[20];
        char format[20];
        snprintf(format, 20, "%%%dd", numberOfDigits);

        int len = snprintf(string, 20, format, *number);
        guiDrawTextZT(p,pos.x+5 /*+ maxDigitWidth*digits*/,pos.y+5 + maxDigitHeight, string);
        if(cursor && context.active == number) {
            XRectangle overallInk;
            XRectangle overallLog;

            Xutf8TextExtents(sdlFontSet, string, context.pos, &overallInk, &overallLog);
            guiSetForeground(p, 0xffffffff);
            XDrawLine(xdisplay, p->drawable, p->rend,
                      pos.x + 5 + overallInk.width, pos.y + 5,
                      pos.x + 5 + overallInk.width, pos.y + 5 + maxDigitHeight);
        }
        if(sdlEvent.type == TimerEvent) {
            cursor = !cursor;
        }
//  }
    feedbackSize(size);
}

#pragma GCC push_options
#pragma GCC optimize ("Ofast")
void guiFillRawRectangle(RawPicture *p, int x, int y, int w, int h, char r, char g, char b)
{
    assert(w >= 0);
    assert(h >= 0);
    assert(x >= 0);
    assert(y >= 0);
    if(x+w >= p->w) {
        w = p->w - x - 1;
    }
    if(y+h+1 >= p->h) {
        h = p->h - y - 2;
    }
    int color = rgb(r,g,b);
    for(int i = y; i <= y + h+1; i++) {
        for(int j = x; j <= x+w; j++) {
            *((int*)(p->data) + i*p->w + j) = color;
        }
    }
}
#pragma GCC pop_options

