#ifndef ACTIONSREAPER_H
#define ACTIONSREAPER_H
#include <mutex>
#include <functional>
#include <condition_variable>
#include "melody.h"
extern thread_local bool reaperMainThread;
extern struct ActionChannel {
    std::function<void()> action;
    std::mutex mutex;
    std::condition_variable cv;
    bool pending=false;
    const char* name;
    template <typename F, typename... Args>
    void runInMainThread(F f, Args... args); // this is a blocking function, it returns when the action is completed
    // so it's okay to pass pointers to local variables to it
} actionChannel;

#endif // ACTIONSREAPER_H
