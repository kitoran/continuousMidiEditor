#ifndef ACTIONSREAPER_H
#define ACTIONSREAPER_H
#include <mutex>
#include <condition_variable>
#include "melody.h"
extern thread_local bool reaperMainThread;
extern struct ActionChannel {
    enum  {
        none,
        insertNote,
        deleteNote,
        consoleMessage
    } action;
    union {
        Note note;
        int intgr;
        char string[1024];
    };
    std::mutex mutex;
    std::condition_variable cv;
} actionChannel;
extern "C"
void message(const char* format, ...);

#endif // ACTIONSREAPER_H
