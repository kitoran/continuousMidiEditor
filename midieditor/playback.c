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
    while(trueLen > 0) {
        int samplesToFillNow = trueLen;
        int filledPositions = 0;
        int freqs[CHANNELS] = {0};
        for(int i = 0; i < arrlen(piece) && filledPositions < CHANNELS; i++) {
            if(piece[i].start <= currentPositionInSamples*1.0/have.freq &&
                    piece[i].start + piece[i].length > currentPositionInSamples*1.0/have.freq) {
                freqs[filledPositions] = piece[i].freq;
                filledPositions++;
                int remSam = countSamples(piece[i].start + piece[i].length) - currentPositionInSamples;
                remSam = MAX(remSam, 1);
                if(remSam<samplesToFillNow) samplesToFillNow = remSam;
            }
            if(countSamples(piece[i].start) > currentPositionInSamples) {
                int remSam = countSamples(piece[i].start) - currentPositionInSamples;
                remSam = MAX(remSam, 1);
                if(remSam<samplesToFillNow) samplesToFillNow = remSam;
                break;
            }
        }
        if(filledPositions  < CHANNELS) freqs[filledPositions] = -1;

        static u32 phase[CHANNELS] = {0};
        for(int i = 0; i < samplesToFillNow; i++) {
            stream[i] = 0;
            FOR(j, CHANNELS) {
                stream[i] += (sin(phase[j]/4.0*tau/(SDL_MAX_UINT32/4))*1000);
//                if(state.framesLeftLastFreq[j] == 0) {
                    phase[j] += (SDL_MAX_UINT32/((double)have.freq)*(freqs[j]));
//                } else {
//                    phase[j] +=
//                             ((SDL_MAX_UINT32/((double)have.freq)*(state.lastFreq[j])))*
//                                    (state.framesLeftLastFreq[j]/(double)EASE_FRAMES)
//                            + (SDL_MAX_UINT32/((double)have.freq)*(state.freq[j]))*
//                                    (1-state.framesLeftLastFreq[j]/(double)EASE_FRAMES);
//                    state.framesLeftLastFreq[j]--;
//                }
            }
        }
        currentPositionInSamples+=samplesToFillNow;
        stream += samplesToFillNow;
        trueLen -= samplesToFillNow;
    }
    SDL_UserEvent userevent = {PlaybackEvent, 0, 0, 0, 0, 0};
    SDL_Event event; event.user = userevent;
    SDL_PushEvent(&event);
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

int timeToSamples(double time)
{
    return round(time*have.freq);
}
