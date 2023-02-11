#include "editorinstance.h"
#include "combobox.h"
#include <gui.h>
//struct editorInstance
//{
//    MediaItem_Take take;
//    editorInstance();
//};

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




#include <gui.h>

SDL_Renderer* renderer;

char* appName = "piano roll continous";
#ifdef _WIN32
#define FILE_DIALOG_PATH "C:\src\exercises\build-FileDialog-Desktop_Qt_6_4_1_MSVC2019_64bit-Debug\debug\FileDialog.exe"
#else
#define FILE_DIALOG_PATH "/home/n/exercises/build-FileDialog-Desktop-Debug/FileDialog"
#endif
SDL_mutex* mutex_; // I use SDL_mutex because MSVC doesn't provide thread.h
SDL_cond* condVar;
extern bool timeToLeave;
extern bool timeToShow;
extern void SetThreadName(u32 dwThreadID, const char* threadName);
extern int PlaybackEvent = 0;
int pianorollgui(void) {
//    running = true;
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
//    STATIC(bool, inited, guiInit());
    guiStartDrawingEx(false);
    mutex_ = SDL_CreateMutex();

    condVar = SDL_CreateCond();

    SetThreadName(-1, "midicont");
#ifndef REAPER
    openAudio();
#endif
    Grid grid = allocateGrid(100, 100, 5);
    grid.gridStart.x = grid.gridStart.y = 0;
    pushLayout(&grid);

    SDL_LockMutex(mutex_);


    PlaybackEvent = SDL_RegisterEvents(1);
    while(1) {
        SDL_UnlockMutex(mutex_);
        guiNextEvent(); // let's hope that i don't add any
        // support for hooks that are to be run inside guiNextEvent
        // and potentially have access to things that should
        // be protected with mutex_
        SDL_LockMutex(mutex_);
        if(timeToLeave) {
            break;
        }
        if(timeToShow) {
            int windowSizeIndex  = hmgeti(config, currentGuid);
            CONTINUOUSMIDIEDITOR_Config cfg;
            if(windowSizeIndex >= 0) {
                cfg = config[windowSizeIndex];
            } else {
                cfg = (CONTINUOUSMIDIEDITOR_Config){
                    .key = currentGuid,
                    .value = {.windowGeometry = { 400, 400, 700, 700 },
                                .horizontalScroll = 0,
                        .verticalScroll = 0.5,
                        .horizontalFrac = 0.1,
                        .verticalFrac = 0.1 }
                };
                hmputs(config, cfg);
                windowSizeIndex  = hmgeti(config, currentGuid);
            }
            Rect rect = config[windowSizeIndex].value.windowGeometry;
            guiMoveWindow(rootWindow, rect.x, rect.y);
            SDL_SetWindowSize(rootWindow, rect.w, rect.h);
            SDL_ShowWindow(rootWindow);
            SDL_RaiseWindow(rootWindow);
            timeToShow = false;
        }
        if(playing) {
            static int frames = 0;
            STATIC(int, time, SDL_GetTicks());
            int newTime = SDL_GetTicks();
            frames++;
            currentPositionInSamples += 44100*(newTime-time)/1000.0; //TODO: 44100
            if((newTime/1000)*1000 > time) {
                message("%d frames\n", frames);
                frames = 0;
            }
            time = newTime;
        }

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
#ifndef REAPER
                bool paused = SDL_GetAudioDeviceStatus(audioDevice) == SDL_AUDIO_PAUSED;
                if(paused) SDL_PauseAudioDevice(audioDevice, 0);
                else SDL_PauseAudioDevice(audioDevice, 1);
#else

#endif
                const u32 playStop = 40044;
                reaperOnCommand(playStop);
            }
        }
        if(event.type==SDL_WINDOWEVENT) {
            if(event.window.event == SDL_WINDOWEVENT_CLOSE) {
                message("closing the window"
                        "\n");
//                arrsetlen(piece, 0);
#ifndef REAPER
                break;
#else
                SDL_HideWindow(rootWindow);
#endif
            }
            if(event.window.event == SDL_WINDOWEVENT_MOVED) {
                int windowSizeIndex  = hmgeti(config, currentGuid);
                if(windowSizeIndex >= 0) {
                    config[windowSizeIndex].value.windowGeometry.x = event.window.data1;
                    config[windowSizeIndex].value.windowGeometry.y = event.window.data2;
                } else {
                    abort();
                }
            }
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                int windowSizeIndex  = hmgeti(config, currentGuid);
                if(windowSizeIndex >= 0) {
                    config[windowSizeIndex].value.windowGeometry.w = event.window.data1;
                    config[windowSizeIndex].value.windowGeometry.h = event.window.data2;
                } else {
                    abort();
                }
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
#ifndef REAPER
        persistentDoubleField(&rootWindowPainter, 6, qpm); gridNextColumn();
#endif
        guiLabelZT(&rootWindowPainter, "bpm"); gridNextColumn();

        guiDoubleField(&rootWindowPainter, 6, &pitchRange); gridNextColumn();
        // TODO: draw tooltips
//        const char* elements;
        guiEnumComboBox(&rootWindowPainter, midi_mode_enum, &midiMode);//"use 1st channel", "don't use");
        setCurrentGridPos(3,0);
        roll(&rootWindowPainter, getGridBottom(topLayout()));
//        SDL_RenderPresent(renderer);

        if(event.type == SDL_QUIT) {
            continue;
        }
    }
    message("broke out of loop"
            "\n");

    popLayout();
    SDL_FlushEvents(0, SDL_LASTEVENT);
    SDL_HideWindow(rootWindow);
//    running = false;

    return 0;
}
//*/
