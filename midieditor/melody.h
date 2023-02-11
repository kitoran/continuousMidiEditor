#ifndef MELODY_H
#define MELODY_H
#include <stdbool.h>
#define CHANNELS 16
typedef struct Note {
    double freq;
    double start;
    double length;
} Note;
#define NOTE_FORMAT "freq = %lf, start = %lf, length = %lf"
// unhygienic macro
#define NOTE_ARGS(a) a.freq, a.start, a.length
#define FOR_NOTES(a, b) FOR_STB_ARRAY(Note*, a, b)
extern Note* piece;
#ifdef __cplusplus
extern "C" {
#else
extern
#endif
int insertNote(Note note, bool propagate);
void removeNote(int ind);
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
extern double end;

#ifdef __cplusplus
}
#endif
bool saveMelody(char* filename);
bool loadMelody(char* filename);


#endif // MELODY_H
