#include "roll.h"
#include "misc.h"
#include "gridlayout.h"
#include "editorinstance.h"
#include "gui.h"
#include "extmath.h"
#include "actions.h"
#include "color.h"
#include "playback.h"
//#include <thread.h>
#include "stb_ds.h"
#include "melody.h"
#include <math.h>
#include <stdio.h>
bool showChannels = false;
bool showScale = false;
bool snap = true;

typedef struct {
    int num;
    int den;
} fraction;
fraction fractions[] = {{1, 11}, {11, 8}, {1,9}, {1,8}, {1, 7}, {1, 6}, {1, 5}, {2,9}, {1, 4}, {2, 7},
                        {1, 3}, {3,8}, {2, 5}, {3, 7}, {4,9}, {1, 2}, {5,9}, {4, 7},
                        {3, 5}, {5,8}, {2, 3}, {5, 7}, {3, 4}, {7, 9}, {4, 5},
                        {5, 6}, {6, 7}, {7,8}, {8,9}, {1, 1}};
typedef struct Step {
    double ratio;
    char const* desc;
    u32 color;
} Step;
Step* scale = NULL;
//todo: make a sound when positioning
// todo: add control for a looping region
//todo: show combination tones?
// todo: base note and selected are different notes, select multiple notes
// todo: when using wheel over a scrollbar just this scrollbar is affected
//todo: go to playhead
// todo: don't abort on middle mouse button
//todo: make a mode where you don't put midi pitch wheel , just work as a normal pianoroll with
// the notes in the correct places for a given scale
//todo: snap button
// todo: allow to choose a scale
// todo: show higher fractions if there is place
// todo: consider putting extension info in reaper's midi item tag instead of using my own
//todo: allow two base notes?
#define MAX_DEN 9
double toDouble(fraction f) {
   return f.num*1.0/f.den;
}
#define SCALE_CENTER 261.625565301
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
Step searchStep(double test) {
    int il = 0, ih = ELEMS(scale)-1;
    while(ih-il>1) {
        Step interm = scale[(ih+il)/2];
        if(test == interm.ratio) return interm;
        if(test > interm.ratio) {
            il = (ih+il)/2;
        } else {
            ih = (ih+il)/2;
        }
    }
    if(fabs(scale[ih].ratio - test) > fabs(scale[il].ratio - test)) {
        return scale[il];
    } else return scale[ih];
}
//double fractions[] = {/*0.0/1,*/ 1.0/7, 1.0/6, 1.0/5, 1.0/4, 2.0/7,
//                    1.0/3, 2.0/5, 3.0/7, 1.0/2, 4.0/7, 3.0/5,
//                    2.0/3, 5.0/7, 3.0/4, 4.0/5, 5.0/6, 6.0/7, 1.0/1};
Point dragStart = {-1,-1};
IdealNote draggedNoteInitialPos;
int base = -1;
int dragged = -1;
IdealNote editedNote = {-1, -1, -1};
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

// Optimized function to print Farey sequence of order n
void makeFareyScale(int n)
{
    // We know first two terms are 0/1 and 1/n
    double x1 = 0, y1 = 1, x2 = 1, y2 = n;

    printf("%.0f/%.0f %.0f/%.0f", x1, y1, x2, y2);

//#define SIZE_OF_DESC_AREA 300
//    char* desc = malloc(SIZE_OF_DESC_AREA);
//    char* area = desc;
    double x, y = 0; // For next terms to be evaluated
    while (y != 1.0) {
        // Using recurrence relation to find the next term
        x = floor((y1 + n) / y2) * x2 - x1;
        y = floor((y1 + n) / y2) * y2 - y1;

        char* desc = malloc(30);
//        int chars;
        int written = snprintf(desc, 30,
                /*SIZE_OF_DESC_AREA - (desc-area), */
                               "%d/%d", (int)x, (int)y);
//        ASSERT(written == chars, "allocate more");
        if((int)x %2 != 0 && (int)y %2 != 0) {
            Step s = {
                .ratio = x/y,
                .desc = desc,
                .color = gray(255/y)
            };
    //        desc += written + 1;
            arrpush(scale, s);

            desc = malloc(30);
            written = snprintf(desc, 30,
                                   "%d/%d", (int)y, (int)x);
            s=(Step){    .ratio = y/x,
                .desc = desc,
                .color = gray(255/x)
            };
            arrpush(scale, s);
            // Print next term
            printf(" %.0f/%.0f", x, y);
        }

        // Update x1, y1, x2 and y2 for next iteration
        x1 = x2, x2 = x, y1 = y2, y2 = y;
    }
}

