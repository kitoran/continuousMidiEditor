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
#include "stb_image.h"

#include "melody.h"
#include <math.h>
#include <stdio.h>

//typedef struct {
//    int num;
//    int den;
//} fraction;
//fraction fractions[] = {{1, 11}, {11, 8}, {1,9}, {1,8}, {1, 7}, {1, 6}, {1, 5}, {2,9}, {1, 4}, {2, 7},
//                        {1, 3}, {3,8}, {2, 5}, {3, 7}, {4,9}, {1, 2}, {5,9}, {4, 7},
//                        {3, 5}, {5,8}, {2, 3}, {5, 7}, {3, 4}, {7, 9}, {4, 5},
//                        {5, 6}, {6, 7}, {7,8}, {8,9}, {1, 1}};
typedef struct Step {
    double ratio;
    char * desc;
    u32 color;
} Step;
#include "12edo.h"
#define RANGE_FOR_VELOCITY_DRAGGING 3
#define MAX_DEN 9
//double toDouble(fraction f) {
//   return f.num*1.0/f.den;
//}
#define SCALE_CENTER 261.625565301
//fraction searchFraction(double test) {
//    if(test > 1) {
//        fraction inv = searchFraction(1/test);
//        return  (fraction) { inv.den, inv.num };
//    }
//    int il = 0, ih = ELEMS(fractions)-1;
//    while(ih-il>1) {
//        fraction interm = fractions[(ih+il)/2];
//        if(test == toDouble(interm)) return interm;
//        if(test > toDouble(interm)) {
//            il = (ih+il)/2;
//        } else {
//            ih = (ih+il)/2;
//        }
//    }
//    if(fabs(toDouble(fractions[ih]) - test) > fabs(toDouble(fractions[il]) - test)) {
//        return fractions[il];
//    } else return fractions[ih];
//}
Step searchStep(Step* scale, int n, double test) {
    int il = 0, ih = n-1;
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

int compsteps(Step* a, Step* b) {
    if(a->ratio < b->ratio) return -1;
    if(a->ratio > b->ratio) return 1;
    return 0;
}
// Optimized function to print Farey sequence of order n
bool coprime(int n, int m) {
    int less = MIN(n, m);
    int more = MAX(n, m);
//    int gcd = less;
    while(less > 0) {
        int temp = less;
        less = more%less;
        more = temp;
    }
    return more == 1;
}
bool suitable(int n) {
    int primeIndex = 0;
    while(n > 1 && primeIndex < NUMBER_OF_PRIMES) {
        if(!currentItemConfig->scale.primes[primeIndex]) {
            primeIndex++;
            continue;
        }
        int pp = primes[primeIndex].prime;
        while(n % primes[primeIndex].prime == 0) {
            n /= primes[primeIndex].prime;
        }
        primeIndex++;
    }
    return n == 1;
}
void makeFareyScale()
{
    static int* components = NULL;
    FOR_STB_ARRAY(Step*, step, state.calculatedScale) {
        free(step->desc);
    }
    arrsetlen(state.calculatedScale, 0);
    arrsetlen(components, 0);
    for(int i = 1; i <= currentItemConfig->scale.maxComponent; i++) {
        if(suitable(i)) arrpush(components, i);
    }

    for(int numInd = 0; numInd < arrlen(components); numInd++) {
        for(int denInd = 0; denInd < arrlen(components); denInd++) {
            int x = components[numInd], y = components[denInd];
            if(coprime(components[numInd],components[denInd])) {
                    char* desc = malloc(30);
                    int written = snprintf(desc, 30,
                                           "%d/%d", (int)x, (int)y);
                    Step s = {
                        .ratio = x*1.0/y,
                        .desc = desc,
                        .color = rgb(255/MIN(x, y), MIN(255, 255/MIN(x, y)+120), 255/MIN(x, y)),
                    };
                    arrpush(state.calculatedScale, s);
            }
        }
    }
    qsort(state.calculatedScale, arrlen(state.calculatedScale), sizeof(*state.calculatedScale),
          (_CoreCrtNonSecureSearchSortCompareFunction)compsteps);
}
void makeCombinations()
{
    QUICK_ASSERT(state.base >= 0);
    QUICK_ASSERT(state.base2 >= 0);
    double f1 = piece[state.base].note.freq;
    double f2 = piece[state.base2].note.freq;
    double s = f1+f2, rs = s/2, d = fabs(f1-f2);
    double rd = d * pow(2, floor(log(rs/d)/log(2)));

    arrsetlen(state.combinations, 0);
    Step step = {
        .ratio = s,
        .desc = "sum tone",
        .color = 0xeef255,
    };
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = rs,
            .desc = "reduced sum",
            .color = 0xdde155,
        };
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = d,
            .desc = "difference",
            .color = 0x44e1cc,
        };
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = rd,
            .desc = "red. dif.",
            .color = 0x55e1dd,
        };
    arrpush(state.combinations, step);




    step = (Step){
            .ratio = 1/(1/f1+1/f2),
            .desc = "harm. sum",
            .color = 0xdd55ff,
        };
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = 2/(1/f1+1/f2),
            .desc = "red. harm. sum",
            .color = 0xee55ff,
        };
    double dd = 1/fabs(1/f1-1/f2);
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = dd,
            .desc = "harm. dif.",
            .color = 0x44ee77,
        };
    arrpush(state.combinations, step);
    step = (Step){
            .ratio = dd * pow(2, floor(log(rs/dd)/log(2))),
            .desc = "red. harm. dif.",
            .color = 0x55cc22,
        };
    arrpush(state.combinations, step);

    qsort(state.combinations, arrlen(state.combinations), sizeof(*state.combinations),
          (_CoreCrtNonSecureSearchSortCompareFunction)compsteps);
}
#define FREQ_MIN 8.1758
#define FREQ_MAX 12543.85
void makeEqualScale()
{
    FOR_STB_ARRAY(Step*, step, state.calculatedScale) {
        free(step->desc);
    }
    arrsetlen(state.calculatedScale, 0);
    double range = log(FREQ_MAX)-log(FREQ_MIN);
    double step = log(2)*currentItemConfig->scale.equave / currentItemConfig->scale.divisions/12;
    int i = (int)(-range/step);
    while(i < range/step) {
        char* desc = malloc(30);
        int rem = REM(i, currentItemConfig->scale.divisions);
        if(rem == 0) {
            snprintf(desc, 30,
                           "%d\\%d %d",
                           rem,
                           currentItemConfig->scale.divisions,
                           i/currentItemConfig->scale.divisions);
        } else {
            snprintf(desc, 30,
                           "%d\\%d",
                           rem,
                           currentItemConfig->scale.divisions);
        }
        double f1 = range/step, f2 = f1*i, f3 = exp(f2);
        Step s = {
            .ratio = exp(step*i),
            .desc = desc,
            .color = gray(255*(rem==0))
        };

        arrpush(state.calculatedScale, s);
        i++;
    }
}
void zoomInTime(double myFrac, int amount)
{
    double partOfItemToLeftOfScreen = currentItemConfig->horizontalScroll;
    double partOfItemToLeftOfMouse = partOfItemToLeftOfScreen + currentItemConfig->horizontalFrac*myFrac;
    currentItemConfig->horizontalFrac /= pow(1.5, amount);
    double newPartOfItemToLeftOfScreen = partOfItemToLeftOfMouse - currentItemConfig->horizontalFrac*myFrac;
    currentItemConfig->horizontalScroll = newPartOfItemToLeftOfScreen;
    if(currentItemConfig->horizontalScroll < 0) currentItemConfig->horizontalScroll = 0;
}
void zoomInFreq(double myFrac, int amount)
{
    double partOfRangeBelowScreen = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
    double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->verticalFrac*myFrac;
    currentItemConfig->verticalFrac /= pow(1.5, amount);
    double newPartOfRangeBelowScreen = partOfRangeBelowMouse - currentItemConfig->verticalFrac*myFrac;
    currentItemConfig->verticalScroll = 1 - currentItemConfig->verticalFrac - newPartOfRangeBelowScreen;

    if(currentItemConfig->verticalFrac > 1) currentItemConfig->verticalFrac = 1;
    if(currentItemConfig->verticalScroll < 0) {
        currentItemConfig->verticalScroll = 0;
    }
    if(currentItemConfig->verticalScroll + currentItemConfig->verticalFrac > 1) {
        currentItemConfig->verticalFrac = 1 - currentItemConfig->verticalScroll;
    }
}

