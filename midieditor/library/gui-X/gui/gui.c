#include "gui.h"
#include "guiglobals.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>

Point defaultGetPos() {
    fprintf(stderr, "getPos not set");
    abort();
}
void defaultFeedbackSize(Size s) {
    (void)s;
}

Point (*getPos)() = defaultGetPos;
void (*feedbackSize)(Size) = defaultFeedbackSize;

#warning remove memory leak
void guiLabelWithBackground(Painter* p, char *text, int len, bool back) {
    Point pos = getPos();
    Size extents = guiTextExtents(text, len);


    Size size = {extents.width + 10,
                extents.height + 10};

    if(!(event.type == MotionEvent)) {
//        XRenderColor            xrcolor;
//        XftColor                xftcolor;
//        /* Xft text color */
//        xrcolor.red = 0xffff;
//        xrcolor.green = 0xffff;
//        xrcolor.blue = 0xffff;
//        xrcolor.alpha = 0xffff;
//        XftColorAllocValue( xdisplay, DefaultVisual(xdisplay,DefaultScreen(xdisplay)),
//                    DefaultColormap( xdisplay, DefaultScreen(xdisplay) ), &xrcolor, &xftcolor );
//        XftDraw                 *xftdraw;

//        /* Xft draw context */
//        xftdraw = XftDrawCreate( xdisplay, rootWindow, DefaultVisual(xdisplay,DefaultScreen(xdisplay)),
//                    DefaultColormap( xdisplay, DefaultScreen(xdisplay) ) );
        if(back) {
            guiSetForeground(p,0);
            guiFillRectangle(p, pos.x, pos.y, size.width, size.height);
        }
//        guiSetForeground(p, WhitePixel(xdisplay,
//                                                   DefaultScreen(xdisplay)));
//        fprintf(stderr, "printing label %s %d %d\n",
//                text, pos.x+5 + overallLog.x, pos.y+5 - overallLog.y);
//        XftDrawStringUtf8( xftdraw, &xftcolor, xFont, pos.x+5 , pos.y+5 + extents.height, (XftChar8 *)text, len );
        guiDrawText(p, text, len, STRU(Point, pos.x+5,pos.y+5+ extents.height),0xffffffff);
    }
    feedbackSize(size);
}

