// Bare-bone REAPER extension
//
// 1. Grab reaper_plugin.h from https://github.com/justinfrankel/reaper-sdk/raw/main/sdk/reaper_plugin.h
// 2. Grab reaper_plugin_functions.h by running the REAPER action "[developer] Write C++ API functions header"
// 3. Grab WDL: git clone https://github.com/justinfrankel/WDL.git
// 4. Build then copy or link the binary file into <REAPER resource directory>/UserPlugins
//
// Linux
// =====
//
// c++ -fPIC -O2 -std=c++14 -IWDL/WDL -shared main.cpp -o reaper_barebone.so
//
// macOS
// =====
//
// c++ -fPIC -O2 -std=c++14 -IWDL/WDL -dynamiclib main.cpp -o reaper_barebone.dylib
//
// Windows
// =======
//
// (Use the VS Command Prompt matching your REAPER architecture, eg. x64 to use the 64-bit compiler)
// cl /nologo /O2 /Z7 /Zo /DUNICODE main.cpp /link /DEBUG /OPT:REF /PDBALTPATH:%_PDB% /DLL /OUT:reaper_barebone.dll

#define REAPERAPI_IMPLEMENT
//#define __cplusplus // wtf why was this not defined in the first place/
#include  <thread>
#include  <condition_variable>
#include  <algorithm>
#include "gui.h"
#include "melody.h"
#include "actions.h"
#include "stb_ds.h"
//#include "stb_ds.h"
#include "playback.h"
#include "actionsReaper.h"
#include "editorinstance.h"

#include <cstdio>
#include <SDL.h>
//#include <WinUser.h>
#include "reaper_plugin_functions.h"
//#ifdef __cplusplus
// #define UINT void
//#endif

#include "midiprot.h"
#include <processthreadsapi.h>
extern "C" {
    int pianorollgui(void);
    extern _Atomic bool running;
}
//extern double __declspec(selectany) itemStart;

UINT command;

//void get_line(ProjectStateContext *ctx, char* format...) {
//    char oneline[4096];
//    ctx->GetLine(oneline, sizeof(oneline));
//    va_list arg_ptr;
//    va_start(arg_ptr, format);
//    res = vsscanf(oneline, format, va_args);
//    va_end(arg_ptr);
//}

static bool ProcessExtensionLine(const char *line, ProjectStateContext *ctx, bool /*isUndo*/, struct project_config_extension_t */*reg*/)
{

    char* copy = strdup(line);
    char* saveptr = NULL;
//why did i use sdl_strtokr instead of strtok_r
    char* token = SDL_strtokr(copy, " \n", &saveptr);
    if(strcmp(token, "<CONTINUOUSMIDIEDITOR")) {
        return false;
    }
    token = SDL_strtokr(NULL, " \n", &saveptr); ASSERT(token == NULL, "fail to read lconfig");
    free(copy);
    saveptr = NULL;

    ReaProject* project = GetCurrentProjectInLoadSave();
    while(true) {
        char oneline[4096];
        int getLineRes = ctx->GetLine(oneline, sizeof(oneline));
        token = SDL_strtokr(oneline, " \n", &saveptr);
        if(!strcmp(token, ">")) {
            break;
        }
        ASSERT(!strcmp(token, "<ITEM"), "fail to read lconfig");
        CONTINUOUSMIDIEDITOR_Config cnfg = {
            {0},
            {
                {100, 100, 500, 500},
                0, 1,
                0.2, 0.6,
                midi_mode_mpe,
                48
            }
        };
        while(true){
            char stringParam[/*38*/64];
            int getLineRes = ctx->GetLine(oneline, sizeof(oneline));
//            token = SDL_strtokr(oneline, " \n", &saveptr);
            if( sscanf(oneline, " %1[>]", stringParam) == 1 || getLineRes == -1) {
                break;
            } else if( sscanf(oneline, " GUID %38s", stringParam) == 1 ){
                stringToGuid(stringParam, &cnfg.key);
                continue;
            } else if ( sscanf(oneline, " WNDRECT " RECT_FORMAT, RECT_ARGS(&cnfg.value.windowGeometry)) == 4 ){
                continue;
            } else if ( sscanf(oneline, " INNERRECT %lf %lf %lf %lf ", &cnfg.value.horizontalScroll,
                               &cnfg.value.verticalScroll,
                               &cnfg.value.horizontalFrac,
                               &cnfg.value.verticalFrac
                               ) == 4 ){
                continue;
            } else if ( sscanf(oneline, " FORMAT %29s", stringParam) == 1 ){
                bool succ;
                cnfg.value.midiMode = VALUE_FROM_NAME(midi_mode_enum, stringParam, &succ);
                ASSERT(succ, "fail to read config");
                continue;
            } else if ( sscanf(oneline, " PITCH_RANGE %lf", &cnfg.value.pitchRange) == 1 ){
                continue;
            } else {
                continue;
            }
        }
        cnfg.value.project = project;
        ASSERT(cnfg.key != GUID_NULL, "cant parse project file");


        hmputs(config, cnfg);
    }
    return true;
}
static void SaveExtensionConfig(ProjectStateContext *ctx, bool /*isUndo*/, struct project_config_extension_t */*reg*/) {
    if(hmlen(config) == 0) {
        return;
    }

    ReaProject* project = GetCurrentProjectInLoadSave();
    ctx->AddLine("<CONTINUOUSMIDIEDITOR");
    FOR_STB_MAP(CONTINUOUSMIDIEDITOR_Config*, cnfg, config) {
        if(project != cnfg->value.project) {
            continue;
        }
        ctx->AddLine("<ITEM");

        char guid[/*38*/64]; guidToString((GUID*)&cnfg->key, guid);
        ctx->AddLine("GUID %s", guid);
        ctx->AddLine("WNDRECT " RECT_FORMAT, RECT_ARGS(cnfg->value.windowGeometry));
        ctx->AddLine("INNERRECT %lf %lf %lf %lf", cnfg->value.horizontalScroll,
                     cnfg->value.verticalScroll,
                     cnfg->value.horizontalFrac,
                     cnfg->value.verticalFrac);
        ctx->AddLine("FORMAT %s", NAME_FROM_VALUE(midi_mode_enum, cnfg->value.midiMode));
        ctx->AddLine("PITCH_RANGE %lf", cnfg->value.pitchRange);

        ctx->AddLine(">");
    }
    ctx->AddLine(">");
}


//UINT play = 1007;
//// This function creates the extension menu (flag==0) and handles checking menu items (flag==1).
//// Reaper automatically checks menu items of customized menus using toggleActionHook above,
//// but since we can't tell if a menu is customized we always check either way.
//char fefwef[16];
//static void swsMenuHook(const char* menustr, HMENU hMenu, int flag)
//{
//    ShowConsoleMsg("menustr: ");
//    ShowConsoleMsg(menustr);
//    ShowConsoleMsg("\n");
//    if(strcmp("Media item context", menustr)) {
//        return;
//    }
//    char* rerere = (char*)malloc(16+strlen(menustr)+20);
////    memset(rerere, 0, 16+strlen(menustr)+20);
//    memcpy(rerere, "hi this is menu", 17);
//    memcpy(rerere+16, menustr, strlen(menustr));
//    MENUITEMINFOA mii = {
//        .cbSize=sizeof(MENUITEMINFO),
//        .fMask=MIIM_STRING,
//        .fType=MFT_STRING,
//        .fState=MFS_ENABLED,
//        .wID=command,
//        .hSubMenu=0,
//        .hbmpChecked=0,
//        .hbmpUnchecked=0,

