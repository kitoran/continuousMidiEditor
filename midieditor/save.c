#include "save.h"
#include "melody.h"
#include <stdbool.h>
#include "misc.h"
#include "math.h"
#include "stb_ds.h"
#include <smf.h>

enum MidiEventType {
    note_off = 0b1000 << 4,
    note_on = 0b1001 << 4,
    pitch_wheel = 0b1110 << 4,
};


_Bool export(char* filename)
{

    STATIC(double, logSemitone, log(2)/12);
    STATIC(double, bias, log(440)/logSemitone-69);
    double untilWhenIsChannelBusy[CHANNELS] = {[0 ... CHANNELS-1] = -1};


    smf_t *smf;
    smf_track_t *track;
    smf_event_t *event;

    smf = smf_new();
    if (smf == NULL) {
        ABORT("smf == NULL");
    }

    track = smf_track_new();
    if (track == NULL) {
        ABORT("track == NULL");
    }

    smf_add_track(smf, track);
    FOR_STB_ARRAY(note, piece) {
        bool found = false;
        int channel = 0;
        for(channel = 0; channel < CHANNELS; channel++) {
            if(untilWhenIsChannelBusy[channel] <= note->start) {
                untilWhenIsChannelBusy[channel] = note->start + note->length;
                found = true;
                break;
            }
        } if(!found) {
            ABORT("more than 16 simultaneous notes");
        }
        int key = round(log(note->freq)/logSemitone-bias);
        double freqOfTheKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
        double difference = note->freq/freqOfTheKey;
        double differenceInTones = log(difference)/log(pow(2, 1.0/6));
        int pitchWheel = round(differenceInTones*0x2000)+0x2000;
        ASSERT((pitchWheel & (~0x3fff)) == 0, "pw is 0x%x", pitchWheel);
        event = smf_event_new_from_bytes(pitch_wheel | channel, pitchWheel&0b1111111, pitchWheel>>7);
        if (event == NULL) {
            ABORT("event == NULL");
        }
        smf_track_add_event_seconds(track, event, note->start);

        event = smf_event_new_from_bytes(note_on | channel, key, 100);
        if (event == NULL) {
            ABORT("event == NULL");
        }
        smf_track_add_event_seconds(track, event, note->start);
        event = smf_event_new_from_bytes(note_off | channel, key, 0);
        if (event == NULL) {
            ABORT("event == NULL");
        }
        smf_track_add_event_seconds(track, event, note->start+
                                    note->length);
    }

    _Bool ret = smf_save(smf, filename);
    if (ret) {
        ABORT("Whoops, saving failed for some reason.");
    }
    smf_delete(smf);
    return ret;
}
