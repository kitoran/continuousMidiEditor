#ifndef MIDIPROT_H
#define MIDIPROT_H
#include <misc.h>
#include <shittyintrospection.h>

#define MIDI_CHANNEL_MASK 0x0f
#define MIDI_COMMAND_MASK 0xf0
#define MIDI_7HB_MASK 0x7f

INTROSPECT_ENUM_VALUES (ChannelVoice,
    note_off, 0b10000000,
    note_on, 0b10010000,
    pitch_wheel_event , 0xe0,
    control_event, 0b10110000
)

INTROSPECT_ENUM_VALUES (SystemMessage,
    midi_song_pos , 0b11110010,
    midi_start , 0b11111010,
    midi_cont, 0b11111011,
    midi_stop , 0b11111100
)

INTROSPECT_ENUM_VALUES (ControlMessage,
    midi_allSoundOff , 0b01111000,
    midi_allNotesOff, 0b01111011,
    midi_damperPedal, 0b01000000
)

#endif // MIDIPROT_H
