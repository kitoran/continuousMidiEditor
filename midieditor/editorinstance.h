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
extern MediaItem_Take* take;
extern "C" {
#endif
INTROSPECT_ENUM_VISIBLE_NAMES(midi_mode_enum,
                midi_mode_regular, "Regular MIDI",
                midi_mode_mpe, "MPE")
extern midi_mode_enum midiMode;
extern double itemStart;
extern double pitchRange;
typedef struct CONTINUOUSMIDIEDITOR_Config {
//    union {
//        char guid[16];
//        char key[16];
//    };
    GUID key;
//    union {
//        struct {
//            Rect windowGeometry;
//            Rect innerGeometry;//horizontalscroll, verticalScroll, horizontallScrollFraction, veritcalScrollFraction
//        };
        struct {
            Rect windowGeometry;
            double horizontalScroll;
            double horizontalFrac;
            double verticalScroll;
            double verticalFrac;
        } value;
//    };
} CONTINUOUSMIDIEDITOR_Config;
extern CONTINUOUSMIDIEDITOR_Config* config;
extern GUID currentGuid/*[16]*/;
#ifdef __cplusplus
}
#endif
#endif  // EDITORINSTANCE_H
