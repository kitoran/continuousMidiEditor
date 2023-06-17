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
#include "linelayout.h"
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

//SDL_Renderer* renderer;

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
bool timeToShow = false;
#endif
extern void SetThreadName(u32 dwThreadID, const char* threadName);
extern int PlaybackEvent = 0;

static bool settingsOpen = false;

static GuiWindow settingsWindow = 0;
extern void settingsGui(void) {

    static Painter painter;
    if(settingsWindow == 0) {
        settingsWindow = guiMakeWindow();
        painter = guiMakePainter(settingsWindow);
    }
    LineLayout settingsLayout = makeVerticalLayout(5);
    if(guiSameWindow(&painter, /*false*/true)) {
        guiSetForeground(&painter, 0);
        guiClearWindow(&painter);
        pushLayout(&settingsLayout);
          LineLayout oneLine = makeHorizontalLayout(5);
//          oneLine.pos = getPos(); pushLayout(&oneLine);
//            guiLabelZT(&painter, "Scale type:");
//            guiEnumComboBox(&painter, scale_type_enum, &scale.type);
//          popLayout(); feedbackSize(getLineSize(&oneLine)); oneLine.filled = oneLine.across =0;
          if(scale.type == rational_intervals) {
              guiLabelZT(&painter, "multipliers:");
              STATIC(Grid, primesGrid, allocateGrid(NUMBER_OF_PRIMES, 2, 5));
              primesGrid.gridStart = getPos();
              pushLayout(&primesGrid);
              ExactLayout e = makeExactLayout((Point){0,0});
                for(int i = 0; i < NUMBER_OF_PRIMES; i++) {
                    setCurrentGridPos(0, i);
                    guiLabelZT(&painter, primes[i].text);
                    setCurrentGridPos(1, i);
                    e.exactPos = getPos(); e.exactPos.x += (primesGrid.gridWidths[i]+1)/2-5;
                    pushLayout(&e);
                    guiCheckBox(&painter, &(scale.primes[i]));
                    popLayout();
                    feedbackSize((Size){10, guiFontHeight()+10});
                }
              popLayout();
              feedbackSize(getGridSize(&primesGrid));
              oneLine.pos = getPos(); pushLayout(&oneLine);
                guiLabelZT(&painter, "Max denominator");
                guiIntField(&painter, 2, &scale.maxComponent);
              popLayout(); feedbackSize(getLineSize(&oneLine)); oneLine.filled = oneLine.across =0;
        } else if(scale.type == equal_intervals) {
            oneLine.pos = getPos(); pushLayout(&oneLine);
            guiIntField(&painter, 3, &scale.divisions);
            guiLabelZT(&painter, "equal steps of ");
            guiDoubleField(&painter, 4, &scale.equave);
            popLayout();
            feedbackSize(getLineSize(&oneLine));
            oneLine.filled = oneLine.across =0;
            oneLine.pos = getPos(); pushLayout(&oneLine);
            guiLabelZT(&painter, "(write the equave as one decimal");
            popLayout(); feedbackSize(getLineSize(&oneLine)); oneLine.filled = oneLine.across =0;
            oneLine.pos = getPos(); pushLayout(&oneLine);
            guiLabelZT(&painter, "e.q the fifth = 1.5)");
            popLayout(); feedbackSize(getLineSize(&oneLine)); oneLine.filled = oneLine.across =0;
        }
//        guiRadioButtonGroup(&painter, scale_relativity_enum, &scale.relative);
        if(&scale.relative == scale_absolute) {
            oneLine.pos = getPos(); pushLayout(&oneLine);

            guiLabelZT(&painter, "The root of the scale");
            guiDoubleField(&painter, 5, &scale.root);

            popLayout(); feedbackSize(getLineSize(&oneLine)); oneLine.filled = oneLine.across =0;
        }
        if(guiButtonZT(&painter, "OK")) {
            recalculateScale = true;
            settingsOpen = false;
            guiHideWindow(settingsWindow);
        }
        popLayout();
        if(event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
            recalculateScale = true;
            settingsOpen = false;
            guiHideWindow(settingsWindow);
        }
    }
}
void transportPanel(Size buttonSizes)
{
    LineLayout transportLayout = makeHorizontalLayout(0);
    transportLayout.pos = getPos(); pushLayout(&transportLayout);
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/gen_home.png", false, false, &buttonSizes, 1)) {
        //            SDL_PauseAudioDevice(audioDevice, 0);
        reaperSetPosition(0);
    }
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/play.png", true, playing, &buttonSizes, 1)) {
//            SDL_PauseAudioDevice(audioDevice, 0);
        play();
    }
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/gen_pause.png", true, paused, &buttonSizes, 1)) {
//            SDL_PauseAudioDevice(audioDevice, 0);
        togglePause();
    }
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/gen_stop.png", false, false, &buttonSizes, 1)) {
//            SDL_PauseAudioDevice(audioDevice, 0);
        stop();
    }
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/gen_end.png", false, false, &buttonSizes, 1)) {
//            SDL_PauseAudioDevice(audioDevice, 0);
        reaperSetPosition(pieceLength);
    }
    if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/gen_repeat_off.png", true, repeatOn, &buttonSizes, 1)) {
