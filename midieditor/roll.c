#include "roll.h"
#include "misc.h"
#include "gridlayout.h"
#include "editorinstance.h"
#include "gui.h"
#include "extmath.h"
#include "playback.h"
//#include <thread.h>
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
//double horizontalScale = 20;
typedef struct {
    double freq;
    double time;
} coord;
//static double horizontalScroll = 0;
//static double verticalFrac = 0.1;
//static double verticalScroll = 0.9;

// So there are two ways to do scrolling - rendere everything into a surface/texture and then copy the relevant part of the textuew
// or just render the relevant part of the widget and clip the parts that stick out with SDL_RenderSetClipRect
// i'm doing the worst of both worlds - rendering all the rectangles and clipping themwith this function, but thecode issimpler

//void rollCl(Painter* p, int y);
void navigationBar(Painter* p, Size size);
void noteArea(Painter* p, Size size);
#define NAVIGATION_THICKNESS 30
static CONTINUOUSMIDIEDITOR_Config* cfg;
void roll(Painter* p, int y) {
//    DEBUG_PRINT(y, "in \'roll\'%d");
    cfg = hmgetp(config, currentGuid); //TODO: i don't know how expensive it is to do this every frame
    // i could store the pointer to current config instead of current guid
    STATIC(Grid, grid, allocateGrid(2,3,0));
    if(event.type  == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_RESIZED
                                          ||event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )) {
        for(int i = 0; i < grid.gridWidthsLen; i++)
            grid.gridWidths[i] = 0;
        for(int i = 0; i < grid.gridHeightsLen; i++)
            grid.gridHeights[i] = 0;
    }
    pushLayout(&grid);
    grid.gridStart = (Point){0, y };
    setCurrentGridPos(0, 0);
    Size windowSize = guiGetRect().size;
    Size navigationSize = {windowSize.w - SCROLLBAR_THICKNESS, NAVIGATION_THICKNESS};
    navigationBar(p, navigationSize);
    gridNextRow();
    Size noteAreaSize = {windowSize.w - SCROLLBAR_THICKNESS, windowSize.h-y-NAVIGATION_THICKNESS
                        - SCROLLBAR_THICKNESS};
    noteArea(p, noteAreaSize);
    gridNextRow();
    guiScrollBar(p,windowSize.w-SCROLLBAR_THICKNESS, &cfg->value.horizontalScroll, cfg->value.horizontalFrac, true);
    setCurrentGridPos(1, 1);
    guiScrollBar(p,noteAreaSize.h, &cfg->value.verticalScroll, cfg->value.verticalFrac, false);
    Size size = getGridSize(&grid);
    popLayout();
    //feedbackSize(size); // in principle i should probavly call feedbackSize() here but maybe not
    // if i do then i need to properly define the top level layout uin pmainprogram
}
double xToTime(int width, int mx) {
//    0 -> end*cfg->value.horizontalScroll
//    width -> end*(cfg->value.horizontalScroll+cfg->value.horizontalFrac)
    return mx*1.0/width*end*cfg->value.horizontalFrac + end*cfg->value.horizontalScroll;// horizontalScale+end*cfg->value.horizontalScroll;
}
int timeToX(int width, double time) {

    return (time - end*cfg->value.horizontalScroll)*width/end/cfg->value.horizontalFrac;
            //(time-end*cfg->value.horizontalScroll)*horizontalScale;
}

//void rollCl(Painter* p, int y) {
//    STATIC(Grid, grid, allocateGrid(2,2,0));
//    pushLayout(&grid);

//    grid.gridStart = (Point){0, y};
//    setCurrentGridPos(0, 0);
//    gridNextRow();
//    noteArea(p);
//    gridNextColumn();

//    Size size = getGridSize(&grid);
//    popLayout();
//    feedbackSize(size);
//}

