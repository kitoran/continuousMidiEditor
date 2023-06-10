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

void stop() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&stop);
        return;
    }
    OnStopButton();
}


//void message(const char* format, ...);
//extern double __declspec(selectany) itemStart;
//extern double __declspec(selectany) itemStart;

// TODO: reassign channels if haven't found a free channel but it's possible to find or just let user readdign them
enum MidiEventType: u8 {
    note_off = 0b1000 << 4,
    note_on = 0b1001 << 4,
    pitch_wheel = 0b1110 << 4,
    control_change = 0b1011 << 4,
};
enum MidiCCEvent: u8 {
    all_notes_off = 123,
};
const double LOG_SEMITONE = log(2)/12;
const double BIAS =  log(440)/LOG_SEMITONE-69;
struct MidiPitch {
    int key;
    int wheel;
};

MidiPitch  getMidiPitch(double freq, double pitchRangeInterval) {

    int key = (int)round(log(freq)/LOG_SEMITONE-BIAS);
    CLAMP(key, 0, 127);
    double freqOfTheKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double difference = freq/freqOfTheKey;

    double differenceInSemitones = 12*log(difference)/log(2);
    double differenceInPitchRamgeIntervals = differenceInSemitones/pitchRangeInterval;
    int pitchWheel = (int)round(differenceInPitchRamgeIntervals*0x2000)+0x2000;
    ASSERT(pitchWheel < 0x4000, "pitch out of range");
    ASSERT(pitchWheel >= 0, "pitch out of range");
    return MidiPitch{key, pitchWheel};
}

void insertNoteImpl(const RealNote& note)
{
    double startppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.note.start+itemStart);
    double endppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.note.start+note.note.length+itemStart);

    MediaItem* item = GetMediaItemTake_Item(take);
//    Undo_BeginBlock2(GetItemProjectContext(item));
    MidiPitch mp = getMidiPitch(note.note.freq, currentItemConfig->value.pitchRange);
    char pitchEvent[] = {pitch_wheel | note.midiChannel, mp.wheel&0b1111111, mp.wheel>>7};
    MIDI_InsertEvt(take, false, false, startppqpos-1, pitchEvent, sizeof(pitchEvent));
    bool res = MIDI_InsertNote(take, note.selected, note.note.muted,
                    startppqpos, endppqpos,
                    note.midiChannel, mp.key,
                    note.note.velocity,
                    NULL);
    ASSERT(res, "fail to insert note???");
    fprintf(stderr, "\ninserting note at %lf %lf\n", note.note.start,
            startppqpos);
    double newstrtt;
    int pitchgot;
    MIDI_GetNote(take, 0, NULL, NULL, &newstrtt, NULL, NULL, &pitchgot, NULL);
    double newtime = MIDI_GetProjTimeFromPPQPos(take, newstrtt);
    fprintf(stderr, "got note at %lf %lf\n", newtime,
            newstrtt);

    pieceLength = /*itemStart + */GetMediaItemInfo_Value(item, "D_LENGTH");
//    message("inserting note %lf - %lf\n", startppqpos, endppqpos);
    if(!res) ShowConsoleMsg("note insertion failed");
}