//            SDL_PauseAudioDevice(audioDevice, 0);
        toggleRepeat();
    }
    popLayout(); feedbackSize(getLineSize(&transportLayout));
}
enum {
    COMMAND_IMPORT,
    COMMAND_CONVERT,
    COMMAND_REOPEN,
    COMMAND_SHOWDEBUG
};
HMENU hView;
void makeMenu(/*GuiWindow window*/) {
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(rootWindow, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    HMENU hMenuBar = CreateMenu();
    HMENU hFile = CreateMenu();
//    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hFile, "&File");
//    HMENU hEncoding = CreateMenu();
//    AppendMenuA(hFile, MF_POPUP, (UINT_PTR)hEncoding, "Midi encoding");
//    AppendMenuA(hEncoding, MF_STRING, COMMAND_REOPEN, "Reopen with different encoding...");
//    AppendMenuA(hEncoding, MF_STRING, COMMAND_CONVERT, "Convert to different encoding...");
//    AppendMenuA(hFile, MF_STRING, COMMAND_IMPORT, "Import MIDI file...");

//    HMENU hEdit = CreateMenu();
//    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hEdit, "&Edit");

    hView = CreateMenu();
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hView, "&View");
    AppendMenuA(hView, MF_POPUP, COMMAND_SHOWDEBUG, "Show debug widgets");

    SetMenu(hwnd, hMenuBar);
}
bool showDebug = false;
extern int pianorollgui(void) {
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "2" );
    guiStartDrawingEx(false);
    SDL_Rect d = {10, 10, 200, 200};
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
        }

        if(guiSameWindow(&rootWindowPainter, /*false*/true)) {
            if(event.type == ButtonRelease) {
                DEBUG_PRINT(event.button.which, "%d");
            }
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
//            if(event.type ==SDL_DROPFILE) {      // In case if dropped file
//                char*dropped_filedir = event.drop.file;
//                // Shows directory of dropped file
//                SDL_ShowSimpleMessageBox(
//                    SDL_MESSAGEBOX_INFORMATION,
//                    "File dropped on window",
//                    dropped_filedir,
//                    rootWindow
//                );
//                SDL_free(dropped_filedir);    // Free dropped_filedir memory
//            }
//            if(event.type ==SDL_DROPTEXT) {      // In case if dropped file
//                char*dropped_filedir = event.drop.file;
//                // Shows directory of dropped file
//                SDL_ShowSimpleMessageBox(
//                    SDL_MESSAGEBOX_INFORMATION,
//                    "Text dropped on window",
//                    dropped_filedir,
//                    rootWindow
//                );
//                SDL_free(dropped_filedir);    // Free dropped_filedir memory
//            }
            int keyPressed = -1;
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
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
                SDL_Keymod km = SDL_GetModState();
                if(keyPressed == 'z' && (km & KMOD_CTRL)) {
                    if(km & KMOD_SHIFT) {
                        redo();
                    } else {
                        undo();
                    }
                }
                if(keyPressed == 's' && (km & KMOD_CTRL)) {
                    save();
                }
                if(keyPressed == 'r' && (km & KMOD_CTRL)) {
                    reload();
                }
            }
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            if(event.type==SDL_WINDOWEVENT) {
                if(event.window.event == SDL_WINDOWEVENT_CLOSE) {
                    message("closing the window"
                            "\n");
    //                arrsetlen(piece, 0);
    #ifndef REAPER
                    break;
    #else
                    SDL_HideWindow(rootWindow);
                    take = 0;
    //                currentItemConfig = NULL;
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
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            if(event.type == SDL_SYSWMEVENT) {

                if (event.syswm.msg->msg.win.msg == WM_COMMAND)
                {
                    message("SDL_COMMAND %d!!!", event.syswm.msg->msg.win.wParam);
                    if (event.syswm.msg->msg.win.wParam == COMMAND_SHOWDEBUG)
                    {
                        showDebug = !showDebug;
                        CheckMenuItem(hView, COMMAND_SHOWDEBUG, showDebug ? MF_CHECKED:0);
                        for(int i = 0; i < grid.gridWidthsLen; i++) {
                            grid.gridWidths[i] = 0;
                        }
                    }
                }
                continue;
            }
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            guiSetForeground(&rootWindowPainter,0);
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            guiClearWindow(&rootWindowPainter);
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            setCurrentGridPos(0,0);
    //        SDL_FillRect(rootWindowPainter.drawable, &d, 0xffffff00);
            Size buttonSizes = {26,26};
            transportPanel(buttonSizes);
            gridNextColumn();
//            if(guiButton(&rootWindowPainter, "save", 4)) {
//    #ifndef WIN32
//                FILE* fp = popen(FILE_DIALOG_PATH" --file-selection --save", "r");
//                char line[1024];
//                while (fgets(line,sizeof(line),fp)) fprintf(stderr,
//                        "FileDialog says %s \n",line);

//                if(!WEXITSTATUS(pclose(fp))) {
//                    saveMelody(line);
//                } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
//    #else
//                SDL_ShowSimpleMessageBox(
//                    SDL_MESSAGEBOX_INFORMATION,
//                    "Action not implemented yet",
//                    "Save action not implemented",
//                    rootWindow
//                );
//    #endif
//            } gridNextColumn();
//            if(guiButton(&rootWindowPainter, "load", 4)) {
//    #ifndef WIN32
//                FILE* fp = popen(FILE_DIALOG_PATH" --file-selection", "r");
//                char line[1024];
//                while (fgets(line,sizeof(line),fp)) fprintf(stderr,
//                        "FileDialog says %s \n",line);

//                if(!WEXITSTATUS(pclose(fp))) {
//                    loadMelody(line);
//                } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
//    #else
//                SDL_ShowSimpleMessageBox(
//                    SDL_MESSAGEBOX_INFORMATION,
//                    "Action not implemented yet",
//                    "load action not implemented",
//                    rootWindow
//                );
//    #endif
//            } gridNextColumn();
//            if(guiButtonZT(&rootWindowPainter, "export")) {
//    #ifndef WIN32
//                FILE* fp = popen(FILE_DIALOG_PATH" --file-selection --save", "r");
//                char line[1024];
//                while (fgets(line,sizeof(line),fp)) fprintf(stderr,
//                        "FileDialog says %s \n",line);

//                if(!WEXITSTATUS(pclose(fp))) {
//                    export(line);
//                } else fprintf(stderr,"FileDialog extted unsuccessfully\n");
//    #else
//                SDL_ShowSimpleMessageBox(
//                    SDL_MESSAGEBOX_INFORMATION,
//                    "Action not implemented yet",
//                    "export action not implemented",
//                    rootWindow
//                );
//    #endif
//            } gridNextColumn();
    #ifndef REAPER
            persistentDoubleField(&rootWindowPainter, 6, projectSignature.qpm); gridNextColumn();
            guiLabelZT(&rootWindowPainter, "bpm"); gridNextColumn();
    #endif

            guiLabelZT(&rootWindowPainter, "pitch wheel range in semitones");        gridNextColumn();
            if(guiDoubleField(&rootWindowPainter, 6, &(currentItemConfig->value.pitchRange))); gridNextColumn();
    //        const char* elements;
            guiEnumComboBox(&rootWindowPainter, midi_mode_enum, (int*)&(currentItemConfig->value.midiMode));//"use 1st channel", "don't use");
            gridNextColumn();
            if(showDebug) {
                guiCheckBox(&rootWindowPainter, &showChannels);        gridNextColumn();
                guiLabelZT(&rootWindowPainter, "show channels");        gridNextColumn();
                guiCheckBox(&rootWindowPainter, &showMidi);        gridNextColumn();
                guiLabelZT(&rootWindowPainter, "show midi");        gridNextColumn();
            }
//            guiCheckBox(&rootWindowPainter, &showScale);        gridNextColumn();
//            guiLabelZT(&rootWindowPainter, "show 16edo scale");        gridNextColumn();

//            STATIC(IMAGE*, gear, loadImageZT(GUI_RESOURCE_PATH, "settings30x30.png"));
            Size size = {30,30};
            if(guiToolButtonEx(&rootWindowPainter, GUI_RESOURCE_PATH "/settings30x30.png", false, false, &size, 0)) {
                settingsOpen = true;
                guiShowWindow(settingsWindow);
                guiRaiseWindow(settingsWindow);
            }   gridNextColumn();
//            STATIC(IMAGE*, magnet, loadImageZT(GUI_RESOURCE_PATH, "magnetic-icon.png"));
            if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/magnetic-vertical.png", true, verticalsnap, &size, 2)) {
                verticalsnap = !verticalsnap;
            }   gridNextColumn();
            if(guiToolButtonEx(&rootWindowPainter, MY_PATH "/resources/magnetic-horizontal.png", true, horizontalsnap, &size, 2)) {
                horizontalsnap = !horizontalsnap;
            }   gridNextColumn();
            setCurrentGridPos(3,0);
            roll(&rootWindowPainter, getGridBottom(topLayout()));
        }
        if(settingsOpen) {
            settingsGui();
        }

        if(timeToShow) {
            SDL_SetWindowTitle(rootWindow, appName);
            SDL_UpdateWindowSurface(rootWindow);
            SDL_ShowWindow(rootWindow);
            SDL_RaiseWindow(rootWindow);
            timeToShow = false;
        }

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
