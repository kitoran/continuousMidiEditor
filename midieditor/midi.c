
#ifndef _MSC_VER

#include "midi.h"
#include "midiprot.h"
#include <rtmidi/rtmidi_c.h>
#include "misc.h"
#include <stdio.h>
#include <stdlib.h>
PortMidiStream* stream;
void callback(double timeStamp, const unsigned char* message,
                                 size_t messageSize, void *userData) {

    static char highMask = 0xf0;
    static char channelMask = 0x0f;
    unsigned char type = (*message);
    unsigned char high = type&highMask;
    int parsed = 1;
    if(high == 0xf0 /*system messages*/) {
        const char* name = NAME_FROM_VALUE(SystemMessage, type);
        if(name)        fprintf(stderr, "%s: ", name);
        else {
            fprintf(stderr, "(%x): ", type);
            exit(0);
        }
    }
    else {
        const char* name = NAME_FROM_VALUE(ChannelVoice, high);
        fprintf(stderr, "channel %d ", type & channelMask);
        if(name) {
            fprintf(stderr, "%s: ", name);
            if(high == control) {
                u8 cm =message[parsed++];
                name = NAME_FROM_VALUE(ControlMessage, cm);
                if(name) {
                    fprintf(stderr, "%s: ", name);
                }
                else {
                    fprintf(stderr, "(%x): ", cm);
                    exit(0);
                }
            }
        }
        else {
            fprintf(stderr, "(%x): ", high);
            exit(0);
        }
    }
    for(int i = parsed; i < messageSize; i++) {
        fprintf(stderr, "%x ", message[i]);
    }
    fprintf(stderr, "\n");

}

void midi_open(void)
{
//    RtMidiWrapper w = {0,0,true,NULL};// ind = rtmidi_in_create_default ();
    RtMidiInPtr ip = rtmidi_in_create(RTMIDI_API_UNSPECIFIED,
                                      "",
                                      0);


    rtmidi_open_virtual_port (ip, "");
    rtmidi_in_set_callback(ip, callback, NULL);

    system("aconnect 130 129");
    while(1);


//    int err = snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);
//    fprintf(stderr, "%d\n", err);
//    snd_seq_nonblock(seq_handle, true);
//    err = snd_seq_set_client_name(seq_handle, "Midi Listener");
//    fprintf(stderr, "%d\n", err);
//    in_port = snd_seq_create_simple_port(seq_handle, "listen:in",
//                      SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
//                      SND_SEQ_PORT_TYPE_APPLICATION);
//    fprintf(stderr, "%d\n", err);
 }
void subscribe() {
//    snd_seq_port_subscribe_t *portSub = 0;
//    snd_seq_addr_t src{129,0};
//    snd_seq_addr_t dest{128,0};
//    int err = snd_seq_port_subscribe_malloc(&portSub);
//    fprintf(stderr, "snd_seq_port_subscribe_malloc: %d\n", err);
//    snd_seq_port_subscribe_set_dest(portSub, &dest);
////    fprintf(stderr, "snd_seq_port_subscribe_set_dest: %d\n", err);
//    snd_seq_port_subscribe_set_sender(portSub, &src);
////    fprintf(stderr, "snd_seq_port_subscribe_set_sender: %d\n", err);
//    snd_seq_port_subscribe_set_queue(portSub, 0);
//    snd_seq_port_subscribe_set_exclusive(portSub, 0);
//    snd_seq_port_subscribe_set_time_update(portSub, 0);
//    snd_seq_port_subscribe_set_time_real(portSub, 0);

//    err = snd_seq_subscribe_port(seq_handle, portSub);
//    fprintf(stderr, "snd_seq_subscribe_port %d\n", err);

}
PmEvent midi_read()
{
//    PmEvent r;
//    int rr = Pm_Read(stream, &r, 1);
//    if(rr == 0) {
//        DEBUG_PRINT(0, "no event");
//    }
//    if(rr == 1) {
//        DEBUG_PRINT(1, "good");
//    }
//    if(rr < 0) {
//        DEBUG_PRINT(-1, "oops");
//    }
//    return r;
}
#endif
