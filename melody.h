#ifndef MELODY_H
#define MELODY_H
#include <stdbool.h>
#define CHANNELS 16
typedef struct IdealNote {
    double freq;
    double start;
    double length;
    bool muted;
    int velocity;
} IdealNote;

typedef struct RealNote {
    IdealNote note;
    int midiChannel;
    bool selected;
    int reaperNumber;
} RealNote;
#define REAL_NOTE_FORMAT "freq = %lf, start = %lf, length = %lf channel = %d"
// unhygienic macro
#define REAL_NOTE_ARGS(a) a.note.freq, a.note.start, a.note.length, a.midiChannel
#define FOR_NOTES(a, b) FOR_STB_ARRAY(RealNote*, a, b)
#ifdef __cplusplus
extern "C" {
#endif

extern RealNote* piece;
int piecelen();
//void moveNotes(double timeChange, double freqChange, int* dragged, int *base);
//void copyNotes(int *dragged, int* base);
int insertNote(IdealNote note);
void appendRealNote(RealNote note);
void removeNotes(int* base, int *base2);

void commitChanges(int *dragged, int* base, int *base2, char* undoName);
void clearPiece();
typedef struct {
    double when;
    double qpm;
    int num;
    int denom;
    // BPM (at least in reaper) measures the amount of the quarter notes per minute, so i call it qpm.
    // This matters if the denominator of the time signature is not 4

} TempoMarker;
extern TempoMarker* tempoMarkers;
extern TempoMarker projectSignature;
extern double pieceLength;

bool saveMelody(char* filename);
bool loadMelody(char* filename);

typedef struct MidiPitch {
    int key;
    int wheel;
} MidiPitch ;

MidiPitch  getMidiPitch(double freq, double pitchRangeInterval) ;

#ifdef __cplusplus
}
#endif
#endif // MELODY_H
