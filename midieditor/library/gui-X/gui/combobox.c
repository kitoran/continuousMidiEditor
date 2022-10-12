#include "gui.h"
#include "guiglobals.h"
#include "loadImage.h"
//#include <math.h>
//#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

//#include <X11/Xlib.h>
//#define XK_LATIN1
//#define XK_MISCELLANY
//#include <X11/keysymdef.h>
//#include <sys/select.h>
//#include <stdlib.h>

bool guiComboBoxZT(Painter *p, const char * const *elements, int* current)
{
    STATIC(GuiWindow, listWindow, guiMakeWindow())
    STATIC(Painter, gc, guiMakePainter(listWindow))


    Point pos = getPos();
    const char*const* c = elements;
    Rect overallLogMax = {0, 0, 0, 0};
    int numberOfElements = 0;
    while(*c != NULL) {
        numberOfElements++;
        Size overallLog = guiTextExtents(*c, strlen(*c));
        overallLogMax.width = MAX(overallLog.width, overallLogMax.width);
        overallLogMax.height = MAX(overallLog.height, overallLogMax.height);
//        int newx = MIN(overallLogMax.x, overallLog.x);
//        int newy = MIN(overallLogMax.y, overallLog.y);
//        int newr = MAX(overallLogMax.x + overallLogMax.width,
//                       overallLog.x + overallLog.width);
//        int newb = MAX(overallLogMax.y + overallLogMax.height,
//                       overallLog.y + overallLog.height);
//        overallLogMax.x = newx;
//        overallLogMax.y = newy;
//        overallLogMax.width = newr - newx;
//        overallLogMax.height = newb - newy;
        c++;
    }

    overallLogMax.width += 10;//no fucking idea why
    static IMAGE* triangle = NULL;
    if(triangle ==  NULL) {
        triangle = loadImageZT(GUI_RESOURCE_PATH, "triangle.png");
    }

    Size size = {overallLogMax.width + triangle->IMAGE_WIDTH + 10,
                MAX(overallLogMax.height, (u32)triangle->IMAGE_HEIGHT) + 10};

    bool res = false;
//    if(event.xany.window == listWindow) {
        if(event.type == ButtonRelease && context.active == current) {
            int mx = GET_X(event)-pos.x;
            int my = GET_Y(event)-pos.y - (int)size.height;
//            , ,
            for(int i = 0; i < numberOfElements; i++) {
                if(mx >= 1 && mx <= (int)size.width-2  +  1 &&
                    my >= (i)*(int)size.height+1 && my <= (int)size.height-2   +   (i)*size.height+1) {
                    res = true;
                    *current = i;
                }
            }
        }
//    }

    if(guiSameWindow(p)) {
        if(event.type ==ButtonRelease) {
            int mx = GET_X(event);
            int my = GET_Y(event);
            if(mx >= pos.x && mx <= pos.x + (int)size.width &&
                my >= pos.y && my <= pos.y + (int)size.height) {
    //            if(event.type == ButtonRelease) {
    //                res = true;
                    fprintf(stderr, "Mapping window\n");
                    guiMoveResizeWindow(listWindow, pos.x, pos.y + (int)size.height,
                                      size.width+1/*непонятно почему здесь окно получается на пиксель тоньше*/, (size.height)*numberOfElements);
//                    XSetBackground(xdisplay, &gc, 0xffffffff);
                    guiSetForeground(&gc, 0xffffffff);
                    guiDrawRectangle(&gc, 5,5,30,30);
                    guiShowWindow(listWindow);
    //            }
                context.active = current;
    //            if(consume) *consume = true;
            }
            else {
                if(context.active == current) {
                    context.active = NULL;
                    fprintf(stderr, "Unmapping window %d %d %d %d %d %d\n",
                            mx, my, pos.x, pos.y
                            , pos.x+ (int)size.width, pos.y+ (int)size.height);
                    guiHideWindow(listWindow);
                }
            }
        }
        if(event.type != MotionEvent) {
            guiSetForeground(p, 0xff555555);
            guiFillRectangle(p, pos.x, pos.y,
                           size.width, size.height);
            guiSetForeground(p, 0xffffffff);
            guiDrawRectangle(p, pos.x, pos.y,
                           size.width, size.height);
            guiSetForeground(p, 0xffffffff);
            int len = strlen(elements[*current]);
            guiDrawText(p,elements[*current], len,
                        STRU(Point,pos.x+5 + overallLogMax.x,
                    pos.y+5 - overallLogMax.y),0xffffffff);

            guiDrawImage(p, triangle,
                      pos.x+overallLogMax.x+overallLogMax.width,
                         pos.y+5);
            if(context.active == current) {
                guiClearWindow(listWindow);
                for(int i = 0; i <numberOfElements; i++) {
                    guiSetForeground(&gc, 0xff333333);
                    guiFillRectangle(&gc, 1, (i)*size.height+1,
                                   size.width-2, size.height-2);

                    int len = strlen(elements[i]);
                    guiSetForeground(&gc, 0xffffffff);
                    guiDrawText(&gc,
                                elements[i], len,
                               STRU(Point,
                                    5 + overallLogMax.x,
                                    (i)*size.height+5 - overallLogMax.y),
                                0xffffffff);
                }
            }
        }
    }
    feedbackSize(size);
    return res;
}