void roll(Painter* p, Size size) {
//    Point pos = getPos();
    if(state.calculatedScale == 0 || state.recalculateScale) {
//        FOR(i, 20) {
//            arrpush(currentItemConfig->scale, s);
//
//            Step s = {f->num*1.0/f->den, desc, 0xffffffff};
//        }]
        if(currentItemConfig->scale.type == rational_intervals) {
            makeFareyScale();
        } else {
            makeEqualScale();
        }
        state.recalculateScale = false;
    }
    if(currentItemConfig->horizontalScroll > 1) {
       currentItemConfig->horizontalScroll = 0.9;
    }

//    DEBUG_PRINT(y, "in \'roll\'%d");
//    STATIC(Grid, grid, allocateGrid(2,3,0));
//    if(event.type  == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_RESIZED
//                                          ||event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED )) {
//        for(int i = 0; i < grid.gridWidthsLen; i++)
//            grid.gridWidths[i] = 0;
//        for(int i = 0; i < grid.gridHeightsLen; i++)
//            grid.gridHeights[i] = 0;
//    }
    Point pos = getPos();
    ExactLayout el = makeExactLayout(pos);
//    grid.gridStart = getPos();//(Point){0, y };
    pushLayout(&el);
//    setCurrentGridPos(0, 0);
//    Size windowSize; SDL_GetWindowSize(rootWindow, &windowSize.w, &windowSize.h);//.size;
    Size navigationSize = {size.w - SCROLLBAR_THICKNESS, NAVIGATION_THICKNESS};
    navigationBar(p, navigationSize);
    el.exactPos.y+=NAVIGATION_THICKNESS;
//    gridNextRow();
    Size noteAreaSize = {size.w - SCROLLBAR_THICKNESS, size.h-NAVIGATION_THICKNESS+5/*-NAVIGATION_THICKNESS*/
                        - SCROLLBAR_THICKNESS};
    noteArea(p, noteAreaSize);
//    gridNextRow();
    el.exactPos.y+=noteAreaSize.h;
    guiScrollBar(p,size.w-SCROLLBAR_THICKNESS*3, &currentItemConfig->horizontalScroll, currentItemConfig->horizontalFrac, true);
    if(currentItemConfig->horizontalScroll < 0) {
        currentItemConfig->horizontalScroll = 0;
    }
    //    setCurrentGridPos(1, 1);
    Size bsize = {SCROLLBAR_THICKNESS, SCROLLBAR_THICKNESS};
    el.exactPos.x+=size.w-SCROLLBAR_THICKNESS*3;
    if(guiToolButtonEx(p, "plus.png", false, false, &bsize, 0)) {
        zoomInTime(0.5, 1);
    }
    el.exactPos.x+=SCROLLBAR_THICKNESS;
    if(guiToolButtonEx(p, "minus.png", false, false, &bsize, 0)) {
        zoomInTime(0.5, -1);
    }

    el.exactPos.y=pos.y+NAVIGATION_THICKNESS;
    el.exactPos.x=pos.x+noteAreaSize.w;
    guiScrollBar(p,noteAreaSize.h-=SCROLLBAR_THICKNESS*2, &currentItemConfig->verticalScroll, currentItemConfig->verticalFrac, false);
    el.exactPos.y+=noteAreaSize.h;
    if(guiToolButtonEx(p, "plus.png", false, false, &bsize, 0)) {
        zoomInFreq(0.5, 1);
    }
    el.exactPos.y+=SCROLLBAR_THICKNESS;
    if(guiToolButtonEx(p, "minus.png", false, false, &bsize, 0)) {
        zoomInFreq(0.5, -1);
    }
    popLayout();
//    Size size = getGridSize(&grid);
    feedbackSize(size); // in principle i should probavly call feedbackSize() here but maybe not
    // if i do then i need to properly define the top level layout uin pmainprogram
}
double xToTime(int width, int mx) {
//    0 -> pieceLength*currentItemConfig->horizontalScroll
//    width -> pieceLength*(currentItemConfig->horizontalScroll+currentItemConfig->horizontalFrac)
    return mx*1.0/width*pieceLength*currentItemConfig->horizontalFrac + pieceLength*currentItemConfig->horizontalScroll;// horizontalScale+pieceLength*currentItemConfig->horizontalScroll;
}
int timeToX(int width, double time) {
    return (int)round((time - pieceLength*currentItemConfig->horizontalScroll)*width/pieceLength/currentItemConfig->horizontalFrac);
            //(time-pieceLength*currentItemConfig->horizontalScroll)*horizontalScale;
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
    guiSetForeground(p, gray(0x44));
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
        int c = timeToX(size.w, t->when-state.itemStart);
        guiDrawLine(p, c, pos.y, c, pos.y+size.h/2);
    }
    feedbackSize(size);
}
double yToFreqLinear(int height, Point pos, int my) {
    double verticalScrollMirrored = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+currentItemConfig->verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    int relativeY = my - pos.y;
    double myFrac = 1 - relativeY * 1.0/ height;
    return myFrac*(freqVMax-freqVMin) + freqVMin;
}
double freqToYLinear(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
    double freqVMin = verticalScrollMirrored * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;
    double freqVMax = (verticalScrollMirrored+currentItemConfig->verticalFrac) * (FREQ_MAX-FREQ_MIN) + FREQ_MIN;

    double myFrac = (freq - freqVMin) / (freqVMax-freqVMin);
    int relativeY = (int)round((1-myFrac)*height);
    return relativeY + pos.y;
}
//double yToFreq(int height, Point pos, int my) {
////    double verticalScrollMirrored = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
////    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
////    double freqVMax = (verticalScrollMirrored+currentItemConfig->verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

