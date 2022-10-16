#ifndef GUI_H
#define GUI_H
//#include <X11/Xlib.h>
//#include <assert.h>
#include "backend.h"
#include "misc.h"
#include <stdbool.h>
#include <stddef.h>
#include "loadImage.h"

//enum mode {

//};
//typedef struct State {
//    bool press;
//    int x;
//    int y;
//} State;
typedef int i32;
//typedef struct Image Image;
extern Event event;

_Bool guiSameWindow(Painter* p);
Size guiTextExtents(/*Painter*p,*/const char *text, int len);
void guiDrawImage(Painter* p, IMAGE* i, int x, int y);


//extern State state;
//extern Display * xdisplay;
//extern XFontStruct *xFontStruct;
//extern struct _XftFont *xFont;
extern GuiWindow rootWindow;
extern Painter rootWindowPainter;
//extern int xDepth;

typedef struct Point {
    int x;
    int y;
} Point;
typedef struct Size {
    u32 width;
    u32 height;
} Size;
typedef struct Rect {
    u32 x;
    u32 y;
    u32 width;
    u32 height;
} Rect;
extern Point (*getPos)();
extern void (*feedbackSize)(Size);


//Point getSavedPos();
//void savePos(int x, int y);

typedef struct RawPicture {
    char* data;
    size_t w;
    size_t h;
} RawPicture ;
// in interactive widgets p->drawable should be a Window
bool guiComboBoxZT(Painter *p, char const*const*elements, int* current);
bool guiNumberEdit(Painter*p, int digits, int* number, bool* consume);
void guiDoubleEdit(Painter*p, int digits, double* number);
bool guiButton(Painter *p, char* text, int len);
//bool guiButtonWindow(Painter *p, char* text, int len);
bool guiCheckBox(Painter* p, bool* v);
bool guiToolButton(Painter* p, IMAGE* i, bool *consume);
bool guiToolButtonA(Painter* p, IMAGE* i, bool active, bool *consume);
Size guiDrawText(Painter* p, const char *text, int len,
                 Point pos, i32 color);
Size guiDrawTextZT(Painter* p, char *text, Point pos, i32 color);
void guiLabel(Painter* p, char *text, int len);
void guiLabelZT(Painter* p, char *text);
void guiLabelZTWithBackground(Painter* p, char *text, bool back);
void guiLabelWithBackground(Painter* p, char *text, int len, bool back);
bool guiButtonZT(Painter* p, char *text);
bool guiSlider(Painter*, double* v, double start, double end);

void guiDrawLine(Painter*, int, int, int, int);
void guiDrawRectangle(Painter*, int, int, int, int);
void guiFillRectangle(Painter *a, int b, int c, int d, int e);
void guiFillRawRectangle(RawPicture *p, int x, int y, int w, int h, char r, char g, char b);
void guiSetForeground(Painter*, unsigned long);
void guiDrawTextWithLen(Painter*, int, int, char*, unsigned long);
void guiSetSize(u32, u32);
Size guiGetSize();


static const unsigned int GuiDarkMagenta = 0x880088;
static inline unsigned int rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}
static inline unsigned int rgbf(double r, double g, double b) {
    return rgb(r*255, g*255, b*255);
}
void guiStartDrawing();
void guiNextEvent();
void guiRedraw();

#endif // GUI_H