//        .dwItemData=0,
//        .dwTypeData=rerere,
//        .cch=(UINT)strlen(rerere),
//        .hbmpItem=0



//    };
//    if(flag==0) {
////        InsertMenu(hMenu, 0, 0, 0, NULL);
//        InsertMenuItemA(hMenu, 0, TRUE, &mii);
//    }
//}
static char takeHash[16];
void loadTake()
{
    clearPiece();
    char guid[/*38*/64];
    GetSetMediaItemTakeInfo_String(take, "GUID", guid, false);
    message("take name is %s\nguid is %s", GetTakeName(take), guid);

    GUID currentGuid;
    stringToGuid(guid, &currentGuid);
    currentItemConfig = hmgetp_null(config, currentGuid);
//            cfg;
    MediaItem* item = GetMediaItemTake_Item(take);
    ReaProject* project = GetItemProjectContext(item);
    if(currentItemConfig  == NULL) {
        CONTINUOUSMIDIEDITOR_Config  cfg = {
            .key = currentGuid,
            .value = {
                .windowGeometry = { 400, 400, 700, 700 },
                .horizontalScroll = 0,
                .horizontalFrac = 0.1,
                .verticalScroll = 0.5,
                .verticalFrac = 0.1,
                .midiMode = midi_mode_mpe,
                .pitchRange = 48,
                .project = project
            }
        };
        hmputs(config, cfg);
        currentItemConfig  = hmgetp_null(config, currentGuid);
        ASSERT(currentItemConfig , "");
    }

    bool res = true; //MIDI_GetAllEvts(take, events, &size);
    double ppqpos;
//        int vel;
    int i = 0;
    itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
    pieceLength = /*itemStart + */GetMediaItemInfo_Value(item, "D_LENGTH");
    // but allocate it anyway just 'cause
    int noteNumber = 0;
    struct channelProperty {
        double pitch;
                // don't care about channel 0 because in MPE it's different
        double savedPitch;
        double noteStart;
        int noteNumber;
        bool selected; bool muted;
    } channelProperties[16] = {0};

    for(int i = 0; i < 16; i++) {
        channelProperties[i] = channelProperty{1, -1, -1, false, false};
//        channelPitches[i] = 1;
//        channelNoteStarts[i]=-1;
    }
#define MAX_MIDI_EVENT_LENGTH 3
#define PADINCASEIMISSEDSOMELONGEREVENTS 10000
    u8 msg[MAX_MIDI_EVENT_LENGTH+PADINCASEIMISSEDSOMELONGEREVENTS];

    int notecntOut, ccevtcntOut, textsyxevtcntOut;
    int allevts = MIDI_CountEvts(take, &notecntOut,
                                 &ccevtcntOut,
                                 &textsyxevtcntOut);

    int size = 10003;
    double trertse;
    bool getallevre = MIDI_GetAllEvts(take, (char*)msg, &size);

    size = 10003;
    getallevre = MIDI_GetEvt(take, 0, 0, 0, &trertse, (char*)(&msg[0]), &size);
    while(true) {

        bool muted;
        bool selected;
        int size = 3;
        res = MIDI_GetEvt(take, i++, &selected, &muted, &ppqpos, (char*)(&msg[0]), &size);
        if(!res) break;
        ASSERT(size <= 3, "got midi event longer than 3 bytes, dying\n");
        u8 channel = msg[0] & MIDI_CHANNEL_MASK;
        if((msg[0] & MIDI_COMMAND_MASK) == pitchWheelEvent) {
           i16 pitchWheel =
                   (msg[2] << 7) |
                   (msg[1]&MIDI_7HB_MASK);
           double differenceInTones =
                   double(pitchWheel-0x2000)/0x2000 * currentItemConfig->value.pitchRange / 2.0;
           double ratio = pow(2, differenceInTones/6);
           channelProperties[channel].pitch = ratio;
        }
//          res = MIDI_GetNote(take, i++, 0, 0, , &endppqpos, 0, &pitch, &vel);

// we think that all the simultaneous notes are on different channels
// so to get note's key we only need to read it from onteOff event
        double pos = MIDI_GetProjTimeFromPPQPos(take, ppqpos);
        if((msg[0] & MIDI_COMMAND_MASK) == noteOn) {
            channelProperties[channel].noteStart = pos;
            channelProperties[channel].noteNumber = noteNumber;
            channelProperties[channel].selected = selected;
            channelProperties[channel].muted = muted;
            channelProperties[channel].savedPitch = channelProperties[channel].pitch;
            noteNumber++;
        }
        if((msg[0] & MIDI_COMMAND_MASK) == noteOff) {
            int key = msg[1];
            int vel = msg[2];
            double freq = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
            freq *= channelProperties[channel].savedPitch;
//            message("st %lf end %lf pitch %d vel %d\n"
//                    "start %lf  freq %lf", pos, channelNoteStarts[channel] , key, vel
//                    , start, freq);
            appendRealNote({.note = {.freq = freq,
                                     .start = channelProperties[channel].noteStart  - itemStart,
                                     .length = pos-channelProperties[channel].noteStart,
                                     .muted = channelProperties[channel].muted,
                                     .velocity = vel
                                    },
                            .midiChannel = channel,
                            .selected = channelProperties[channel].selected,
                           .reaperNumber = channelProperties[channel].noteNumber});
        }
    }
    // GetProjectTimeSignature2 actually returns beats, not quarters, per minute.
    // It also doesn't return the denominator of the time signature, so it's not very useful
//        GetProjectTimeSignature2(NULL, &qpm, &bpiOut);
    // TimeMap_GetTimeSigAtTime's docs say that it returns something called "tempo" in the third argument,
    // experiments show it actually returns quarters per minute
    TimeMap_GetTimeSigAtTime(NULL, 0, &projectSignature.num,
                             &projectSignature.denom, &projectSignature.qpm);
    arrsetlen(tempoMarkers, 0);

    // Okay, hot take: time signature denominators are CRINGE. They may add some interpretive context
    // to musicians but it's actually not needed; in midi editors we don't specify when a certain note is D#
    // or Eb, and noone misses that, and the function of the note is clear from context and you can't
    // fully specify it with these alteration marks anyway. In the same way the denominator of the time signature
    // doesn't add enough rhytmical information to justify its usage.

    int numberOfTempoMarkers = CountTempoTimeSigMarkers(NULL);
    for(int i = 0; i < numberOfTempoMarkers; i ++) {
        double timepos, beatpos, mbpm;
        int measurepos, timesig_num, timesig_denom;
        bool linearTempo;
        GetTempoTimeSigMarker(NULL, i, &timepos, &measurepos, &beatpos, &mbpm, &timesig_num, &timesig_denom, &linearTempo);
        arrpush(tempoMarkers, (TempoMarker{ .when = timepos-itemStart, .qpm = mbpm,
                                            .num = timesig_num, .denom = timesig_denom}));
    }
    bool getHashRes = MIDI_GetHash(take, false, takeHash, sizeof(takeHash));
    ASSERT(getHashRes, "MIDI_GetHash returned false");
    std::stable_sort(piece, piece+arrlen(piece), [](const RealNote& a, const RealNote& b){
        return a.reaperNumber < b.reaperNumber;
    });

}

