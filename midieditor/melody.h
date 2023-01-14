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
extern "C"
#else
extern
#endif
int insertNote(Note note, bool propagate);
void removeNote(int ind);
bool saveMelody(char* filename);
bool loadMelody(char* filename);
#ifdef __cplusplus
extern "C"
#else
extern
#endif
double bpm;
#define BEAT (60/bpm)

extern double end;
#endif // MELODY_H
