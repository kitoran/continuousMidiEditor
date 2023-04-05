#include "SDL.h"
#include "stdio.h"
#include <windows.h>
#include <processthreadsapi.h>
void play() {
    SDL_PauseAudio(0);
}
void stop() {
    SDL_PauseAudio(1);
}

void startPlayingNote(double freq) {
    abort();
}
void stopPlayingNote(double freq) {
    abort();
}
void message(const char* format, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, format);
    vfprintf(stderr, format, arg_ptr);
    va_end(arg_ptr);
}

#ifdef WIN32
void SetThreadName(DWORD dwThreadID, const char* threadName) {
//    THREADNAME_INFO info;
//    info.dwType = 0x1000;
//    info.szName = threadName;
//    info.dwThreadID = dwThreadID;
//    info.dwFlags = 0;
//#pragma warning(push)
//#pragma warning(disable: 6320 6322)
////    __try{
//        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
////    }
////    __except (EXCEPTION_EXECUTE_HANDLER){
////    }
//#pragma warning(pop)
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

#endif