void timer_function() {
    std::unique_lock lk(actionChannel.mutex);
    if(actionChannel.pending) {
        actionChannel.action();
        MIDI_GetHash(take, false, takeHash, sizeof(takeHash));
        actionChannel.pending = false;
    }
    actionChannel.action = decltype(actionChannel.action)();
    if(take != 0) {
        MediaItem_Take* newTake = GetMediaItemTakeByGUID((ReaProject*)currentItemConfig->value.project, &currentItemConfig->key);
        ASSERT(newTake == 0 || newTake == take, "GetMediaItemTakeByGUID returned something weird");
        if(newTake == 0) {
            arrsetlen(tempoMarkers, 0);
            arrsetlen(piece, 0);
            take = 0;

            SDL_WindowEvent windowEvent = {SDL_WINDOWEVENT,
                                           SDL_GetTicks(),
                                           SDL_GetWindowID(rootWindow),
                                           SDL_WINDOWEVENT_CLOSE};
            SDL_Event event; event.window = windowEvent;
            SDL_PushEvent(&event);
        } else {
            char hash[sizeof(takeHash)];
            bool getHashRes = MIDI_GetHash(take, false, hash, sizeof(hash));
            ASSERT(getHashRes, "MIDI_GetHash returned false ¯\\_(ツ)_/¯");
            if(memcmp(hash, takeHash, sizeof(takeHash))) {
                loadTake();
            }
            guiRedrawFromOtherThread(rootWindow);
        }
    }
    lk.unlock();
    actionChannel.cv.notify_one();
    currentPositionInSamples =  (int)round((GetPlayPosition()-itemStart)*44100);
    cursorPosition = GetCursorPosition()-itemStart;
    double deb1 = GetPlayPosition();
    double deb2 = GetCursorPosition();
    sprintf(DebugBuffer, "playp %lf curp %lf", deb1, deb2);
    static int previousPlaying = false;
    int playstate = GetPlayState();
    playing = playstate & 1;
    paused = playstate & 2;
//    if(previousPlaying < 10) {
//        currentPositionInSamples = cursorPosition*44100;
//    }
    previousPlaying = playing*(previousPlaying + playing);
    if(playing && !paused) {
        SDL_UserEvent userevent = {(u32)PlaybackEvent, SDL_GetTicks(), 0, 0, 0, 0};
        SDL_Event event; event.user = userevent;
        SDL_PushEvent(&event);
    }
    repeatOn = GetSetRepeat(-1);
}
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
 } THREADNAME_INFO;
#pragma pack(pop)
extern "C" void SetThreadName(DWORD dwThreadID, const char* threadName) {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
    __try{
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER){
    }
#pragma warning(pop)
}
//std::thread th;
extern "C" SDL_mutex* mutex_; // I use SDL_mutex because MSVC doesn't provide thread.h
//extern "C" SDL_cond* condVar;
//bool data_ready = false;
extern "C" extern bool timeToLeave;
extern "C" bool timeToShow;
// extern "C" bool timeToHide  ?;
//std::condition_variable ;
//std::atomic<MediaItem*> item = NULL;

void sdlThread() {
    SetThreadName ((DWORD)-1, "contMidiEditorThread");
    reaperMainThread = false;

//    while(true) {
//        std::unique_lock lk(mutex_);
//        condVar.wait(lk, [](){return data_ready;});
//        if(timeToLeave) {

//            lk.unlock();
//            break;
//        }
        pianorollgui();
//        data_ready = false;
//    }
}
void closeSDLWindow() {
//    if(!mutex_.try_lock()) {
        SDL_WindowEvent windowevent = {
            SDL_WINDOWEVENT, SDL_GetTicks(), SDL_GetWindowID(rootWindow),
            SDL_WINDOWEVENT_CLOSE};
        SDL_Event event; event.window = windowevent;
        SDL_PushEvent(&event);
//        while(!mutex_.try_lock()) {
//            timer_function();
//        }
//    }
//   TODO: wait for sdl window to be closed (for pianorollgui() to return)
}
//void showSDLWindow() {
////    if(!mutex_.try_lock()) {
//        SDL_WindowEvent windowevent = {
//            SDL_WINDOWEVENT, SDL_GetTicks(), SDL_GetWindowID(rootWindow),
//            SDL_WINDOWEVENT_S};
//        SDL_Event event; event.window = windowevent;
//        SDL_PushEvent(&event);
////        while(!mutex_.try_lock()) {
////            timer_function();
////        }
////    }
////   TODO: wait for sdl window to be closed (for pianorollgui() to return)
//}
// FreeLibrary(GetModuleHandleA("reaper_midieditor.dll"));
std::thread th;
bool sdlThreadStarted;


bool hookCommandProc(int iCmd, int /*flag*/)
{
    char msg[100];
    snprintf(msg, 100, "hookCommandProc! %d\n", iCmd);
    ShowConsoleMsg(msg);
    if(iCmd == command) {


        if(aborted) {
            MB("Can't open the microtonal midi editor because it has experienced an error earlier. "
               "If this impacts you, consider writing to the developer at kitttoran@gmail.com and "
               "I will fix this problem",
               "Can't open the microtonal midi editor", 0);
            return true;
        }
    


        if(!sdlThreadStarted) {
            th=std::thread(sdlThread);
            mutex_ = SDL_CreateMutex();
                    sdlThreadStarted = true;
         }
//        closeSDLWindow();
        SDL_LockMutex(mutex_);
//            std::lock_guard lg(mutex_);
        MediaItem* item = GetSelectedMediaItem(NULL, 0);

        take = GetActiveTake(item);

        ReaProject* project = GetItemProjectContext(item);

        double ppqpos = MIDI_GetPPQPosFromProjTime(take, 1.1);
        volatile double pos = MIDI_GetProjTimeFromPPQPos(take, ppqpos);


//        ReaProject* project = GetItemProjectContext(item);
        loadTake();
//        data_ready = true;
        timeToShow = true;
        SDL_UnlockMutex(mutex_);
//        condVar.notify_one();
//        if(th.joinable()) {
//            SDL_WindowEvent windowevent = {
//                SDL_WINDOWEVENT, SDL_GetTicks(), SDL_GetWindowID(rootWindow),
//                SDL_WINDOWEVENT_CLOSE};
//            SDL_Event event; event.window = windowevent;
//            SDL_PushEvent(&event);
//            th.join();
//            ASSERT(!th.joinable());
//        }

//        ASSERT(!th.joinable());
//        th = std::thread(pianorollgui);

//        SetThreadName ((DWORD)-1, "Hi this is thresf");
//        SetThreadName ((DWORD)th.native_handle(), "Hi this is thread another");
//        SetThreadDescription(
//        th.native_handle(),
//                    L"ContMidi!");
//        th.detach();
        return true;
    }
    return false;
}

