#include "actionsReaper.h"
#include "misc.h"
#include "math.h"
#include "editorinstance.h"
//#include "midiprot.h"
#include "reaper_plugin_functions.h"
#include "melody.h"
#include "actions.h"

thread_local bool reaperMainThread;

//TODO: need to call these all from the main thread
void play() {
  OnPlayButton();
}

void stop() {
  OnStopButton();
}

//void message(const char* format, ...);
//extern double __declspec(selectany) itemStart;
//extern double __declspec(selectany) itemStart;
// TODO: setting of whether channel 1 is master channel or regular channel
// TODO: setting of whether pitch bend is 2 semitones or 48 semitones

// TODO: reassign channels if haven't found a free channel but it's possible to find
enum MidiEventType: u8 {
    note_off = 0b1000 << 4,
    note_on = 0b1001 << 4,
    pitch_wheel = 0b1110 << 4,
};
const double logSemitone = log(2)/12;
const double bias =  log(440)/logSemitone-69;
void reaperInsert(Note note) {

    double startppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.start+itemStart);
    double endppqpos = MIDI_GetPPQPosFromProjTime(take,
                                                    note.start+note.length+itemStart);
    static u8 channel =1;
    // TODO: make vertical zoom relative to mouse
    // and horizontal zoom relative to cursor
    if(midiMode ==  midi_mode_regular)
        channel = (channel+1)%16;
    else  // MPE
        channel = (channel)%15+1;
    int key = round(log(note.freq)/logSemitone-bias);
    double freqOfTheKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double difference = note.freq/freqOfTheKey;

    double differenceInSemitones = 12*log(difference)/log(2);
    double differenceInPitchRamgeIntervals = differenceInSemitones/pitchRange;
    int pitchWheel = round(differenceInPitchRamgeIntervals*0x2000)+0x2000;

    char pitchEvent[] = {pitch_wheel | channel, pitchWheel&0b1111111, pitchWheel>>7};
    MIDI_InsertEvt(take, false, false, startppqpos-1, pitchEvent, sizeof(pitchEvent));
    bool res = MIDI_InsertNote(take, false, false,
                    startppqpos, endppqpos,
                    channel, key,
                    100,
                    NULL);
    message("inserting note %lf - %lf\n", startppqpos, endppqpos);
    if(!res) ShowConsoleMsg("note insertion failed");

}
void reaperSetPosition(double d) {

    //TODO: this is prob wrong
    SetEditCurPos(d+itemStart, false, true);
}


void reaperOnCommand(u32 command) {
    Main_OnCommand(command, 0);
}
void reaperDelete(int note) {
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

    if(true /*reaperMainThread*/) {
        char msg[100];
        vsnprintf(msg, 100,format, arg_ptr);
        ShowConsoleMsg(msg);
    } else {
        std::unique_lock lk(actionChannel.mutex);
        vsnprintf(actionChannel.string,
                  sizeof(actionChannel.string),
                  // TODO: sizeof(..)-1 on msvc??
                  format,
                  arg_ptr);
        actionChannel.action = actionChannel.consoleMessage;
        actionChannel.cv.wait(lk, [](){
                return actionChannel.action
                        == actionChannel.none;});

    }
    va_end(arg_ptr);
}
ActionChannel actionChannel;
