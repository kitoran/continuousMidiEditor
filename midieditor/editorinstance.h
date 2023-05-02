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
class ReaProject;
extern "C" {
extern MediaItem_Take* take;
#else
struct MediaItem_Take;
struct ReaProject;
typedef struct ReaProject ReaProject;
extern struct MediaItem_Take* take;
#endif

INTROSPECT_ENUM_VISIBLE_NAMES(scale_type_enum,
                              rational_intervals, "Rational intervals",
                              equal_intervals, "equal steps");
#define NUMBER_OF_PRIMES 15
typedef struct Scale {
    scale_type_enum type;
    bool primes[NUMBER_OF_PRIMES];
    int maxComponent;
} Scale;
extern bool recalculateScale;
extern Scale scale;
static struct {
    int prime;
    char text[4];
#define X(a) {a, #a},
} primes[] = {
    FOREACH(X, (2,
            3,
            5,
            7,
            11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71))
        };
#undef X
static_assert (sizeof(primes)/sizeof(*primes) >= NUMBER_OF_PRIMES, "");




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
            ReaProject* project;
        } value;
} CONTINUOUSMIDIEDITOR_Config;
extern CONTINUOUSMIDIEDITOR_Config* config;
//extern GUID currentGuid/*[16]*/;
extern CONTINUOUSMIDIEDITOR_Config* currentItemConfig;
#ifdef __cplusplus
}
#endif
#endif  // EDITORINSTANCE_H
