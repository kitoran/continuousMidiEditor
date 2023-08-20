#ifndef ACTIONSREAPER_H
#define ACTIONSREAPER_H
#include <mutex>
#include <functional>
#include <condition_variable>
#include "melody.h"

#include "misc.h"
enum MidiEventType: u8 {
    note_off = 0b1000 << 4,
    note_on = 0b1001 << 4,
    pitch_wheel = 0b1110 << 4,
    control_change = 0b1011 << 4,
};
enum MidiCCEvent: u8 {
    all_notes_off = 123,
};
extern thread_local bool reaperMainThread;
extern struct ActionChannel {
    std::function<void()> action;
//    std::mutex mutex;
//    std::condition_variable cv;
    bool pending=false;
    const char* name;
    template <typename F, typename... Args>
    void runInMainThread(F f, Args... args); // this is a blocking function, it returns when the action is completed
    // so it's okay to pass pointers to local variables to it
} actionChannel;
#pragma pack(push)
#pragma pack( 1)
    static struct reapermidimessage {
        i32 offset = 0;
        i8 flag = 0;
        i32 msglen = 3;
        unsigned char msg[3] = { control_change | 1, all_notes_off, 0};
    } notesOff;
#pragma pack(pop)
#endif // ACTIONSREAPER_H
