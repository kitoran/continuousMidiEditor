#include "playback.h"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_events.h>
#include "stb_ds.h"
#include "melody.h"
#include "extmath.h"
#include "misc.h"
#include <stdbool.h>

int currentPositionInSamples = 0;
double smoothstep (double edge0, double edge1, double x)
{
   if (x < edge0)
      return 0;

   if (x >= edge1)
      return 1;

   // Scale/bias into [0..1] range
   x = (x - edge0) / (edge1 - edge0);

   return x * x * (3 - 2 * x);
}
SDL_AudioSpec  have;
#define EASE_FRAMES 40


int countSamples(double time) {
    return time * have.freq;
}
#define CHANNELS 16
SDL_AudioDeviceID audioDevice;
//int binaryFind(int samples) {
//    int l = 0, h = arrlen(piece)-1;
//    while(h-l > 1) {
//        if(
//    }
//}
void buffer(void* userdata,
            Uint8* stream8,
            int wrongLen)
{
    if(arrlen(piece) == 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        return;
    }
    int trueLen = wrongLen/2;


    i16* stream = (i16*)stream8;
//    typedef struct StrippedNote {
//        int samples;
//        double freq;
//    } StrippedNote;

    void newNotes(double freqs[CHANNELS], int *samples) {
//        static int positionEnd = 0;
//        static int positionStart = 0;
//        static bool started = false;
        int filledPositions = 0;
        *samples = INT32_MAX;//0x7fffffff;
        int i ;
        for(i = 0; i < arrlen(piece) && filledPositions < CHANNELS; i++) {
            if(piece[i].start <= currentPositionInSamples*1.0/have.freq &&
                    piece[i].start + piece[i].length > currentPositionInSamples*1.0/have.freq) {
                freqs[filledPositions] = piece[i].freq;
                filledPositions++;
                int remSam = countSamples(piece[i].start + piece[i].length) - currentPositionInSamples;
                remSam = MAX(remSam, 1);
                if(remSam<*samples) *samples = remSam;
            }
            if(countSamples(piece[i].start) > currentPositionInSamples) {
                int remSam = countSamples(piece[i].start) - currentPositionInSamples;
                remSam = MAX(remSam, 1);
                if(remSam<*samples) *samples = remSam;
                break;
            }
        }
        if(filledPositions  < CHANNELS) freqs[filledPositions] = -1;
//        if(filledPositions == 0) {
//            FOR_STB_ARRAY(anote, piece) {
//                if(countSamples(anote->start) > currentPositionInSamples) {
//                    *samples = countSamples(anote->start) - currentPositionInSamples;
//                    break;
//                }
//            }
//        }
        if(i >= arrlen(piece) && filledPositions == 0) {
                *samples = 0;
                currentPositionInSamples = 0;
        }
//        Note* currentNote = piece+position;
//        static double time = 0;
//        const Note silenceEnd = {INFINITY, INFINITY, 0};
//        const Note silenceStart = {0, 0, 0};
//        const Note* nextNote = position+1 < arrlen(piece)? piece+position+1 : &silenceEnd;
//        const Note* previousNote = position > 0? piece+position-1 : &silenceStart;
//        double duration = started?MIN(currentNote->length, nextNote->start - currentNote->start):
//                               MAX(currentNote->start - (previousNote->start+previousNote->length), 0);

//        StrippedNote res = { countSamples(duration),
//                             started? currentNote->freq : 0};
//        if(!started) started = true;
//        else {
//            positionEnd++;
////            positionStart++;
//            started = false;
//            if(position >= arrlen(piece)) {
//                    /*positionStart = */positionEnd = 0;
//                    currentPositionInSamples = 0;
//            }
//        }
//        return res;
    }

    static struct State {
        int samplesToFill;
        double freq[CHANNELS];
        bool smooth;
        double lastFreq[CHANNELS];
        int framesLeftLastFreq[CHANNELS];
    } state;
    loop:
    int samplesToFillNow = MIN(trueLen, state.samplesToFill);

    static u32 phase[CHANNELS] = {0};
    for(int i = 0; i < samplesToFillNow; i++) {
        stream[i] = 0;
        FOR(j, CHANNELS) {
            stream[i] += (sin(phase[j]/4.0*tau/(SDL_MAX_UINT32/4))*1000);
            if(state.framesLeftLastFreq[j] == 0) {
                phase[j] += (SDL_MAX_UINT32/((double)have.freq)*(state.freq[j]));
            } else {
                phase[j] +=
                         ((SDL_MAX_UINT32/((double)have.freq)*(state.lastFreq[j])))*
                                (state.framesLeftLastFreq[j]/(double)EASE_FRAMES)
                        + (SDL_MAX_UINT32/((double)have.freq)*(state.freq[j]))*
                                (1-state.framesLeftLastFreq[j]/(double)EASE_FRAMES);
                state.framesLeftLastFreq[j]--;
            }
        }
        currentPositionInSamples++;
    }
    SDL_UserEvent userevent = {PlaybackEvent, 0, 0, 0, 0, 0};
    SDL_Event event; event.user = userevent;
    SDL_PushEvent(&event);
    state.samplesToFill -= samplesToFillNow;
    stream += samplesToFillNow;
    trueLen -= samplesToFillNow;
    if(state.samplesToFill == 0) {
//        fprintf(stderr, "this just for breakpoint:");
        double freqs[CHANNELS];
        int samples;
        newNotes(freqs, &samples);
        struct State newstate = {
            samples,
            {0},
            true,
            {0},
            {0},//EASE_FRAMES
        };
        FOR(i, CHANNELS) {
            newstate.lastFreq[i] = state.freq[i];
            newstate.freq[i] = freqs[i];//.freq;
        }
        state = newstate;
        goto loop;
    }

}
int PlaybackEvent;
void openAudio() {
    PlaybackEvent = SDL_RegisterEvents(1);
    SDL_AudioSpec  want;
    memset(&want,0,sizeof(want));

    want.freq = 44100;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 4096;
    want.callback = buffer;

    audioDevice =  SDL_OpenAudioDevice(
                NULL,
                SDL_FALSE,
                &want, &have,
                0);
    fprintf(stderr,"%s, returend %d %d %d %d %d %d \n",
            SDL_GetError(), want.format, want.channels, want.samples,have.format, have.channels, have.samples);

    SDL_PauseAudioDevice(audioDevice, 0);
}

double samplesToTime(int samples)
{
    return samples*1.0/have.freq;
}
