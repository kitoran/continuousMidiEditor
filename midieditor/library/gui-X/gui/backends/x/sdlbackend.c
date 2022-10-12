#include "backend.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <assert.h>
#include "gui.h"
#include <math.h>
// i'm so sorry
//typedef struct
//{
//    float x;
//    float y;
//} SDL_FPoint;

//typedef struct
//{
//    float x;
//    float y;
//    float w;
//    float h;
//} SDL_FRect;

//struct SDL_Renderer
//{
//    const void *magic;

//    void (*WindowEvent) (SDL_Renderer * renderer, const SDL_WindowEvent *event);
//    int (*CreateTexture) (SDL_Renderer * renderer, SDL_Texture * texture);
//    int (*SetTextureColorMod) (SDL_Renderer * renderer,
//                               SDL_Texture * texture);
//    int (*SetTextureAlphaMod) (SDL_Renderer * renderer,
//                               SDL_Texture * texture);
//    int (*SetTextureBlendMode) (SDL_Renderer * renderer,
//                                SDL_Texture * texture);
//    int (*UpdateTexture) (SDL_Renderer * renderer, SDL_Texture * texture,
//                          const SDL_Rect * rect, const void *pixels,
//                          int pitch);
//    int (*LockTexture) (SDL_Renderer * renderer, SDL_Texture * texture,
//                        const SDL_Rect * rect, void **pixels, int *pitch);
//    void (*UnlockTexture) (SDL_Renderer * renderer, SDL_Texture * texture);
//    int (*SetRenderTarget) (SDL_Renderer * renderer, SDL_Texture * texture);
//    int (*UpdateViewport) (SDL_Renderer * renderer);
//    int (*RenderClear) (SDL_Renderer * renderer);
//    int (*RenderDrawPoints) (SDL_Renderer * renderer, const SDL_FPoint * points,
//                             int count);
//    int (*RenderDrawLines) (SDL_Renderer * renderer, const SDL_FPoint * points,
//                            int count);
//    int (*RenderFillRects) (SDL_Renderer * renderer, const SDL_FRect * rects,
//                            int count);
//    int (*RenderCopy) (SDL_Renderer * renderer, SDL_Texture * texture,
//                       const SDL_Rect * srcrect, const SDL_FRect * dstrect);
//    int (*RenderCopyEx) (SDL_Renderer * renderer, SDL_Texture * texture,
//                       const SDL_Rect * srcquad, const SDL_FRect * dstrect,
//                       const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
//    int (*RenderReadPixels) (SDL_Renderer * renderer, const SDL_Rect * rect,
//                             Uint32 format, void * pixels, int pitch);
//    void (*RenderPresent) (SDL_Renderer * renderer);
//    void (*DestroyTexture) (SDL_Renderer * renderer, SDL_Texture * texture);

//    void (*DestroyRenderer) (SDL_Renderer * renderer);

//    int (*GL_BindTexture) (SDL_Renderer * renderer, SDL_Texture *texture, float *texw, float *texh);
//    int (*GL_UnbindTexture) (SDL_Renderer * renderer, SDL_Texture *texture);

//    /* The current renderer info */
//    SDL_RendererInfo info;

//    /* The window associated with the renderer */
//    SDL_Window *window;
//    SDL_bool hidden;
//    SDL_bool resized;

//    /* The logical resolution for rendering */
//    int logical_w;
//    int logical_h;
//    int logical_w_backup;
//    int logical_h_backup;

//    /* The drawable area within the window */
//    SDL_Rect viewport;
//    SDL_Rect viewport_backup;

//    /* The render output coordinate scale */
//    SDL_FPoint scale;
//    SDL_FPoint scale_backup;

//    /* The list of textures */
//    SDL_Texture *textures;
//    SDL_Texture *target;

//    Uint8 r, g, b, a;                   /**< Color for drawing operations values */
//    SDL_BlendMode blendMode;            /**< The drawing blend mode */

//    void *driverdata;
//};
extern char* appName;
//XFontStruct *xFontStruct;
extern int xDepth;
extern int maxDigitWidth, maxDigitHeight;
extern _Bool  redraw;
extern Size rootWindowSize;
Event event;

