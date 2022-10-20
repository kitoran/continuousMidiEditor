#include <gui.h>
#include <gridlayout.h>
#include <stdbool.h>
#include "stb_ds.h"
//const char* appName = "FractionalMidi";
//int main() {
//    guiStartDrawing();
//}
///*
#include "misc.h"
#include <SDL2/SDL_ttf.h>
//#include <SDL2/SDL2.h>
enum { SCREEN_WIDTH = 800,
       SCREEN_HEIGHT = 600};
int windowWidth = SCREEN_WIDTH;
int windowHeight = SCREEN_HEIGHT;
//const double tau = 6.283185307179586;
TTF_Font* Sans;
typedef struct textures {
    char* key;
    SDL_Texture*value;
} textures;
textures *textTextures = NULL;
void drawText(char* text, SDL_Renderer* renderer,  int x, int y) {
    fprintf(stderr,"drawText {%s}" ,text);
    //  var zigs = std.mem.span(text);
    //  var value = textTextures.get(zigs);
    //  if (value == NULL) {
    SDL_Color White= {.r = 255, .g = 255, .b = 255, .a = 255};
    SDL_Surface *surfaceMessage =
            TTF_RenderUTF8_Solid(Sans, text, White);
    SDL_Texture*textext = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    //     textTextures->put(zigs, textext);
    SDL_Texture*value = textext;
    SDL_FreeSurface(surfaceMessage);
    //  }
    SDL_Texture* texture = value;
    int w ;  int h  ;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
    SDL_Rect  Message_rect= {.x = x,  //controls the rect's x coordinate
                             .y = y, // controls the rect's y coordinte
                             .w = w, // controls the width of the rect
                             .h = h}; // controls the height of the rect
    SDL_RenderCopy(renderer, texture, 0, &Message_rect);
    SDL_DestroyTexture(textext);
}

typedef struct Note {
    double freq;
    double start;
    double length;
} Note;

Note* piece = NULL;
double smoothstep (double edge0, double edge1, double x)
{
   if (x < edge0)
      return 0;

   if (x >= edge1)
      return 1;

   // Scale/bias into [0..1] range
   x = (x - edge0) / (edge1 - edge0);

   return x * x * (3 - 2 * x);
}
double  freq1 = 110.0;
double  freq2 = 110.0*1.09050773267 ;
double  freq3 = 110.0*1.09050773267 *1.09050773267 ;//*1.61803398875;
SDL_AudioSpec  have;
#define EASE_FRAMES 40


int countSamples(double time) {
    return time * have.freq;
}

void buffer(void* userdata,
            Uint8* stream8,
            int wrongLen)
{
    int trueLen = wrongLen/2;
    i16* stream = stream8;
    typedef struct StrippedNote {
        int samples;
        double freq;
    } StrippedNote;

    StrippedNote newNote() {
        static int position = 0;
        static bool started = false;
        Note* currentNote = piece+position;
//        static double time = 0;
        const Note silenceEnd = {INFINITY, INFINITY, 0};
        const Note silenceStart = {0, 0, 0};
        Note* nextNote = position+1 < arrlen(piece)? piece+position+1 : &silenceEnd;
        const Note* previousNote = position > 0? piece+position-1 : &silenceStart;
        double duration = started?MIN(currentNote->length, nextNote->start - currentNote->start):
                               MAX(currentNote->start - (previousNote->start+previousNote->length), 0);

        StrippedNote res = { countSamples(duration),
                             started? currentNote->freq : 0};
        if(!started) started = true;
        else {
            position++;
            started = false;
            if(position >= arrlen(piece)) position = 0;
        }
        return res;
    }

    static struct State {
        int samplesToFill;
        double freq;
        bool smooth;
        double lastFreq;
        int framesLeftLastFreq;
    } state;
    loop:
    int samplesToFillNow = MIN(trueLen, state.samplesToFill);

    static u32 phase = 0;
    for(int i = 0; i < samplesToFillNow; i++) {
        stream[i] = (sin(phase/4.0*tau/(SDL_MAX_UINT32/4))*1000);
        if(state.framesLeftLastFreq == 0) {
            phase = phase+(SDL_MAX_UINT32/((double)have.freq)*(state.freq));
        } else {
            phase = phase +
                     ((SDL_MAX_UINT32/((double)have.freq)*(state.lastFreq)))*
                            (state.framesLeftLastFreq/(double)EASE_FRAMES)
                    + (SDL_MAX_UINT32/((double)have.freq)*(state.freq))*
                            (1-state.framesLeftLastFreq/(double)EASE_FRAMES);
            state.framesLeftLastFreq--;
        }
    }
    state.samplesToFill -= samplesToFillNow;
    stream += samplesToFillNow;
    trueLen -= samplesToFillNow;
    if(state.samplesToFill == 0) {
        StrippedNote note = newNote();
        struct State newstate = {
            note.samples,
            note.freq,
            true,
            state.freq,
            EASE_FRAMES
        };
        state = newstate;
        goto loop;
    }

}




