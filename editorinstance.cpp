#include "editorinstance.h"
#include "stdlib.h"
extern "C" {

Scale scale = {.type = rational_intervals,
               .primes = {true, true, true, true},
               .maxComponent = 16,
              .relative = scale_relative,
              .equave = 2,
              .divisions = 31};
bool recalculateScale = false;

double  itemStart;
CONTINUOUSMIDIEDITOR_Config* config = 0;
CONTINUOUSMIDIEDITOR_Config* currentItemConfig = 0;
GUID currentGuid = {0};

midi_mode_enum midiMode = midi_mode_mpe;
double pitchRange = 48;
}
#ifdef REAPER
MediaItem_Take* take = 0;
#endif
