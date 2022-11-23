#ifndef MELODY_H
#define MELODY_H

#define CHANNELS 16
typedef struct Note {
    double freq;
    double start;
    double length;
} Note;
#define NOTE_FORMAT "freq = %lf, start = %lf, length = %lf"
// unhygienic macro
#define NOTE_ARGS(a) a.freq, a.start, a.length

extern Note* piece;
int insertNote(Note note);
void removeNote(int ind);
_Bool saveMelody(char* filename);
_Bool loadMelody(char* filename);
extern double bpm;
#define BEAT (60/bpm)

extern double end;
#endif // MELODY_H