////    int relativeY = my - pos.y;
////    return exp(myFrac*((freqVMax)-(freqVMin)) + (freqVMin));

//    double myFrac = 1 - (my - pos.y) * 1.0/ height;
//    double range = log(FREQ_MAX)-log(FREQ_MIN);
//    double partOfRangeBelowScreen = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
//    double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->verticalFrac*myFrac;
//    double rangeBelowMouse = partOfRangeBelowMouse*range;
//    return exp(rangeBelowMouse);
//}
double yToFreq(int height, Point pos, int my) {
//    double verticalScrollMirrored = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
//    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
//    double freqVMax = (verticalScrollMirrored+currentItemConfig->verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

//    int relativeY = my - pos.y;
//    return exp(myFrac*((freqVMax)-(freqVMin)) + (freqVMin));

    double myFrac = 1 - (my - pos.y) * 1.0/ height;
    double range = log(FREQ_MAX)-log(FREQ_MIN);
    double partOfRangeBelowScreen = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
    double partOfRangeBelowMouse = partOfRangeBelowScreen + currentItemConfig->verticalFrac*myFrac;
    double rangeBelowMouse = partOfRangeBelowMouse*range;
    return exp(rangeBelowMouse+(log(FREQ_MIN)));
}
int freqToY(int height, Point pos, double freq) {
    double verticalScrollMirrored = 1-currentItemConfig->verticalFrac-currentItemConfig->verticalScroll;
    double freqVMin = verticalScrollMirrored * (log(FREQ_MAX)-log(FREQ_MIN)) + log(FREQ_MIN);
    double freqVMax = (verticalScrollMirrored+currentItemConfig->verticalFrac) * (log(FREQ_MAX)-log(FREQ_MIN)) +log( FREQ_MIN);

    double myFrac = (log(freq) - (freqVMin)) / ((freqVMax)-(freqVMin));
    int relativeY = (int)round((1-myFrac)*height);
    return (int)round(relativeY + pos.y);
}
Rect noteToRect(Size size, Point pos, IdealNote n) {
    Point s = { timeToX(size.w, n.start), freqToY(size.h, pos, n.freq) };
    int right = timeToX(size.w, n.start + n.length);
    return (Rect) { s.x, s.y - 5, MAX(right - s.x, 3), 10 };
}

double closestTime(int width, int x) {
    double test = xToTime(width, x);
    if(!currentItemConfig->horizontalSnap || (SDL_GetModState() & KMOD_SHIFT)) return test;
    int il = 0, ih = (int)arrlen(state.timesOfGridLines)-1;
    while(ih-il>1) {
        double interm = state.timesOfGridLines[(ih+il)/2];
        if(test == interm) return test;
        if(test > interm) {
            il = (ih+il)/2;
        } else {
            ih = (ih+il)/2;
        }
    }
    double closest;
    if(fabs(state.timesOfGridLines[ih] - test) > fabs(state.timesOfGridLines[il] - test)) {
        closest = state.timesOfGridLines[il];
    } else closest = state.timesOfGridLines[ih];
    double le = xToTime(width, x - 5);
    double mo = xToTime(width, x + 5);
    if(le < closest &&
       mo > closest) {
        return closest;
    } else {
        return test;
    }
}