//void notes(   void*  userdata,
//                        Uint8* stream8,
//                        int wrongLen)
//{
//    bool buffer(double freq, int samples) {
//        void fill( i16* stream, int toFill, double freq,
//                   bool smooth, double prev, int whereInSmoothTransition) {
//            static u32 phase = 0;
//            for(int i = 0; i < toFill; i++) {
//                stream[i] = (sin(phase/4.0*tau/(SDL_MAX_UINT32/4))*1000);
//                if(framesLeftLastFreq == 0) {
//                    phase = phase+(SDL_MAX_UINT32/((double)have.freq)*(freq));
//                } else {
//                    phase = phase +
//                             ((SDL_MAX_UINT32/((double)have.freq)*(lastFreq)))*(framesLeftLastFreq/(double)EASE_FRAMES)
//                            + (SDL_MAX_UINT32/((double)have.freq)*(freq))*(1-framesLeftLastFreq/(double)EASE_FRAMES);
//                    framesLeftLastFreq--;
//                }
//            }
//        }
//        int correntLen = wrongLen / 2;
//        static int filledPart = 0;



//        static double lastFreq = 0;
//        static int framesLeftLastFreq = 0;

////        len /= 2; // in samples
//        static i64 lenSinceLast = 0;
//        static i64 totalLenSoFar = 0;

//        i16* stream = (i16*)stream8;
//        int filledPart = 0;
//        int toFill = MIN(wrongLen-filledPart, duration);
//        fill(stream+filledPart, toFill, currentNote->freq);
//        lenSinceLast += toFill;
//        filledPart += toFill;
//        fprintf(stderr, "filled for first time %d, %d unfilled\n", toFill, wrongLen - filledPart);
//    }

//    loopStart: // to potential employers: i won't use labels and gotos unless you
//                // ask me. In my own codebase i just think it's fun to use language features
//     bool more = buffer(started?0:currentNote->freq, countSamples(duration));
//    if(more) {
//        position++;
//        goto loopStart;
//    }

////    static i64 fulllen = 0;
////    fulllen += len;
////    static i64 lenToNext = 0;
////    fulllen += len;

//    if(toFill < wrongLen) {
//        lastFreq = currentNote->freq;
//        framesLeftLastFreq = EASE_FRAMES;
//        if(


//        position++;
//        if(position >= arrlen(piece)) position = 0;
//        lenSinceLast = 0;

//        currentNote = piece+position;
//        duration = currentNote->length*have.freq - lenSinceLast;
//        toFill = MIN(wrongLen-filledPart, duration);
//        fill(stream+filledPart, toFill, currentNote->freq);
//        lenSinceLast += toFill;
//        filledPart += toFill;
//        fprintf(stderr, "filled for second time %d, %d unfilled\n", toFill, wrongLen - filledPart);
//    }

////    fprintf(stderr, "unfilled part is %d\n", len - filledPart);
////    for(int i = filledPart; i < MIN(len, duration+filledPart); i++) {
////        stream[i] = (sin(phase * position->freq * tau / (have.freq))*1000);
////        phase = phase+1;
////    }
//    //  fprintf(stderr,"callback {}" ,len);
//    //      userdata;
//    //    u32  i = 0;
////    while(i <  len/ 2) {
////        stream[i] = (sin(phase * freq1 * tau / (have.freq))*1000) +(sin(phase * freq2 * tau / (have.freq))*1000)
////                +(sin(phase * freq3 * tau / (have.freq))*1000);
////        //        stream[i] = (sin(phase * freq1 * tau / (have.freq))*1000) +(sin(phase * freq2 * tau / (have.freq))*1000)
////        //                +(sin(phase * freq3 * tau / (have.freq))*1000);
////        i+=1;
////    }
//}


