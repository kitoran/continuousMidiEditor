#include "roll.h"
#include "misc.h"
#include "gridlayout.h"
#include "gui.h"
#include "extmath.h"
#include "playback.h"
#include "stb_ds.h"
#include "melody.h"
#include <math.h>
#include <stdio.h>
typedef struct {
    int num;
    int den;
} fraction;
fraction fractions[] = {{1,9}, {1,8}, {1, 7}, {1, 6}, {1, 5}, {2,9}, {1, 4}, {2, 7},
                        {1, 3}, {3,8}, {2, 5}, {3, 7}, {4,9}, {1, 2}, {5,9}, {4, 7},
                        {3, 5}, {5,8}, {2, 3}, {5, 7}, {3, 4}, {7, 9}, {4, 5},
                        {5, 6}, {6, 7}, {7,8}, {8,9}, {1, 1}};

#define MAX_DEN 9
double toDouble(fraction f) {
   return f.num*1.0/f.den;
}

fraction searchFraction(double test) {
    if(test > 1) {
        fraction inv = searchFraction(1/test);
        return  (fraction) { inv.den, inv.num };
    }
    int il = 0, ih = ELEMS(fractions)-1;
    while(ih-il>1) {
        fraction interm = fractions[(ih+il)/2];
        if(test == toDouble(interm)) return interm;
        if(test > toDouble(interm)) {
            il = (ih+il)/2;
        } else {
            ih = (ih+il)/2;
        }
    }
    if(fabs(toDouble(fractions[ih]) - test) > fabs(toDouble(fractions[il]) - test)) {
        return fractions[il];
    } else return fractions[ih];
}

//double fractions[] = {/*0.0/1,*/ 1.0/7, 1.0/6, 1.0/5, 1.0/4, 2.0/7,
//                    1.0/3, 2.0/5, 3.0/7, 1.0/2, 4.0/7, 3.0/5,
//                    2.0/3, 5.0/7, 3.0/4, 4.0/5, 5.0/6, 6.0/7, 1.0/1};
Point dragStart = {-1,-1};
int base = -1;
int dragged = -1;
Note editedNote = {-1, -1, -1};
double horizontalScale = 20;
typedef struct {
    double freq;
    double time;
} coord;
static double horizontalScroll = 0;

// So there are two ways to do scrolling - rendere everything into a surface/texture and then copy the relevant part of the textuew
// or just render the relevant part of the widget and clip the parts that stick out with SDL_RenderSetClipRect
// i'm doing the worst of both worlds - rendering all the rectangles and clipping themwith this function, but thecode issimpler

void rollCl(Painter* p, int y);
void roll(Painter* p, int y) {
//    DEBUG_PRINT(y, "in \'roll\'%d");
    STATIC(Grid, grid, allocateGrid(2,2,0));
    pushLayout(&grid);
    grid.gridStart = (Point){ 0, y };
    setCurrentGridPos(0, 0);
    rollCl(p, y);
    gridNextRow();
    Size windowSize = guiGetSize();
    guiScrollBar(p,windowSize.w-SCROLLBAR_THICKNESS, &horizontalScroll, 0.1);
    popLayout();
}
double xToTime(int mx) {
    return mx/horizontalScale+end*horizontalScroll;
}
int timeToX(double time) {
    return (time-end*horizontalScroll)*horizontalScale;
}

void navigationBar(Painter* p);
void noteArea(Painter* p);
void rollCl(Painter* p, int y) {
    STATIC(Grid, grid, allocateGrid(2,2,0));
    pushLayout(&grid);

    grid.gridStart = (Point){0, y};
    setCurrentGridPos(0, 0);
    navigationBar(p);
    gridNextRow();
    noteArea(p);

    Size size = getGridSize(&grid);
    popLayout();
    feedbackSize(size);
}

