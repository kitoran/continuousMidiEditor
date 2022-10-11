#include <X11/Xlib.h> // Every Xlib program must include this
#include <X11/extensions/Xrender.h>
#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds

#include <stdio.h>
#include <X11/Xft/Xft.h>
#include <stdbool.h>
#include <locale.h>
#include "stb_image.h"
#include "persistent.h"
#include "loadImage.h"
#include "gridlayout.h"
//#include <X11/Xft/Xft.h>
//XftFont* font;
//XftDraw* xftDraw;
XImage *ximage;
char* appName = "guiExample";
int fe;
void loop(Painter* p) {
//
    XClearWindow(xdisplay, rootWindow);
//    XFlush(xdisplay);
    setCurrentGridPos(0,0);
//    guiSetForeground(p, 0xffff0000);
    if(guiToolButton(p, ximage)) {
        fprintf(stderr, "pressed!:)\n");
    }
    setCurrentGridPos(0,1);
    guiLabelZT(p, "⏹□▢▢▢▢				    ◽◽◽◽◼◽ 	◾■□■■▢▣▤▥▦▧▨▩▪▫▬▭▮▯◘◙◺◻◼◽◾◿");
//    guiLabelZT(p, "yes   🤔 ");
    setCurrentGridPos(1,0);
    guiLabel(p, "i", 1);
    setCurrentGridPos(1,1);
    guiLabelZT(p, "am   ▼ ‣");
    setCurrentGridPos(0,2);
    char* opts[] = {"awerg", NULL};
    guiComboBoxZT(p, opts,0);
    setCurrentGridPos(1,2);
    guiLabelZT(p, "ברך אתה");

    setCurrentGridPos(2,2);
    if(guiButton(p, "button", 6)) {
        fprintf(stderr, "button was pressed");
    }

    setCurrentGridPos(2,1);

    char hi[] = "привет→";
    if(guiButton(p, hi, sizeof(hi)-1)) {
        fprintf(stderr, "button2 was pressed");
    }
    setCurrentGridPos(2,0);
    persistentNumberEdit(p, 9, &fe);
}

XEvent xEvent;

int main()
{

//    fprintf(stderr,stbi_failure_reason());
//    exit(0);
    char* l = setlocale(LC_ALL, "C.UTF-8");
    l = setlocale(LC_ALL, "C.UTF-8");
    guiStartDrawing();
    int s = DefaultScreen(xdisplay);



    getPos = gridGetPos;
    feedbackSize = gridFeedbackSize;
    gridStart.x = 5;
    gridStart.y = 5;

    GC gc2 = XCreateGC(xdisplay, rootWindow, 0, NULL);
    Painter pa = {rootWindow, gc2};
//    loop(&pa);
    XClearWindow(xdisplay, rootWindow);
    Pixmap p = XCreatePixmap(xdisplay, rootWindow, 300, 200, xDepth );
//                 XCreateImage(display, visual, depth, format, offset, data, width, height, bitmap_pad,
//                                         bytes_per_line)
    // Create a "Graphics Context"
    GC gc = XCreateGC(xdisplay, p, 0, NULL);
    // Tell the GC we draw using the white color
    XSetForeground(xdisplay, gc, 0xffa4a4a4);
//    XFillRectangle(xdisplay, p, gc, 0,0,300,200);
    XSetForeground(xdisplay, gc, 0xffffffff);
    XDrawLine(xdisplay, p, gc, 10, 60, 180, 20);
    XTextItem ti = {
        "hello i am a text",
        strlen(ti.chars),
        0,
        None
    };
//    XDrawText(xdisplay, p, gc, 30, 30, &ti, 1);
    // Draw the line

//    Visual *visual = DefaultVisual(display, Defascreen);
    char data[300*200*4];
//    struct funcs funcs;
//    XPointer xp = 0;
//    XImage image = {
//        300, 300,
//        0,
//        XYPixmap,
//        malloc(300*300*4),
//        LSBFirst,
//        8, LSBFirst, 8, //for bitmaps
//        24/*8*/, // (8 or 24?) depth
//        300*4, //bytes_per_line
//        32, //for zpixmap
//        0x00ff0000,//        unsigned long red_mask;    /* bits in z arrangement */
//        0x0000ff00, //unsigned long green_mask;
//        0x000000ff, //unsigned long blue_mask;
//        &image,
//        funcs,
//    };
    
//    XftDrawCreate
//    GuiImage rr = {300, 200, data};
    for(int i = 0; i < 300*200*4; i++) {
        data[i]=i%253;
    }
    XDrawLine(xdisplay,p,gc,0,10,290,180);
//    drawVerticalLine(&rr, 50, 10, 150, 0x00ff0000);
//    drawHorizontalLine(&rr, 50, 150, 50, 0x0000ff00);
//    XCopyArea(xdisplay, p, rootWindow, gc2, 0,0, 300, 200, 0,0);
//    XImage* image = XCreateImage(display, visual, 24, ZPixmap, 0, data,
//                                 300, 200, 32, 0);
    XRenderPictFormat * format =
        XRenderFindStandardFormat (xdisplay,
                       PictStandardRGB24);
    Picture picture =
        XRenderCreatePicture (xdisplay, rootWindow,
                      format,
                      0,
                      NULL);
    XRenderColor c = {
        65535,
        0,
        0,
        65535
    };
//    XRenderFillRectangle(xdisplay, PictOpSrc, picture, &c, 10, 10, 100, 100);
    XRenderColor c1 = {
        0,
        0,
        65535,
        65535/4*2
    };
    XRenderColor whiteOpaque = {
        65535,
        0,
        65535,
        32535
    };
//    XRenderFillRectangle(xdisplay, PictOpAtop, picture, &c1, 20, 20, 100, 100);

    Window www = {rootWindow};
    int xx = 10, yy = 10;
    int ww, hh, hhmax;

//    GC gcrootwindow = XCreateGC(xdisplay, rootWindow, 0, NULL);
//    , int*x, int*y
    ximage = loadLocalImageZT("rec.png");

//    XPutImage(xdisplay, p, gc, ximage, 0, 0, 0, 0, x, y);
//    XCopyArea(xdisplay, p, rootWindow, gc2, 0, 0,
//              x, y, 10, 10);

    XFlush(xdisplay);
    loop(&pa);
//    XNextEvent(xdisplay, &xEvent);


    while(true) {
//        XFreeGC(xdisplay, pa.gc);
//        pa.gc = XCreateGC(xdisplay, rootWindow, 0, 0);

        guiNextEvent();
        loop(&pa);


//        XRenderColor r = {16000, 16000, 16000, 16000};
//        XftDrawStringUtf8 (xftDraw,
//                        &r,
//                        font,
//                        100,
//                        100,
//                        hi, sizeof(hi)-1);
//        XFlush(xdisplay);
//        XEvent e;
//        switch(e.type)
//        {
        if(xEvent.type == DestroyNotify) {
            goto exit;
        };
//        case Expose: {
//            fprintf(stderr, "expose %d %d %d\n", e.xexpose.window,
//                   www.window, l.window);
////            XPutImage(xdisplay, rootWindow, gc, image, 0, 0, 0, 0, 300, 200);
//        } break;
//        case ButtonPress: {
//            guiSetSize(&www, 400, 500);
////            XPutImage(xdisplay, rootWindow, gc, image, 0, 0, 0, 0, 300, 200);
//        } break;
//        }
    }
    exit:
    XDestroyWindow(xdisplay,
                   rootWindow);

    XCloseDisplay(xdisplay);
    return 0;
}