void insert(Note note) {
    int pos = 0;
    while(arrlen(piece) > pos && piece[pos].start < note.start) pos++;
    Note temp;
    while(pos < arrlen(piece)) {
        temp = piece[pos];
        piece[pos] = note;
        note = temp;
        pos++;
    }
    arrpush(piece, note);
}

Uint32 timerCallback (Uint32 interval, void *param) {
    freq1 = freq1*1.17398499671;
    freq2 = freq2*1.17398499671;
    freq3 = freq3*1.17398499671;

    return 1000;
    /*static int pure = 0;
    switch(pure%4) {
    case 0: {
        freq2 = freq1/pow(2,3.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    case 1: {
        freq2 = freq1/6.0*5;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 2: {
        freq2 = freq1/5*4;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 3: {
        freq2 = freq1/pow(2,4.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    }
    pure = (pure+1)%4;
    fprintf(stderr, "%lf %lf %d\n", freq1/freq2,freq1/freq3, pure);
//    pure = !pure;
    return 2000;
    */
    (void)param;
    //        char notes[] = {5,4,4,5,4,4,5,4,4,5,4,4,2,1,0,2};
    char notes[] = {9,8,8,9,8,8,9,8,8,9,8,8,4,3,0,4};
    static int index = 0;
    double in = pow(2, 1.0/16);

    freq1 = 220*pow(in, notes[index]);
    index = (index+1)%sizeof(notes);
    fprintf(stderr, "freq=%lf in=%lf\n", freq1,in);
    return 250;
}

SDL_Renderer* renderer;
void draw() {

    drawText("440", renderer, 10, windowHeight/2);
    // SDL_Log("Window %d resized to %dx%d",
    // event->window.windowID, ,
    // );
    SDL_SetRenderDrawColor(renderer, 255,0,255,255);
//    int start = 0;
    FOR_STB_ARRAY(anote, piece) {
        SDL_Rect r = {anote->start*20, windowHeight/2-(anote->freq-400), anote->length*20, 10};
        SDL_RenderFillRect(renderer, &r);
//        start += r.w;
    }
//    SDL_RenderPresent(renderer);

}
SDL_AudioDeviceID ret;
void audio() {
    SDL_AudioSpec  want;
    memset(&want,0,sizeof(want));//= std.mem.zeroes(SDL_AudioSpec );
    // SDL_AudioDeviceID  dev;

    // inline for (want) |i| {
    // i = 0;
    // }
    //  @memset(&want, 0, sizeof(want)); /* || SDL_zero(want) * /
    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;
    want.callback = buffer;  // you wrote this function elsewhere.
    //   SDL_AudioDeviceID   dev= SDL_OpenAudioDevice(NULL, 0,  SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    ret =  SDL_OpenAudioDevice(
                NULL,
                SDL_FALSE,
                &want, &have,
                0);
    fprintf(stderr,"%s, returend %d %d %d %d %d %d \n",
            SDL_GetError(), want.format, want.channels, want.samples,have.format, have.channels, have.samples);

    SDL_PauseAudioDevice(ret, 0);
}

