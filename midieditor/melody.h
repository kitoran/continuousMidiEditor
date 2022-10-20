#ifndef MELODY_H
#define MELODY_H

typedef struct Note {
    double freq;
    double start;
    double length;
} Note;

extern Note* piece;
void insert(Note note);

#endif // MELODY_H
