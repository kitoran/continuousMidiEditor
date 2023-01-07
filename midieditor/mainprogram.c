#include <gui.h>
#include <persistent.h>
#include <gridlayout.h>
#include "save.h"
#include <stdbool.h>
#include <stdio.h>
#include "stb_ds.h"
#include "melody.h"
#include "playback.h"
#include "actions.h"
#include "extmath.h"
#include "roll.h"

#include "misc.h"
#include "midi.h"
#include <SDL.h>

#include "newFile.h"
#include "mainprogram.h"






SDL_Renderer* renderer;

char* appName = "piano roll continous";
#ifdef _WIN32
#define FILE_DIALOG_PATH "C:\src\exercises\build-FileDialog-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug\FileDialog.exe"
#else
#define FILE_DIALOG_PATH "/home/n/exercises/build-FileDialog-Desktop-Debug/FileDialog"
#endif
int pianorollgui(void) {
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    guiStartDrawing();
#ifndef REAPER
    openAudio();
#endif
    Grid grid = allocateGrid(100, 100, 5);
    grid.gridStart.x = grid.gridStart.y = 0;
    pushLayout(&grid);

    while(1) {
        guiNextEvent();
        if(event.type == ButtonRelease) {
            DEBUG_PRINT(event.button.which, "%d");
        }

        if(event.type ==SDL_DROPFILE) {      // In case if dropped file
            char*dropped_filedir = event.drop.file;
            // Shows directory of dropped file
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "File dropped on window",
                dropped_filedir,
                rootWindow
            );
            SDL_free(dropped_filedir);    // Free dropped_filedir memory
        }
        if(event.type ==SDL_DROPTEXT) {      // In case if dropped file
            char*dropped_filedir = event.drop.file;
            // Shows directory of dropped file
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "Text dropped on window",
                dropped_filedir,
                rootWindow
            );
            SDL_free(dropped_filedir);    // Free dropped_filedir memory
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
//            SDL_PauseAudioDevice(audioDevice, 0);
            play();
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "stop", 4)) {
//            SDL_PauseAudioDevice(audioDevice, 1);
            stop();
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "save", 4)) {
#ifndef WIN32
            FILE* fp = popen(FILE_DIALOG_PATH" --file-selection --save", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                saveMelody(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
#else
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "Action not implemented yet",
                "Save action not implemented",
                rootWindow
            );
#endif
        } gridNextColumn();
        if(guiButton(&rootWindowPainter, "load", 4)) {
#ifndef WIN32
            FILE* fp = popen(FILE_DIALOG_PATH" --file-selection", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                loadMelody(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
#else
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "Action not implemented yet",
                "load action not implemented",
                rootWindow
            );
#endif
        } gridNextColumn();
        if(guiButtonZT(&rootWindowPainter, "export")) {
#ifndef WIN32
            FILE* fp = popen(FILE_DIALOG_PATH" --file-selection --save", "r");
            char line[1024];
            while (fgets(line,sizeof(line),fp)) fprintf(stderr,
                    "FileDialog says %s \n",line);

            if(!WEXITSTATUS(pclose(fp))) {
                export(line);
            } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
#else
            SDL_ShowSimpleMessageBox(
                SDL_MESSAGEBOX_INFORMATION,
                "Action not implemented yet",
                "export action not implemented",
                rootWindow
            );
#endif
        } gridNextColumn();
        persistentDoubleField(&rootWindowPainter, 6, bpm); gridNextColumn();
        guiLabelZT(&rootWindowPainter, "bpm"); gridNextColumn();
        static int d;
        guiIntField(&rootWindowPainter, 6, &d); gridNextColumn();
        setCurrentGridPos(3,0);
        roll(&rootWindowPainter, getGridBottom(topLayout()));
//        SDL_RenderPresent(renderer);

        if(event.type == SDL_QUIT) {
            return 0;
        }
    }
}
//*/
