#ifndef CHANNEL_H
#define CHANNEL_H
#define _GNU_SOURCE
#include <pthread.h>
//#include <queue>
#include <sys/syscall.h>
//#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#ifdef __cplusplus
#define _Thread_local thread_local
#endif
inline static pid_t gettid() {
    return syscall(SYS_gettid);
}
#define CHANNEL_MAX_SIZE 127
struct Channel {
    char thing[CHANNEL_MAX_SIZE  ];
    _Bool full;
    pthread_cond_t cv;
    pthread_mutex_t mutex;
    pid_t waitingTid;
    const char* threadName;
};

int pthread_setname_np(pthread_t thread, const char *name);
       int pthread_getname_np(pthread_t thread,
                              char *name, size_t len);

inline static void blockAndPut(struct Channel* channel, const void* thing_, size_t size) {
    assert(size < CHANNEL_MAX_SIZE );
    pthread_t cur = pthread_self();
    pthread_getname_np(cur, channel->threadName, 30);
//    fprintf(stderr, "obtaining %d %s %s", gettid(), thread_name, __func__);
    pthread_mutex_lock(&(channel->mutex));
    memcpy(channel->thing, thing_, size);
    channel->full = true;
//    fprintf(stderr, "releasing %d %s %s", gettid(), thread_name, __func__);
    pthread_mutex_unlock(&(channel->mutex));
    pthread_cond_signal(&(channel->cv));
}

inline static void takeC(struct Channel* channel, void* buffer, size_t size) {
    assert(size < CHANNEL_MAX_SIZE);
    pthread_t cur = pthread_self();
    pthread_getname_np(cur, channel->threadName, 30);
//    fprintf(stderr, "obtaining %d %s %s", gettid(), thread_name, __func__);
    pthread_mutex_lock(&(channel->mutex));
//        cv.wait(lck, [this]{ return full; });
    channel->full = false;
//    fprintf(stderr, "releasing %d %s %s", gettid(), thread_name, __func__);
    memcpy(buffer, channel->thing, size);
    pthread_mutex_unlock(&(channel->mutex));
}

inline bool noBlockAndCheck(struct Channel* channel) {
    if(pthread_mutex_trylock(&channel->mutex)) {
        bool res = channel->full;
        pthread_mutex_unlock(&(channel->mutex));
        return res;
    }
    return false;
}
//    void wake() {
//        raise(SIGALRM);
//    }
inline static   void wait(struct Channel* channel) {
    channel->waitingTid = gettid();
//        char name[40];
//        waitingName = pthread_getname_np(pthread_self(), name, 40);
//        pause();
//    fprintf(stderr, "obtaining first %d %s %s", gettid(), thread_name, __func__);

    pthread_mutex_lock(&channel->mutex);
    if(channel->full) {
        channel->waitingTid = -1;
        pthread_mutex_unlock(&channel->mutex);
        return;
    }
    fprintf(stderr, "waiting on cv %d %s %s", gettid(), channel->threadName, __func__);
    pthread_cond_wait(&channel->cv, &channel->mutex);//, cv.wait(lck, [this]{ return full; });
    fprintf(stderr, "after waiting on cv %d %s %s", gettid(), channel->threadName, __func__);

    channel->waitingTid = -1;
//    fprintf(stderr, "releasing %d %s %s", gettid(), thread_name, __func__);

    pthread_mutex_unlock(&channel->mutex);
    return;
}




#endif // CHANNEL_H
