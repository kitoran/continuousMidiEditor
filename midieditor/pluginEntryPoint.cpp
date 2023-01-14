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
#include "gui.h"
#include "melody.h"
#include "actions.h"
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
void timer_function() {
    std::unique_lock lk(actionChannel.mutex);
    if(actionChannel.action==actionChannel.insertNote) {
        reaperInsert(actionChannel.note);
    }
    if(actionChannel.action==actionChannel.deleteNote) {
        reaperDelete(actionChannel.intgr);
    }
    if(actionChannel.action==actionChannel.consoleMessage) {
        ShowConsoleMsg(actionChannel.string);
    }
    actionChannel.action = actionChannel.none;
    lk.unlock();
    actionChannel.cv.notify_one();
//    ShowConsoleMsg("unlockedandnotified");
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
std::mutex mutex_;
bool data_ready = false;
bool timeToLeave = false;
std::condition_variable condVar;
//std::atomic<MediaItem*> item = NULL;
void sdlThread() {

    SetThreadName ((DWORD)-1, "contMidiEditorThread");
    reaperMainThread = false;

    while(true) {
        std::unique_lock lk(mutex_);
        condVar.wait(lk, [](){return data_ready;});
        if(timeToLeave) {
            lk.unlock();
            break;
        }
        pianorollgui();
        data_ready = false;
    }
}
void closeSDLWindowAndLockMutex() {
    if(!mutex_.try_lock()) {
        SDL_WindowEvent windowevent = {
            SDL_WINDOWEVENT, SDL_GetTicks(), SDL_GetWindowID(rootWindow),
            SDL_WINDOWEVENT_CLOSE};
        SDL_Event event; event.window = windowevent;
        SDL_PushEvent(&event);
        while(!mutex_.try_lock()) {
            timer_function();
        }
    }
}
std::thread th;
bool hookCommandProc(int iCmd, int flag)
{
    char msg[100];
    snprintf(msg, 100, "hookCommandProc! %d\n", iCmd);
    ShowConsoleMsg(msg);
    if(iCmd == command) {
        STATIC(bool, init, (th=std::thread(sdlThread), true)) //TODO: check if this
                // use-case of STATIC warrants its own macro
        closeSDLWindowAndLockMutex();
//            std::lock_guard lg(mutex_);
        MediaItem* item = GetSelectedMediaItem(NULL, 0);
        take = GetActiveTake(item);
        bool res = true; //MIDI_GetAllEvts(take, events, &size);
        double ppqpos;
        int vel;
        int i = 0;
        itemStart = GetMediaItemInfo_Value(item, "D_POSITION");
        double channelPitches[16];// don't care about channel 0 because in MPE it's different
        // but allocate it anyway just 'cause
        double channelNoteStarts[16];
        for(int i = 0; i < 16; i++) {
            channelPitches[i] = 0;
            channelNoteStarts[i]=-1;
        }
        while(true) {
            //TODO: use getAllEvents? i don't want to because there's no way to get needed buffer size ahead
            // of time. Maybe if midi track is more than 4 kb i can just tell the user that tey are too
            // musical for this extension // there's MIDI_CountEvts
            bool mutedUnusedForNow;
#define MAX_MIDI_EVENT_LENGTH 3
#define PADINCASEIMISSEDSOMELONGEREVENTS 100
            u8 msg[MAX_MIDI_EVENT_LENGTH+PADINCASEIMISSEDSOMELONGEREVENTS];
            int size = 3;
            res = MIDI_GetEvt(take, i++, 0, &mutedUnusedForNow, &ppqpos, (char*)(&msg[0]), &size);
            if(!res) break;
            ASSERT(size <= 3, "got midi event longer than 3 bytes, dying\n");
            int channel = msg[0] & MIDI_CHANNEL_MASK;
            if((msg[0] & MIDI_COMMAND_MASK) == pitchWheelEvent) {
               i16 pitchWheel =
                       (msg[2] << 7) |
                       (msg[1]&MIDI_7HB_MASK);
               // TODO: setting of whether pitch bend is 2 semitones or 48 semitones
               double differenceInTones =
                       double(pitchWheel-0x2000)/0x2000;
               double differenceInHz = pow(2, differenceInTones/6);
               channelPitches[channel] = differenceInHz;
            }
            //TODO: store velocity value too
//          res = MIDI_GetNote(take, i++, 0, 0, , &endppqpos, 0, &pitch, &vel);

            // TODO: indicate when we changed the take
    // we think that all the simultaneous notes are on different channels
    // so to get note's key we only need to read it from onteOff event
            double pos = MIDI_GetProjTimeFromPPQPos(take, ppqpos);
            if((msg[0] & MIDI_COMMAND_MASK) == noteOn) {
                channelNoteStarts[channel] = pos;
            }
            if((msg[0] & MIDI_COMMAND_MASK) == noteOff) {
                int key = msg[1];
                double freq = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
                freq += channelPitches[channel];
                message("st %lf end %lf pitch %d vel %d\n"
                        "start %lf  freq %lf", pos, channelNoteStarts[channel] , key, vel
                        , start, freq);
                insertNote({.freq= freq,
                           .start = channelNoteStarts[channel]  - itemStart,
                           .length = pos-channelNoteStarts[channel] }, false);
            }
        }
        double bpiOut;
//                double* bpmOut, double* bpiOut
        GetProjectTimeSignature2(NULL, &bpm, &bpiOut);
        data_ready = true;
        mutex_.unlock();
        condVar.notify_one();
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

bool hookCommandProc2(KbdSectionInfo* sec, int cmdId, int val, int valhw, int relmode, HWND hwnd)
{
    return hookCommandProc(cmdId, 0);
//    char msg[100];
//    snprintf(msg, 100, "hookCommandProc2! %d\n", cmdId);
//    ShowConsoleMsg(msg);
//    return false;
}


extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(
  REAPER_PLUGIN_HINSTANCE instance, reaper_plugin_info_t *rec)
{
    reaperMainThread = true;
  if(!rec) {
    // cleanup code here
    closeSDLWindowAndLockMutex();
    timeToLeave = true;
    data_ready = true;
    mutex_.unlock();
    condVar.notify_one();
    th.join();
    return 0;
  }

  if(rec->caller_version != REAPER_PLUGIN_VERSION)
    return 0;

  // see also https://gist.github.com/cfillion/350356a62c61a1a2640024f8dc6c6770
#define GET_FUNC(a) a = (decltype(a))rec->GetFunc(#a); if(!a) abort();
  GET_FUNC(AddMediaItemToTrack)     	GET_FUNC(GetEnvelopePointEx)     	GET_FUNC(InsertAutomationItem)     	GET_FUNC(SetRegionRenderMatrix)
  GET_FUNC(AddProjectMarker)     	GET_FUNC(GetEnvelopeScalingMode)     	GET_FUNC(InsertEnvelopePoint)     	GET_FUNC(SetTakeMarker)
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
  GET_FUNC(CSurf_FlushUndo)     	GET_FUNC(GetMediaSourceType)     	GET_FUNC(MIDI_EnumSelTextSysexEvts)     	GET_FUNC(TakeFX_GetNumParams)
  GET_FUNC(CSurf_GetTouchState)     	GET_FUNC(GetMediaTrackInfo_Value)     	GET_FUNC(MIDI_GetAllEvts)     	GET_FUNC(TakeFX_GetOffline)
  GET_FUNC(CSurf_GoEnd)     	GET_FUNC(GetMIDIInputName)     	GET_FUNC(MIDI_GetCC)     	GET_FUNC(TakeFX_GetOpen)
  GET_FUNC(CSurf_GoStart)     	GET_FUNC(GetMIDIOutputName)     	GET_FUNC(MIDI_GetCCShape)     	GET_FUNC(TakeFX_GetParam)
  GET_FUNC(CSurf_NumTracks)     	GET_FUNC(GetMixerScroll)     	GET_FUNC(MIDI_GetEvt)     	GET_FUNC(TakeFX_GetParameterStepSizes)
  GET_FUNC(CSurf_OnArrow)     	GET_FUNC(GetMouseModifier)     	GET_FUNC(MIDI_GetGrid)     	GET_FUNC(TakeFX_GetParamEx)
  GET_FUNC(CSurf_OnFwd)     	GET_FUNC(GetMousePosition)     	GET_FUNC(MIDI_GetHash)     	GET_FUNC(TakeFX_GetParamFromIdent)
  GET_FUNC(CSurf_OnFXChange)     	GET_FUNC(GetNumAudioInputs)     	GET_FUNC(MIDI_GetNote)     	GET_FUNC(TakeFX_GetParamIdent)
  GET_FUNC(CSurf_OnInputMonitorChange)     	GET_FUNC(GetNumAudioOutputs)     	GET_FUNC(MIDI_GetPPQPos_EndOfMeasure)     	GET_FUNC(TakeFX_GetParamName)
  GET_FUNC(CSurf_OnInputMonitorChangeEx)     	GET_FUNC(GetNumMIDIInputs)     	GET_FUNC(MIDI_GetPPQPos_StartOfMeasure)     	GET_FUNC(TakeFX_GetParamNormalized)
  GET_FUNC(CSurf_OnMuteChange)     	GET_FUNC(GetNumMIDIOutputs)     	GET_FUNC(MIDI_GetPPQPosFromProjQN)     	GET_FUNC(TakeFX_GetPinMappings)
  GET_FUNC(CSurf_OnMuteChangeEx)     	GET_FUNC(GetNumTakeMarkers)     	GET_FUNC(MIDI_GetPPQPosFromProjTime)     	GET_FUNC(TakeFX_GetPreset)
  GET_FUNC(CSurf_OnPanChange)     	GET_FUNC(GetNumTracks)     	GET_FUNC(MIDI_GetProjQNFromPPQPos)     	GET_FUNC(TakeFX_GetPresetIndex)
  GET_FUNC(CSurf_OnPanChangeEx)     	GET_FUNC(GetOS)     	GET_FUNC(MIDI_GetProjTimeFromPPQPos)     	GET_FUNC(TakeFX_GetUserPresetFilename)
  GET_FUNC(CSurf_OnPause)     	GET_FUNC(GetOutputChannelName)     	GET_FUNC(MIDI_GetRecentInputEvent)     	GET_FUNC(TakeFX_NavigatePresets)
  GET_FUNC(CSurf_OnPlay)     	GET_FUNC(GetOutputLatency)     	GET_FUNC(MIDI_GetScale)     	GET_FUNC(TakeFX_SetEnabled)
  GET_FUNC(CSurf_OnPlayRateChange)     	GET_FUNC(GetParentTrack)     	GET_FUNC(MIDI_GetTextSysexEvt)     	GET_FUNC(TakeFX_SetNamedConfigParm)
  GET_FUNC(CSurf_OnRecArmChange)     	GET_FUNC(GetPeakFileName)     	GET_FUNC(MIDI_GetTrackHash)     	GET_FUNC(TakeFX_SetOffline)
  GET_FUNC(CSurf_OnRecArmChangeEx)     	GET_FUNC(GetPeakFileNameEx)     	GET_FUNC(midi_init)     	GET_FUNC(TakeFX_SetOpen)
  GET_FUNC(CSurf_OnRecord)     	GET_FUNC(GetPeakFileNameEx2)     	GET_FUNC(MIDI_InsertCC)     	GET_FUNC(TakeFX_SetParam)
  GET_FUNC(CSurf_OnRecvPanChange)     	GET_FUNC(GetPlayPosition)     	GET_FUNC(MIDI_InsertEvt)     	GET_FUNC(TakeFX_SetParamNormalized)
  GET_FUNC(CSurf_OnRecvVolumeChange)     	GET_FUNC(GetPlayPosition2)     	GET_FUNC(MIDI_InsertNote)     	GET_FUNC(TakeFX_SetPinMappings)
  GET_FUNC(CSurf_OnRew)     	GET_FUNC(GetPlayPosition2Ex)     	GET_FUNC(MIDI_InsertTextSysexEvt)     	GET_FUNC(TakeFX_SetPreset)
  GET_FUNC(CSurf_OnRewFwd)     	GET_FUNC(GetPlayPositionEx)     	GET_FUNC(midi_reinit)     	GET_FUNC(TakeFX_SetPresetByIndex)
  GET_FUNC(CSurf_OnScroll)     	GET_FUNC(GetPlayState)     	GET_FUNC(MIDI_SelectAll)     	GET_FUNC(TakeFX_Show)
  GET_FUNC(CSurf_OnSelectedChange)     	GET_FUNC(GetPlayStateEx)     	GET_FUNC(MIDI_SetAllEvts)     	GET_FUNC(TakeIsMIDI)
  GET_FUNC(CSurf_OnSendPanChange)     	GET_FUNC(GetProjectLength)     	GET_FUNC(MIDI_SetCC)     	GET_FUNC(ThemeLayout_GetLayout)
  GET_FUNC(CSurf_OnSendVolumeChange)     	GET_FUNC(GetProjectName)     	GET_FUNC(MIDI_SetCCShape)     	GET_FUNC(ThemeLayout_GetParameter)
  GET_FUNC(CSurf_OnSoloChange)     	GET_FUNC(GetProjectPath)     	GET_FUNC(MIDI_SetEvt)     	GET_FUNC(ThemeLayout_RefreshAll)
  GET_FUNC(CSurf_OnSoloChangeEx)     	GET_FUNC(GetProjectPathEx)     	GET_FUNC(MIDI_SetItemExtents)     	GET_FUNC(ThemeLayout_SetLayout)
  GET_FUNC(CSurf_OnStop)     	GET_FUNC(GetProjectStateChangeCount)     	GET_FUNC(MIDI_SetNote)     	GET_FUNC(ThemeLayout_SetParameter)
  GET_FUNC(CSurf_OnTempoChange)     	GET_FUNC(GetProjectTimeOffset)     	GET_FUNC(MIDI_SetTextSysexEvt)     	GET_FUNC(time_precise)
  GET_FUNC(CSurf_OnTrackSelection)     	GET_FUNC(GetProjectTimeSignature)     	GET_FUNC(MIDI_Sort)     	GET_FUNC(TimeMap2_beatsToTime)
  GET_FUNC(CSurf_OnVolumeChange)     	GET_FUNC(GetProjectTimeSignature2)     	GET_FUNC(MIDIEditor_EnumTakes)     	GET_FUNC(TimeMap2_GetDividedBpmAtTime)
  GET_FUNC(CSurf_OnVolumeChangeEx)     	GET_FUNC(GetProjExtState)     	GET_FUNC(MIDIEditor_GetActive)     	GET_FUNC(TimeMap2_GetNextChangeTime)
  GET_FUNC(CSurf_OnWidthChange)     	GET_FUNC(GetResourcePath)     	GET_FUNC(MIDIEditor_GetMode)     	GET_FUNC(TimeMap2_QNToTime)
  GET_FUNC(CSurf_OnWidthChangeEx)     	GET_FUNC(GetSelectedEnvelope)     	GET_FUNC(MIDIEditor_GetSetting_int)     	GET_FUNC(TimeMap2_timeToBeats)
  GET_FUNC(CSurf_OnZoom)     	GET_FUNC(GetSelectedMediaItem)     	GET_FUNC(MIDIEditor_GetSetting_str)     	GET_FUNC(TimeMap2_timeToQN)
  GET_FUNC(CSurf_ResetAllCachedVolPanStates)     	GET_FUNC(GetSelectedTrack)     	GET_FUNC(MIDIEditor_GetTake)     	GET_FUNC(TimeMap_curFrameRate)
  GET_FUNC(CSurf_ScrubAmt)     	GET_FUNC(GetSelectedTrack2)     	GET_FUNC(MIDIEditor_LastFocused_OnCommand)     	GET_FUNC(TimeMap_GetDividedBpmAtTime)
  GET_FUNC(CSurf_SetAutoMode)     	GET_FUNC(GetSelectedTrackEnvelope)     	GET_FUNC(MIDIEditor_OnCommand)     	GET_FUNC(TimeMap_GetMeasureInfo)
  GET_FUNC(CSurf_SetPlayState)     	GET_FUNC(GetSet_ArrangeView2)     	GET_FUNC(MIDIEditor_SetSetting_int)     	GET_FUNC(TimeMap_GetMetronomePattern)
  GET_FUNC(CSurf_SetRepeatState)     	GET_FUNC(GetSet_LoopTimeRange)     	GET_FUNC(mkpanstr)     	GET_FUNC(TimeMap_GetTimeSigAtTime)
  GET_FUNC(CSurf_SetSurfaceMute)     	GET_FUNC(GetSet_LoopTimeRange2)     	GET_FUNC(mkvolpanstr)     	GET_FUNC(TimeMap_QNToMeasures)
  GET_FUNC(CSurf_SetSurfacePan)     	GET_FUNC(GetSetAutomationItemInfo)     	GET_FUNC(mkvolstr)     	GET_FUNC(TimeMap_QNToTime)
  GET_FUNC(CSurf_SetSurfaceRecArm)     	GET_FUNC(GetSetAutomationItemInfo_String)     	GET_FUNC(MoveEditCursor)     	GET_FUNC(TimeMap_QNToTime_abs)
  GET_FUNC(CSurf_SetSurfaceSelected)     	GET_FUNC(GetSetEnvelopeInfo_String)     	GET_FUNC(MoveMediaItemToTrack)     	GET_FUNC(TimeMap_timeToQN)
  GET_FUNC(CSurf_SetSurfaceSolo)     	GET_FUNC(GetSetEnvelopeState)     	GET_FUNC(MuteAllTracks)     	GET_FUNC(TimeMap_timeToQN_abs)
  GET_FUNC(CSurf_SetSurfaceVolume)     	GET_FUNC(GetSetEnvelopeState2)     	GET_FUNC(my_getViewport)     	GET_FUNC(ToggleTrackSendUIMute)
  GET_FUNC(CSurf_SetTrackListChange)     	GET_FUNC(GetSetItemState)     	GET_FUNC(NamedCommandLookup)     	GET_FUNC(Track_GetPeakHoldDB)
  GET_FUNC(CSurf_TrackFromID)     	GET_FUNC(GetSetItemState2)     	GET_FUNC(OnPauseButton)     	GET_FUNC(Track_GetPeakInfo)
  GET_FUNC(CSurf_TrackToID)     	GET_FUNC(GetSetMediaItemInfo_String)     	GET_FUNC(OnPauseButtonEx)     	GET_FUNC(TrackCtl_SetToolTip)
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
  GET_FUNC(Envelope_Evaluate)     	GET_FUNC(GetTrackEnvelopeByChunkName)     	GET_FUNC(ReverseNamedCommandLookup)     	GET_FUNC(TrackFX_GetUserPresetFilename)
  GET_FUNC(Envelope_FormatValue)     	GET_FUNC(GetTrackEnvelopeByName)     	GET_FUNC(ScaleFromEnvelopeMode)     	GET_FUNC(TrackFX_NavigatePresets)
  GET_FUNC(Envelope_GetParentTake)     	GET_FUNC(GetTrackFromPoint)     	GET_FUNC(ScaleToEnvelopeMode)     	GET_FUNC(TrackFX_SetEnabled)
  GET_FUNC(Envelope_GetParentTrack)     	GET_FUNC(GetTrackGUID)     	GET_FUNC(SelectAllMediaItems)     	GET_FUNC(TrackFX_SetEQBandEnabled)
  GET_FUNC(Envelope_SortPoints)     	GET_FUNC(GetTrackMediaItem)     	GET_FUNC(SelectProjectInstance)     	GET_FUNC(TrackFX_SetEQParam)
  GET_FUNC(Envelope_SortPointsEx)     	GET_FUNC(GetTrackMIDILyrics)     	GET_FUNC(SetActiveTake)     	GET_FUNC(TrackFX_SetNamedConfigParm)
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
  if(!ShowConsoleMsg) {
    fprintf(stderr, "[reaper_barebone] Unable to import ShowConsoleMsg\n");
    return 0;
  }
//  .if (!rec->Register("hookcustommenu", (void*)swsMenuHook)) {
//     abort();
//  }
  if (!rec->Register("hookcommand2", (void*)hookCommandProc2))
      abort();

  if (!rec->Register("hookcommand", (void*)hookCommandProc))
      abort();
  if (!rec->Register("timer", (void*)timer_function))
      abort();

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
    if(!command) abort();

      memcpy(name, "Transport: Play", strlen("Transport: Play")+1);
  int play =  rec->Register("command_id_lookup", name);


  char msg[100];
  snprintf(msg, 100, "Hello My Thing! %d play %d\n", command, play);
  ShowConsoleMsg(msg);
//  pianorollgui();
  return 1;
}
