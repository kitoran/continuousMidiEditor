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
#include <toolbuttongroup.h>
#include "save.h"
#include "actions.h"
#include <stdbool.h>
#include <stdio.h>
#include "stb_ds.h"
#include "melody.h"
#include <SDL_syswm.h>
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
#ifdef reaper
extern bool timeToLeave;
extern bool timeToShow;
#else
bool timeToLeave  = false;
bool timeToShow = true;
#endif
extern void SetThreadName(u32 dwThreadID, const char* threadName);
extern int PlaybackEvent = 0;
extern int pianorollgui(void) {
//    running = true;
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
//    STATIC(bool, inited, guiInit());
    guiStartDrawingEx(false);
    makeMenu();
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

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
        guiNextEvent(/*.dontblock = *//*playing*/); // let's hope that i don't add any
        // support for hooks that are to be run inside guiNextEvent
        // and potentially have access to things that should
        // be protected with mutex_
        SDL_LockMutex(mutex_);
        if(timeToLeave) {
            break;
        }
        if(timeToShow) {
            Rect rect = currentItemConfig->value.windowGeometry;
            guiMoveWindow(rootWindow, rect.x, rect.y);
            SDL_SetWindowSize(rootWindow, rect.w, rect.h);
            SDL_SetWindowTitle(rootWindow, appName);
            SDL_ShowWindow(rootWindow);
            SDL_RaiseWindow(rootWindow);
            timeToShow = false;
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
#ifdef REAPER
                reaperOnCommand(playStop);
#else
                stop();
#endif
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
//                currentItemConfig = NULL;
                //TODO: probably wise to wait here on timeToShow
#endif
            }
            if(event.window.event == SDL_WINDOWEVENT_MOVED) {
                ASSERT(currentItemConfig != 0, "")
                currentItemConfig->value.windowGeometry.x = event.window.data1;
                currentItemConfig->value.windowGeometry.y = event.window.data2;
            }
            if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                ASSERT(currentItemConfig != 0, "")
                    currentItemConfig->value.windowGeometry.w = event.window.data1;
                    currentItemConfig->value.windowGeometry.h = event.window.data2;
            }
//            continue;
        }
        if(event.type == SDL_SYSWMEVENT) {

            if (event.syswm.msg->msg.win.msg == WM_COMMAND)
            {
                message("SDL_COMMAND %d!!!", event.syswm.msg->msg.win.wParam);
                if (event.syswm.msg->msg.win.wParam == 0x453434)
                {
                    message("Encoding!!!");
                }
            }
            continue;
        }
        guiSetForeground(&rootWindowPainter,0);
        guiClearWindow(rootWindow);
        setCurrentGridPos(0,0);
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
        persistentDoubleField(&rootWindowPainter, 6, projectSignature.qpm); gridNextColumn();
        guiLabelZT(&rootWindowPainter, "bpm"); gridNextColumn();
#endif

        if(guiDoubleField(&rootWindowPainter, 6, &(currentItemConfig->value.pitchRange))); gridNextColumn();
        // TODO: draw tooltips
//        const char* elements;
        guiEnumComboBox(&rootWindowPainter, midi_mode_enum, (int*)&(currentItemConfig->value.midiMode));//"use 1st channel", "don't use");
        gridNextColumn();
        guiCheckBox(&rootWindowPainter, &showChannels);        gridNextColumn();
        guiLabelZT(&rootWindowPainter, "show channels");        gridNextColumn();
//TODO: save all fields to midi take properties
// TODO: add combination tones (difference tones, sum tones, means??)
        guiCheckBox(&rootWindowPainter, &showScale);        gridNextColumn();
        guiLabelZT(&rootWindowPainter, "show 16edo scale");        gridNextColumn();

        setCurrentGridPos(3,0);
        roll(&rootWindowPainter, getGridBottom(topLayout()));
//        SDL_RenderPresent(renderer);

        if(event.type == SDL_QUIT) {
            continue;
        }
    }

    SDL_UnlockMutex(mutex_);
    message("broke out of loop"
            "\n");

    popLayout();
    SDL_FlushEvents(0, SDL_LASTEVENT);
    SDL_HideWindow(rootWindow);
//    running = false;

    return 0;
}
//*/
