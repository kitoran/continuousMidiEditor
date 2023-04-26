#ifndef EDITORINSTANCE_H
#define EDITORINSTANCE_H
#include "misc.h"
#include "shittyintrospection.h"
//__pragma( warning (push) )
//__pragma( warning (disable:4100 ) )
//#include "reaper_plugin.h"
//__pragma( warning (pop))
#include <gui.h>
//struct editorInstance
//{
//    MediaItem_Take take;
//    editorInstance();
//};
#include <guiddef.h>
#ifdef __cplusplus
class MediaItem_Take;
extern "C" {
extern MediaItem_Take* take;
#else
struct MediaItem_Take;
extern struct MediaItem_Take* take;
#endif
INTROSPECT_ENUM_VISIBLE_NAMES(midi_mode_enum,
                midi_mode_regular, "Regular MIDI",
                midi_mode_mpe, "MPE")

extern bool showRationalIntervals;
extern bool showScale;

extern double itemStart;
typedef struct CONTINUOUSMIDIEDITOR_Config {
    GUID key;
        struct {
            Rect windowGeometry;
            double horizontalScroll;
            double horizontalFrac;
            double verticalScroll;
            double verticalFrac;
            midi_mode_enum midiMode;
            double pitchRange;
            void* project;
        } value;
} CONTINUOUSMIDIEDITOR_Config;
extern CONTINUOUSMIDIEDITOR_Config* config;
//extern GUID currentGuid/*[16]*/;
extern CONTINUOUSMIDIEDITOR_Config* currentItemConfig;
#ifdef __cplusplus
}
#endif
#endif  // EDITORINSTANCE_H