void roll(Painter* p, int y) {

    if(scale == 0) {
//        FOR(i, 20) {
//            arrpush(scale, s);
//
//            Step s = {f->num*1.0/f->den, desc, 0xffffffff};
//        }]
        makeFareyScale(20);

    }

//    DEBUG_PRINT(y, "in \'roll\'%d");
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
    Size windowSize; SDL_GetWindowSize(rootWindow, &windowSize.w, &windowSize.h);//.size;
    Size navigationSize = {windowSize.w - SCROLLBAR_THICKNESS, NAVIGATION_THICKNESS};
    navigationBar(p, navigationSize);
    gridNextRow();
    Size noteAreaSize = {windowSize.w - SCROLLBAR_THICKNESS, windowSize.h-y-NAVIGATION_THICKNESS
                        - SCROLLBAR_THICKNESS};
    noteArea(p, noteAreaSize);
    gridNextRow();
    guiScrollBar(p,windowSize.w-SCROLLBAR_THICKNESS, &currentItemConfig->value.horizontalScroll, currentItemConfig->value.horizontalFrac, true);
    setCurrentGridPos(1, 1);
    guiScrollBar(p,noteAreaSize.h, &currentItemConfig->value.verticalScroll, currentItemConfig->value.verticalFrac, false);
    popLayout();
//    Size size = getGridSize(&grid);
    //feedbackSize(size); // in principle i should probavly call feedbackSize() here but maybe not
    // if i do then i need to properly define the top level layout uin pmainprogram
}
double xToTime(int width, int mx) {
//    0 -> pieceLength*currentItemConfig->value.horizontalScroll
//    width -> pieceLength*(currentItemConfig->value.horizontalScroll+currentItemConfig->value.horizontalFrac)
    return mx*1.0/width*pieceLength*currentItemConfig->value.horizontalFrac + pieceLength*currentItemConfig->value.horizontalScroll;// horizontalScale+pieceLength*currentItemConfig->value.horizontalScroll;
}
int timeToX(int width, double time) {
    return (int)round((time - pieceLength*currentItemConfig->value.horizontalScroll)*width/pieceLength/currentItemConfig->value.horizontalFrac);
            //(time-pieceLength*currentItemConfig->value.horizontalScroll)*horizontalScale;
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
    _Bool update = event.type == ButtonPress;
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
    double verticalScrollMirrored = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+currentItemConfig->value.verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    int relativeY = my - pos.y;
    double myFrac = 1 - relativeY * 1.0/ height;
    return myFrac*(freqVMax-freqVMin) + freqVMin;
}
double freqToYLinear(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+currentItemConfig->value.verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    double myFrac = (freq - freqVMin) / (freqVMax-freqVMin);
    int relativeY = (int)round((1-myFrac)*height);
    return relativeY + pos.y;
}
//double yToFreq(int height, Point pos, int my) {
////    double verticalScrollMirrored = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
////    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
////    double freqVMax = (verticalScrollMirrored+currentItemConfig->value.verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

////    int relativeY = my - pos.y;
////    return exp(myFrac*((freqVMax)-(freqVMin)) + (freqVMin));