int main() {
    // Note that info level log messages are by default printed only in Debug
    // and ReleaseSafe build modes.
    Note n = {
        440, 0, 0.5
    };
    double sq = pow(2, 1.0/12);
    arrpush(piece, n);

    double pos = 1;
    arrpush(piece, STRU(Note, 440*sq*sq, 0, 0.5));
//    arrpush(piece, STRU(Note, 440*sq*sq*sq*sq, pos++/2, 0.5));
//    arrpush(piece, STRU(Note, 440*sq*sq, pos++/2, 0.5));
//    arrpush(piece, STRU(Note, 440, pos++/2, 0.5));
//    arrpush(piece, STRU(Note, 440*sq*sq*sq*sq*sq*sq*sq, pos++/2, 0.5));
//    arrpush(piece, STRU(Note, 440*sq*sq, pos++/2, 0.5));
    //    guiStartDrawing();
    fprintf(stderr,"All your codebase are belong to us." );
    //    if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO  ) < 0) {
    //        fprintf(stderr,"SDL could not initialize! SDL_Error: {s} \n", SDL_GetError());
    //        abort();
    //    }
    SDL_Init(SDL_INIT_AUDIO);
    audio();
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    //    SDL_Window *wind = SDL_CreateWindow(
    //        "SDL2 Demo",
    //        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //        SCREEN_WIDTH, SCREEN_HEIGHT,
    //        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    //    );         SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );

    guiStartDrawing();
    //    guiShowWindow(rootWindow);
    renderer = SDL_CreateRenderer(rootWindow, -1,  0);
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_Rect f = {0,0,windowWidth, windowHeight};
    SDL_RenderFillRect(renderer, &f);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    SDL_RenderDrawLine(renderer,
                       10.5, 10.5, 105.5, 30.5);
    //        SDL_RenderClear(renderer);
    //        SDL_RenderCopy(renderer, bitmapTex, NULL, NULL);
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    Painter p = {renderer, SDL_GetWindowSurface(rootWindow), rootWindow};
    TTF_Init();

    //this opens a font style and sets a size

    //        const rw = SDL_RWFromConstMem(ComicSans, ComicSans.len);
    Sans = TTF_OpenFont("/home/n/.fonts/comici.ttf", 24);
    //        const Sans = TTF_OpenFont("C:\\Windows\\Fonts\\comic.ttf", 24);//C:\\Windows\\WinSxS\\amd64_microsoft-windows-font-truetype-corbel_31bf3856ad364e35_10.0.19041.1_none_889a9b699e1310d9\\corbell.ttf", 24);
//    fprintf(stderr, "{*}", Sans);
//    draw();
//    SDL_Event  e;

    SDL_AddTimer(250,
                 timerCallback,
                 NULL);
    getPos = gridGetPos; feedbackSize = gridFeedbackSize;
    Grid grid = allocateGrid(100, 100, 5);
    grid.gridStart.x = grid.gridStart.y = 0;
    pushGrid(&grid);
    draw();
//    static int er = 30;
//    SDL_RenderDrawLine(renderer,
//                       10.5, 10.5, 50.5, er++);

    while(1) {
//        SDL_RenderDrawLine(renderer,
//                           10.5, 10.5, 105.5, er++);

        guiNextEvent();
//        fprintf(stderr, "got event {x} res {}", event.type, pollres);
        guiSetForeground(&p,0);
        if(event.type != SDL_MOUSEMOTION)
            guiFillRectangle(&p,0,0,windowWidth,windowHeight);
        setCurrentGridPos(0,0);
        if(guiButton(&p, "play", 4)) {
//            play();
            SDL_PauseAudioDevice(ret, 0);
//            play = true;
        } gridNextColumn();
        if(guiButton(&p, "stop", 4)) {
//            play();
            SDL_PauseAudioDevice(ret, 1);
//            play = false;
        }
        draw();
        SDL_RenderPresent(renderer);

        if(event.type == SDL_MOUSEBUTTONUP) {
            SDL_MouseButtonEvent e =
                    event.button;
            Note n = {-(e.y-windowHeight/2)+400,
                     e.x / 20.0,
                     0.5};
            insert(n);
            draw();
        }
        if(event.type == SDL_WINDOWEVENT) {
            if(event.window.event == SDL_WINDOWEVENT_RESIZED  || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                fprintf(stderr,"resizeevent" );
                windowWidth = event.window.data1;
                windowHeight = event.window.data2;
//                SDL_SetRenderDrawColor(renderer, 0,0,0,255);
//                SDL_Rect f = {0,0,windowWidth, windowHeight};
//                SDL_RenderFillRect(renderer, &f);
                //                drawToolButton
//                draw();
            }
            if(event.window.event == SDL_WINDOWEVENT_CLOSE) {
                return;
            }
        }

        if(event.type == SDL_MOUSEMOTION) {
            //            freq = event.motion.y + 140.0;
            fprintf(stderr,"motion {%d} {%lf}", event.motion.y, event.motion.y + 140.0);
        }
        //          fprintf(stderr,"{}", @intToEnum(SDL_EventType, event.type));
        // fprintf(stderr, "{}", SDL_POLLSENTINEL          );
        if(event.type == SDL_QUIT) {
            return;
        }
        if(event.type == SDL_MOUSEBUTTONDOWN) {
//            getFreq();
        }
    }
}
//*/
