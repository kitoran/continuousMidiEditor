#ifndef PLAYBACK_H
#define PLAYBACK_H
#include "misc.h"
#include "stdbool.h"
#ifdef __cplusplus
extern "C" {
#endif
extern int currentPositionInSamples;
extern double cursorPosition;
extern bool playing;
extern int PlaybackEvent;
#ifdef __cplusplus
}
#endif
extern u32 audioDevice;
double samplesToTime(int samples);
int timeToSamples(double time);
void openAudio();

#endif // PLAYBACK_H
