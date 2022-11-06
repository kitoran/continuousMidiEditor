#include "roll.h"
#include "misc.h"
#include "gui.h"
#include "extmath.h"
#include "stb_ds.h"
#include "melody.h"
typedef struct {
    int num;
    int den;
} fraction;
fraction fractions[] = {{1,8}, {1, 7}, {1, 6}, {1, 5}, {1, 4}, {2, 7},
                        {1, 3}, {3,8}, {2, 5}, {3, 7}, {1, 2}, {4, 7},
                        {3, 5}, {5,8}, {2, 3}, {5, 7}, {3, 4}, {4, 5},
                        {5, 6}, {6, 7}, {7,8}, {1, 1}};
double toDouble(fraction f) {
   return f.num*1.0/f.den;
}

fraction searchFraction(double test) {
    if(test > 1) {
        fraction inv = searchFraction(1/test);
        return STRU(fraction, inv.den, inv.num);
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
Note* base = NULL;
Note editedNote = {-1, -1, -1};
typedef struct {
    double freq;
    double time;
} coord;
void roll(Painter* p, int y) {
    Size windowSize = guiGetSize();
    int height = windowSize.height-y;
    coord toCoord(int mx, int my) {
        return STRU(coord,
                    -(my-y-height/2)+400,
                    mx/20.0);
    }
    Point fromCoord(coord c) {
        return STRU(Point,
                    c.time*20,
                    y + height/2-(c.freq-400));
    }
    Rect noteToRect(Note n) {
        coord c = {n.freq, n.start};
        Point s = fromCoord(c);
        c.time += n.length;
        Point e = fromCoord(c);
        return STRU(Rect, s.x, s.y-5, e.x-s.x, 10);
    }
//    Point pos = getPos();

    guiDrawTextZT(p, "440", STRU(Point, 10, y + height/2), 0xffffffff);
    // SDL_Log("Window %d resized to %dx%d",
    // event->window.windowID, ,
    // );
//    int start = 0;
    if(base) {
        guiSetForeground(p, 0xff111111);
        FOR_STATIC_ARRAY(frac, fractions) {
//            for(int i = 0; i < 3; i++) {
            coord c = {base->freq * frac->num/frac->den, 0};
            Point r = fromCoord(c);
            if(r.y >= y) guiDrawLine(p, 0, r.y, windowSize.width, r.y);
            char str[30];
            sprintf(str, "%d/%d", frac->num, frac->den);
            guiDrawTextZT(p, str, r, 0xffffffff);
            c = STRU(coord, base->freq / frac->num*frac->den, 0);
            r = fromCoord(c);
            if(r.y >= y) guiDrawLine(p, 0, r.y, windowSize.width, r.y);
            sprintf(str, "%d/%d", frac->num, frac->den);
            guiDrawTextZT(p, str, r, 0xffffffff);
//            }
        }
    }
    FOR_STB_ARRAY(anote, piece) {
        Rect r = noteToRect(*anote);
        if(anote == base) {
            guiSetForeground(p, 0xffffffff);
        } else {
            guiSetForeground(p, 0xffff00ff);
        }
        guiFillRectangle(p,
                         r.x, r.y, r.width, r.height);
    }
    if(editedNote.freq >= 0) {
        Rect r = noteToRect(editedNote);
        guiSetForeground(p, 0xffff77ff);
        guiFillRectangle(p,
                         r.x, r.y, r.width, r.height);
    }
    if(event.type == ButtonRelease) {
        dragStart = STRU(Point, -1, -1);
//        SDL_MouseButtonEvent e =
//                event.button;
//        coord c = toCoord(e.x, e.y);
        if(editedNote.freq >= 0) {
            insert(editedNote);
        }
        editedNote = STRU(Note,-1,-1,-1);
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
    if(event.type == ButtonPress) {
        SDL_MouseButtonEvent e =
                event.button;
        if(e.y < y) return;
        dragStart = STRU(Point, e.x, e.y);
        base = NULL;
        FOR_STB_ARRAY(anote, piece) {
            if(pointInRect(STRU(Point, e.x, e.y),
                           noteToRect(*anote))) {
                base = anote;
                DEBUG_PRINT(anote->start, "%lf");
            }
        }
        if(base == NULL && e.button == 1) {
            coord c = toCoord(e.x, e.y);
            editedNote = STRU(Note, c.freq, c.time, 0.5);
        }
        if(e.button==3) {

        }
    }
    if(event.type == MotionEvent) {
        SDL_MouseMotionEvent e =
                event.motion;
        if(e.y < y) return;
        coord c = toCoord(e.x, e.y);
        if(  editedNote.freq >= 0) {
            guiSetForeground(p,0xff000000);
            Rect r = noteToRect(editedNote);
            guiFillRectangle(p, r.x, r.y, r.width, r.height);
            SDL_RenderPresent(p->gc);
            coord s = toCoord(dragStart.x, dragStart.y);
            double start = MIN(s.time, c.time);
            double end = MAX(s.time, c.time);
            editedNote = STRU(Note, s.freq, start, end-start);

            r = noteToRect(editedNote);
            guiSetForeground(p,0xffff77ff);
            guiFillRectangle(p, r.x, r.y, r.width, r.height);

            //            guiClearWindow(rootWindow);
//            guiRedraw();
        }
//        if(base) {
//            c.freq/base->freq
//        }
    }
}