void commit() {
#pragma pack(push)
#pragma pack( 1)
    struct reapermidimessage {
        int offset = 0;
        char flag = 0;
        int msglen = 3;
        unsigned char msg[3] = { control_change | 1, all_notes_off, 0};
    } notesOff;
    reapermidimessage d[3001];
#pragma pack(pop)
//    d
//    int offset = MIDI_get
//    MIDI_SetAllEvts(take, (const char*)&notesOff, sizeof(notesOff));
//    FOR_NOTES(anote, piece) {
//        insertNoteImpl(*anote);
//    }
    ASSERT(arrlen(piece) < 1000, "hi^)");
    for(int i = 0; i < arrlen(piece); i++) {
        RealNote note = piece[i];
        int  startppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                                                     note.note.start+itemStart);
        int endppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                                                     note.note.start+note.note.length+itemStart);

        MidiPitch mp = getMidiPitch(note.note.freq, currentItemConfig->value.pitchRange);
        d[3*i] = {
            startppqpos,
            (char)(note.selected | (note.note.muted < 1)),
            3,
            {
                (u8)(note_on | note.midiChannel),
                (u8)mp.key,
                (u8)127}
        };
        d[3*i+1] = {
            endppqpos,
            (char)(note.selected | (note.note.muted < 1)),
            3, {(u8)(note_off | note.midiChannel), (u8)mp.key, (u8)100}
        };
        d[3*i+2] = {
            startppqpos, (char)(note.selected | (note.note.muted < 1)), 3,
            {(u8)(pitch_wheel | note.midiChannel), (u8)(mp.wheel&0b1111111), (u8)(mp.wheel>>7)}
        };
    }
    int  takeendqpos = MIDI_GetPPQPosFromProjTime(take,
                                                                                 pieceLength);

    d[3*arrlen(piece)] = {
            takeendqpos, 0, 3,
            {(u8)(control_change | 0), (u8)(123), (u8)(0)}
        };
    std::sort(d, d+arrlen(piece)*3, [](const reapermidimessage& a, const reapermidimessage& b) {
        return a.offset < b.offset;
    });
    int lastOffset = 0;
    for(int i = 0; i < arrlen(piece)*3; i++) {
        int r = d[i].offset;
        d[i].offset -= lastOffset;
        lastOffset = r;
    }
    MIDI_SetAllEvts(take, (const char*)&d, sizeof(reapermidimessage)*arrlen(piece)*3);
}

void reaperInsert(RealNote note) {
    if(!reaperMainThread) {
        ASSERT(note.note.freq>1,"")
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperInsert, note);
        return;
    }

    MediaItem* item = GetMediaItemTake_Item(take);
//    Undo_BeginBlock2(GetItemProjectContext(item));
    commit();
//    Undo_EndBlock2(GetItemProjectContext(item), "Move notes", 4);


    //    Undo_OnStateChange_Item(GetItemProjectContext(item), "Insert Note", item);
//    insertNoteImpl(note);
//    reload();
}
void reaperSetPosition(double d) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperSetPosition, d);
        return;
    }
    //TODO: this is prob wrong
    SetEditCurPos(d+itemStart, false, true);
}
void reaperOnCommand(u32 command) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperOnCommand, command);
        return;
    }
    Main_OnCommand(command, 0);
}
void reaperDeleteSelected() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperDeleteSelected);
        return;
    }

    MediaItem* item = GetMediaItemTake_Item(take);
    Undo_BeginBlock2(GetItemProjectContext(item));
    commit();
//    MIDI_DisableSort(take);
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
//        bool res = MIDI_DeleteNote(take, anote-piece);
//        if(!res) ShowConsoleMsg("note deletion failed");
//        fprintf(stderr, "deleting note %d\n", (int)(anote-piece));
//    }
//    MIDI_Sort(take);
//    Undo_OnStateChange_Item(GetItemProjectContext(item), "Delete Notes", item);

    Undo_EndBlock2(GetItemProjectContext(item), "Delete notes", 4);
//    reload();
}
void reaperMoveNotes(double time, double freq) {
    //TODO: remove the arguments
    if(!reaperMainThread) {
//        ASSERT((**selectedNotes).note.freq > 1, "");
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperMoveNotes, time, freq);
        return;
    }
    MediaItem* item = GetMediaItemTake_Item(take);
    Undo_BeginBlock2(GetItemProjectContext(item));
    commit();
//    MIDI_DisableSort(take);
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
//        bool res = MIDI_DeleteNote(take, anote->reaperNumber);
//        if(!res) ShowConsoleMsg("note deletion (while moving) failed");
//    }
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
//        insertNoteImpl(*anote);
//    }
    MIDI_Sort(take);

    Undo_EndBlock2(GetItemProjectContext(item), "Move notes", 4);
//    reload();

}
void reaperCopyNotes() {
    if(!reaperMainThread) {
//        ASSERT((**selectedNotes).note.freq > 1, "");
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperCopyNotes);
        return;
    }
    MediaItem* item = GetMediaItemTake_Item(take);
    Undo_BeginBlock2(GetItemProjectContext(item));
    MIDI_DisableSort(take);

    commit();
