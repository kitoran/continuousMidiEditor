#include "editorinstance.h"
#include "stdlib.h"
extern "C" {
double  itemStart;
CONTINUOUSMIDIEDITOR_Config* config = 0;
CONTINUOUSMIDIEDITOR_Config* currentItemConfig = 0;
GUID currentGuid/*[16]*/ = {0};

midi_mode_enum midiMode = midi_mode_mpe;
double pitchRange = 48;
}
#ifdef REAPER
MediaItem_Take* take = 0;
#endif
