#include "stb_ds.h"
#include "melody.h"
#include "misc.h"
#include <math.h>
#include <stdio.h>
Note* piece = 0;
double bpm;
void insert(Note note) {
    int pos = 0;
    while(arrlen(piece) > pos && piece[pos].start < note.start) pos++;
    Note temp;
    while(pos < arrlen(piece)) {
        temp = piece[pos];
        piece[pos] = note;
        note = temp;
        pos++;
    }
    arrpush(piece, note);
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
