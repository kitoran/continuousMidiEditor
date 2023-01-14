#ifndef MIDIPROT_H
#define MIDIPROT_H
#include <misc.h>
#include <shittyintrospection.h>


INTROSPECT_ENUM_VALUES (ChannelVoice,
    noteOff , 0b10000000,
    noteOn  , 0b10010000,
    pitchWheelEvent , 0b11100000,
    control, 0b10110000
)

INTROSPECT_ENUM_VALUES (SystemMessage,
    songPos , 0b11110010,
    start , 0b11111010,
    cont , 0b11111011,
    stop , 0b11111100
)

INTROSPECT_ENUM_VALUES (ControlMessage,
    allSoundOff , 0b01111000,
    allNotesOff, 0b01111011,
    damperPedal, 0b01000000
)



#endif // MIDIPROT_H
