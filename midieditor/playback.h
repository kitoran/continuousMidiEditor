#ifndef PLAYBACK_H
#define PLAYBACK_H
#include "misc.h"
extern int currentPositionInSamples;
extern u32 audioDevice;
double samplesToTime(int samples);
void openAudio();
extern int PlaybackEvent;

#endif // PLAYBACK_H
