#include "editorinstance.h"
#include "stdlib.h"
extern "C" {

//Scale scale = {.type = rational_intervals,
//               .primes = {true, true, true, true},
//               .maxComponent = 16,
//              .relative = scale_relative,
//              .equave = 2,
//              .divisions = 31};

CONTINUOUSMIDIEDITOR_Config* config = 0;
ConfigValue* currentItemConfig = 0;
GUID currentGuid = {0};
//double pitchRange = 48;
//}

transientState state = {
    .take = NULL,
    .project = NULL,
    .calculatedScale = NULL,
    .recalculateScale = true,
    .dragStart = {-1,-1},
    .draggedNoteInitialPos = {0},
    .base = -1,
    .dragged = -1,
    .newNote = {-1, -1, -1},
    .itemStart = 0,
    .timesOfGridLines = NULL,
    .mouseMode = transientState::justMovingMouse

};

}
//#ifdef REAPER
//MediaItem_Take* take = 0;
//#endif