#define NAVIGATION_THICKNESS 30
void navigationBar(Painter* p) {
    Point pos = getPos();
    Size size = {guiGetSize().w, NAVIGATION_THICKNESS};
    Rect rect = {pos.x, pos.y, size.w, size.h};
    _Bool update = event.type == SDL_MOUSEBUTTONDOWN;
    guiSetForeground(p, gray(20));
    guiFillRectangle(p, pos.x, pos.y, size.w, size.h);
    if(event.type == SDL_MOUSEMOTION) {
        update = event.motion.state != 0;
    }
    if(update)  {
        Point mp = {GET_X(event), GET_Y(event)};
        if(pointInRect(mp, rect)) {
            currentPositionInSamples = timeToSamples(xToTime(mp.x));
        }
    }

    feedbackSize(size);
}
double yToFreq(int height, Point pos, int my) {
    return ((double)(-(((int)(my))-pos.y-height/2)+500));
}
double freqToY(int height, Point pos, double freq) {
    return ((int)(pos.y + height/2-(((double)(freq))-500)));
}
Rect noteToRect(int height, Point pos, Note n) { // TODO use sdl renderer drawing range or whatsitcalled
    Point s = { timeToX(n.start), freqToY(height, pos, n.freq) };
    int right = timeToX(n.start + n.length);
    return (Rect) { s.x, s.y - 5, right - s.x, 10 };
}
double closestTime(int x) {
    double tr = xToTime(x);
    double closest = round(tr / BEAT) * BEAT;
    double le = xToTime(x - 5);
    double mo = xToTime(x + 5);
    if (le < closest &&
        mo > closest) {
        tr = closest;
    }
    return tr;
}
double closestFreq(int height, Point pos, int y) {
    if (base < 0) return yToFreq(height, pos, y);
    fraction frac = searchFraction(yToFreq(height, pos, y) / piece[base].freq);
    double le = yToFreq(height, pos, y + 5);
    double mo = yToFreq(height, pos, y - 5);
    if (le / piece[base].freq < toDouble(frac) &&
        mo / piece[base].freq > toDouble(frac)
        ) {
        return toDouble(frac) * piece[base].freq;
    }
    else return yToFreq(height, pos, y);
}void noteArea(Painter* p) {
    Point pos = getPos();
    Size windowSize = guiGetSize();
    int height = windowSize.h-pos.y - SCROLLBAR_THICKNESS*10;
    Rect rect = {0, pos.y, windowSize.w, height};
    guiSetClipRect(p, rect);
//    guiSetForeground(p,0);
//    guiFillRectangle(p, rect.x,rect.y, rect.width, rect.height);
//    if(redraw) {


//    }
    //    ry + height






    int cur = timeToX(samplesToTime(currentPositionInSamples));
    guiSetForeground(p, 0xff333333);
    guiDrawLine(p, cur, pos.y, cur, pos.y+height);
    STATIC(int, digSize, guiTextExtents("3/5", 3).h);
    double lastVisibleTime = xToTime(windowSize.w);
    guiSetForeground(p, gray(0x11));
    for(double s = 0; s < lastVisibleTime; s+= BEAT) {
        int c = timeToX(s);
        guiDrawLine(p, c, pos.y, c, pos.y+height);
    }

    if(base >= 0) {
        FOR_STATIC_ARRAY(fraction*, frac, fractions) {

            guiSetForeground(p, gray((MAX_DEN+1-frac->den) *255 / (MAX_DEN+1)));
//            for(int i = 0; i < 3; i++) {
            int r = freqToY(height, pos, piece[base].freq * frac->num/frac->den);
            char str[30];
            if(r >= pos.y && r < pos.y+height) {
                guiDrawLine(p, 0, r, windowSize.w, r);
                sprintf(str, "%d/%d", frac->num, frac->den);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
            }
            r = freqToY(height, pos, piece[base].freq / frac->num*frac->den);
            if(r >= pos.y && r < pos.y+height) {
                guiDrawLine(p, 0, r, windowSize.w, r);
                sprintf(str, "%d/%d", frac->den, frac->num);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
            }
        }

    }
    FOR_NOTES(anote, piece) {
        Rect r = noteToRect(height, pos, *anote);
        if(anote == piece + base) {
            guiSetForeground(p, 0xffffffff);
        } else {
            guiSetForeground(p, 0xffff00ff);
        }
        guiFillRectangle(p,
                         r.x, r.y, r.width, r.height);
    }
    if(editedNote.freq >= 0) {
        Rect r = noteToRect(height, pos, editedNote);
        guiSetForeground(p, 0xffff77ff);
        guiFillRectangle(p,
                         r.x, r.y, r.width, r.height);
    }
    if(event.type == ButtonRelease) {
        SDL_MouseButtonEvent e =
                event.button;
        dragged = -1;
        double releaseTime = closestTime(e.x);
        if(editedNote.freq >= 0) {
            double pressTime = closestTime(dragStart.x);
            double start = MIN(releaseTime, pressTime);
            double end = MAX(releaseTime, pressTime);
//
            editedNote = (Note){ editedNote.freq, start, end - start };
            if(base >= insertNote(editedNote, true)) {
                base++;
            }
        }
        dragStart = (Point){ -1, -1 };
        editedNote = (Note){ -1,-1,-1 };
//        if(e.button == 1) {
//            Note n = {c.freq,
//                     c.time,
//                     0.5};
//            fprintf(stderr,
//                    "freq %d start %d\n",
//                    c.freq, c.time);
//            insert(n);
//            guiRedraw();
//        }
    }
    if(event.type == MouseWheel) {
        SDL_MouseWheelEvent e = event.wheel;

        DEBUG_PRINT(e.x, "%d");
        DEBUG_PRINT(e.y, "%d");

        /*if(e.y >  0)*/ horizontalScale *= pow(1.5, e.y);// e.direction == SDL_MOUSEWHEEL_
    }
    if(event.type == ButtonPress) {
        SDL_MouseButtonEvent e =
                event.button;
        if(!pointInRect((Point){e.x, e.y}, rect)) goto fuckThisEvent;
        dragStart = (Point){ e.x, e.y };
        Note* select = NULL;
        FOR_NOTES(anote, piece) {
            if (pointInRect((Point) { e.x, e.y },
                           noteToRect(height, pos, *anote))) {
                select = anote;//-piece;
//                DEBUG_PRINT(anote->start, "%lf");
            }
        }

        if(select) {
            if(e.button == 2) base = select-piece;
            if(e.button == 1) dragged  = select-piece;
        }

        coord c = {yToFreq(height, pos, e.y), xToTime(e.x)};
        double time = closestTime(e.x);
//        DEBUG_PRINT(select, "%ld");

        if(base >= 0 && select == NULL && e.button == 1) {

            double freq = closestFreq(height, pos, e.y);
            editedNote = (Note){ freq, time, 0.5};
        }
        if(base < 0 && select == NULL && e.button == 1) {
//            fprintf(stderr, "oops im here !!!L_)(\n");
            editedNote = (Note){ c.freq, time, 0.5};
        }
    }
    if(event.type == MotionEvent) {
        SDL_MouseMotionEvent e =
                event.motion;
        if(e.y < pos.y) goto fuckThisEvent;
        coord c = {closestFreq(height, pos, e.y), closestTime(e.x)};
//        DEBUG_PRINT(editedNote.freq, "%lf")

        if(editedNote.freq >= 0) {
            guiSetForeground(p,0xff000000);
            Rect r = noteToRect(height, pos, editedNote);
            guiFillRectangle(p, r.x, r.y, r.width, r.height);
            SDL_RenderPresent(p->gc);
            coord s = {yToFreq(height, pos, dragStart.y), closestTime(dragStart.x)};
            double start = MIN(s.time, c.time);
            double end = MAX(s.time, c.time);
            editedNote = (Note){ /*s.freq,*/ editedNote.freq, start, end-start};
            r = noteToRect(height, pos, editedNote);
            guiSetForeground(p,0xffff77ff);
            guiFillRectangle(p, r.x, r.y, r.width, r.height);
        } else if(dragged >= 0) {
            Note draggedNote = piece[dragged];
            draggedNote.freq = c.freq;
            removeNote(dragged);
            dragged = insertNote(draggedNote, true);
        }
    }
    if(event.type == KeyPress && (GET_KEYSYM(event) == '\x7f' || GET_KEYSYM(event) == backspace) && base >= 0) {
        removeNote(base);
        base = -1;
    }

    fuckThisEvent:
    guiUnsetClipRect(p);
    feedbackSize((Size){ windowSize.w, height});

}