void guiLabelZTWithBackground(Painter* p, char *text, bool back) {
    guiLabelWithBackground(p, text, strlen(text), back);
}
void guiLabel(Painter* p, char *text, int len)
{
    guiLabelWithBackground(p,text,len,false);
}
void guiLabelZT(Painter* p, char *text) {
    guiLabelZTWithBackground(p, text, false);
}
bool guiButton(Painter *p, char* text, int len)
{
    if(!guiSameWindow(p)) return false;
    Point pos = getPos();
    Size overallLog = guiTextExtents(text, len);
    Size size = {overallLog.width + 10,
                overallLog.height + 10};
    if(!(event.type == MotionEvent)) {
        guiSetForeground(p, 0xff555555);
//        XSetBackground(xdisplay, l->gc, 0xffff5555);
//        fprintf(stderr, "filling rect (%d, %d) %dx%d\n", pos.x, pos.y,
//                size.width, size.height);
        guiFillRectangle(p, pos.x, pos.y,
                       size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        guiDrawText(p, text, len, STRU(Point,
                    pos.x+5 /*+ overallLog.x*/, pos.y+5 /*+ overallLog.height*/),
                    0xffffffff);
    }
//    XPutImage(xdisplay, l->window, l->gc, l->x+5, l->y+5, width, height);
    bool res = false;
    if(event.type == ButtonRelease) {
        int mx = GET_X(event);
        int my = GET_Y(event);
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
bool guiToolButton(Painter *p, Image *i, bool *consume) {
    return guiToolButtonA(p,i,false,consume);
}
bool guiToolButtonA(Painter *p, Image *i, bool active, bool *consume) {
//    if(consume) *consume = false;
    if(!guiSameWindow(p)) return false;
    volatile Point pos = getPos();
    Size size = {i->IMAGE_WIDTH+2,
                 i->IMAGE_HEIGHT+2};
    if(!(event.type == MotionEvent)) {
//        if(active) {
            guiSetForeground(p, active?0xffff9999:0xff000000);
            guiFillRectangle(p, pos.x, pos.y, size.width, size.height);
//        }
        guiDrawImage(p, i, pos.x+1, pos.y+1);
    }
    bool res = false;
    if(event.type == ButtonRelease
            || event.type == ButtonPress) {
        int mx = GET_X(event);
        int my = GET_Y(event);
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            if(event.type == ButtonRelease) {
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
    //    int numberOfDigits = 0;
//    {
//        int curNumber = *number;
//        while(curNumber) {
//            numberOfDigits++;
//            curNumber/=10;
//        }
//        if(*number <= 0) {
//            numberOfDigits++;
//        }
//    }
    bool res = false;
    void commit()
    {
        context.active = 0;
        context.editedString[context.editedStringLen] = 0;
        sscanf(context.editedString, "%d", number);
        res = true;
    }
//    fprintf(stderr, "%d digits", numberOfDigits);
    if(event.type == KeyPress && context.active == number) {
        if(consume) *consume = true;
        GuiKeySym sym = GET_KEYSYM(event);
        fprintf(stderr, "%c %c! \n", (int)sym, right);
        if(sym == right) {
            if(context.pos < context.editedStringLen) {
                context.pos++;
            }
            cursor = true;
        } else if(sym == enter) {
            commit();
        } else if(sym == left) {
            if(context.pos > 0) {
                context.pos--;
            }
            cursor = true;
        } else if(sym == backspace) {
            if(context.pos == 0) {
                goto keyPressBreak;
            }
            for(int i = context.pos; i < context.editedStringLen-1; i++) {
                context.editedString[i] = context.editedString[i+1];
            }
            context.editedStringLen--;
            context.pos--;
//                res = true;
        } else if((sym >= '0' && sym <= '9') || sym == '-') {
            if(context.editedStringLen == MAX_DIGITS) {
                goto keyPressBreak;
            }
            for(int i = context.editedStringLen-1; i > context.pos; i--) {
                context.editedString[i] = context.editedString[i-1];
            }
            context.editedStringLen++;
            context.editedString[context.pos] = sym;
            context.pos++;
//                res = true;
        }
    }
    keyPressBreak:

    if(event.type == ButtonPress || event.type == ButtonRelease) {
        int mx = GET_X(event);
        int my = GET_Y(event);
//        fprintf(stderr, "%d", *consume);
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            if(consume) *consume = true;
            fprintf(stderr, "fwefwefw!\n");
            if(context.active != number) {
                context.editedStringLen = snprintf(context.editedString,
                         MAX_DIGITS,
                         "%d",
                         *number);
            }
            context.active = number;
//            fprintf(stderr, "%d", *consume);
            int newPos = (mx - pos.x - 5)/maxDigitWidth;
            context.pos = MIN(MAX(newPos, 0), context.editedStringLen);
        } else if(context.active == number) {
            commit();
        }
    }
    if(!(event.type == MotionEvent)) {
        guiSetForeground(p, 0xff333333);
//        fprintf(stderr, "filling rect (%d, %d) %dx%d\n", pos.x, pos.y,
//                size.width, size.height);
        guiFillRectangle(p, pos.x, pos.y,
                       size.width, size.height);
//        fprintf(stderr, "%d", *consume);
        if(context.active == number) {
            guiSetForeground(p, 0xff0000ff);
        } else {
            guiSetForeground(p, 0xffffffff);
        }
        guiDrawRectangle(p, pos.x, pos.y,
                       size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        char stringN[20];
        char* string;
        if(context.active == number) {
            string = context.editedString;
            guiDrawText(p, context.editedString, context.editedStringLen,
                     STRU(Point, pos.x+5, pos.y+5 + maxDigitHeight), 0xffffffff);
        } else {
            string = stringN;
            int len = snprintf(stringN, 20, "%d", *number);
            guiDrawText(p, stringN, len,
                     STRU(Point, pos.x+5, pos.y+5 + maxDigitHeight), 0xffffffff);
        }
        if(cursor && context.active == number) {
            Size overallInk = guiTextExtents(string, context.pos);//, &overallInk, &overallLog);
            guiSetForeground(p, 0xffffffff);
            guiDrawLine(p,
                      pos.x + 5 + overallInk.width, pos.y + 5,
                      pos.x + 5 + overallInk.width, pos.y + 5 + maxDigitHeight);
//            fprintf(stderr, "%d", *consume);

        }
        if(event.type == TimerEvent) {
            cursor = !cursor;
        }
  }
    feedbackSize(size);
//    fprintf(stderr, "%d", *consume);
    return res;
}

extern const char* appName;
const char* __attribute__((weak))  appName = "gui application";



Size guiGetSize()
{
    return rootWindowSize;
   /* Window w;
    int x, y;
    Size s;
    uint bw, d;
    XGetGeometry(xdisplay, rootWindow, &w,
                 &x, &y,
                 &s.width, &s.height,
                 &bw, &d);
    return s;*/
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

    if(event.type == ButtonPress) {
        int mx = GET_X(event);
        int my = GET_Y(event);
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
        guiFillRectangle(p, pos.x, pos.y,
                       size.width, size.height);
        if(context.active == number) {
            guiSetForeground(p, 0xff0000ff);
        } else {
            guiSetForeground(p, 0xffffffff);
        }
        guiDrawRectangle(p, pos.x, pos.y,
                       size.width, size.height);
        guiSetForeground(p, 0xffffffff);
        char string[20];
        char format[20];
        snprintf(format, 20, "%%%dd", numberOfDigits);

        int len = snprintf(string, 20, format, *number);
        guiDrawText(p, string, len, STRU(Point, pos.x+5 /*+ maxDigitWidth*digits*/,
                        pos.y+5 + maxDigitHeight), 0xffffffff);
        if(cursor && context.active == number) {
            Size overallInk = guiTextExtents(string,
                                             context.pos);
            guiSetForeground(p, 0xffffffff);
            guiDrawLine(p,
                      pos.x + 5 + overallInk.width, pos.y + 5,
                      pos.x + 5 + overallInk.width, pos.y + 5 + maxDigitHeight);
        }
        if(event.type == TimerEvent) {
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
    if(x+w >= (int)(p->w)) {
        w = p->w - x - 1;
    }
    if(y+h+1 >= (int)(p->h)) {
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


Size guiDrawTextZT(Painter *p, char *text, Point pos, i32 color)
{
    return guiDrawText(p, text, strlen(text), pos, color);
}

bool guiCheckBox(Painter *p, bool *v)
{
    Point pos = getPos();
    Size size = {10,
                 10};
    bool res = false;
    if(event.type == ButtonPress) {
        int mx = GET_X(event);
        int my = GET_Y(event);
        if(mx >= pos.x && mx <= pos.x + (int)size.width &&
            my >= pos.y && my <= pos.y + (int)size.height) {
            *v = !*v;
            res = true;
        }
    }

    if(*v) {
        guiDrawLine(p, pos.x, pos.y+size.height,
                  pos.x+size.width, pos.y);
        guiDrawLine(p, pos.x, pos.y,
                  pos.x+size.width, pos.y+size.height);
    } else {
        guiSetForeground(p, 0xff000000);
        guiFillRectangle(p, pos.x, pos.y,
                       size.width, size.height);
    }
    guiSetForeground(p, 0xffffffff);
    guiDrawRectangle(p, pos.x, pos.y,
                   size.width, size.height);
    feedbackSize(size);
    return res;
}

_Bool pointInRect(Point p, Rect r)
{
    return p.x >= r.x && p.x <= r.x+(i32)r.width &&
            p.y >= r.y && p.y <= r.y +(i32)r.height;
}