double closestFreq(int height, Point pos, int y) {
    if (!currentItemConfig->verticalSnap || (SDL_GetModState() & KMOD_SHIFT)) return yToFreq(height, pos, y);

    double le = yToFreq(height, pos, y + 5);
    double mo = yToFreq(height, pos, y - 5);
    if(state.base2 >= 0) {
        Step step = searchStep(state.combinations, arrlen(state.combinations), yToFreq(height, pos, y));
        double tarfreq = step.ratio;
        if (le < tarfreq &&
            mo > tarfreq
            ) {
            return (step.ratio);
        } else return yToFreq(height, pos, y);
    }

    if(state.base >= 0 && arrlen(state.calculatedScale) > 0) {
        double whatSearching;
        if(currentItemConfig->scale.relative == scale_relative) whatSearching = yToFreq(height, pos, y) / piece[state.base].note.freq;
        else whatSearching = yToFreq(height, pos, y);
        Step step = searchStep(state.calculatedScale, arrlen(state.calculatedScale), whatSearching);

        int mm = freqToY(height, pos, mo);
        volatile double mo2 = yToFreq(height, pos, y);
        volatile int mm2 = freqToY(height, pos, mo2);
        double tarfreq = step.ratio  * piece[state.base].note.freq;
        if (le < tarfreq &&
            mo > tarfreq
            ) {
            return (currentItemConfig->scale.relative == scale_relative?piece[state.base].note.freq:1) * step.ratio;
        }
    }

    if(currentItemConfig->showScale) {
        Step step12edo = searchStep(twelveEdo, ELEMS(twelveEdo), yToFreq(height, pos, y));
        double tarfreq12 = step12edo.ratio;
        if (le < tarfreq12 &&
            mo > tarfreq12
            ) {
            return (step12edo.ratio);
        }
    }
    return yToFreq(height, pos, y);
}
inline double subbeatTime(TempoMarker* tm) {
//    return 60.0/tm->qpm*4/tm->denom;
    return 60.0/tm->qpm*4/currentItemConfig->subgridDen;
}

//RealNote** selectionSA = NULL;
const char* channelnames[]={"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16"};


//void ensureNoteGood(IdealNote* note)
//{
//    if(note->start < 0) {
//        note->start = 0;
//    }
//    if(note->start >= pieceLength) {
//        note->start = pieceLength - 0.002;
//    }
//    if(note->start+note->length >= pieceLength) {
//        note->length = pieceLength - note->start - 0.001;
//    }
//    if(note->length <= 0) {
//        note->length = 0.001;
//    }
//}

void noteArea(Painter* p, Size size) {
    Point pos = getPos();
    Rect rect = {0, pos.y, size.w, size.h};
    guiSetForeground(p,0x555555);
    guiFillRectangle(p, rect);
    STATIC(int, digSize, guiTextExtents("3/5", 3).h);
    double lastVisibleTime = xToTime(size.w, size.w);
//    double a = state.itemStart/BEAT;
//    double a = state.itemStart/BEAT;
    TempoMarker last = projectSignature; int next = 0; //double lastFraction = 0;
    int lastpixel = -10;
    int numOfBeat = 0;
    int numOfSubgrid = 0;
    double projectTime = 0;
    int measure = 1;
    arrsetlen(state.timesOfGridLines, 0);

    for(; projectTime < lastVisibleTime+state.itemStart; projectTime += subbeatTime(&last)) {
        while(next < arrlen(tempoMarkers) && projectTime >= tempoMarkers[next].when) {
            ASSERT(tempoMarkers[next].num != 0, "time signature is 0????");
            if(tempoMarkers[next].num > 0) {
                numOfBeat = 0;
                projectTime = tempoMarkers[next].when;
                last = tempoMarkers[next];
            } else {
                double partOfTheBeatBeforeMarker = (tempoMarkers[next].when - (projectTime-subbeatTime(&last)))/subbeatTime(&last);
                last.when = tempoMarkers[next].when;
                last.qpm = tempoMarkers[next].qpm;
//                projectTime = tempoMarkers[next].when + (1-partOfTheBeatBeforeMarker)*(beatTime(&last));
                projectTime = tempoMarkers[next].when + (1-partOfTheBeatBeforeMarker)*(subbeatTime(&last));
            }
            next++;
        }
        int c = timeToX(size.w, projectTime-state.itemStart);
        if(c >= rect.w) {
            break;
        }
        lastpixel = c;
        if(c >= 0) {
//            fprintf(stderr, "%d -----------------%d\n", c, c>=0);
            if(numOfBeat == 0 && numOfSubgrid == 0) {
                guiSetForeground(p, 0x626262);
            } else if(numOfSubgrid == 0) {
                guiSetForeground(p, 0x434343);
            } else {
                guiSetForeground(p, 0x4b4b4b);
            }
            arrpush(state.timesOfGridLines, projectTime-state.itemStart);
            guiDrawLine(p, c, pos.y, c, pos.y+size.h);
            if(numOfBeat == 0 && numOfSubgrid == 0) {
                guiDrawLine(p, c, pos.y - NAVIGATION_THICKNESS, c, pos.y);
                char num[5];
                snprintf(num, 5, "%d", measure);
                guiDrawTextZT(p, num, (Point){c+2, pos.y-15}, 0xaaaaaa);
            }
        }
        numOfSubgrid = (numOfSubgrid+1)%(currentItemConfig->subgridDen/last.denom);
        if(numOfSubgrid == 0) {
            numOfBeat = last.num?(numOfBeat+1)%last.num:0;
            if(numOfBeat == 0) {
                measure++;
            }
        }
    }
    guiSetClipRect(p, rect);
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
        if(playback > size.w) {
            currentItemConfig->horizontalScroll += currentItemConfig->horizontalFrac;
        }
        if(playback < 0) {
            currentItemConfig->horizontalScroll -= currentItemConfig->horizontalFrac;
        }
        guiSetForeground(p,  0x55999955);
        guiDrawLine(p, playback, pos.y, playback, pos.y+size.h);
    }
    guiSetForeground(p,0xff339887);
    guiDrawLine(p, cursor , pos.y, cursor , pos.y+size.h);
