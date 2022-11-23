#include <gui.h>
#include <persistent.h>
#include <gridlayout.h>
#include "save.h"
#include <stdbool.h>
#include "stb_ds.h"
#include "melody.h"
#include "playback.h"
#include "extmath.h"
#include "roll.h"

#include "misc.h"
#include <SDL2/SDL.h>

#include "newFile.h"






SDL_Renderer* renderer;

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
    openAudio();

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
        if(event.type == ButtonRelease) {
            DEBUG_PRINT(event.button.which, "%d");
        }
        guiSetForeground(&rootWindowPainter,0);
//        if(event.type != MotionEvent) {
            guiClearWindow(rootWindow);
//            DEBUG_PRINT(event.type, "%x");
//        }
        setCurrentGridPos(0,0);
        int keyPressed = -1;
        if(event.type==KeyPress) {
            keyPressed = GET_KEYSYM(event);
            if(keyPressed == ' ') {
                bool paused = SDL_GetAudioDeviceStatus(audioDevice) == SDL_AUDIO_PAUSED;
                if(paused) SDL_PauseAudioDevice(audioDevice, 0);
                else SDL_PauseAudioDevice(audioDevice, 1);
            }
        }
        if(guiButton(&rootWindowPainter, "play", 4)) {
            SDL_PauseAudioDevice(audioDevice, 0);
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "stop", 4)) {
            SDL_PauseAudioDevice(audioDevice, 1);
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "save", 4)) {
            FILE* fp = popen("/home/n/exercises/build-FileDialog-Desktop-Debug/FileDialog --file-selection --save", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                saveMelody(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "load", 4)) {
            FILE* fp = popen("/home/n/exercises/build-FileDialog-Desktop-Debug/FileDialog --file-selection", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                loadMelody(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
        } gridNextColumn();
        if(guiButtonZT(&rootWindowPainter, "export")) {
            FILE* fp = popen("/home/n/exercises/build-FileDialog-Desktop-Debug/FileDialog --file-selection --save", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                export(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
        } gridNextColumn();
        persistentDoubleField(&rootWindowPainter, 6, bpm); gridNextColumn();
        guiLabelZT(&rootWindowPainter, "bpm"); gridNextColumn();
        static int d;
        guiIntField(&rootWindowPainter, 6, &d); gridNextColumn();
        setCurrentGridPos(3,0);
        roll(&rootWindowPainter, getGridBottom(topGrid()));
//        SDL_RenderPresent(renderer);

        if(event.type == SDL_QUIT) {
            return 0;
        }
    }
}
//*/