TTF_Font* font;
SDL_Window* rootWindow = 0;


void guiShowWindow(GuiWindow w) {
    SDL_ShowWindow(w);
}

void guiHideWindow(GuiWindow w) {
    SDL_HideWindow(w);
}

void guiMoveResizeWindow(GuiWindow win, int x,int y,int w,int h) {
    SDL_SetWindowSize(win,w,h);//SDL_absMoveResizeWindow(xdisplay,
    SDL_SetWindowPosition(win,x,y);
}

GuiWindow guiMakeWindow() {
    GuiWindow listWindow = SDL_CreateWindow("", 0, 0,
                                         700, 700, 0);
//    XSelectInput(xdisplay, listWindow, ButtonReleaseMask);
    return listWindow;
}

Painter guiMakePainter(GuiWindow w) {
    Painter p = {SDL_CreateRenderer(w, -1,0),
                 SDL_GetWindowSurface(w),
                 w};
    return p;
}

_Bool guiSameWindow(Painter *p) {
    u32 wid = SDL_GetWindowID(p->window);
    switch(event.type) {
    case SDL_DROPBEGIN:
    case SDL_DROPCOMPLETE:
    case SDL_DROPTEXT:
    case SDL_DROPFILE:
        return wid == event.drop.windowID;
    case SDL_WINDOWEVENT:
        return wid == event.window.windowID;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        return wid == event.key.windowID;
    case SDL_TEXTINPUT:
        return wid == event.text.windowID;
    case SDL_TEXTEDITING:
        return wid == event.edit.windowID;
    case SDL_MOUSEMOTION:
        return wid == event.motion.windowID;
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
        return wid == event.edit.windowID;
    }
}

void guiDrawLine(Painter *a, int b, int c, int d, int e)
{
    SDL_RenderDrawLine(a->gc, b, c, d, e);
}

void guiDrawRectangle(Painter *a, int b, int c, int d, int e)
{
    SDL_Rect r = {b,c,d,e};
    SDL_RenderDrawRect(a->gc, &r);
}
void guiFillRectangle(Painter *a, int b, int c, int d, int e)
{
    SDL_Rect r = {b,c,d,e};
//    fprintf(stderr, "filling rect (%d, %d) %dx%d\n", b, c, d,e);
    SDL_RenderFillRect(a->gc, &r);
}

void guiSetForeground(Painter *a, unsigned long b)
{
    SDL_SetRenderDrawColor(a->gc, (b >> 16) & 0xff,
                           (b >> 8) & 0xff,
            (b >> 0) & 0xff,
            (b >> 24) & 0xff);
}

//void guiDrawTextWithLen(Painter *a, int b, int c, char *d, unsigned long e)
//{
//    Xutf8DrawString(xdisplay, a->drawable, xFontSet, a->gc, b, c, d, e);
//}

void guiSetSize(u32 w, u32 h)
{
    SDL_SetWindowSize(rootWindow, w, h);
}
//void guiMoveResizeWindow(GuiWindow win, int x, int y, int w, int h) {
//    XMoveResizeWindow(xdisplay,
//                      win,
//                      x,y, h, w);
//}
void guiClearWindow(GuiWindow w) {
    SDL_RenderClear(SDL_GetRenderer(w));
}
#warning remove leak
Size guiDrawText(Painter* p, const char *text, int len, Point pos,
                 i32 color) {
    (void)len;
    SDL_Color sdlcolor = {
            (color >> 16)&0xff,
            (color >> 8)&0xff,
            (color >> 0)&0xff,
            255};
    SDL_Surface *surfaceMessage =
            TTF_RenderUTF8_Solid(font, text, sdlcolor);
    Size res = {surfaceMessage->w, surfaceMessage->h};
    SDL_Rect rect = {
        pos.x,
        pos.y,
        res.width,
        res.height
    };
    SDL_BlitSurface(surfaceMessage, 0, p->drawable, &rect);

    SDL_FreeSurface(surfaceMessage);
    return res;
}

void guiDrawImage(Painter* p, IMAGE* i, int x, int y) {

    SDL_Rect rect = {x, y, i->w, i->h};
    SDL_BlitSurface(i, NULL, p->drawable, &rect);
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
