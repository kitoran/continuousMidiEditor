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
double pieceLength = 0;
int piecelen() {
    return arrlen(piece);
}
void reassignChannels() {
//    int pos = 0;
    double untilWhenChannelsOccupied[CHANNELS];
    for(int i = 0; i < CHANNELS; i++) untilWhenChannelsOccupied[i] = -1;
    bool warned = false;
    FOR_NOTES(anote, piece) {
        bool found = false;
        for(int i = (currentItemConfig->midiMode==midi_mode_mpe?1:0);i<16;i++) {
            if(untilWhenChannelsOccupied[i] <= anote->note.start) {
                anote->midiChannel=i;
                untilWhenChannelsOccupied[i] = anote->note.start + anote->note.length;
                found = true;
                break;
            }
        }
        if(!warned && !found) {
            MessageBoxInfo("not enough channels", "this piece may sound wrong because it's too polyphonic");
            warned = true;
        }
    }
//    while(arrlen(piece) > pos && piece[pos].note.start < note.start) {
//    }
//    bool occupiedChannels[CHANNELS] = {0};
}

int insertNote(IdealNote note) {

    int pos = 0;
    bool occupiedChannels[CHANNELS] = {0};
    while(arrlen(piece) > pos && piece[pos].note.start < note.start) {
        if(piece[pos].note.start+piece[pos].note.length >= note.start) {
            occupiedChannels[piece[pos].midiChannel] = true;
        }
        pos++;
    }

    int res = pos;
    RealNote runningNote = {.note = note, .midiChannel = 1};
    RealNote temp;
    while(pos < arrlen(piece)) {
        if(piece[pos].note.start < note.start+note.length) {
            occupiedChannels[piece[pos].midiChannel] = true;
        }
        temp = piece[pos];
        piece[pos] = runningNote;
        runningNote = temp;
        pos++;
    }
    arrpush(piece, runningNote);

    bool freeChannelFound = false;
    for(int i = (currentItemConfig->midiMode==midi_mode_mpe?1:0);i<16;i++) {
        if(occupiedChannels[i] == false) {
            piece[res].midiChannel=i;
            freeChannelFound = true;
            break;
        }
    }
    if(!freeChannelFound) {
        MessageBoxInfo("no free channel found", "this note may affect how other notes sound");
    }
    reassignChannels();
//    if(note.start + note.length > end) end = note.start + note.length;
#ifdef REAPER
    reaperCommitChanges("insert note");
#endif
    return res;
}
void appendRealNote(RealNote note) {
    arrpush(piece, note);
//    if(note.note.start + note.note.length > end) end = note.note.start + note.note.length;
}
void removeNotes(int* base) {
//    assert(note - piece < arrlen(piece));
//#if REAPER
//    reaperDeleteSelected();
//#endif
    int writingIndex = 0;
    int readingIndex = 0;
    for(; readingIndex < arrlen(piece); readingIndex++) {
        if(piece[readingIndex].selected) {
            if(*base == readingIndex) {
                *base = -1;
            }
        } else {
            if(*base == readingIndex) {
                *base = writingIndex;
            }
            piece[writingIndex] = piece[readingIndex];
            writingIndex++;
        }
    }
    arrsetlen(piece, writingIndex);
#if REAPER
    reaperCommitChanges("delete note(s)");
#endif
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


int cmpStarts(void const* n1, void const*n2) { // shut up
    RealNote* rn1 = n1, *rn2 = n2;
    return (rn1->note.start > rn2->note.start)?1:
           (rn1->note.start < rn2->note.start)?-1:
            (rn1->note.length > rn2->note.length)?1:
            (rn1->note.length < rn2->note.length)?-1:
            (rn1->note.freq > rn2->note.freq)?1:
            (rn1->note.freq < rn2->note.freq)?-1:
            (rn1->note.muted > rn2->note.muted)?1:
            (rn1->note.muted < rn2->note.muted)?-1:
            (rn1->note.velocity > rn2->note.velocity)?1:
            (rn1->note.velocity < rn2->note.velocity)?-1:
                                                    0;
}

//void sortMelody() {


//}

void commitChanges(int *dragged, int* base, char* undoName)
{
    reassignChannels();
#ifdef REAPER
    reaperCommitChanges(undoName);
    RealNote draggedNote = piece[*dragged];
    RealNote baseNote = {0}; if(*base>=0)baseNote=piece[*base]; // I'm sorry i'm doing it this way, i really should just sort with my own code or reload piece from reaper
#else
    ABORT("");
#endif
    qsort(piece, arrlen(piece), sizeof(*piece), cmpStarts); //Now i'm reloading after every move, so no need to sort
    RealNote* newDragged = bsearch(&draggedNote, piece, arrlen(piece), sizeof(*piece), cmpStarts);
    ASSERT(newDragged, "");
    *dragged = 0;//(int)(newDragged-piece);
    if(*base>=0) {
        RealNote* newBase = bsearch(&baseNote, piece, arrlen(piece), sizeof(*piece), cmpStarts);
        *base = (int)(newBase-piece);
    }

}


#define LOG_SEMITONE (log(2)/12)
#define BIAS (log(440)/LOG_SEMITONE-69)


MidiPitch  getMidiPitch(double freq, double pitchRangeInterval) {
    int key = (int)round(log(freq)/LOG_SEMITONE-BIAS);
    CLAMP(key, 0, 127);
    double freqOfTheKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double difference = freq/freqOfTheKey;

    double differenceInSemitones = 12*log(difference)/log(2);
    double differenceInPitchRamgeIntervals = differenceInSemitones/pitchRangeInterval;
    int pitchWheel = (int)round(differenceInPitchRamgeIntervals*0x2000)+0x2000;
    ASSERT(pitchWheel < 0x4000, "pitch out of range");
    ASSERT(pitchWheel >= 0, "pitch out of range");
    return (MidiPitch){key, pitchWheel};
}


//void copyNotes(int *dragged, int* base)
//{

//    reassignChannels();
//#ifdef REAPER
//    reaperCopyNotes();
//#else
//    ABORT("");
//#endif
//    RealNote draggedNote = piece[*dragged];
//    RealNote baseNote = {0}; if(*base>=0)baseNote=piece[*base]; // I'm sorry i'm doing it this way, i really should just sort with my own code or reload piece from reaper
//    qsort(piece, arrlen(piece), sizeof(*piece), cmpStarts);
//    RealNote* newDragged = bsearch(&draggedNote, piece, arrlen(piece), sizeof(*piece), cmpStarts);
//    ASSERT(newDragged, "");
//    *dragged = (int)(newDragged-piece);
//    if(*base>=0) {
//        RealNote* newBase = bsearch(&baseNote, piece, arrlen(piece), sizeof(*piece), cmpStarts);
//        *base = (int)(newBase-piece);
//    }

//}
