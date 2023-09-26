#include "actionsReaper.h"
#include "misc.h"
#include "math.h"
#include "editorinstance.h"
//#include "midiprot.h"
#include "reaper_plugin_functions.h"
#include "melody.h"
#include "actions.h"

#include "stb_ds.h"
#include <algorithm>
#include <SDL_thread.h>
thread_local bool reaperMainThread;
extern "C" {
extern bool timeToLeave;
}
void play() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&play);
        return;
    }
    OnPlayButton();
}
void togglePause() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&togglePause);
        return;
    }
    OnPauseButton();
}
void stop() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&stop);
        return;
    }
    OnStopButton();
}
void toggleRepeat() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&toggleRepeat);
        return;
    }
    GetSetRepeat(2);
}


//void message(const char* format, ...);
//extern double __declspec(selectany) state.itemStart;
//extern double __declspec(selectany) state.itemStart;



static reapermidimessage3bytes d[3002]; //FIXME make a normal number
static void setTakeMidiData() {

//    d
//    int offset = MIDI_get
//    MIDI_SetAllEvts(state.take, (const char*)&notesOff, sizeof(notesOff));
//    FOR_NOTES(anote, piece) {
//        insertNoteImpl(*anote);
//    }
    ASSERT(arrlen(piece) < 1000, "there can be 1000 notes maximum");
    for(int i = 0; i < arrlen(piece); i++) {
        RealNote note = piece[i];
        int startppqpos = (int)MIDI_GetPPQPosFromProjTime(state.take,
                                                                                     note.note.start+state.itemStart);
        int endppqpos = (int)MIDI_GetPPQPosFromProjTime(state.take,
                                                                                     note.note.start+note.note.length+state.itemStart);

        MidiPitch mp = getMidiPitch(note.note.freq, currentItemConfig->pitchRange);
        d[3*i] = {
            startppqpos,
            (char)(note.selected | (note.note.muted << 1)),
            3,
            {
                (u8)(note_on | note.midiChannel),
                (u8)mp.key,
                (u8)note.note.velocity}
        };
        d[3*i+1] = {
            endppqpos,
            (char)(note.selected | (note.note.muted << 1)),
            3, {(u8)(note_off | note.midiChannel), (u8)mp.key, (u8)note.note.velocity}
        };
        d[3*i+2] = {
            startppqpos, (char)(note.selected | (note.note.muted << 1)), 3,
            {(u8)(pitch_wheel | note.midiChannel), (u8)(mp.wheel&0b1111111), (u8)(mp.wheel>>7)}
        };
        QUICK_ASSERT(startppqpos < endppqpos);
    }
    int  takeendqpos = MIDI_GetPPQPosFromProjTime(state.take, state.itemStart+pieceLength);
                                                                  //               pieceLength);

    d[3*arrlen(piece)] = {
            takeendqpos, 0, 3,
            {(u8)(control_change | 0), (u8)(123), (u8)(0)}
        };
    d[3*arrlen(piece)+1] = {
            takeendqpos, 0, 3,
            {(u8)(all_notes_off | 0), (u8)(123), (u8)(0)}
        };
    std::sort(d, d+arrlen(piece)*3, [](const reapermidimessage3bytes& a, const reapermidimessage3bytes& b) {
        if(a.offset != b.offset) return a.offset < b.offset;
        return a.msg[0] < b.msg[0]; // we want note_off messages
        // to be earlier than note_on
    });
    int lastOffset = 0;
    for(int i = 0; i <= arrlen(piece)*3; i++) {
        int r = d[i].offset;
        d[i].offset -= lastOffset;
        lastOffset = r;

        QUICK_ASSERT(d[i].offset >= 0);
        QUICK_ASSERT(d[i].offset < 2000000);
    }

    MIDI_SetAllEvts(state.take, (const char*)&d, sizeof(reapermidimessage3bytes)*(arrlen(piece)*3+1));
}

void reaperSetPosition(double d) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperSetPosition, d);
        return;
    }
    SetEditCurPos(d+state.itemStart, false, true);
}
void reaperOnCommand(u32 command) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperOnCommand, command);
        return;
    }
    Main_OnCommand(command, 0);
}

void reaperCommitChanges(char* undoMessage) {
    if(!reaperMainThread) {
//        ASSERT((**selectedNotes).note.freq > 1, "");
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperCommitChanges, undoMessage/*, time, freq*/);
        return;
    }
    MediaItem* item = GetMediaItemTake_Item(state.take);
    Undo_BeginBlock2(GetItemProjectContext(item));
    setTakeMidiData();
