#ifndef GUI_H
#define GUI_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
//#include <assert.h>

#include <stdbool.h>

#define TimerEvent LASTEvent+1

//enum mode {

//};
//typedef struct State {
//    bool press;
//    int x;
//    int y;
//} State;
typedef struct GuiImage {

} GuiImage;
extern SDL_Event sdlEvent;
//extern State state;
//extern Display * xdisplay;
extern TTF_Font *xFontStruct;
extern SDL_Window rootWindow;
extern int sdlDepth;

typedef struct Point {
    int x;
    int y;
} Point;
typedef struct Size {
    uint width;
    uint height;
} Size;
extern Point (*getPos)();
extern void (*feedbackSize)(Size);


//Point getSavedPos();
//void savePos(int x, int y);

typedef struct Painter {
    SDL_Texture* drawable;
    SDL_Renderer* rend;
    SDL_Color textColor;
} Painter;
typedef struct RawPicture {
    char* data;
    size_t w;
    size_t h;
} RawPicture ;
// in interactive widgets p->drawable should be a Window
void guiLabel(Painter* p, char *text, int len);
bool guiComboBoxZT(Painter *p, char const*const*elements, int* current);
bool guiNumberEdit(Painter*p, int digits, int* number, bool* consume);
void guiDoubleEdit(Painter*p, int digits, double* number);
bool guiButton(Painter *p, char* text, int len);
//bool guiButtonWindow(Painter *p, char* text, int len);
bool guiToolButton(Painter* p, SDL_Texture* i, bool *consume);
void guiLabelZT(Painter* p, char *text);
bool guiButtonZT(Painter* p, char *text);
bool guiSlider(Painter*, double* v, double start, double end);

void guiDrawLine(Painter*, int, int, int, int);
void guiDrawRectangle(Painter*, int, int, int, int);
void guiFillRectangle(Painter *a, int b, int c, int d, int e);
void guiFillRawRectangle(RawPicture *p, int x, int y, int w, int h, char r, char g, char b);
void guiSetForeground(Painter*, unsigned long);
void guiDrawTextZT(Painter*, int, int, char*);
void guiSetSize(SDL_Window *, uint, uint);
Size guiGetSize();


static const unsigned int GuiDarkMagenta = 0x880088;
static inline unsigned int rgb(int r, int g, int b) {
    return r << 16 | g << 8 | b;
}
static inline unsigned int rgbf(double r, double g, double b) {
    return rgb(r*255, g*255, b*255);
}
void guiStartDrawing(const char *name);
void guiNextEvent();
void guiRedraw();
#endif // GUI_H