void navigationBar(Painter* p, Size size) {
    Point pos = getPos();
    Rect rect = {pos, size};
    _Bool update = event.type == SDL_MOUSEBUTTONDOWN;
    guiSetForeground(p, gray(20));
    guiFillRectangle(p, rect);
    if(event.type == SDL_MOUSEMOTION) {
        update = event.motion.state != 0;
    }
    if(update)  {
        Point mp = {GET_X(event), GET_Y(event)};
        if(pointInRect(mp, rect)) {
            currentPositionInSamples = timeToSamples(xToTime(size.w, mp.x));
            cursorPosition = xToTime(size.w, mp.x);
#if REAPER
            reaperSetPosition(xToTime(size.w, mp.x));
#endif
        }
    }

    FOR_STB_ARRAY(TempoMarker*, t, tempoMarkers)  {
        guiSetForeground(p, gray(0x33));
        int c = timeToX(size.w, t->when-itemStart);
        guiDrawLine(p, c, pos.y, c, pos.y+size.h/2);
    }
    feedbackSize(size);
}
#define FREQ_MIN 20
#define FREQ_MAX 20000
double yToFreqLinear(int height, Point pos, int my) {
    double verticalScrollMirrored = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+cfg->value.verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    int relativeY = my - pos.y;
    double myFrac = 1 - relativeY * 1.0/ height;
    return myFrac*(freqVMax-freqVMin) + freqVMin;
}
double freqToYLinear(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+cfg->value.verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    double myFrac = (freq - freqVMin) / (freqVMax-freqVMin);
    int relativeY = (1-myFrac)*height;
    return relativeY + pos.y;
}
double yToFreq(int height, Point pos, int my) {
//    double verticalScrollMirrored = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
//    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
//    double freqVMax = (verticalScrollMirrored+cfg->value.verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

//    int relativeY = my - pos.y;
//    return exp(myFrac*((freqVMax)-(freqVMin)) + (freqVMin));

    double myFrac = 1 - (my - pos.y) * 1.0/ height;
    double range = log(FREQ_MAX)-log(FREQ_MIN);
    double partOfRangeBelowScreen = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
    double partOfRangeBelowMouse = partOfRangeBelowScreen + cfg->value.verticalFrac*myFrac;
    double rangeBelowMouse = partOfRangeBelowMouse*range;
    return exp(rangeBelowMouse);
}
double freqToY(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
    double freqVMax = (verticalScrollMirrored+cfg->value.verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

    double myFrac = (log(freq) - (freqVMin)) / ((freqVMax)-(freqVMin));
    int relativeY = (1-myFrac)*height;
    return relativeY + pos.y;
}
Rect noteToRect(Size size, Point pos, Note n) { // TODO use sdl renderer drawing range or whatsitcalled
    Point s = { timeToX(size.w, n.start), freqToY(size.h, pos, n.freq) };
    int right = timeToX(size.w, n.start + n.length);
    return (Rect) { s.x, s.y - 5, right - s.x, 10 };
}
double closestTime(int width, int x) { // TODO: this function
//    double tr = xToTime(width, x);
//    double closest = round(tr / BEAT) * BEAT;
//    double le = xToTime(width, x - 5);
//    double mo = xToTime(width, x + 5);
//    if (le < closest &&
//        mo > closest) {
//        tr = closest;
//    }
//    return tr;
    return xToTime(width, x);
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
}
inline double beatTime(TempoMarker* tm) {
    return 60.0/tm->qpm*4/tm->denom;
}
void noteArea(Painter* p, Size size) {
    Point pos = getPos();
    Rect rect = {0, pos.y, size.w, size.h};
    guiSetClipRect(p, rect);
    STATIC(int, digSize, guiTextExtents("3/5", 3).h);
    double lastVisibleTime = xToTime(size.w, size.w);
//    double a = itemStart/BEAT;
//    double a = itemStart/BEAT;

    TempoMarker last = projectSignature; int next = 0; //double lastFraction = 0;
//TODO: watch for added/deleted tempo markers
    int numOfBeat = 0; // TODO: maybe use TimeMap_GetTimeSigAtTime
    for(double projectTime = 0; projectTime < lastVisibleTime+itemStart; projectTime += beatTime(&last)) {
        while(next < arrlen(tempoMarkers) && projectTime >= tempoMarkers[next].when) { // TODO: does work with multiple markers in one beat?
            ASSERT(tempoMarkers[next].num != 0, "time signature is 0????");
            if(tempoMarkers[next].num > 0) {
                numOfBeat = 0;
                projectTime = tempoMarkers[next].when;
                last = tempoMarkers[next];
            } else {
                double partOfTheBeatBeforeMarker = (tempoMarkers[next].when - (projectTime-beatTime(&last)))/beatTime(&last);
                last.when = tempoMarkers[next].when;
                last.qpm = tempoMarkers[next].qpm;
                projectTime = tempoMarkers[next].when + (1-partOfTheBeatBeforeMarker)*(beatTime(&last));
            }
            next++;
        }
        if(numOfBeat == 0) {
            guiSetForeground(p, gray(0x44));
        } else {
            guiSetForeground(p, gray(0x11));
        }
        int c = timeToX(size.w, projectTime-itemStart);
        guiDrawLine(p, c, pos.y, c, pos.y+size.h);
        numOfBeat = (numOfBeat+1)%last.num;
    }
    if(end < lastVisibleTime) {
        int endX = timeToX(size.w, end);
        Rect r = { endX, pos.y, pos.x+size.w-endX, size.h};
        SDL_SetRenderDrawBlendMode(p->gc, SDL_BLENDMODE_BLEND);
        guiSetForeground(p, 0x66333333);
        guiFillRectangle(p, r);
        SDL_SetRenderDrawBlendMode(p->gc, SDL_BLENDMODE_NONE);
    }
    int cur;
    if(playing) {
        cur = timeToX(size.w, samplesToTime(currentPositionInSamples));
        guiSetForeground(p,  0xff555533);
        guiDrawLine(p, cur, pos.y, cur, pos.y+size.h);
    }
    cur = timeToX(size.w, cursorPosition);
    guiSetForeground(p,0xff335566);
    guiDrawLine(p, cur, pos.y, cur, pos.y+size.h);
    for(double freq = 440.0/32; freq < 20000; freq *= 2) {
        char ferf[30]; snprintf(ferf,30,"%5lf", freq);
        guiDrawTextZT(p, ferf, (Point) { 10, freqToY(size.h,
                                        pos,
                                        freq)- digSize / 2 }, 0xffffffff);
    }
//    guiDrawTextZT(p, "880", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    880)- digSize / 2 }, 0xffffffff);
//    guiDrawTextZT(p, "1760", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    1760)- digSize / 2 }, 0xffffffff);

    if(base >= 0) {
        FOR_STATIC_ARRAY(fraction*, frac, fractions) {

            guiSetForeground(p, gray((MAX_DEN+1-frac->den) *255 / (MAX_DEN+1)));
//            for(int i = 0; i < 3; i++) {
            int r = freqToY(size.h, pos, piece[base].freq * frac->num/frac->den);
            char str[30];
            if(r >= pos.y && r < pos.y+size.h) {
                guiDrawLine(p, 0, r, size.w, r);
                sprintf(str, "%d/%d", frac->num, frac->den);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
            }
            r = freqToY(size.h, pos, piece[base].freq / frac->num*frac->den);
            if(r >= pos.y && r < pos.y+size.h) {
                guiDrawLine(p, 0, r, size.w, r);
                sprintf(str, "%d/%d", frac->den, frac->num);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
            }
        }

    }
    FOR_NOTES(anote, piece) {
        Rect r = noteToRect(size, pos, *anote);
        if(anote == piece + base) {
            guiSetForeground(p, 0xffffffff);
        } else {
            guiSetForeground(p, 0xffff00ff);
        }
        guiFillRectangle(p,
                         r);
    }
    if(editedNote.freq >= 0) {
        Rect r = noteToRect(size, pos, editedNote);
        guiSetForeground(p, 0xffff77ff);
        guiFillRectangle(p,
                         r);
    }
    if(event.type == ButtonRelease) {
        SDL_MouseButtonEvent e =
                event.button;
        dragged = -1;
        double releaseTime = closestTime(size.w, e.x);
        if(editedNote.freq >= 0) {
            double pressTime = closestTime(size.w, dragStart.x);
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
        SDL_Keymod km = SDL_GetModState();
        if(km & KMOD_CTRL) {
            double myFrac = 1 - (e.mouseY - pos.y) * 1.0/ size.h;
            double partOfRangeBelowScreen = 1-cfg->value.verticalFrac-cfg->value.verticalScroll;
            double partOfRangeBelowMouse = partOfRangeBelowScreen + cfg->value.verticalFrac*myFrac;
            cfg->value.verticalFrac /= pow(1.5, e.y);
            double newPartOfRangeBelowScreen = partOfRangeBelowMouse - cfg->value.verticalFrac*myFrac;
            cfg->value.verticalScroll = 1 - cfg->value.verticalFrac - newPartOfRangeBelowScreen;

        } else {
            cfg->value.horizontalFrac /= pow(1.5, e.y);
            double playhead = samplesToTime(currentPositionInSamples);
            cfg->value.horizontalScroll = (playhead - 1.0/2*end*cfg->value.horizontalFrac)/end;
            cfg->value.horizontalScroll = MAX(0, cfg->value.horizontalScroll);
        }
        /*if(e.y >  0)*/
    }
    if(event.type == ButtonPress) {
        SDL_MouseButtonEvent e =
                event.button;
        if(!pointInRect((Point){e.x, e.y}, rect)) goto fuckThisEvent;
        dragStart = (Point){ e.x, e.y };
        Note* select = NULL;
        FOR_NOTES(anote, piece) {
            if (pointInRect((Point) { e.x, e.y },
                           noteToRect(size, pos, *anote))) {
                select = anote;//-piece;
//                DEBUG_PRINT(anote->start, "%lf");
            }
        }

        if(select) {
            if(e.button == 2) base = select-piece;
            if(e.button == 1) dragged  = select-piece;
        }

        coord c = {yToFreq(size.h, pos, e.y), xToTime(size.w, e.x)};
        double time = closestTime(size.w, e.x);
//        DEBUG_PRINT(select, "%ld");

        if(base >= 0 && select == NULL && e.button == 1) {

            double freq = closestFreq(size.h, pos, e.y);
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
        coord c = {closestFreq(size.h, pos, e.y), closestTime(size.w, e.x)};
//        DEBUG_PRINT(editedNote.freq, "%lf")

        if(editedNote.freq >= 0) {
            guiSetForeground(p,0xff000000);
            Rect r = noteToRect(size, pos, editedNote);
            guiFillRectangle(p, r);
            SDL_RenderPresent(p->gc);
            coord s = {yToFreq(size.h, pos, dragStart.y), closestTime(size.w, dragStart.x)};
            double start = MIN(s.time, c.time);
            double end = MAX(s.time, c.time);
            editedNote = (Note){ /*s.freq,*/ editedNote.freq, start, end-start};
            r = noteToRect(size, pos, editedNote);
            guiSetForeground(p,0xffff77ff);
            guiFillRectangle(p, r);
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
    feedbackSize(size);

}
