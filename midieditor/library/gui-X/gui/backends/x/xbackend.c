#include "backend.h"
#include <X11/Xlib.h>
#include <assert.h>
#include "gui.h"
#include <math.h>
#include <X11/Xft/Xft.h>

extern Display * xdisplay;
extern char* appName;
//XFontStruct *xFontStruct;
extern XFontSet xFontSet;
struct _XftFont;
extern struct _XftFont *xFont;
extern Window rootWindow;
extern int xDepth;
extern int maxDigitWidth, maxDigitHeight;
extern _Bool  redraw;
extern Size rootWindowSize;
XEvent event;

Display * xdisplay = 0;
//XFontStruct *xFontStruct;
//XEvent xEvent;
XFontSet xFontSet;
Window rootWindow = 0;
struct _XftFont  *xFont;
int xDepth = 0;


void guiShowWindow(GuiWindow w) {
    XMapWindow(xdisplay, w);
}

void guiHideWindow(GuiWindow w) {
    XUnmapWindow(xdisplay, w);
}

void guiMoveResizwWindow(GuiWindow win, int x,int y,int w,int h) {
    XMoveResizeWindow(xdisplay,
                      win,x,y,w,h);
}

GuiWindow guiMakeWindow() {
    Window listWindow = XCreateSimpleWindow(xdisplay, rootWindow, 0, 0,
                                         700, 700, 0,
                                               WhitePixel(xdisplay,
                                                          DefaultScreen(xdisplay)),
                                               WhitePixel(xdisplay,
                                                       DefaultScreen(xdisplay)));
    XSelectInput(xdisplay, listWindow, ButtonReleaseMask);
    return listWindow;
}

Painter guiMakePainter(GuiWindow w) {
    GC gc = XCreateGC(xdisplay, w, 0,0);
    Painter res = {w,gc};
    return res;
}
_Bool guiSameWindow(Painter *p) {
    return event.xany.window == p->drawable;
}

void guiDrawLine(Painter *a, int b, int c, int d, int e)
{
    XDrawLine(xdisplay, a->drawable, a->gc, b, c, d, e);
}

void guiDrawRectangle(Painter *a, int b, int c, int d, int e)
{
    XDrawRectangle(xdisplay, a->drawable, a->gc, b, c, d, e);
}
void guiFillRectangle(Painter *a, int b, int c, int d, int e)
{
//    fprintf(stderr, "filling rect (%d, %d) %dx%d\n", b, c, d,e);
    XFillRectangle(xdisplay, a->drawable, a->gc, b, c, d, e);
}

void guiSetForeground(Painter *a, unsigned long b)
{
    XSetForeground(xdisplay, a->gc, b);
}

//void guiDrawTextWithLen(Painter *a, int b, int c, char *d, unsigned long e)
//{
//    Xutf8DrawString(xdisplay, a->drawable, xFontSet, a->gc, b, c, d, e);
//}

void guiSetSize(u32 w, u32 h)
{
    XResizeWindow(xdisplay, rootWindow, w, h);
}
void guiMoveResizeWindow(GuiWindow win, int x, int y, int w, int h) {
    XMoveResizeWindow(xdisplay,
                      win,
                      x,y, h, w);
}
void guiClearWindow(GuiWindow w) {
    XClearWindow(xdisplay, w);
}

Size guiDrawText(Painter* p, const char *text, int len, Point pos,
                 i32 color) {
    XRenderColor            xrcolor;
    XftColor                xftcolor;

    XGlyphInfo extents;
    XftTextExtentsUtf8( xdisplay, xFont, (XftChar8 *)text, len, &extents );

    /* Xft text color */
    xrcolor.red = (color >> 16)&0xff * 256;
    xrcolor.green = (color >> 8)&0xff * 256;
    xrcolor.blue = (color >> 0)&0xff * 256;
    xrcolor.alpha = 0xffff;
    XftColorAllocValue( xdisplay, DefaultVisual(xdisplay,DefaultScreen(xdisplay)),
                DefaultColormap( xdisplay, DefaultScreen(xdisplay) ), &xrcolor, &xftcolor );
    XftDraw                 *xftdraw;

    /* Xft draw context */
    xftdraw = XftDrawCreate( xdisplay, p->drawable, DefaultVisual(xdisplay,DefaultScreen(xdisplay)),
                DefaultColormap( xdisplay, DefaultScreen(xdisplay) ) );
    XftDrawStringUtf8( xftdraw, &xftcolor, xFont, pos.x, pos.y, (XftChar8 *)text, len );
    Size res = {extents.width,extents.height};
    return res;
}

void guiDrawImage(Painter* p, IMAGE* i, int x, int y) {
    XPutImage(xdisplay, p->drawable, p->gc, i,
              0,0, x,y,
              i->width, i->height);
}


Size guiTextExtents(/*Painter*p,*/const char*text,int len) {
    XGlyphInfo extents;
    XftTextExtentsUtf8( xdisplay, xFont, (XftChar8 *)text, len, &extents );
    Size res = {extents.width,extents.height};
    return res;
}


void guiStartDrawing(/*const char* appName*/) {

    xdisplay = XOpenDisplay(NULL);
    assert(xdisplay);

    char **missingList;
    int missingCount;
    char *defString;
    xFontSet =
        XCreateFontSet(xdisplay,  "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
                       &missingList, &missingCount, &defString);
    xFont = XftFontOpenName( xdisplay, DefaultScreen( xdisplay ), "morpheus-12" );
    if (xFontSet == NULL) {
        fprintf(stderr, "Failed to create fontset\n");
        abort();
    }
    XFreeStringList(missingList);


    for(char d = '0'; d < '9'; d++) {
        XRectangle overallInk;
        XRectangle overallLog;
        Xutf8TextExtents(xFontSet, &d, 1, &overallInk, &overallLog);
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
    xDepth = DefaultDepth(xdisplay, screen);

//    int numberOfDepths;
//    int* depths = XListDepths(xdisplay, screen, &numberOfDepths);
    printf("Depth %d\n", xDepth);
    // Create the window
    rootWindow = XCreateSimpleWindow(xdisplay , DefaultRootWindow(xdisplay ), 0, 0,
                                        700, 700, 0, blackColor, blackColor);
//    XSetWindowAttributes ats = {
//     None, blackColor, CopyFromParent, whiteColor,
//        ForgetGravity, NorthWestGravity, NotUseful,
//        -1, 0, False, 0, 0, False, CopyFromParent, None
//    };
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
        event.type = Expose;
        return;
    }

    if (XPending(xdisplay) || wait_fd(ConnectionNumber(xdisplay),0.530)) {
       XNextEvent(xdisplay, &event);
       if(event.xany.window != rootWindow) {
            fprintf(stderr, "got wrong event %d %lud\n", event.type, event.xany.window );
           abort();
       }

        if(event.type == ConfigureNotify) {
            Size newSize = {
                event.xconfigure.width,
                event.xconfigure.height};
            rootWindowSize = newSize;
//            fprintf(stderr, "reconfigure  %d x %d\n!!",
//                    newSize.width,
//                                          newSize.height);;
        }
       return;
    } else {
        event.type = TimerEvent;
    }
}
