#include "editorinstance.h"
extern "C" {
double  itemStart;
CONTINUOUSMIDIEDITOR_Config* config = 0;
GUID currentGuid/*[16]*/ = {0};

midi_mode_enum midiMode = midi_mode_mpe;
double pitchRange = 48;
}
MediaItem_Take* take = 0;
