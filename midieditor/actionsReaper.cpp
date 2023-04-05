#include "actionsReaper.h"
#include "misc.h"
#include "math.h"
#include "editorinstance.h"
//#include "midiprot.h"
#include "reaper_plugin_functions.h"
#include "melody.h"
#include "actions.h"

thread_local bool reaperMainThread;

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

// TODO: reassign channels if haven't found a free channel but it's possible to find
enum MidiEventType: u8 {
    note_off = 0b1000 << 4,
    note_on = 0b1001 << 4,
    pitch_wheel = 0b1110 << 4,
};
const double logSemitone = log(2)/12;
const double bias =  log(440)/logSemitone-69;
struct MidiPitch {
    int key;
    int wheel;
};

MidiPitch getMidiPitch(double freq, double pitchRangeInterval) {

    int key = (int)round(log(freq)/logSemitone-bias);
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

void reaperInsert(RealNote note) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperInsert, note);
        return;
    }
    double startppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.note.start+itemStart);
    double endppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.note.start+note.note.length+itemStart);
   // static u8 channel =1;
//    if(midiMode ==  midi_mode_regular)
//        channel = (channel+1)%16;
//    else  // MPE
//        channel = (channel)%15+1;
    MidiPitch mp = getMidiPitch(note.note.freq, pitchRange);
    char pitchEvent[] = {pitch_wheel | note.midiChannel, mp.wheel&0b1111111, mp.wheel>>7};
    MIDI_InsertEvt(take, false, false, startppqpos-1, pitchEvent, sizeof(pitchEvent));
    bool res = MIDI_InsertNote(take, false, false,
                    startppqpos, endppqpos,
                    note.midiChannel, mp.key,
                    100,
                    NULL);
    message("inserting note %lf - %lf\n", startppqpos, endppqpos);
    if(!res) ShowConsoleMsg("note insertion failed");

}
//TODO: fix the stuff where "end" is not always the same as the midi item/take length
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
void reaperDelete(int note) {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&reaperDelete, note);
        return;
    }
    bool res = MIDI_DeleteNote(take, note);
    message("deleting note %d", note);
    if(!res) ShowConsoleMsg("note deletion failed");
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


//    if(reaperMainThread) {
        char msg[100];
        vsnprintf(msg, 100,format, arg_ptr);
        if(!reaperMainThread) {
            actionChannel.name = __func__;
            actionChannel.runInMainThread(ShowConsoleMsg, msg);
            return;
        }
        ShowConsoleMsg(msg);
//    } else {
//        std::unique_lock lk(actionChannel.mutex);
//        vsnprintf(actionChannel.string,
//                  sizeof(actionChannel.string),
//                  // TODO: sizeof(..)-1 on msvc??
//                  format,
//                  arg_ptr);
//        actionChannel.action = actionChannel.consoleMessage;
//        actionChannel.cv.wait(lk, [](){
//                return actionChannel.action
//                        == actionChannel.none;});

//    }
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
void startPlayingNote(double freq) {

    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&startPlayingNote, freq);
        return;
    }
    double pitchInterval = midiMode ==  midi_mode_regular?pitchRange:2;
    MidiPitch mp = getMidiPitch(freq, pitchInterval);
    int channel = 0;
    char pitchEvent[] = {pitch_wheel | channel, mp.wheel&0b1111111, mp.wheel>>7};
    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);
    char noteOnEvent[] = {note_on | channel, mp.key, 100};
    StuffMIDIMessage(0, noteOnEvent[0], noteOnEvent[1], noteOnEvent[2]);
    playedKey = mp.key;
}
void stopPlayingNote() {
    if(!reaperMainThread) {
        actionChannel.name = __func__;
        actionChannel.runInMainThread(&stopPlayingNote);
        return;
    }
    ASSERT(playedKey >=0 && playedKey < 128, "hi");
    int channel = 0;
    char noteOffEvent[] = {note_off | channel, playedKey, 100};
    StuffMIDIMessage(0, noteOffEvent[0], noteOffEvent[1], noteOffEvent[2]);
    char pitchEvent[] = {pitch_wheel | channel, 0, 0b01000000};
    StuffMIDIMessage(0, pitchEvent[0], pitchEvent[1], pitchEvent[2]);
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
