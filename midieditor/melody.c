#include "stb_ds.h"
#include "melody.h"
#include "misc.h"
#include "actions.h"
#include "editorinstance.h"
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
RealNote* piece = 0;
extern TempoMarker* tempoMarkers = NULL;
extern TempoMarker projectSignature = {
    0,0,0,0
};
//double qpm;
//int numeratorOfProjectTimeSig;
extern double end = 0;
int insertNote(IdealNote note) {

    int pos = 0;
    bool occupiedChannels[CHANNELS] = {0};
    while(arrlen(piece) > pos && piece[pos].note.start < note.start) {
        pos++;
        if(piece[pos].note.start+piece[pos].note.length >= note.start) {
            occupiedChannels[piece[pos].midiChannel] = true;
        }
    }

    int res = pos;
    RealNote realNote = {.note = note, .midiChannel = 1};
    RealNote temp;
    while(pos < arrlen(piece)) {
        if(piece[pos].note.start < note.start+note.length) {
            occupiedChannels[piece[pos].midiChannel] = true;
        }
        temp = piece[pos];
        piece[pos] = realNote;
        realNote = temp;
        pos++;
    }
    arrpush(piece, temp);

    bool freeChannelFound = false;
    for(int i = (midiMode==midi_mode_mpe?1:0);i<16;i++) {
        if(occupiedChannels[i] == false) {
            piece[res].midiChannel=i;
            freeChannelFound = true;
            break;
        }
    }
    if(!freeChannelFound) {
        MessageBoxInfo("no free channel found", "this note may affect how other notes sound");
    }

    if(note.start + note.length > end) end = note.start + note.length;
#ifdef REAPER
    reaperInsert(piece[res]);
#endif
    return res;
}
void appendRealNote(RealNote note) {
    arrpush(piece, note);
    if(note.note.start + note.note.length > end) end = note.note.start + note.note.length;
}
void removeNote(int ind) {
//    assert(note - piece < arrlen(piece));
#if REAPER
    reaperDelete(ind);
#endif
    for(; ind < arrlen(piece)-1; ind++) {
        piece[ind]=piece[ind+1];//*note = *(note+1);
    }
    arrsetlen(piece, ind);
//    end = 0;
//    FOR_STB_ARRAY_I(i, piece) {
//        if(piece[i].start+piece[i].length > end) {
//            end = piece[i].start+piece[i].length;
//        }
//    }
}

_Bool saveMelody(char* filename) {
    FILE* f = fopen(filename, "w");
    FOR_NOTES(note, piece) {
        fprintf(f, REAL_NOTE_FORMAT "\n", REAL_NOTE_ARGS((*note)));
    }
    fclose(f);
    return 0;
}

_Bool loadMelody(char* filename) {
    FILE* f = fopen(filename, "r");
    arrsetlen(piece, 0);
    RealNote scanned;
    // FIXME vot eto ==3 zdes' plohoe
    while(fscanf_s(f, REAL_NOTE_FORMAT "\n", REAL_NOTE_ARGS(&scanned)) == 3) {
        appendRealNote(scanned);
    }

    fclose(f);
    return 0;
}

double  freq1 = 110.0;
double  freq2 = 110.0*1.09050773267 ;
double  freq3 = 110.0*1.09050773267 *1.09050773267 ;//*1.61803398875;

u32 timerCallback (u32 interval, void *param) {
    freq1 = freq1*1.17398499671;
    freq2 = freq2*1.17398499671;
    freq3 = freq3*1.17398499671;

    return 1000;
    /*static int pure = 0;
    switch(pure%4) {
    case 0: {
        freq2 = freq1/pow(2,3.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    case 1: {
        freq2 = freq1/6.0*5;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 2: {
        freq2 = freq1/5*4;//  /4*5;
        freq3 = freq1/6.0*4;//  /4*5;
        break;
    }
    case 3: {
        freq2 = freq1/pow(2,4.0/12);
        freq3 = freq1/pow(2,7.0/12);
        break;
    }
    }
    pure = (pure+1)%4;
    fprintf(stderr, "%lf %lf %d\n", freq1/freq2,freq1/freq3, pure);
//    pure = !pure;
    return 2000;
    */
    (void)param;
    //        char notes[] = {5,4,4,5,4,4,5,4,4,5,4,4,2,1,0,2};
    char notes[] = {9,8,8,9,8,8,9,8,8,9,8,8,4,3,0,4};
    static int index = 0;
    double in = pow(2, 1.0/16);

    freq1 = 220*pow(in, notes[index]);
    index = (index+1)%sizeof(notes);
    fprintf(stderr, "freq=%lf in=%lf\n", freq1,in);
    return 250;
}



void clearPiece()
{
    arrsetlen(piece, 0);
}
