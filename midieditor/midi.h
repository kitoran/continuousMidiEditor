#ifndef MIDI_H
#define MIDI_H
#include <portmidi.h>
PmEvent midi_read();
void midi_open(void);

#endif // MIDI_H