//    for(double freq = 440.0/32; freq < 20000; freq *= 2) {
//        char ferf[30]; snprintf(ferf,30,"%5lf", freq);
//        guiDrawTextZT(p, ferf, (Point) { 10, freqToY(size.h,
//                                        pos,
//                                        (int)round(freq))- digSize / 2 }, 0xffffffff);
//    }
//    guiDrawTextZT(p, "880", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    880)- digSize / 2 }, 0xffffffff);
//    guiDrawTextZT(p, "1760", (Point) { 10, freqToY(size.h,
//                                    pos,
//                                    1760)- digSize / 2 }, 0xffffffff);
    if(state.base2 >= 0) {
        FOR_STB_ARRAY(Step*, frac, state.combinations) {
            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, frac->ratio);
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                guiDrawLine(p, 0, r, size.w, r);
            }
        }
    } else if(state.base >= 0 || currentItemConfig->scale.relative == scale_absolute) {
//        FOR_STATIC_ARRAY(fraction*, frac, fractions) {

//            guiSetForeground(p, gray((MAX_DEN+1-frac->den) *255 / (MAX_DEN+1)));
////            for(int i = 0; i < 3; i++) {
//            int r = freqToY(size.h, pos, piece[state.base].note.freq * frac->num/frac->den);
//            char str[30];
//            if(r >= pos.y && r < (int)(pos.y+size.h)) {
//                guiDrawLine(p, 0, r, size.w, r);
//                sprintf(str, "%d/%d", frac->num, frac->den);
//                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
//            }
//            r = freqToY(size.h, pos, piece[state.base].note.freq / frac->num*frac->den);
//            if(r >= pos.y && r < (int)(pos.y+size.h)) {
//                guiDrawLine(p, 0, r, size.w, r);
//                sprintf(str, "%d/%d", frac->den, frac->num);
//                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xffffffff);
//            }
//        }
        FOR_STB_ARRAY(Step*, frac, state.calculatedScale) {
            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, (currentItemConfig->scale.relative == scale_relative?piece[state.base].note.freq:1) * frac->ratio);
//            char str[30];
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                guiDrawLine(p, 0, r, size.w, r);
            }
        }
    }
    if(currentItemConfig->showScale) {
//        double multiplier = pow(2, 1.0/16);
//        for(double freq = SCALE_CENTER/8; freq < FREQ_MAX; freq *=multiplier) {
//            guiSetForeground(p, 0x555555);
//            int r = freqToY(size.h, pos, freq);
//            if(r >= pos.y && r < (int)(pos.y+size.h)) {
//                guiDrawLine(p, 0, r, size.w, r);
//            }
//        }
        FOR_STATIC_ARRAY(Step*, s, twelveEdo) {
            int r = freqToY(size.h, pos, s->ratio);
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                guiSetForeground(p, s->color);
                guiDrawLine(p, 0, r, size.w, r);
            }
        }
    }
