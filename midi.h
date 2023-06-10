#ifndef MIDI_H
#define MIDI_H

#ifndef _MSC_VER
#include <portmidi.h>
PmEvent midi_read();
void midi_open(void);
#endif
#endif // MIDI_H