bool hookCommandProc2(KbdSectionInfo* /*sec*/, int cmdId, int /*val*/, int /*valhw*/, int /*relmode*/, HWND /*hwnd*/)
{
    return hookCommandProc(cmdId, 0);
//    char msg[100];
//    snprintf(msg, 100, "hookCommandProc2! %d\n", cmdId);
//    ShowConsoleMsg(msg);
//    return false;
}
//class CONMIDException {};

extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
  REAPER_PLUGIN_HINSTANCE /*instance*/, reaper_plugin_info_t *rec)
{


//    throw CONMIDException();
    reaperMainThread = true;
  if(!rec) {

    // cleanup code here
    if( sdlThreadStarted) {
      closeSDLWindow();
        SDL_LockMutex(mutex_);
        timeToLeave = true;
        SDL_UnlockMutex(mutex_);
        timer_function();
    //    data_ready = true;
    //    SDL_DestroyMutex(mutex_);
    //    mutex_.unlock();
    //    condVar.notify_one();
//        while(th.joinable()) {
//            closeSDLWindow();
//            timer_function();
//        }
        th.join();
    }
    return 0;
  }

  if(rec->caller_version != REAPER_PLUGIN_VERSION)
    return 0;

  // see also https://gist.github.com/cfillion/350356a62c61a1a2640024f8dc6c6770
#define GET_FUNC(a) a = (decltype(a))rec->GetFunc(#a); if(!a)  {MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
    "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL); return false; }

  GET_FUNC(AddProjectMarker2)     	GET_FUNC(GetEnvelopeStateChunk)     	GET_FUNC(InsertEnvelopePointEx)     	GET_FUNC(SetTakeStretchMarker)
  GET_FUNC(AddRemoveReaScript)     	GET_FUNC(GetExePath)     	GET_FUNC(InsertMedia)     	GET_FUNC(SetTakeStretchMarkerSlope)
  GET_FUNC(AddTakeToMediaItem)     	GET_FUNC(GetExtState)     	GET_FUNC(InsertMediaSection)     	GET_FUNC(SetTempoTimeSigMarker)
  GET_FUNC(AddTempoTimeSigMarker)     	GET_FUNC(GetFocusedFX)     	GET_FUNC(InsertTrackAtIndex)     	GET_FUNC(SetThemeColor)
  GET_FUNC(adjustZoom)     	GET_FUNC(GetFocusedFX2)     	GET_FUNC(IsMediaExtension)     	GET_FUNC(SetToggleCommandState)
  GET_FUNC(AnyTrackSolo)     	GET_FUNC(GetFreeDiskSpaceForRecordPath)     	GET_FUNC(IsMediaItemSelected)     	GET_FUNC(SetTrackAutomationMode)
  GET_FUNC(APIExists)     	GET_FUNC(GetFXEnvelope)     	GET_FUNC(IsProjectDirty)     	GET_FUNC(SetTrackColor)
  GET_FUNC(APITest)     	GET_FUNC(GetGlobalAutomationOverride)     	GET_FUNC(IsTrackSelected)     	GET_FUNC(SetTrackMIDILyrics)
  GET_FUNC(ApplyNudge)     	GET_FUNC(GetHZoomLevel)     	GET_FUNC(IsTrackVisible)     	GET_FUNC(SetTrackMIDINoteName)
  GET_FUNC(ArmCommand)     	GET_FUNC(GetInputChannelName)     	GET_FUNC(joystick_create)     	GET_FUNC(SetTrackMIDINoteNameEx)
  GET_FUNC(Audio_Init)     	GET_FUNC(GetInputOutputLatency)     	GET_FUNC(joystick_destroy)     	GET_FUNC(SetTrackSelected)
  GET_FUNC(Audio_IsPreBuffer)     	GET_FUNC(GetItemEditingTime2)     	GET_FUNC(joystick_enum)     	GET_FUNC(SetTrackSendInfo_Value)
  GET_FUNC(Audio_IsRunning)     	GET_FUNC(GetItemFromPoint)     	GET_FUNC(joystick_getaxis)     	GET_FUNC(SetTrackSendUIPan)
  GET_FUNC(Audio_Quit)     	GET_FUNC(GetItemProjectContext)     	GET_FUNC(joystick_getbuttonmask)     	GET_FUNC(SetTrackSendUIVol)
  GET_FUNC(AudioAccessorStateChanged)     	GET_FUNC(GetItemStateChunk)     	GET_FUNC(joystick_getinfo)     	GET_FUNC(SetTrackStateChunk)
  GET_FUNC(AudioAccessorUpdate)     	GET_FUNC(GetLastColorThemeFile)     	GET_FUNC(joystick_getpov)     	GET_FUNC(ShowActionList)
  GET_FUNC(AudioAccessorValidateState)     	GET_FUNC(GetLastMarkerAndCurRegion)     	GET_FUNC(joystick_update)     	GET_FUNC(ShowConsoleMsg)
  GET_FUNC(BypassFxAllTracks)     	GET_FUNC(GetLastTouchedFX)     	GET_FUNC(LICE_ClipLine)     	GET_FUNC(ShowMessageBox)
  GET_FUNC(CalculateNormalization)     	GET_FUNC(GetLastTouchedTrack)     	GET_FUNC(LocalizeString)     	GET_FUNC(ShowPopupMenu)
  GET_FUNC(ClearAllRecArmed)     	GET_FUNC(GetMainHwnd)     	GET_FUNC(Loop_OnArrow)     	GET_FUNC(SLIDER2DB)
  GET_FUNC(ClearConsole)     	GET_FUNC(GetMasterMuteSoloFlags)     	GET_FUNC(Main_OnCommand)     	GET_FUNC(SnapToGrid)
  GET_FUNC(ClearPeakCache)     	GET_FUNC(GetMasterTrack)     	GET_FUNC(Main_OnCommandEx)     	GET_FUNC(SoloAllTracks)
  GET_FUNC(ColorFromNative)     	GET_FUNC(GetMasterTrackVisibility)     	GET_FUNC(Main_openProject)     	GET_FUNC(Splash_GetWnd)
  GET_FUNC(ColorToNative)     	GET_FUNC(GetMaxMidiInputs)     	GET_FUNC(Main_SaveProject)     	GET_FUNC(SplitMediaItem)
  GET_FUNC(CountAutomationItems)     	GET_FUNC(GetMaxMidiOutputs)     	GET_FUNC(Main_UpdateLoopInfo)     	GET_FUNC(stringToGuid)
  GET_FUNC(CountEnvelopePoints)     	GET_FUNC(GetMediaFileMetadata)     	GET_FUNC(MarkProjectDirty)     	GET_FUNC(StuffMIDIMessage)
  GET_FUNC(CountEnvelopePointsEx)     	GET_FUNC(GetMediaItem)     	GET_FUNC(MarkTrackItemsDirty)     	GET_FUNC(TakeFX_AddByName)
  GET_FUNC(CountMediaItems)     	GET_FUNC(GetMediaItem_Track)     	GET_FUNC(Master_GetPlayRate)     	GET_FUNC(TakeFX_CopyToTake)
  GET_FUNC(CountProjectMarkers)     	GET_FUNC(GetMediaItemInfo_Value)     	GET_FUNC(Master_GetPlayRateAtTime)     	GET_FUNC(TakeFX_CopyToTrack)
  GET_FUNC(CountSelectedMediaItems)     	GET_FUNC(GetMediaItemNumTakes)     	GET_FUNC(Master_GetTempo)     	GET_FUNC(TakeFX_Delete)
  GET_FUNC(CountSelectedTracks)     	GET_FUNC(GetMediaItemTake)     	GET_FUNC(Master_NormalizePlayRate)     	GET_FUNC(TakeFX_EndParamEdit)
  GET_FUNC(CountSelectedTracks2)     	GET_FUNC(GetMediaItemTake_Item)     	GET_FUNC(Master_NormalizeTempo)     	GET_FUNC(TakeFX_FormatParamValue)
  GET_FUNC(CountTakeEnvelopes)     	GET_FUNC(GetMediaItemTake_Peaks)     	GET_FUNC(MB)     	GET_FUNC(TakeFX_FormatParamValueNormalized)
  GET_FUNC(CountTakes)     	GET_FUNC(GetMediaItemTake_Source)     	GET_FUNC(MediaItemDescendsFromTrack)     	GET_FUNC(TakeFX_GetChainVisible)
  GET_FUNC(CountTCPFXParms)     	GET_FUNC(GetMediaItemTake_Track)     	GET_FUNC(MIDI_CountEvts)     	GET_FUNC(TakeFX_GetCount)
  GET_FUNC(CountTempoTimeSigMarkers)     	GET_FUNC(GetMediaItemTakeByGUID)     	GET_FUNC(MIDI_DeleteCC)     	GET_FUNC(TakeFX_GetEnabled)
  GET_FUNC(CountTrackEnvelopes)     	GET_FUNC(GetMediaItemTakeInfo_Value)     	GET_FUNC(MIDI_DeleteEvt)     	GET_FUNC(TakeFX_GetEnvelope)
  GET_FUNC(CountTrackMediaItems)     	GET_FUNC(GetMediaItemTrack)     	GET_FUNC(MIDI_DeleteNote)     	GET_FUNC(TakeFX_GetFloatingWindow)
  GET_FUNC(CountTracks)     	GET_FUNC(GetMediaSourceFileName)     	GET_FUNC(MIDI_DeleteTextSysexEvt)     	GET_FUNC(TakeFX_GetFormattedParamValue)
  GET_FUNC(CreateNewMIDIItemInProj)     	GET_FUNC(GetMediaSourceLength)     	GET_FUNC(MIDI_DisableSort)     	GET_FUNC(TakeFX_GetFXGUID)
  GET_FUNC(CreateTakeAudioAccessor)     	GET_FUNC(GetMediaSourceNumChannels)     	GET_FUNC(MIDI_EnumSelCC)     	GET_FUNC(TakeFX_GetFXName)
  GET_FUNC(CreateTrackAudioAccessor)     	GET_FUNC(GetMediaSourceParent)     	GET_FUNC(MIDI_EnumSelEvts)     	GET_FUNC(TakeFX_GetIOSize)
  GET_FUNC(CreateTrackSend)     	GET_FUNC(GetMediaSourceSampleRate)     	GET_FUNC(MIDI_EnumSelNotes)     	GET_FUNC(TakeFX_GetNamedConfigParm)
        GET_FUNC(GetMediaSourceType)     	GET_FUNC(MIDI_EnumSelTextSysexEvts)     	GET_FUNC(TakeFX_GetNumParams)
        GET_FUNC(GetMediaTrackInfo_Value)     	GET_FUNC(MIDI_GetAllEvts)     	GET_FUNC(TakeFX_GetOffline)
        GET_FUNC(GetMIDIInputName)     	GET_FUNC(MIDI_GetCC)     	GET_FUNC(TakeFX_GetOpen)
        GET_FUNC(GetMIDIOutputName)     	GET_FUNC(MIDI_GetCCShape)     	GET_FUNC(TakeFX_GetParam)
        GET_FUNC(GetMixerScroll)     	GET_FUNC(MIDI_GetEvt)     	GET_FUNC(TakeFX_GetParameterStepSizes)
        GET_FUNC(GetMouseModifier)     	GET_FUNC(MIDI_GetGrid)     	GET_FUNC(TakeFX_GetParamEx)
        GET_FUNC(GetMousePosition)     	GET_FUNC(MIDI_GetHash)     	GET_FUNC(TakeFX_GetParamFromIdent)
        GET_FUNC(GetNumAudioInputs)     	GET_FUNC(MIDI_GetNote)     	GET_FUNC(TakeFX_GetParamIdent)
        GET_FUNC(GetNumAudioOutputs)     	GET_FUNC(MIDI_GetPPQPos_EndOfMeasure)     	GET_FUNC(TakeFX_GetParamName)
        GET_FUNC(GetNumMIDIInputs)     	GET_FUNC(MIDI_GetPPQPos_StartOfMeasure)     	GET_FUNC(TakeFX_GetParamNormalized)
        GET_FUNC(GetNumMIDIOutputs)     	GET_FUNC(MIDI_GetPPQPosFromProjQN)     	GET_FUNC(TakeFX_GetPinMappings)
        GET_FUNC(GetNumTakeMarkers)     	GET_FUNC(MIDI_GetPPQPosFromProjTime)     	GET_FUNC(TakeFX_GetPreset)
        GET_FUNC(GetNumTracks)     	GET_FUNC(MIDI_GetProjQNFromPPQPos)     	GET_FUNC(TakeFX_GetPresetIndex)
        GET_FUNC(GetOS)     	GET_FUNC(MIDI_GetProjTimeFromPPQPos)     	GET_FUNC(TakeFX_GetUserPresetFilename)
        GET_FUNC(GetOutputChannelName)     	GET_FUNC(MIDI_GetRecentInputEvent)     	GET_FUNC(TakeFX_NavigatePresets)
        GET_FUNC(GetOutputLatency)     	GET_FUNC(MIDI_GetScale)     	GET_FUNC(TakeFX_SetEnabled)
        GET_FUNC(GetParentTrack)     	GET_FUNC(MIDI_GetTextSysexEvt)     	GET_FUNC(TakeFX_SetNamedConfigParm)
        GET_FUNC(GetPeakFileName)     	GET_FUNC(MIDI_GetTrackHash)     	GET_FUNC(TakeFX_SetOffline)
        GET_FUNC(GetPeakFileNameEx)     	GET_FUNC(midi_init)     	GET_FUNC(TakeFX_SetOpen)
        GET_FUNC(GetPeakFileNameEx2)     	GET_FUNC(MIDI_InsertCC)     	GET_FUNC(TakeFX_SetParam)
        GET_FUNC(GetPlayPosition)     	GET_FUNC(MIDI_InsertEvt)     	GET_FUNC(TakeFX_SetParamNormalized)
        GET_FUNC(GetPlayPosition2)     	GET_FUNC(MIDI_InsertNote)     	GET_FUNC(TakeFX_SetPinMappings)
        GET_FUNC(GetPlayPosition2Ex)     	GET_FUNC(MIDI_InsertTextSysexEvt)     	GET_FUNC(TakeFX_SetPreset)
        GET_FUNC(GetPlayPositionEx)     	GET_FUNC(midi_reinit)     	GET_FUNC(TakeFX_SetPresetByIndex)
        GET_FUNC(GetPlayState)     	GET_FUNC(MIDI_SelectAll)     	GET_FUNC(TakeFX_Show)
        GET_FUNC(GetPlayStateEx)     	GET_FUNC(MIDI_SetAllEvts)     	GET_FUNC(TakeIsMIDI)
        GET_FUNC(GetProjectLength)     	GET_FUNC(MIDI_SetCC)     	GET_FUNC(ThemeLayout_GetLayout)
        GET_FUNC(GetProjectName)     	GET_FUNC(MIDI_SetCCShape)     	GET_FUNC(ThemeLayout_GetParameter)
        GET_FUNC(GetProjectPath)     	GET_FUNC(MIDI_SetEvt)     	GET_FUNC(ThemeLayout_RefreshAll)
        GET_FUNC(GetProjectPathEx)     	GET_FUNC(MIDI_SetItemExtents)     	GET_FUNC(ThemeLayout_SetLayout)
        GET_FUNC(GetProjectStateChangeCount)     	GET_FUNC(MIDI_SetNote)     	GET_FUNC(ThemeLayout_SetParameter)
        GET_FUNC(GetProjectTimeOffset)     	GET_FUNC(MIDI_SetTextSysexEvt)     	GET_FUNC(time_precise)
        GET_FUNC(GetProjectTimeSignature)     	GET_FUNC(MIDI_Sort)     	GET_FUNC(TimeMap2_beatsToTime)
        GET_FUNC(GetProjectTimeSignature2)     	GET_FUNC(MIDIEditor_EnumTakes)     	GET_FUNC(TimeMap2_GetDividedBpmAtTime)
        GET_FUNC(GetProjExtState)     	GET_FUNC(MIDIEditor_GetActive)     	GET_FUNC(TimeMap2_GetNextChangeTime)
        GET_FUNC(GetResourcePath)     	GET_FUNC(MIDIEditor_GetMode)     	GET_FUNC(TimeMap2_QNToTime)
        GET_FUNC(GetSelectedEnvelope)     	GET_FUNC(MIDIEditor_GetSetting_int)     	GET_FUNC(TimeMap2_timeToBeats)
        GET_FUNC(GetSelectedMediaItem)     	GET_FUNC(MIDIEditor_GetSetting_str)     	GET_FUNC(TimeMap2_timeToQN)
        GET_FUNC(GetSelectedTrack)     	GET_FUNC(MIDIEditor_GetTake)     	GET_FUNC(TimeMap_curFrameRate)
        GET_FUNC(GetSelectedTrack2)     	GET_FUNC(MIDIEditor_LastFocused_OnCommand)     	GET_FUNC(TimeMap_GetDividedBpmAtTime)
        GET_FUNC(GetSelectedTrackEnvelope)     	GET_FUNC(MIDIEditor_OnCommand)     	GET_FUNC(TimeMap_GetMeasureInfo)
        GET_FUNC(GetSet_ArrangeView2)     	GET_FUNC(MIDIEditor_SetSetting_int)     	GET_FUNC(TimeMap_GetMetronomePattern)
        GET_FUNC(GetSet_LoopTimeRange)     	GET_FUNC(mkpanstr)     	GET_FUNC(TimeMap_GetTimeSigAtTime)
        GET_FUNC(GetSet_LoopTimeRange2)     	GET_FUNC(mkvolpanstr)     	GET_FUNC(TimeMap_QNToMeasures)
        GET_FUNC(GetSetAutomationItemInfo)     	GET_FUNC(mkvolstr)     	GET_FUNC(TimeMap_QNToTime)
        GET_FUNC(GetSetAutomationItemInfo_String)     	GET_FUNC(MoveEditCursor)     	GET_FUNC(TimeMap_QNToTime_abs)
        GET_FUNC(GetSetEnvelopeInfo_String)     	GET_FUNC(MoveMediaItemToTrack)     	GET_FUNC(TimeMap_timeToQN)
        GET_FUNC(GetSetEnvelopeState)     	GET_FUNC(MuteAllTracks)     	GET_FUNC(TimeMap_timeToQN_abs)
        GET_FUNC(GetSetEnvelopeState2)     	GET_FUNC(my_getViewport)     	GET_FUNC(ToggleTrackSendUIMute)
        GET_FUNC(GetSetItemState)     	GET_FUNC(NamedCommandLookup)     	GET_FUNC(Track_GetPeakHoldDB)
        GET_FUNC(GetSetItemState2)     	GET_FUNC(OnPauseButton)     	GET_FUNC(Track_GetPeakInfo)
        GET_FUNC(GetSetMediaItemInfo_String)     	GET_FUNC(OnPauseButtonEx)     	GET_FUNC(TrackCtl_SetToolTip)
  GET_FUNC(DB2SLIDER)     	GET_FUNC(GetSetMediaItemTakeInfo_String)     	GET_FUNC(OnPlayButton)     	GET_FUNC(TrackFX_AddByName)
  GET_FUNC(DeleteEnvelopePointEx)     	GET_FUNC(GetSetMediaTrackInfo_String)     	GET_FUNC(OnPlayButtonEx)     	GET_FUNC(TrackFX_CopyToTake)
  GET_FUNC(DeleteEnvelopePointRange)     	GET_FUNC(GetSetProjectAuthor)     	GET_FUNC(OnStopButton)     	GET_FUNC(TrackFX_CopyToTrack)
  GET_FUNC(DeleteEnvelopePointRangeEx)     	GET_FUNC(GetSetProjectGrid)     	GET_FUNC(OnStopButtonEx)     	GET_FUNC(TrackFX_Delete)
  GET_FUNC(DeleteExtState)     	GET_FUNC(GetSetProjectInfo)     	GET_FUNC(OpenColorThemeFile)     	GET_FUNC(TrackFX_EndParamEdit)
  GET_FUNC(DeleteProjectMarker)     	GET_FUNC(GetSetProjectInfo_String)     	GET_FUNC(OpenMediaExplorer)     	GET_FUNC(TrackFX_FormatParamValue)
  GET_FUNC(DeleteProjectMarkerByIndex)     	GET_FUNC(GetSetProjectNotes)     	GET_FUNC(OscLocalMessageToHost)     	GET_FUNC(TrackFX_FormatParamValueNormalized)
  GET_FUNC(DeleteTakeMarker)     	GET_FUNC(GetSetRepeat)     	GET_FUNC(parse_timestr)     	GET_FUNC(TrackFX_GetByName)
  GET_FUNC(DeleteTakeStretchMarkers)     	GET_FUNC(GetSetRepeatEx)     	GET_FUNC(parse_timestr_len)     	GET_FUNC(TrackFX_GetChainVisible)
  GET_FUNC(DeleteTempoTimeSigMarker)     	GET_FUNC(GetSetTrackGroupMembership)     	GET_FUNC(parse_timestr_pos)     	GET_FUNC(TrackFX_GetCount)
  GET_FUNC(DeleteTrack)     	GET_FUNC(GetSetTrackGroupMembershipHigh)     	GET_FUNC(parsepanstr)     	GET_FUNC(TrackFX_GetEnabled)
  GET_FUNC(DeleteTrackMediaItem)     	GET_FUNC(GetSetTrackSendInfo_String)     	GET_FUNC(PCM_Sink_Enum)     	GET_FUNC(TrackFX_GetEQ)
  GET_FUNC(DestroyAudioAccessor)     	GET_FUNC(GetSetTrackState)     	GET_FUNC(PCM_Sink_GetExtension)     	GET_FUNC(TrackFX_GetEQBandEnabled)
  GET_FUNC(Dock_UpdateDockID)     	GET_FUNC(GetSetTrackState2)     	GET_FUNC(PCM_Sink_ShowConfig)     	GET_FUNC(TrackFX_GetEQParam)
  GET_FUNC(DockGetPosition)     	GET_FUNC(GetSubProjectFromSource)     	GET_FUNC(PCM_Source_BuildPeaks)     	GET_FUNC(TrackFX_GetFloatingWindow)
  GET_FUNC(DockIsChildOfDock)     	GET_FUNC(GetTake)     	GET_FUNC(PCM_Source_CreateFromFile)     	GET_FUNC(TrackFX_GetFormattedParamValue)
  GET_FUNC(DockWindowActivate)     	GET_FUNC(GetTakeEnvelope)     	GET_FUNC(PCM_Source_CreateFromFileEx)     	GET_FUNC(TrackFX_GetFXGUID)
  GET_FUNC(DockWindowAdd)     	GET_FUNC(GetTakeEnvelopeByName)     	GET_FUNC(PCM_Source_CreateFromType)     	GET_FUNC(TrackFX_GetFXName)
  GET_FUNC(DockWindowAddEx)     	GET_FUNC(GetTakeMarker)     	GET_FUNC(PCM_Source_Destroy)     	GET_FUNC(TrackFX_GetInstrument)
  GET_FUNC(DockWindowRefresh)     	GET_FUNC(GetTakeName)     	GET_FUNC(PCM_Source_GetPeaks)     	GET_FUNC(TrackFX_GetIOSize)
  GET_FUNC(DockWindowRefreshForHWND)     	GET_FUNC(GetTakeNumStretchMarkers)     	GET_FUNC(PCM_Source_GetSectionInfo)     	GET_FUNC(TrackFX_GetNamedConfigParm)
  GET_FUNC(DockWindowRemove)     	GET_FUNC(GetTakeStretchMarker)     	GET_FUNC(PluginWantsAlwaysRunFx)     	GET_FUNC(TrackFX_GetNumParams)
  GET_FUNC(EditTempoTimeSigMarker)     	GET_FUNC(GetTakeStretchMarkerSlope)     	GET_FUNC(PreventUIRefresh)     	GET_FUNC(TrackFX_GetOffline)
  GET_FUNC(EnsureNotCompletelyOffscreen)     	GET_FUNC(GetTCPFXParm)     	GET_FUNC(PromptForAction)     	GET_FUNC(TrackFX_GetOpen)
  GET_FUNC(EnumerateFiles)     	GET_FUNC(GetTempoMatchPlayRate)     	GET_FUNC(ReaScriptError)     	GET_FUNC(TrackFX_GetParam)
  GET_FUNC(EnumerateSubdirectories)     	GET_FUNC(GetTempoTimeSigMarker)     	GET_FUNC(RecursiveCreateDirectory)     	GET_FUNC(TrackFX_GetParameterStepSizes)
  GET_FUNC(EnumPitchShiftModes)     	GET_FUNC(GetThemeColor)     	GET_FUNC(reduce_open_files)     	GET_FUNC(TrackFX_GetParamEx)
  GET_FUNC(EnumPitchShiftSubModes)     	GET_FUNC(GetThingFromPoint)     	GET_FUNC(RefreshToolbar)     	GET_FUNC(TrackFX_GetParamFromIdent)
  GET_FUNC(EnumProjectMarkers)     	GET_FUNC(GetToggleCommandState)     	GET_FUNC(RefreshToolbar2)     	GET_FUNC(TrackFX_GetParamIdent)
  GET_FUNC(EnumProjectMarkers2)     	GET_FUNC(GetToggleCommandStateEx)     	GET_FUNC(relative_fn)     	GET_FUNC(TrackFX_GetParamName)
  GET_FUNC(EnumProjectMarkers3)     	GET_FUNC(GetTooltipWindow)     	GET_FUNC(RemoveTrackSend)     	GET_FUNC(TrackFX_GetParamNormalized)
  GET_FUNC(EnumProjects)     	GET_FUNC(GetTrack)     	GET_FUNC(RenderFileSection)     	GET_FUNC(TrackFX_GetPinMappings)
  GET_FUNC(EnumProjExtState)     	GET_FUNC(GetTrackAutomationMode)     	GET_FUNC(ReorderSelectedTracks)     	GET_FUNC(TrackFX_GetPreset)
  GET_FUNC(EnumRegionRenderMatrix)     	GET_FUNC(GetTrackColor)     	GET_FUNC(Resample_EnumModes)     	GET_FUNC(TrackFX_GetPresetIndex)
  GET_FUNC(EnumTrackMIDIProgramNames)     	GET_FUNC(GetTrackDepth)     	GET_FUNC(resolve_fn)     	GET_FUNC(TrackFX_GetRecChainVisible)
  GET_FUNC(EnumTrackMIDIProgramNamesEx)     	GET_FUNC(GetTrackEnvelope)     	GET_FUNC(resolve_fn2)     	GET_FUNC(TrackFX_GetRecCount)
        GET_FUNC(GetTrackEnvelopeByChunkName)     	GET_FUNC(ReverseNamedCommandLookup)     	GET_FUNC(TrackFX_GetUserPresetFilename)
  GET_FUNC(GetTrackEnvelopeByName)     	GET_FUNC(ScaleFromEnvelopeMode)     	GET_FUNC(TrackFX_NavigatePresets)
  GET_FUNC(GetTrackFromPoint)     	GET_FUNC(ScaleToEnvelopeMode)     	GET_FUNC(TrackFX_SetEnabled)
  GET_FUNC(GetTrackGUID)     	GET_FUNC(SelectAllMediaItems)     	GET_FUNC(TrackFX_SetEQBandEnabled)
  GET_FUNC(GetTrackMediaItem)     	GET_FUNC(SelectProjectInstance)     	GET_FUNC(TrackFX_SetEQParam)
  GET_FUNC(GetTrackMIDILyrics)     	GET_FUNC(SetActiveTake)     	GET_FUNC(TrackFX_SetNamedConfigParm)
  GET_FUNC(ExecProcess)     	GET_FUNC(GetTrackMIDINoteName)     	GET_FUNC(SetAutomationMode)     	GET_FUNC(TrackFX_SetOffline)
  GET_FUNC(file_exists)     	GET_FUNC(GetTrackMIDINoteNameEx)     	GET_FUNC(SetCurrentBPM)     	GET_FUNC(TrackFX_SetOpen)
  GET_FUNC(FindTempoTimeSigMarker)     	GET_FUNC(GetTrackMIDINoteRange)     	GET_FUNC(SetCursorContext)     	GET_FUNC(TrackFX_SetParam)
  GET_FUNC(format_timestr)     	GET_FUNC(GetTrackName)     	GET_FUNC(SetEditCurPos)     	GET_FUNC(TrackFX_SetParamNormalized)
  GET_FUNC(format_timestr_len)     	GET_FUNC(GetTrackNumMediaItems)     	GET_FUNC(SetEditCurPos2)     	GET_FUNC(TrackFX_SetPinMappings)
  GET_FUNC(format_timestr_pos)     	GET_FUNC(GetTrackNumSends)     	GET_FUNC(SetEnvelopePoint)     	GET_FUNC(TrackFX_SetPreset)
  GET_FUNC(genGuid)     	GET_FUNC(GetTrackReceiveName)     	GET_FUNC(SetEnvelopePointEx)     	GET_FUNC(TrackFX_SetPresetByIndex)
  GET_FUNC(get_config_var_string)     	GET_FUNC(GetTrackReceiveUIMute)     	GET_FUNC(SetEnvelopeStateChunk)     	GET_FUNC(TrackFX_Show)
  GET_FUNC(get_ini_file)     	GET_FUNC(GetTrackReceiveUIVolPan)     	GET_FUNC(SetExtState)     	GET_FUNC(TrackList_AdjustWindows)
  GET_FUNC(GetActiveTake)     	GET_FUNC(GetTrackSendInfo_Value)     	GET_FUNC(SetGlobalAutomationOverride)     	GET_FUNC(TrackList_UpdateAllExternalSurfaces)
  GET_FUNC(GetAllProjectPlayStates)     	GET_FUNC(GetTrackSendName)     	GET_FUNC(SetItemStateChunk)     	GET_FUNC(Undo_BeginBlock)
  GET_FUNC(GetAppVersion)     	GET_FUNC(GetTrackSendUIMute)     	GET_FUNC(SetMasterTrackVisibility)     	GET_FUNC(Undo_BeginBlock2)
  GET_FUNC(GetArmedCommand)     	GET_FUNC(GetTrackSendUIVolPan)     	GET_FUNC(SetMediaItemInfo_Value)     	GET_FUNC(Undo_CanRedo2)
  GET_FUNC(GetAudioAccessorEndTime)     	GET_FUNC(GetTrackState)     	GET_FUNC(SetMediaItemLength)     	GET_FUNC(Undo_CanUndo2)
  GET_FUNC(GetAudioAccessorHash)     	GET_FUNC(GetTrackStateChunk)     	GET_FUNC(SetMediaItemPosition)     	GET_FUNC(Undo_DoRedo2)
  GET_FUNC(GetAudioAccessorSamples)     	GET_FUNC(GetTrackUIMute)     	GET_FUNC(SetMediaItemSelected)     	GET_FUNC(Undo_DoUndo2)
  GET_FUNC(GetAudioAccessorStartTime)     	GET_FUNC(GetTrackUIPan)     	GET_FUNC(SetMediaItemTake_Source)     	GET_FUNC(Undo_EndBlock)
  GET_FUNC(GetAudioDeviceInfo)     	GET_FUNC(GetTrackUIVolPan)     	GET_FUNC(SetMediaItemTakeInfo_Value)     	GET_FUNC(Undo_EndBlock2)
  GET_FUNC(GetConfigWantsDock)     	GET_FUNC(GetUnderrunTime)     	GET_FUNC(SetMediaTrackInfo_Value)     	GET_FUNC(Undo_OnStateChange)
  GET_FUNC(GetCurrentProjectInLoadSave)     	GET_FUNC(GetUserFileNameForRead)     	GET_FUNC(SetMIDIEditorGrid)     	GET_FUNC(Undo_OnStateChange2)
  GET_FUNC(GetCursorContext)     	GET_FUNC(GetUserInputs)     	GET_FUNC(SetMixerScroll)     	GET_FUNC(Undo_OnStateChange_Item)
  GET_FUNC(GetCursorContext2)     	GET_FUNC(GoToMarker)     	GET_FUNC(SetMouseModifier)     	GET_FUNC(Undo_OnStateChangeEx)
  GET_FUNC(GetCursorPosition)     	GET_FUNC(GoToRegion)     	GET_FUNC(SetOnlyTrackSelected)     	GET_FUNC(Undo_OnStateChangeEx2)
  GET_FUNC(GetCursorPositionEx)     	GET_FUNC(GR_SelectColor)     	GET_FUNC(SetProjectGrid)     	GET_FUNC(UpdateArrange)
  GET_FUNC(GetDisplayedMediaItemColor)     	GET_FUNC(GSC_mainwnd)     	GET_FUNC(SetProjectMarker)     	GET_FUNC(UpdateItemInProject)
  GET_FUNC(GetDisplayedMediaItemColor2)     	GET_FUNC(guidToString)     	GET_FUNC(SetProjectMarker2)     	GET_FUNC(UpdateTimeline)
  GET_FUNC(GetEnvelopeInfo_Value)     	GET_FUNC(HasExtState)     	GET_FUNC(SetProjectMarker3)     	GET_FUNC(ValidatePtr)
  GET_FUNC(GetEnvelopeName)     	GET_FUNC(HasTrackMIDIPrograms)     	GET_FUNC(SetProjectMarker4)     	GET_FUNC(ValidatePtr2)
  GET_FUNC(GetEnvelopePoint)     	GET_FUNC(HasTrackMIDIProgramsEx)     	GET_FUNC(SetProjectMarkerByIndex)     	GET_FUNC(ViewPrefs)
  GET_FUNC(GetEnvelopePointByTime)     	GET_FUNC(Help_Set)     	GET_FUNC(SetProjectMarkerByIndex2)
  GET_FUNC(GetEnvelopePointByTimeEx)     	GET_FUNC(image_resolve_fn)     	GET_FUNC(SetProjExtState)

//CreatePopupMenu();

//  .if (!rec->Register("hookcustommenu", (void*)swsMenuHook)) {
//     abort();
//  }
  if (!rec->Register("hookcommand2", (void*)hookCommandProc2)) {
      MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
         "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL);
      return false;
  }

  if (!rec->Register("hookcommand", (void*)hookCommandProc)) {
      MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
         "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL);
      return false;
  }
  if (!rec->Register("timer", (void*)timer_function)) {
      MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
         "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL);
      return false;
  }
  static project_config_extension_t pce = {
      ProcessExtensionLine,
      SaveExtensionConfig,
      0, 0
  };
  if (!rec->Register("projectconfig", &pce)) {
      MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
         "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL);
      return false;
  }

  char* name = (char*)malloc(200);
//  memcpy(name, "MyCommandName", strlen("MyCommandName")+1);
//  command = rec->Register("command_id",name);
//  if(!command) abort();
  // initialization code here

  custom_action_register_t action =
  {
    0, // 0/100=main/main alt, 32063=media explorer, 32060=midi editor, 32061=midi event list editor, 32062=midi inline editor, etc
    "MyCommandName", // must be unique across all sections for actions, NULL for reascripts (automatically generated)
    "My Command Name", // name as it is displayed in the action list, or full path to a reascript file
    NULL // reserved for future use
  };
  command = rec->Register("custom_action",&action);
    if(!command) {
        MessageBoxA(0, "please tell the developer at kitttoran@gmail.com", \
           "can't load microtonal midi editor", MB_OK | MB_SYSTEMMODAL);
        return false;
    }

      memcpy(name, "Transport: Play", strlen("Transport: Play")+1);
  int play =  rec->Register("command_id_lookup", name);


  char msg[100];
  snprintf(msg, 100, "Hello My Thing! %d play %d\n", command, play);
  ShowConsoleMsg(msg);
//  pianorollgui();
  return 1;
}
