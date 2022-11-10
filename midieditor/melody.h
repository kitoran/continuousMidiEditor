#ifndef MELODY_H
#define MELODY_H

typedef struct Note {
    double freq;
    double start;
    double length;
} Note;

extern Note* piece;
int insertNote(Note note);
void removeNote(int ind);
extern double bpm;
#define BEAT (60/bpm)
#endif // MELODY_H