//    for(RealNote* anote = piece + arrlen(piece) - 1; anote >= piece; anote--) {
//        if(!anote->selected) continue;
////        bool res = MIDI_DeleteNote(take, anote-piece);
////        if(!res) ShowConsoleMsg("note deletion (while moving) failed");
////        anote->freq+=freq;
////        anote->start+=time;
//        insertNoteImpl(*anote);
//    }
//    MIDI_Sort(take);

    Undo_EndBlock2(GetItemProjectContext(item), "Copy notes", 4);
//    reload();
}
//void doReaperAction(action theAction, ActionArgs* largs) {
//    std::unique_lock lk(actionMutex);
//    actionCV.wait(lk, [](){action == action_none;});
//    args = largs;
//    action = theAction;
//}

extern "C" void message(const char* format, ...) {
    va_list arg_ptr;

    va_start(arg_ptr, format);

    char msg[100];
    vsnprintf(msg, 100,format, arg_ptr);
    if(!reaperMainThread) {
        if(!timeToLeave) {
            actionChannel.name = __func__;
            actionChannel.runInMainThread(ShowConsoleMsg, msg);
        } else {
            fprintf(stderr, "%s", msg);
        }
        return;
    }
    ShowConsoleMsg(msg);
    va_end(arg_ptr);
}
static int playedKey;
//static int playedVelocity;
template <typename F, typename... Args>
void ActionChannel::runInMainThread(F f, Args... args) {
    std::unique_lock lk(actionChannel.mutex);
    auto closure = [=] () { f(args...); };
    actionChannel.action = closure;
    actionChannel.pending = true;
    actionChannel.cv.wait(lk, [](){
                         return actionChannel.pending
                                 == false;});

}
UINT32 pack3(u8* a) {
    return a[2]<<16|a[1]<<8|a[0];
}
static HMIDIOUT fweefwefwe;
void startPlayingNote(double freq) {
    int numMidiDevs = midiOutGetNumDevs();
    fprintf(stderr, "wefgwefwefwefw %d\n", numMidiDevs);
    MIDIOUTCAPSA dfgsdfsd;
    for(int i = 0; i <= numMidiDevs; i++) {
        midiOutGetDevCapsA(i, &dfgsdfsd, sizeof(dfgsdfsd));
        fprintf(stderr, "%d: %s\n", i, dfgsdfsd.szPname);
    }
    //todo: at leas let user check which port is activated
    STATIC(bool, init, (midiOutOpen(&fweefwefwe, 1, NULL, 0, CALLBACK_NULL), true
                        ));



    double pitchInterval = currentItemConfig->value.pitchRange;
    MidiPitch mp = getMidiPitch(freq, pitchInterval);
    ASSERT(mp.key < 128, "this note is too high");
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&startPlayingNote, freq);
        return;
    }
    int channel = 1;
    u8 noteOnEvent[] = {note_on | channel, mp.key, 127};
    StuffMIDIMessage(0, noteOnEvent[0], noteOnEvent[1], noteOnEvent[2]);
    u8 pitchEvent[] = {pitch_wheel | channel, mp.wheel&0b1111111, mp.wheel>>7};
    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);
    playedKey = mp.key;
//    midiOutShortMsg(fweefwefwe,pack3((u8*)(i8*)noteOnEvent));
//    midiOutShortMsg(fweefwefwe,pack3(pitchEvent));
    u32 efgerg = pack3(noteOnEvent);
    u32 efdgerg = pack3(pitchEvent);
    fprintf(stderr, "%d \n", mp.key);
}
void stopPlayingNote() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&stopPlayingNote);
        return;
    }
    ASSERT(playedKey >=0 && playedKey < 128, "hi");
    int channel = 1;
    u8 noteOffEvent[] = {note_off | channel, playedKey, 100};
    StuffMIDIMessage(0, noteOffEvent[0], noteOffEvent[1], noteOffEvent[2]);
    char pitchEvent[] = {pitch_wheel | channel, 0, 0b01000000};
//    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);

//    midiOutShortMsg(fweefwefwe,pack3((u8*)noteOffEvent));

}
void loadTake();
void undo() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&undo);
        return;
    }
    Undo_DoUndo2(currentItemConfig->value.project);
    loadTake();
}
void redo() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&redo);
        return;
    }
    Undo_DoRedo2(currentItemConfig->value.project);
    loadTake();
}
void save() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&save);
        return;
    }
    Main_SaveProject(GetItemProjectContext(GetMediaItemTake_Item(take)), false);
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