//    double myFrac = 1 - (my - pos.y) * 1.0/ height;
//    double range = log(FREQ_MAX)-log(FREQ_MIN);
//    double partOfRangeBelowScreen = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
//    double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->value.verticalFrac*myFrac;
//    double rangeBelowMouse = partOfRangeBelowMouse*range;
//    return exp(rangeBelowMouse);
//}
double yToFreq(int height, Point pos, int my) {
//    double verticalScrollMirrored = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
//    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
//    double freqVMax = (verticalScrollMirrored+currentItemConfig->value.verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

//    int relativeY = my - pos.y;
//    return exp(myFrac*((freqVMax)-(freqVMin)) + (freqVMin));

    double myFrac = 1 - (my - pos.y) * 1.0/ height;
    double range = log(FREQ_MAX)-log(FREQ_MIN);
    double partOfRangeBelowScreen = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
    double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->value.verticalFrac*myFrac;
    double rangeBelowMouse = partOfRangeBelowMouse*range;
    return exp(rangeBelowMouse+(log(FREQ_MIN)));
}
int freqToY(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
    double freqVMax = (verticalScrollMirrored+currentItemConfig->value.verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

    double myFrac = (log(freq) - (freqVMin)) / ((freqVMax)-(freqVMin));
    int relativeY = (int)round((1-myFrac)*height);
    return (int)round(relativeY + pos.y);
}
Rect noteToRect(Size size, Point pos, IdealNote n) { // TODO use sdl renderer drawing range or whatsitcalled
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

double closestFreq(int height, Point pos, int y) { // TODO: this function
    if (base < 0 || ELEMS(scale) == 0) return yToFreq(height, pos, y);
    Step step = searchStep(yToFreq(height, pos, y) / piece[base].note.freq);
    double le = yToFreq(height, pos, y + 5);
    double mo = yToFreq(height, pos, y - 5);
    if (le / piece[base].note.freq < step.ratio &&
        mo / piece[base].note.freq > step.ratio
        ) {
        return step.ratio * piece[base].note.freq;
    }
    else return yToFreq(height, pos, y);
}
inline double beatTime(TempoMarker* tm) {
    return 60.0/tm->qpm*4/tm->denom;
}
static bool selectingARange = false;
static bool copying = false;
//RealNote** selectionSA = NULL;
const char* channelnames[]={"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};
void noteArea(Painter* p, Size size) {
    Point pos = getPos();
    Rect rect = {0, pos.y, size.w, size.h};
    guiSetClipRect(p, rect);
    guiSetForeground(p,0x4d4d4d);
    guiFillRectangle(p, rect);
    STATIC(int, digSize, guiTextExtents("3/5", 3).h);
    double lastVisibleTime = xToTime(size.w, size.w);
//    double a = itemStart/BEAT;
//    double a = itemStart/BEAT;
// TODO:режим обозначения каналов
  //  TODO: horizontal scaling doesnt work rightc
    TempoMarker last = projectSignature; int next = 0; //double lastFraction = 0;
    int lastpixel = -10;
//TODO: watch for added/deleted tempo markers
    int numOfBeat = 0; // TODO: maybe use TimeMap_GetTimeSigAtTime
    double projectTime = 0;
    for(; projectTime < lastVisibleTime+itemStart; projectTime += beatTime(&last)) {
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
            guiSetForeground(p, 0x626262);
        } else {
            guiSetForeground(p, 0x434343);
        }
        int c = timeToX(size.w, projectTime-itemStart);
        if(c >= rect.w) {
            break;
        }
        lastpixel = c;
        if(c >= 0) {
//            fprintf(stderr, "%d -----------------%d\n", c, c>=0);
            guiDrawLine(p, c, pos.y, c, pos.y+size.h);
        }
        numOfBeat = last.num?(numOfBeat+1)%last.num:0;
    }
    if(pieceLength < lastVisibleTime) {
        int endX = timeToX(size.w, pieceLength);
        Rect r = { endX, pos.y, pos.x+size.w-endX, size.h};
        SDL_SetSurfaceBlendMode(SDL_GetWindowSurface(p->window), SDL_BLENDMODE_BLEND);
        guiSetForeground(p, 0x33333343);
        guiFillRectangle(p, r);
        SDL_SetSurfaceBlendMode(SDL_GetWindowSurface(p->window), SDL_BLENDMODE_NONE);
    }
    int playback;
    int cursor = timeToX(size.w, cursorPosition);
    if(playing) {
        playback = timeToX(size.w, samplesToTime(currentPositionInSamples));
        guiSetForeground(p,  0x55999955);
        guiDrawLine(p, playback, pos.y, playback, pos.y+size.h);
        if(abs(cursor-playback) > 20) {
            volatile int e = 9;
        }
    }
    guiSetForeground(p,0xff335566);
    guiDrawLine(p, cursor , pos.y, cursor , pos.y+size.h);
    for(double freq = 440.0/32; freq < 20000; freq *= 2) {
        char ferf[30]; snprintf(ferf,30,"%5lf", freq);
        guiDrawTextZT(p, ferf, (Point) { 10, freqToY(size.h,
                                        pos,
                                        (int)round(freq))- digSize / 2 }, 0xffffffff);
    }
//    guiDrawTextZT(p, "880", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    880)- digSize / 2 }, 0xffffffff);
//    guiDrawTextZT(p, "1760", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    1760)- digSize / 2 }, 0xffffffff);

    if(base >= 0) {
//        FOR_STATIC_ARRAY(fraction*, frac, fractions) {

//            guiSetForeground(p, gray((MAX_DEN+1-frac->den) *255 / (MAX_DEN+1)));
////            for(int i = 0; i < 3; i++) {
//            int r = freqToY(size.h, pos, piece[base].note.freq * frac->num/frac->den);
//            char str[30];
//            if(r >= pos.y && r < (int)(pos.y+size.h)) {
//                guiDrawLine(p, 0, r, size.w, r);
//                sprintf(str, "%d/%d", frac->num, frac->den);
//                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
//            }
//            r = freqToY(size.h, pos, piece[base].note.freq / frac->num*frac->den);
//            if(r >= pos.y && r < (int)(pos.y+size.h)) {
//                guiDrawLine(p, 0, r, size.w, r);
//                sprintf(str, "%d/%d", frac->den, frac->num);
//                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
//            }
//        }
        FOR_STB_ARRAY(Step*, frac, scale) {

            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, piece[base].note.freq * frac->ratio);
            char str[30];
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                guiDrawLine(p, 0, r, size.w, r);
            }
        }
    }
    if(showScale) {
        double multiplier = pow(2, 1.0/16);
        for(double freq = SCALE_CENTER/8; freq < FREQ_MAX; freq *=multiplier) {
            guiSetForeground(p, 0x555555);
            int r = freqToY(size.h, pos, freq);
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                guiDrawLine(p, 0, r, size.w, r);
            }
        }
    }
    FOR_NOTES(anote, piece) {
        Rect r = noteToRect(size, pos, anote->note);
        if(anote == piece + base) {
            guiSetForeground(p, 0xffffffff);
        } else if(anote->note.muted) {
            guiSetForeground(p, 0xff1b1b1b);
        } else {
            if(showChannels) {
                double hue = 360/16*anote->midiChannel;
                guiSetForeground(p, hsvd2bgr(hue,anote->selected?0.3:1,1));
            } else {
                guiSetForeground(p, anote->selected?0xffe0ece4:0xffd0bc04);
            }
        }
        guiFillRectangle(p,
                         r);
        if(showChannels) {
            guiDrawTextZT(p, channelnames[anote->midiChannel], r.pos,
                    anote->selected||anote == piece + base?0:0xffffffff);
        }
    }
    if(base >= 0) {
        FOR_STB_ARRAY(Step*, frac, scale) {
            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, piece[base].note.freq * frac->ratio);
            char str[30];
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                sprintf(str, "%s", frac->desc);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xff000000);
            }
        }
    }
    if(editedNote.freq >= 0) {
        Rect r = noteToRect(size, pos, editedNote);
        guiSetForeground(p, 0xffff77ff);
        guiFillRectangle(p,
                         r);
    }
    if(selectingARange) {
        Point mouse; SDL_GetMouseState(&mouse.x, &mouse.y);
//        mouse.x -= pos.x; mouse.y -= pos.y;
        Rect r = { .pos = dragStart, .size = {mouse.x-dragStart.x, mouse.y-dragStart.y} };
//        int succ = SDL_SetSurfaceBlendMode(SDL_GetWindowSurface(p->window), SDL_BLENDMODE_NONE);
//        ASSERT(succ == 0, "hi^)");
        STATIC(SDL_Surface*, transparent, SDL_CreateRGBSurfaceWithFormat(0, 300, 300, 32, SDL_PIXELFORMAT_RGBA8888));
//        guiSetForeground(p, 0x33333343);
//        guiFillRectangle(p, r);
        SDL_Rect rr = {0,0,200,200};
        SDL_FillRect(transparent, NULL, 0x33333343);
//        SDL_SetAlpha(sfc, SDL_SRCALPHA, alpha);
        SDL_Rect r1 = { .x= MIN(r.x, r.x+r.w),
                        .y= MIN(r.y, r.y+r.h),
                        .w=abs(r.w),
                        .h=abs(r.h) };
        SDL_BlitScaled(transparent, 0, SDL_GetWindowSurface(p->window), &r1);
//        SDL_UpdateWindowSurface(p->window);
//        SDL_SetSurfaceBlendMode(SDL_GetWindowSurface(p->window), SDL_BLENDMODE_NONE);
        guiSetForeground(p, 0xffdddddd);
        guiDrawRectangle(p, r);
    }

    if(event.type == ButtonRelease) {
        SDL_MouseButtonEvent e =
                event.button;
        if(dragged >= 0) {
            IdealNote draggedNote = piece[dragged].note;
            if(copying) {
                copyNotes( &dragged, &base);
            } else {
                moveNotes(draggedNote.start - draggedNoteInitialPos.start,
                           draggedNote.freq - draggedNoteInitialPos.freq,
                           &dragged, &base);
            }
        }
        dragged = -1;
        double releaseTime = closestTime(size.w, e.x);
        if(editedNote.freq >= 0) {
            if(pointInRect((Point){e.x, e.y}, rect)) {
                double pressTime = closestTime(size.w, dragStart.x);
                double start = MIN(releaseTime, pressTime);
                double end = MAX(releaseTime, pressTime);

                const u8* keys = SDL_GetKeyboardState(NULL);
                bool muted = false;
                if(keys[SDL_SCANCODE_M]) {
                    muted = true;
                }
                editedNote = (IdealNote){ editedNote.freq, start, end - start, muted, 100};

                if(base >= insertNote(editedNote)) {
                    base++;
                }
            }
        }
        if(selectingARange) {
            Rect selectionRect = { .x = MIN(dragStart.x, e.x),
                                   .y = MIN(dragStart.y, e.y),
                                   .w = abs(e.x-dragStart.x),
                                   .h = abs(e.y-dragStart.y) };
            FOR_NOTES(anote, piece) {
                Rect noteRect = noteToRect( size, pos, anote->note);
                bool horizontalOverlap = MIN(noteRect.x+noteRect.w, selectionRect.x+selectionRect.w)
                                > MAX(noteRect.x, selectionRect.x);
                bool verticalOverlap = MIN(noteRect.y+noteRect.h, selectionRect.y+selectionRect.h)
                                > MAX(noteRect.y, selectionRect.y);
                anote->selected = horizontalOverlap && verticalOverlap;
            }
        }
        dragStart = (Point){ -1, -1 };
        editedNote = (IdealNote){ -1,-1,-1 };
        selectingARange = false;
        copying = false;
        stopPlayingNote();
    }
    if(event.type == MouseWheel) {
        SDL_MouseWheelEvent e = event.wheel;

        DEBUG_PRINT(e.x, "%d");
        DEBUG_PRINT(e.y, "%d");
        SDL_Keymod km = SDL_GetModState();
        if(km & KMOD_CTRL) {
            double myFrac = 1 - (e.mouseY - pos.y) * 1.0/ size.h;
            double partOfRangeBelowScreen = 1-currentItemConfig->value.verticalFrac-currentItemConfig->value.verticalScroll;
            double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->value.verticalFrac*myFrac;
            currentItemConfig->value.verticalFrac /= pow(1.5, e.y);
            double newPartOfRangeBelowScreen = partOfRangeBelowMouse - currentItemConfig->value.verticalFrac*myFrac;
            currentItemConfig->value.verticalScroll = 1 - currentItemConfig->value.verticalFrac - newPartOfRangeBelowScreen;

        } else {
            currentItemConfig->value.horizontalFrac /= pow(1.5, e.y);
            double playhead = samplesToTime(currentPositionInSamples);
            currentItemConfig->value.horizontalScroll = (playhead - 1.0/2*pieceLength*currentItemConfig->value.horizontalFrac)/pieceLength;
            currentItemConfig->value.horizontalScroll = MAX(0, currentItemConfig->value.horizontalScroll);
        }
        /*if(e.y >  0)*/
    }
    if(event.type == ButtonPress) {
        SDL_MouseButtonEvent e =
                event.button;
        if(!pointInRect((Point){e.x, e.y}, rect)) goto fuckThisEvent;

        dragStart = (Point){ e.x, e.y };
        RealNote* select = NULL;
        FOR_NOTES(anote, piece) {
            if (pointInRect((Point) { e.x, e.y },
                           noteToRect(size, pos, anote->note))) {
                select = anote;//-piece;
//                DEBUG_PRINT(anote->start, "%lf");
            }
        }
        if(select && !select->selected) {
            FOR_NOTES(anote, piece) {
                anote->selected = false;
            }
            select->selected = true;
        }
        double freq = 0;
        if(select) {
            if(e.button == 2) base = (int)(select-piece);
            if(e.button == 1) dragged  = (int)(select-piece);
            freq = select->note.freq;
            draggedNoteInitialPos = select->note;
        }

        coord c = {yToFreq(size.h, pos, e.y), xToTime(size.w, e.x)};
        double time = closestTime(size.w, e.x);
//        DEBUG_PRINT(select, "%ld");

        if(base >= 0 && select == NULL && e.button == 1) {

            freq = closestFreq(size.h, pos, e.y);
            editedNote = (IdealNote){ freq, time, 0.5};
        }
        if(base < 0 && select == NULL && e.button == 1) {
//            fprintf(stderr, "oops im here !!!L_)(\n");
            editedNote = (IdealNote){ c.freq, time, 0.5};
            freq = c.freq;
        }
        if(select == NULL && e.button == 3) {
            selectingARange = true;
        }
        if(freq) startPlayingNote(freq);
    }
    if(event.type == MotionEvent) {
        SDL_MouseMotionEvent e =
                event.motion;
        if(e.y < pos.y) goto fuckThisEvent;
//        DEBUG_PRINT(editedNote.freq, "%lf")

        if(editedNote.freq >= 0) {
            coord c = {closestFreq(size.h, pos, e.y), closestTime(size.w, e.x)};
            guiSetForeground(p,0xff000000);
            Rect r = noteToRect(size, pos, editedNote);
            guiFillRectangle(p, r);
//            SDL_RenderPresent(p->gc);
            coord s = {yToFreq(size.h, pos, dragStart.y), closestTime(size.w, dragStart.x)};
            double start = MIN(s.time, c.time);
            double end = MAX(s.time, c.time);
            editedNote = (IdealNote){ /*s.freq,*/ editedNote.freq, start, end-start};
            r = noteToRect(size, pos, editedNote);
            guiSetForeground(p,0xffff77ff);
            guiFillRectangle(p, r);
        } else if(dragged >= 0) {
            ASSERT(piece[dragged].selected, "dragging unselected note?..");
            SDL_Keymod km = SDL_GetModState();
            if((km & KMOD_CTRL) && !copying) {
                copying = true;

                int initialSize = arrlen(piece);
                FOR(i, initialSize) { // we change the array so we can't iterate with pointer
                    if(piece[i].selected) {
                        RealNote newNote = piece[i];
                        newNote.selected = true;
                        piece[i].selected = false;
                        arrpush(piece, newNote);
                        if(dragged == i) {
                            dragged = arrlen(piece)-1;
                        }
                    }
                }
            }
            coord c = {closestFreq(size.h, pos, e.y), closestTime(size.w, e.x)};
            IdealNote draggedNote = piece[dragged].note;
            if(abs(e.y - dragStart.y) > 5) {
                draggedNote.freq = c.freq;
            } else {
                draggedNote.freq = draggedNoteInitialPos.freq;
            }
            if(abs(e.x - dragStart.x) > 5) {
                draggedNote.start = closestTime(size.w,
                                                e.x -
                                                dragStart.x + timeToX(size.w, draggedNoteInitialPos.start));
            } else {
                draggedNote.start = draggedNoteInitialPos.start;
            }
//            volatile int lll = arrlen(selectionSA);
//            int lastDragged = dragged;
            double chs = draggedNote.start-piece[dragged].note.start,
                   chf = draggedNote.freq/piece[dragged].note.freq;

            FOR_NOTES(anote, piece) {
                if(anote->selected) {
                    anote->note.start += chs;
                    anote->note.freq *= chf;
                }
            }

            //            removeNote(dragged);
                        //            dragged = insertNote(draggedNote);
//            if(dragged != lastDragged) {
//                volatile int  wer = 2;
//            }
            stopPlayingNote();
            startPlayingNote(draggedNote.freq);
        }
    }
    if(event.type == KeyPress && (GET_KEYSYM(event) == '\x7f' || GET_KEYSYM(event) == backspace)) {
        removeNotes(&base);
    }

    fuckThisEvent:
    guiUnsetClipRect(p);
    feedbackSize(size);

}
