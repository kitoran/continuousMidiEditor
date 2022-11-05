#include <gui.h>
#include <gridlayout.h>
#include <stdbool.h>
#include "stb_ds.h"
#include "melody.h"
#include "extmath.h"
#include "roll.h"

#include "misc.h"

#include <SDL2/SDL.h>
//void drawText(char* text, SDL_Renderer* renderer,  int x, int y) {
//    fprintf(stderr,"drawText {%s}" ,text);
//    //  var zigs = std.mem.span(text);
//    //  var value = textTextures.get(zigs);
//    //  if (value == NULL) {
//    SDL_Color White= {.r = 255, .g = 255, .b = 255, .a = 255};
//    SDL_Surface *surfaceMessage =
//            TTF_RenderUTF8_Solid(Sans, text, White);
//    SDL_Texture*textext = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
//    //     textTextures->put(zigs, textext);
//    SDL_Texture*value = textext;
//    SDL_FreeSurface(surfaceMessage);
//    //  }
//    SDL_Texture* texture = value;
//    int w ;  int h  ;
//    SDL_QueryTexture(texture, NULL, NULL, &w, &h);
//    SDL_Rect  Message_rect= {.x = x,  //controls the rect's x coordinate
//                             .y = y, // controls the rect's y coordinte
//                             .w = w, // controls the width of the rect
//                             .h = h}; // controls the height of the rect
//    SDL_RenderCopy(renderer, texture, 0, &Message_rect);
//    SDL_DestroyTexture(textext);
//}

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
SDL_AudioSpec  have;
#define EASE_FRAMES 40


int countSamples(double time) {
    return time * have.freq;
}

SDL_AudioDeviceID audioDevice;
void buffer(void* userdata,
            Uint8* stream8,
            int wrongLen)
{
    if(arrlen(piece) == 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        return;
    }

    int trueLen = wrongLen/2;
    i16* stream = (i16*)stream8;
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
        const Note* nextNote = position+1 < arrlen(piece)? piece+position+1 : &silenceEnd;
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







SDL_Renderer* renderer;

void audio() {
    SDL_AudioSpec  want;
    memset(&want,0,sizeof(want));

    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;
    want.callback = buffer;

    audioDevice =  SDL_OpenAudioDevice(
                NULL,
                SDL_FALSE,
                &want, &have,
                0);
    fprintf(stderr,"%s, returend %d %d %d %d %d %d \n",
            SDL_GetError(), want.format, want.channels, want.samples,have.format, have.channels, have.samples);

    SDL_PauseAudioDevice(audioDevice, 0);
}
char* appName = "piano roll continous";
int main() {
    // Note that info level log messages are by default printed only in Debug
    // and ReleaseSafe build modes.
//    Note n = {
//        440, 0, 0.5
//    };
//    double sq = pow(2, 1.0/12);
//    arrpush(piece, n);

//    arrpush(piece, STRU(Note, 440*sq*sq, 0, 0.5));
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
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    //    SDL_Window *wind = SDL_CreateWindow(
    //        "SDL2 Demo",
    //        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    //        SCREEN_WIDTH, SCREEN_HEIGHT,
    //        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    //    );         SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );

    guiStartDrawing();
//    SDL_Init(SDL_INIT_AUDIO);
    audio();

    getPos = gridGetPos; feedbackSize = gridFeedbackSize;

    Grid grid = allocateGrid(100, 100, 5);
    grid.gridStart.x = grid.gridStart.y = 0;
    pushGrid(&grid);

//    static int er = 30;
//    SDL_RenderDrawLine(renderer,
//                       10.5, 10.5, 50.5, er++);

    while(1) {
//        SDL_RenderDrawLine(renderer,
//                           10.5, 10.5, 105.5, er++);

        guiNextEvent();
        guiSetForeground(&rootWindowPainter,0);
        if(event.type != MotionEvent)
            guiClearWindow(rootWindow);
        setCurrentGridPos(0,0);
        if(guiButton(&rootWindowPainter, "play", 4)) {
            SDL_PauseAudioDevice(audioDevice, 0);
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "stop", 4)) {
            SDL_PauseAudioDevice(audioDevice, 1);
        }
        roll(&rootWindowPainter, getGridBottom(topGrid()));
        SDL_RenderPresent(renderer);

        if(event.type == SDL_QUIT) {
            return 0;
        }
    }
}
//*/
