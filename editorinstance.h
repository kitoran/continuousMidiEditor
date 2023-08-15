#ifndef EDITORINSTANCE_H
#define EDITORINSTANCE_H
#include "misc.h"
#include "shittyintrospection.h"
#include "melody.h"
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
#define CLASS(a) class a;
class MediaItem_Take;
class ReaProject;
extern "C" {
#else
#define CLASS(a) struct a; typedef struct a a;
struct MediaItem_Take;
struct ReaProject;
typedef struct ReaProject ReaProject;
typedef struct MediaItem_Take MediaItem_Take;
#endif
struct Step;
typedef struct Step Step;
//struct IdealNote;
//typedef struct IdealNote Step;
INTROSPECT_ENUM_VISIBLE_NAMES(scale_type_enum,
                              rational_intervals, "Rational intervals",
                              equal_intervals, "equal steps");
INTROSPECT_ENUM_VISIBLE_NAMES(scale_relativity_enum,
                              scale_absolute, "Fixed scale",
                              scale_relative, "Scale relative to the selected base");
#define NUMBER_OF_PRIMES 15
typedef struct Scale {
    scale_type_enum type;
    bool primes[NUMBER_OF_PRIMES];
    int maxComponent;
    scale_relativity_enum relative;
    double equave;
    int divisions;
    double root;
} Scale;
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


//extern int subgridDen;

INTROSPECT_ENUM_VISIBLE_NAMES(midi_mode_enum,
                midi_mode_regular, "No MPE (use 16 channels)",
                midi_mode_mpe, "MPE (use channels 2-16)")

//extern bool showRationalIntervals;
//extern bool showScale;

typedef struct CONTINUOUSMIDIEDITOR_Config {
    GUID key;
        struct ConfigValue {
            Rect windowGeometry;
            double horizontalScroll;
            double horizontalFrac;
            double verticalScroll;
            double verticalFrac;
            midi_mode_enum midiMode;
            double pitchRange;
            bool combinations;

            Scale scale;

            bool showChannels;
            bool showMidi ;
            bool showScale ;
            bool verticalSnap ;
            bool horizontalSnap ;


            int subgridDen;
        } value;
} CONTINUOUSMIDIEDITOR_Config;
extern CONTINUOUSMIDIEDITOR_Config* config;
//extern GUID currentGuid/*[16]*/;
#ifdef __cplusplus
typedef CONTINUOUSMIDIEDITOR_Config::ConfigValue ConfigValue;
#else
typedef struct ConfigValue ConfigValue;
#endif
extern ConfigValue* currentItemConfig;
extern GUID currentGuid;
typedef struct transientState {

    MediaItem_Take* take;
    ReaProject* project;


    Step* calculatedScale;
    Step* combinations;

    bool recalculateScale;

    Point dragStart;
    IdealNote draggedNoteInitialPos;
    int base;
    int base2;
    int dragged;
    IdealNote newNote;

    double  itemStart;
    double* timesOfGridLines;
    enum {
        justMovingMouse,
        selectingARange,
        draggingLeftEdge,
        draggingRightEdge,
        draggingVelocity,
        movingNote,
        copyingNote,
        limboCopyingOrDeselecting,
        stretchingRight
    } mouseMode;

} transientState;
extern transientState state;
#ifdef __cplusplus
}
#endif
#endif  // EDITORINSTANCE_H