//    MIDI_DisableSort(state.take);
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
//        bool res = MIDI_DeleteNote(state.take, anote->reaperNumber);
//        if(!res) ShowConsoleMsg("note deletion (while moving) failed");
//    }
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
//        insertNoteImpl(*anote);
//    }
//    MIDI_Sort(state.take);
Undo_OnStateChange_Item(GetItemProjectContext(item), "undoMessage", item);
    Undo_EndBlock2(GetItemProjectContext(item), "undoMessage", 4);
//    reload();

}
extern "C" void message(const char* format, ...) {
    va_list arg_ptr;

    va_start(arg_ptr, format);

    char msg[100];
    vsnprintf(msg, 100,format, arg_ptr);
    if(!reaperMainThread) {
        if(!timeToLeave) {
            actionChannel.name = __func__;
//            actionChannel.runInMainThread(ShowConsoleMsg, msg);
        } else {
            DEBUG_FPRINTF("%s", msg);
        }
        return;
    }
//    ShowConsoleMsg(msg);
    va_end(arg_ptr);
}
static int playedKey;
static int playedVelocity;
//static int playedVelocity;
extern "C"  SDL_mutex* mutex_; // I use SDL_mutex because MSVC doesn't provide thread.h
extern "C"  SDL_cond* condVar;
template <typename F, typename... Args>
void ActionChannel::runInMainThread(F f, Args... args) {
//    std::unique_lock lk(actionChannel.mutex);
    auto closure = [=] () { f(args...); };
    actionChannel.action = closure;
    actionChannel.pending = true;
//    actionChannel.cv.wait(lk, [](){
//                         return actionChannel.pending
//                                 == false;});
    SDL_CondWait(condVar, mutex_);
}
UINT32 pack3(u8* a) {
    return a[2]<<16|a[1]<<8|a[0];
}
//static HMIDIOUT fweefwefwe;
void startPlayingNote(double freq, int vel) {
//    int numMidiDevs = midiOutGetNumDevs();
//    fprintf(stderr, "wefgwefwefwefw %d\n", numMidiDevs);
//    MIDIOUTCAPSA dfgsdfsd;
//    for(int i = 0; i <= numMidiDevs; i++) {
//        midiOutGetDevCapsA(i, &dfgsdfsd, sizeof(dfgsdfsd));
//        fprintf(stderr, "%d: %s\n", i, dfgsdfsd.szPname);
//    }
//    STATIC(bool, init, (midiOutOpen(&fweefwefwe, 1, NULL, 0, CALLBACK_NULL), true
//                        ));



    double pitchInterval = currentItemConfig->pitchRange;
    MidiPitch mp = getMidiPitch(freq, pitchInterval);
    ASSERT(mp.key < 128, "this note is too high");
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&startPlayingNote, freq, vel);
        return;
    }
    if(playedKey >= 0 && playedKey != mp.key || playedVelocity != vel) {
        stopPlayingNoteIfPlaying();
    }


    int channel = 1;
    if(playedKey < 0) {
        u8 noteOnEvent[] = {note_on | channel, mp.key, vel};
        StuffMIDIMessage(0, noteOnEvent[0], noteOnEvent[1], noteOnEvent[2]);
    }
    u8 pitchEvent[] = {pitch_wheel | channel, mp.wheel&0b1111111, mp.wheel>>7};
    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);
    playedKey = mp.key;
//    midiOutShortMsg(fweefwefwe,pack3((u8*)(i8*)noteOnEvent));
//    midiOutShortMsg(fweefwefwe,pack3(pitchEvent));
//    u32 efgerg = pack3(noteOnEvent);
//    u32 efdgerg = pack3(pitchEvent);
//    fprintf(stderr, "%d \n", mp.key);
}
void stopPlayingNoteIfPlaying() {
    if(playedKey < 0) return;
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&stopPlayingNoteIfPlaying);
        return;
    }
    ASSERT(playedKey >=0 && playedKey < 128, "hi %d", playedKey);
    int channel = 1;
    u8 noteOffEvent[] = {note_off | channel, playedKey, 100};
    StuffMIDIMessage(0, noteOffEvent[0], noteOffEvent[1], noteOffEvent[2]);
    char pitchEvent[] = {pitch_wheel | channel, 0, 0b01000000};
//    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);
    playedKey = -1;
//    midiOutShortMsg(fweefwefwe,pack3((u8*)noteOffEvent));

}
void loadTake();
void undo() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&undo);
        return;
    }
    Undo_DoUndo2(state.project);
    loadTake();
}
void redo() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&redo);
        return;
    }
    Undo_DoRedo2(state.project);
    loadTake();
}
void save() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&save);
        return;
    }
    Main_SaveProject(GetItemProjectContext(GetMediaItemTake_Item(state.take)), false);
}
void reload() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reload);
        return;
    }
    loadTake();
}

void MessageBoxInfo(char* title, char* message)
{
    MessageBoxA(
      NULL,
      message,
      title,
      MB_OK
    );
}
ActionChannel actionChannel;