//    static bool midimaploaded = false;
//    if(!
    int dummy = (int)arrlen(piece);
    STATIC(unsigned char *, midiNoteMap, stbi_load(MY_PATH "/midi_note_colormap.png",
                                        &dummy, &dummy, &dummy, 0));

    FOR_NOTES(anote, piece) {
        Rect r = noteToRect(size, pos, anote->note);
        guiSetForeground(p, anote->selected?0xff777777:0xff333333);
        bool base = anote == piece + state.base ||
                anote == piece + state.base2;
        if(base) guiSetForeground(p, anote->selected?0xff222222:0xff000000);

        guiFillRectangle(p,
                         r);
        if(anote->note.muted) {
            guiSetForeground(p, 0xff1b1b1b);
        } else {
            if(currentItemConfig->showChannels) {
                double hue = 360/16*anote->midiChannel;
                guiSetForeground(p, hsvd2bgr(hue,anote->selected?0.3:1,1));
            } else {
                unsigned char* velbase = midiNoteMap+3*12+3*anote->note.velocity;
                if(anote->selected) velbase += 157*65*3;
                guiSetForeground(p, rgb(velbase[0], velbase[1], velbase[2]));
            }
        }
        int coef = (base)?3:1;
        guiFillRectangle(p,
                         (Rect){.x=r.x+1*coef, .y=r.y+1*coef, .w=r.w-2*coef, .h=r.h-2*coef});
        if(currentItemConfig->showChannels || state.mouseMode == draggingVelocity) {
            //TODO: fix
//            guiDrawTextZT(p, channelnames[anote->midiChannel], r.pos,
//                    anote->selected||anote == piece + state.base?0:0xffffffff);
            char fwef[40];// = "30";
            if(currentItemConfig->showChannels) snprintf(fwef, sizeof(fwef), "%d  %d   %d", (int)(anote-piece), anote->reaperNumber, anote->midiChannel);
            if(currentItemConfig->showMidi) {
                MidiPitch mp = getMidiPitch(anote->note.freq, currentItemConfig->pitchRange);
                snprintf(fwef, sizeof(fwef), "%d %d", mp.key, mp.wheel);
            }
            if(state.mouseMode == draggingVelocity) snprintf(fwef, sizeof(fwef), "%d", anote->note.velocity);
            guiDrawTextZT(p, fwef, r.pos,
                    anote->selected||base?0:0xffffffff);
        }
    }
    if(state.base >= 0|| currentItemConfig->scale.relative == scale_absolute) {
        FOR_STB_ARRAY(Step*, frac, state.base2 < 0?state.calculatedScale:state.combinations) {
            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, (state.base2 > 0 || currentItemConfig->scale.relative == scale_absolute?1:piece[state.base].note.freq)* frac->ratio);
            char str[30];
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                snprintf(str, sizeof(str), "%s", frac->desc);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xff000000);
            }
        }
    }
    if(currentItemConfig->showScale) {
        FOR_STATIC_ARRAY(Step*, frac, twelveEdo) {
            guiSetForeground(p, frac->color);
            int r = freqToY(size.h, pos, frac->ratio);
            char str[30];
            if(r >= pos.y && r < (int)(pos.y+size.h)) {
                snprintf(str, sizeof(str), "%s", frac->desc);
                guiDrawTextZT(p, str, (Point) { 10, r - digSize / 2 }, 0xff000000);
            }
        }
    }
    if(state.newNote.freq >= 0) {
        Rect r = noteToRect(size, pos, state.newNote);
        guiSetForeground(p, 0xffff77ff);
        guiFillRectangle(p,
                         r);
    }

    static int lastTouchedVel = 100;
    if(state.mouseMode == selectingARange) {
        Point mouse; SDL_GetMouseState(&mouse.x, &mouse.y);
//        mouse.x -= pos.x; mouse.y -= pos.y;
        Rect r = { .pos = state.dragStart, .size = {mouse.x-state.dragStart.x, mouse.y-state.dragStart.y} };
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

    SDL_Keymod km = SDL_GetModState();
    if(event.type == ButtonRelease) {

        FOR_STB_ARRAY_I(i, piece) {
            QUICK_ASSERT(piece[i].note.start >= 0);
            QUICK_ASSERT(piece[i].note.length > 0);
        }

        SDL_MouseButtonEvent e =
                event.button;

        if(state.dragged >= 0) {
//            IdealNote draggedNote = piece[state.dragged].note;
            if(state.mouseMode == copyingNote) {
                commitChanges( &state.dragged, &state.base, &state.base2, "copy note(s)");
            } else if(state.mouseMode == stretchingRight ||
                      state.mouseMode == stretchingLeft) {
                commitChanges( &state.dragged, &state.base, &state.base2, "stretch note(s)");
            } else if(state.mouseMode == limboCopyingOrDeselecting) {
                piece[state.dragged].selected = false;
            } else {
                commitChanges( &state.dragged, &state.base, &state.base2, "move note(s)");
            }
        }
        state.dragged = -1;
        double releaseTime = closestTime(size.w, e.x);
        if(state.newNote.freq >= 0) {
//            if(pointInRect((Point){e.x, e.y}, rect)) {
                double pressTime = closestTime(size.w, state.dragStart.x);
                double start = MAX(MIN(releaseTime, pressTime), 0);
                double end = MIN(MAX(releaseTime, pressTime), pieceLength);
                if(end - start < 0.001) {
                    end  = start + 0.001;
                }
                const u8* keys = SDL_GetKeyboardState(NULL);
                bool muted = false;
                if(keys[SDL_SCANCODE_M]) {
                    muted = true;
                }
                state.newNote = (IdealNote){ state.newNote.freq, start, end - start, muted, lastTouchedVel};
                int newIndex = insertNote(state.newNote);
                if(state.base >= newIndex) {
                    state.base++;
                }
                if(state.base2 >= newIndex) {
                    state.base2++;
                }
//            }
        }
        if(state.mouseMode == selectingARange) {
            Rect selectionRect = { .x = MIN(state.dragStart.x, e.x),
                                   .y = MIN(state.dragStart.y, e.y),
                                   .w = abs(e.x-state.dragStart.x),
                                   .h = abs(e.y-state.dragStart.y) };
            FOR_NOTES(anote, piece) {
                Rect noteRect = noteToRect( size, pos, anote->note);
                bool horizontalOverlap = MIN(noteRect.x+noteRect.w, selectionRect.x+selectionRect.w)
                                > MAX(noteRect.x, selectionRect.x);
                bool verticalOverlap = MIN(noteRect.y+noteRect.h, selectionRect.y+selectionRect.h)
                                > MAX(noteRect.y, selectionRect.y);
                if(km & KMOD_SHIFT) {
                    anote->selected = anote->selected ||
                            (horizontalOverlap && verticalOverlap);
                } else if(km & KMOD_CTRL) {
#define XOR !=
                    anote->selected = anote->selected XOR
                            (horizontalOverlap && verticalOverlap);
                } else {
                    anote->selected = horizontalOverlap && verticalOverlap;
                }
            }
        }

//        if(state.mouseMode == limboCopyingOrDeselecting) {
//            piece[state.dragged].selected = !piece[state.dragged].selected;
//        }
        state.dragStart = (Point){ -1, -1 };
        state.newNote = (IdealNote){ -1,-1,-1 };
        state.mouseMode = justMovingMouse;

        stopPlayingNoteIfPlaying();
    }
    if(event.type == MouseWheel) {
        SDL_MouseWheelEvent e = event.wheel;

        DEBUG_PRINT(e.x, "%d");
        DEBUG_PRINT(e.y, "%d");
        if(km & KMOD_CTRL) {
            double myFrac = 1 - (e.mouseY - pos.y) * 1.0/ size.h;
            int amount = e.y;
            zoomInFreq(myFrac, amount);

        } else {
            double myFrac = (e.mouseX - pos.x) * 1.0/ size.w;
            zoomInTime(myFrac, e.y);
//            currentItemConfig->horizontalFrac /= pow(1.5, e.y);
//            double playhead = samplesToTime(currentPositionInSamples);
//            currentItemConfig->horizontalScroll = (playhead - 1.0/2*pieceLength*currentItemConfig->horizontalFrac)/pieceLength;
//            currentItemConfig->horizontalScroll = MAX(0, currentItemConfig->horizontalScroll);
        }
        /*if(e.y >  0)*/
    }
    if(event.type == ButtonPress) {
        SDL_MouseButtonEvent e =
                event.button;
        if(!pointInRect((Point){e.x, e.y}, rect)) goto fuckThisEvent;

        state.dragStart = (Point){ e.x, e.y };
        RealNote* select = NULL;
        FOR_NOTES(anote, piece) {
            if (pointInRect((Point) { e.x, e.y },
                           noteToRect(size, pos, anote->note))) {
                select = anote;//-piece;
//                DEBUG_PRINT(anote->start, "%lf");
            }
        }
//        bool wasSelected = select?select->selected:false;

        double freq = 0;
//        int vel = lastTouchedVel;
        if(select) {
            if(e.button == 2) {
                if(km & (KMOD_SHIFT | KMOD_CTRL) && state.base >= 0) {
                    state.base2 = (int)(select-piece);
                    makeCombinations();
                } else {
                    state.base = (int)(select-piece);
                    state.base2 = -1;
                }
            }
            if(e.button == 1)  {
                Rect noteRect = noteToRect(size, pos, select->note);
                if(e.x >= MAX(noteRect.x+noteRect.w-5, noteRect.x+noteRect.w/2)) {
                    state.mouseMode = km&KMOD_ALT?stretchingRight:draggingRightEdge;
                } else if(e.x <= MIN(noteRect.x+5, noteRect.x+noteRect.w/2)) {
                    state.mouseMode = km&KMOD_ALT?stretchingLeft:draggingLeftEdge;
                } else if(e.y <= noteRect.y+RANGE_FOR_VELOCITY_DRAGGING) {
                    state.mouseMode = draggingVelocity;
                } else if( select->selected
                           && (km & KMOD_CTRL)) {
                    state.mouseMode = limboCopyingOrDeselecting;
                } else {
                    state.mouseMode = movingNote; // the user can then press ctrl to change mode to copying,
                    // but only if they haven't moved the mouse tooo far yet
                    if(!select->selected && !(km & (KMOD_SHIFT | KMOD_CTRL)))  {
                        FOR_NOTES(anote, piece) {
                            anote->selected = false;
                        }
                    }
                    select->selected = true;
                }
                state.dragged  = (int)(select-piece);
            }
            freq = select->note.freq;
            lastTouchedVel = select->note.velocity;
            state.draggedNoteInitialPos = select->note;
        }
        else if (e.x >= timeToX(size.w, pieceLength) - 5
                                &&
                  e.x <= timeToX(size.w, pieceLength) + 5) {
            state.mouseMode = draggingEnd;
        } else
//        coord c = {yToFreq(size.h, pos, e.y), xToTime(size.w, e.x)};
//        DEBUG_PRINT(select, "%ld");

        if(select == NULL && e.button == 1) {
            freq = closestFreq(size.h, pos, e.y);
            double time = closestTime(size.w, e.x);
            state.newNote = (IdealNote){ freq, time, xToTime(size.w, e.x+1)-time, false, lastTouchedVel};
        } else
        if(select == NULL && e.button == 3) {
            state.mouseMode = selectingARange;
        } else
        if(select == NULL && e.button == 2) {
            state.base = state.base2 = -1;
        }
        if(freq) startPlayingNote(freq, lastTouchedVel);
        FOR_STB_ARRAY_I(i, piece) {
            QUICK_ASSERT(piece[i].note.start >= 0);
            QUICK_ASSERT(piece[i].note.length >= 0);
        }
    }
    if(event.type == MotionEvent) {
        SDL_MouseMotionEvent e =
                event.motion;
        if(e.y < pos.y) goto fuckThisEvent;
//        DEBUG_PRINT(editedNote.freq, "%lf")

        if(state.newNote.freq >= 0) {
            coord c = {closestFreq(size.h, pos, e.y), closestTime(size.w, e.x)};
            guiSetForeground(p,0xff000000);
            Rect r = noteToRect(size, pos, state.newNote);
            guiFillRectangle(p, r);
//            SDL_RenderPresent(p->gc);
            coord s = {yToFreq(size.h, pos, state.dragStart.y), closestTime(size.w, state.dragStart.x)};
            double start = MIN(s.time, c.time);
            double end = MAX(s.time, c.time);
            state.newNote = (IdealNote){ /*s.freq,*/ state.newNote.freq, start, end-start};
            r = noteToRect(size, pos, state.newNote);
            guiSetForeground(p,0xffff77ff);
            guiFillRectangle(p, r);
        } else if(state.dragged >= 0) {
//
            static RealNote* rollback = NULL;
            arrsetlen(rollback, arrlen(piece));
            memcpy(rollback, piece, arrlen(piece)*sizeof(piece[0]));

            SDL_Keymod km = SDL_GetModState();
            if((km & KMOD_CTRL) && state.mouseMode == state.mouseMode == movingNote
                    || state.mouseMode == limboCopyingOrDeselecting) {
//                ASSERT(
//            || , "state.mouseMode is %d", state.mouseMode)
                state.mouseMode = copyingNote;

                int initialSize = (int)arrlen(piece);
                FOR(i, initialSize) { // we change the array so we can't iterate with pointer
                    if(piece[i].selected) {
                        RealNote noteCopy = piece[i];
                        noteCopy.selected = true;
                        piece[i].selected = false;
                        arrpush(piece, noteCopy);
                        if(state.dragged == i) {
                            state.dragged = (int)arrlen(piece)-1;
                        }
                    }
                }
            }
            coord c = {closestFreq(size.h, pos, e.y), closestTime(size.w, e.x)};
            IdealNote draggedNote = piece[state.dragged].note;
            if(state.mouseMode == movingNote || state.mouseMode == copyingNote) {
                ASSERT(piece[state.dragged].selected, "dragging unselected note?..");
                if(abs(e.y - state.dragStart.y) > 5 || (SDL_GetModState() & KMOD_SHIFT)) {
                    draggedNote.freq = c.freq;
                } else {
                    draggedNote.freq = state.draggedNoteInitialPos.freq;
                }
                if(abs(e.x - state.dragStart.x) > 5 || (SDL_GetModState() & KMOD_SHIFT)) {
                    draggedNote.start = closestTime(size.w,
                            //TODO: figure out (and comment) why i can't just use e.x
                                                    e.x -
                                                    state.dragStart.x + timeToX(size.w, state.draggedNoteInitialPos.start));
                } else {
                    draggedNote.start = state.draggedNoteInitialPos.start;
                }
            }
            bool draggedSelected = piece[state.dragged].selected;
            if(state.mouseMode == stretchingRight && piece[state.dragged].selected) {
                double start = -1;
                FOR_NOTES(anote,piece ) {
                    if(anote->selected) {
                        start = anote->note.start;
                        break;
                    }
                }
                QUICK_ASSERT(start >= 0);
                double debug1 = (xToTime(size.w,e.x) - start);
                double debug2 = (state.draggedNoteInitialPos.start+
                                 state.draggedNoteInitialPos.length -start);

                double scale = (xToTime(size.w,e.x) - start)/(piece[state.dragged].note.start+
                                                      piece[state.dragged].note.length -start)*0.99;
                FOR_NOTES(anote, piece) {
                    if(anote->selected) {
                        anote->note.start = (anote->note.start - start)*scale+start;
                        anote->note.length *= scale;
                    }
                }
            }
            if(state.mouseMode == stretchingLeft && piece[state.dragged].selected) {
                double end = -1;
                FOR_NOTES(anote,piece ) {
                    if(anote->selected && anote->note.start+anote->note.length>end) {
                        end = anote->note.start+anote->note.length;
                    }
                }
                double scale = (end - xToTime(size.w,e.x))/(end - piece[state.dragged].note.start)*0.99;
                FOR_NOTES(anote, piece) {
                    if(anote->selected) {

//                        double noteEnd = anote->note.start + anote->note.length;
//                        anote->note.end = (anote->note.end - end)*scale+end;
                        anote->note.start = (anote->note.start - end)*scale+end;//(noteEnd - end)*scale -anote->note.length  ;
                        anote->note.length *= scale;

//                        QUICK_ASSERT(anote->note.start >= 0);
//                        QUICK_ASSERT(anote->note.start+anote->note.length <= pieceLength);
//                        QUICK_ASSERT(anote->note.length > 0);
                    }
                }
            }
            if(state.mouseMode == draggingLeftEdge) {
                double t = draggedNote.start;
                draggedNote.start = closestTime(size.w,
                                                e.x -
                                                state.dragStart.x + timeToX(size.w, state.draggedNoteInitialPos.start));
                draggedNote.length += t-draggedNote.start;

//                QUICK_ASSERT(draggedNote.start >= 0);
//                QUICK_ASSERT(draggedNote.start+draggedNote.length <= pieceLength);
//                QUICK_ASSERT(draggedNote.length > 0);
            }
            if(state.mouseMode == draggingRightEdge) {
                double end = closestTime(size.w,
                                                e.x);
                draggedNote.length = end-draggedNote.start;

//                QUICK_ASSERT(draggedNote.start >= 0);
//                QUICK_ASSERT(draggedNote.start+draggedNote.length <= pieceLength);
//                QUICK_ASSERT(draggedNote.length > 0);
            }
            int chv = 0;
            if(state.mouseMode == draggingVelocity) {
                chv = e.yrel;
                SDL_WarpMouseInWindow(rootWindow, e.x, e.y-e.yrel);
            }
//            QUICK_ASSERT(draggedNote.length > 0);
            if(draggedNote.length <= 0) draggedNote.length = 0.01;
            double chs = draggedNote.start-piece[state.dragged].note.start,
                   chf = draggedNote.freq/piece[state.dragged].note.freq,
                   chl = draggedNote.length-piece[state.dragged].note.length;

            if(state.mouseMode != stretchingRight && state.mouseMode != stretchingLeft) {
                if(piece[state.dragged].selected) {
                    FOR_NOTES(anote, piece) {
                        if(anote->selected) {
                            anote->note.start += chs;
                            anote->note.freq *= chf;
                            anote->note.length += chl;

                            anote->note.velocity = CLAMP(anote->note.velocity - chv, 0, 127);

    //                        ensureNoteGood(anote->note);
                        }
                    }
                } else {
                    piece[state.dragged].note.start += chs;
                    piece[state.dragged].note.freq *= chf;
                    piece[state.dragged].note.length += chl;

                    piece[state.dragged].note.velocity = CLAMP(piece[state.dragged].note.velocity - chv, 0, 127);
                }
            }
            bool bad = false;
            FOR_STB_ARRAY_I(i, piece) {
//                ensureNoteGood(&piece[i].note);
                if(piece[i].note.start < 0
                || piece[i].note.start+piece[i].note.length > pieceLength
                || piece[i].note.length <= 0) {
                    bad = true;
                    break;
                }
            }
            if(bad) {
                memcpy(piece, rollback, arrlen(piece)*sizeof(piece[0]));
            }
            memset(rollback, -1, arrlen(rollback)*sizeof(rollback[0]));
            //            removeNote(state.dragged);
                        //            state.dragged = insertNote(draggedNote);
//            if(state.dragged != laststate.dragged) {
//                volatile int  wer = 2;
//            }
//            stopPlayingNote();
            startPlayingNote(draggedNote.freq, draggedNote.velocity);
        } else if(state.mouseMode == draggingEnd) {
            pieceLength = xToTime(size.w, e.x);
//            timeToX(pieceLength) = e.x;
            double leftTime = xToTime(size.w, 0);
//            (pieceLength - pieceLength*horizontalScroll)*width/pieceLength/currentItemConfig->horizontalFrac = e.x;
//            (leftTime - pieceLength*horizontalScroll)*width/pieceLength/currentItemConfig->horizontalFrac = 0;

            currentItemConfig->horizontalFrac = (pieceLength - leftTime)*size.w/pieceLength/e.x;
            currentItemConfig->horizontalScroll = leftTime/pieceLength;


        } else {
            STATIC(SDL_Cursor*, defaultCursor, SDL_GetDefaultCursor());
            STATIC(SDL_Cursor*, leftRight, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE));
            RealNote* noteUnderCursor = NULL;
            FOR_NOTES(anote, piece) {
                Rect noteRect = noteToRect(size, pos, anote->note);
                if (pointInRect((Point) { e.x, e.y },
                               noteRect)) {
                    noteUnderCursor = anote;
                    if(e.x >= MAX(noteRect.x+noteRect.w-5, noteRect.x/*+noteRect.w/2*/)
                            ||
                        e.x <= MIN(noteRect.x+5, noteRect.x+noteRect.w/*/2*/)) {
                        SDL_SetCursor(leftRight);
                    } else if(e.y <= noteRect.y+RANGE_FOR_VELOCITY_DRAGGING) {
                        STATIC(SDL_Cursor*, upDown, SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
                        SDL_SetCursor(upDown);
                    } else {
                        SDL_SetCursor(defaultCursor);
                    }
                }
            }
            if(e.x >= timeToX(size.w, pieceLength)-5
                    &&
                    e.x <= timeToX(size.w, pieceLength)+5) {
                SDL_SetCursor(leftRight);
            } else if(noteUnderCursor == NULL) {
                SDL_SetCursor(defaultCursor);
            }
        }
    }
    if(event.type == KeyPress) {
        int keyPressed = GET_KEYSYM(event);
        SDL_Keymod km = SDL_GetModState();
        if(keyPressed == '\x7f' || keyPressed == backspace) {
            removeNotes(&state.base, &state.base2);
        }
        if(keyPressed == 'a' && (km & KMOD_CTRL)) {
            FOR_NOTES(anote, piece) {
                anote->selected = true;
            }
        }
    }

    fuckThisEvent:
    guiUnsetClipRect(p);
    feedbackSize(size);

}
