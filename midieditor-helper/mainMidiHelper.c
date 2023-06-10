//#include <stdio.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include "midiprot.h"

#include <clap/clap.h>


#include <time.h>




#include <SDL.h>




//extern "C" {
static char const* feature [] = {CLAP_PLUGIN_FEATURE_NOTE_EFFECT, NULL};
static const clap_plugin_descriptor_t s_retune_plug_desc = {
    .clap_version = CLAP_VERSION_INIT,
    .id = "com.something-offensive.Retuner",
    .name = "Retuner",
    //   .vendor = "something-offensive",
    //   .url = "https://your-domain.com/your-plugin",
    //   .manual_url = "https://your-domain.com/your-plugin/manual",
    //   .support_url = "https://your-domain.com/support",
    //   .version = "1.4.2",
    .description = "Retune the notes to 16edo.",
    .features = feature,
};

typedef struct {
    clap_plugin_t                   plugin;
    const clap_host_t              *host;
    const clap_host_latency_t      *host_latency;
    const clap_host_log_t          *host_log;
    const clap_host_thread_check_t *host_thread_check;
    const clap_host_state_t        *host_state;

    uint32_t latency;



    //Param
    double center, edo, pitchRange;
    bool usezerothchannel;
    u8 channelKeys[16];
} retune_plug_t;

////////////////////////////
// clap_plugin_note_ports //
////////////////////////////

static uint32_t retune_plug_note_ports_count(const clap_plugin_t *plugin, bool is_input) {
    // We just declare 1 note input and output
    return 1;
}

static bool retune_plug_note_ports_get(const clap_plugin_t   *plugin,
                                   uint32_t               index,
                                   bool                   is_input,
                                   clap_note_port_info_t *info) {
    if (index > 0)
        return false;
    info->id = 0;
    snprintf(info->name, sizeof(info->name), "%s", "My Port Name");
    info->supported_dialects =
            CLAP_NOTE_DIALECT_MIDI;
    info->preferred_dialect = CLAP_NOTE_DIALECT_MIDI;
    return true;
}

static const clap_plugin_note_ports_t s_retune_plug_note_ports = {
    .count = retune_plug_note_ports_count,
    .get = retune_plug_note_ports_get,
};


/////////// clap_params
static uint32_t retune_plug_params_count(const clap_plugin_t *plugin) {
    return 3;
}

// Copies the parameter's info to param_info. Returns true on success.
// [main-thread]
static bool retune_plug_params_get_info(const clap_plugin_t *plugin,
                         uint32_t             param_index,
                         clap_param_info_t   *param_info) {
    clap_param_info_t center = {
        .id = 1,
        .flags = 0/*CLAP_PARAM_REQUIRES_PROCESS?*/,
        .cookie = 0,
        .name = "retuning center",
        .module = "",
        .min_value = 26,
        .max_value = 8000,
        .default_value = 261.6256
    };
    clap_param_info_t edo = {
        .id = 2,
        .flags = CLAP_PARAM_IS_STEPPED/*CLAP_PARAM_REQUIRES_PROCESS?*/,
        .cookie = 0,
        .name = "number of equal divisions of octave",
        .module = "",
        .min_value = 0.5,
        .max_value = 53,
        .default_value = 16
    };//todo: add a param for using 0th channel or not
    clap_param_info_t range = {
        .id = 3,
        .flags = CLAP_PARAM_IS_STEPPED/*CLAP_PARAM_REQUIRES_PROCESS?*/,
        .cookie = 0,
        .name = "pitch wheel range",
        .module = "",
        .min_value = 0.01,
        .max_value = 48,
        .default_value = 2
    };
    if(param_index == 0) {
        *param_info = center;
        return true;
    }
    if(param_index == 1) {
        *param_info = edo;
        return true;
    }
    if(param_index == 2) {
        *param_info = range;
        return true;
    }
    return false;
}

// Writes the parameter's current value to out_value. Returns true on success.
// [main-thread]
static bool retune_plug_params_get_value(const clap_plugin_t *plugin, clap_id param_id, double *out_value) {
    retune_plug_t *plug = ((retune_plug_t *)plugin->plugin_data);
    if(param_id == 1) {
        *out_value = plug->center;
        return true;
    }
    if(param_id == 2) {
        *out_value = plug->edo;
        return true;
    }
    if(param_id == 3) {
        *out_value = plug->pitchRange;
        return true;
    }
    return false;
}

// Fills out_buffer with a null-terminated UTF-8 string that represents the parameter at the
// given 'value' argument. eg: "2.3 kHz". Returns true on success. The host should always use
// this to format parameter values before displaying it to the user. [main-thread]
static bool retune_plug_params_value_to_text(const clap_plugin_t *plugin,
                              clap_id              param_id,
                              double               value,
                              char                *out_buffer,
                              uint32_t             out_buffer_capacity) {
    snprintf(out_buffer, out_buffer_capacity, "%lf", value);
}

// Converts the null-terminated UTF-8 param_value_text into a double and writes it to out_value.
// Returns true on success. The host can use this to convert user input into a parameter value.
// [main-thread]
static bool retune_plug_params_text_to_value(const clap_plugin_t *plugin,
                              clap_id              param_id,
                              const char          *param_value_text,
                              double              *out_value) {
    sscanf(param_value_text, "%lf", out_value);
}



static void retune_plug_process_param(retune_plug_t *plug,
                                      const clap_event_header_t *hdr) {
    ASSERT(hdr->type == CLAP_EVENT_PARAM_VALUE, "");
    const clap_event_param_value_t *ev = (const clap_event_param_value_t *)hdr;
    if(ev->param_id == 1) {
        plug->center=ev->value;
    }
    if(ev->param_id == 2) {
        plug->edo = ev->value;
    }
    if(ev->param_id == 3) {
        plug->pitchRange = ev->value;
    }
}
// Flushes a set of parameter changes.
// This method must not be called concurrently to clap_plugin->process().
//
// Note: if the plugin is processing, then the process() call will already achieve the
// parameter update (bi-directional), so a call to flush isn't required, also be aware
// that the plugin may use the sample offset in process(), while this information would be
// lost within flush().
//
// [active ? audio-thread : main-thread]
static void retune_plug_params_flush(const clap_plugin_t *plugin,
                      const clap_input_events_t  *in,
                      const clap_output_events_t *out) {
    const uint32_t nev = in->size(in);
    retune_plug_t *plug = (retune_plug_t *)plugin->plugin_data;

    FOR(i, nev) {
        const clap_event_header_t *hdr = in->get(in, i);
        retune_plug_process_param(plug, hdr);
    }

}


static const clap_plugin_params_t s_retune_plug_params = {
    .count = retune_plug_params_count,
    .get_info = retune_plug_params_get_info,
    .get_value = retune_plug_params_get_value,
    .value_to_text = retune_plug_params_value_to_text,
    .text_to_value = retune_plug_params_text_to_value,
    .flush = retune_plug_params_flush
};


//////////////////
// clap_latency //
//////////////////

static uint32_t retune_plug_latency_get(const clap_plugin_t *plugin) {
    const retune_plug_t *plug = (const retune_plug_t *)plugin->plugin_data;
    return plug->latency;
}

static const clap_plugin_latency_t s_retune_plug_latency = {
    .get = retune_plug_latency_get,
};

//////////////////
// clap_gui //
//////////////////

//static uint32_t retune_plug_latency_get(const clap_plugin_t *plugin) {
//    const retune_plug_t *plug = (const retune_plug_t *)plugin->plugin_data;
//    return plug->latency;
//}

//static const clap_plugin_gui_t s_retune_plug_latency = {
//    .get = retune_plug_latency_get,
//};

////////////////
// clap_state //
////////////////

static bool retune_plug_state_save(const clap_plugin_t *plugin, const clap_ostream_t *stream) {
    const retune_plug_t *plug = (const retune_plug_t *)plugin->plugin_data;
    // TODO: write the state into stream
    return true;
}

static bool retune_plug_state_load(const clap_plugin_t *plugin, const clap_istream_t *stream) {
    const retune_plug_t *plug = (const retune_plug_t *)plugin->plugin_data;
    // TODO: read the state from stream
    return true;
}

static const clap_plugin_state_t s_retune_plug_state = {
    .save = retune_plug_state_save,
    .load = retune_plug_state_load,
};

/////////////////
// clap_plugin //
/////////////////

static bool retune_plug_init(const struct clap_plugin *plugin) {
    retune_plug_t *plug = /*const_cast< retune_plug_t *>*/((retune_plug_t *)plugin->plugin_data);

//    fprintf(stdout, "printing to stdour\n");
//    fprintf(stderr, "printing to stderr\n");
    // Fetch host's extensions here
    // Make sure to check that the interface functions are not null pointers
    plug->host_log = (const clap_host_log_t *)plug->host->get_extension(plug->host, CLAP_EXT_LOG);
    plug->host_thread_check = (const clap_host_thread_check_t *)plug->host->get_extension(plug->host, CLAP_EXT_THREAD_CHECK);
    plug->host_latency = (const clap_host_latency_t *)plug->host->get_extension(plug->host, CLAP_EXT_LATENCY);
    plug->host_state = (const clap_host_state_t *)plug->host->get_extension(plug->host, CLAP_EXT_STATE);
    return true;
}

static void retune_plug_destroy(const struct clap_plugin *plugin) {
    retune_plug_t *plug = ( retune_plug_t *)plugin->plugin_data;
    free(plug);
}

static bool retune_plug_activate(const struct clap_plugin *plugin,
                             double                    sample_rate,
                             uint32_t                  min_frames_count,
                             uint32_t                  max_frames_count) {
    return true;
}

static void retune_plug_deactivate(const struct clap_plugin *plugin) {}

static bool retune_plug_start_processing(const struct clap_plugin *plugin) { return true; }

static void retune_plug_stop_processing(const struct clap_plugin *plugin) {}

static void retune_plug_reset(const struct clap_plugin *plugin) {}

static int findChannelOfKey(u8 key, retune_plug_t *plug) {
    for(int i = 1; i < 16; i++) {
        if(plug->channelKeys[i] == key) {
            return i;
        }
    }
    return -1;
}
static int findFreeChannel(retune_plug_t *plug) {
    return findChannelOfKey(0, plug);
}
typedef struct MidiPitch {
    int key;
    int wheel;
} MidiPitch;
//TODO: simplify math
#define LOG_SEMITONE 0.05776226504//log(2)/12;
#define BIAS 36.3763165623 //log(440)/logSemitone-69;
static MidiPitch getMidiPitchLog(double logFreq, double pitchRange) {
    int key = (int)round(logFreq/LOG_SEMITONE-BIAS);
//    fprintf(stderr, "key is
    CLAMP(key, 0, 127);
    double freqOfTheKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double logdifference = logFreq - log(freqOfTheKey);

    double differenceInSemitones = 12*logdifference/log(2);
    double differenceInPitchRamgeIntervals = differenceInSemitones/pitchRange;
    int pitchWheel = (int)round(differenceInPitchRamgeIntervals*0x2000)+0x2000;
    ASSERT(pitchWheel < 0x4000 && pitchWheel >= 0 , "pitch out of range: pitchwheel %d, key %d, freq %lf, pitchrange %lf\n", pitchWheel,
           key, exp(logFreq), pitchRange);
    return (MidiPitch){key, pitchWheel};
}
static MidiPitch calculateMidiPitch(int key, retune_plug_t* plug) {
//                double center = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double freqOfKey = (440.0 / 32) * pow(2, ((key - 9) / 12.0));
    double distance = log(freqOfKey)-log(plug->center);

//                double correctStep = pow(2, 1/edo);
    double multiplier = 12/plug->edo;//log(2)/edo/(log(2)/12);

    double correctDistance = distance*multiplier;
    double correctLogFreq = log(plug->center)+correctDistance;
    fprintf(stderr, "key %d, freqOfKey %lf, dist %lf, mult %lf, cordist %lf, corlogfreq %lf\n",
                    key,     freqOfKey, distance, multiplier, correctDistance, correctLogFreq);
    return getMidiPitchLog(correctLogFreq, plug->pitchRange);
}
const char* appName = "Another Name";
static void retune_plug_process_event(retune_plug_t *plug, const clap_event_header_t *hdr,
                                        const clap_process_t     *process) {
    if (hdr->space_id == CLAP_CORE_EVENT_SPACE_ID) {
        switch (hdr->type) {
        case CLAP_EVENT_MIDI: {
            fprintf(stderr, "CLAP_EVENT_MIDI %s %d\n", appName, SDL_WasInit(SDL_INIT_VIDEO));
            const clap_event_midi_t *ev = (const clap_event_midi_t *)hdr;
            u8* msg = ev->data;

            if((msg[0] & MIDI_COMMAND_MASK) == note_on) {
                int channel= findFreeChannel(plug);
                if(channel < 0) {
                    return;
                }
                fprintf(stderr, "channel %d ", channel);
                FOR_STATIC_ARRAY(u8*, i, plug->channelKeys) {
                    fprintf(stderr, " %d ", *i);
                }
                MidiPitch mp = calculateMidiPitch(msg[1], plug);
                plug->channelKeys[channel] = mp.key;
                char pitchEventData[] = {pitch_wheel_event | channel, mp.wheel&0b1111111, mp.wheel>>7};
                clap_event_midi_t pitchEvent = {.header = ev->header,
                                               .port_index = ev->port_index};
                memcpy(pitchEvent.data, pitchEventData, 3);
                process->out_events->try_push(process->out_events, &pitchEvent.header);

                char noteOnData[] = {note_on | channel, mp.key, msg[2]};
                clap_event_midi_t noteOnEvent = {.header = ev->header,
                                                 .port_index = ev->port_index};
                memcpy(noteOnEvent.data, noteOnData, 3);

                process->out_events->try_push(process->out_events, &noteOnEvent.header);
            } else if((msg[0] & MIDI_COMMAND_MASK) == note_off) {
                MidiPitch mp = calculateMidiPitch(msg[1], plug);
                int channel = findChannelOfKey(mp.key, plug);
                if(channel < 0) {
                    return;
                }
                plug->channelKeys[channel] = 0;
                char noteOffData[] = {note_off | channel, mp.key, msg[2]};
                clap_event_midi_t noteOffEvent = {.header = ev->header,
                                                 .port_index = ev->port_index};
                memcpy(noteOffEvent.data, noteOffData, 3);

                process->out_events->try_push(process->out_events, &noteOffEvent.header);
            } else {
                process->out_events->try_push(process->out_events, &ev->header);
            }

            break;
        }

        case CLAP_EVENT_MIDI_SYSEX: {
            const clap_event_midi_sysex_t *ev = (const clap_event_midi_sysex_t *)hdr;
            process->out_events->try_push(process->out_events, &ev->header);
            break;
        }
        case CLAP_EVENT_PARAM_VALUE: {
            retune_plug_process_param(plug, hdr);
            break;
        }
        }
    }
}

static char message[3000] = {0};
static char* endOfMessage = message;
static clap_process_status retune_plug_process(const struct clap_plugin *plugin,
                                           const clap_process_t     *process) {
    const retune_plug_t *plug = (const retune_plug_t *)plugin->plugin_data;
    const uint32_t nframes = process->frames_count;
    const uint32_t nev = process->in_events->size(process->in_events);
    uint32_t       ev_index = 0;
    uint32_t       next_ev_frame = nev > 0 ? 0 : nframes;

    for (uint32_t i = 0; i < nframes;i++) {//TODO remove this loop
          /* handle every events that happrens at the frame "i" */
          while (ev_index < nev && next_ev_frame == i) {
             const clap_event_header_t *hdr = process->in_events->get(process->in_events, ev_index);

             if (hdr->time != i) {
                next_ev_frame = hdr->time;
                break;
             }
             volatile uint16_t sid = hdr->space_id;
             volatile uint16_t type = hdr->type;

             retune_plug_process_event(plug, hdr, process);
             ++ev_index;

             if (ev_index == nev) {
                // we reached the end of the event list
                next_ev_frame = nframes;
                break;
             }
          }
    }
    return CLAP_PROCESS_CONTINUE;
}

static const void *retune_plug_get_extension(const struct clap_plugin *plugin, const char *id) {
    if (!strcmp(id, CLAP_EXT_LATENCY))
        return &s_retune_plug_latency;
    if (!strcmp(id, CLAP_EXT_AUDIO_PORTS))
        return NULL;
    if (!strcmp(id, CLAP_EXT_NOTE_PORTS))
        return &s_retune_plug_note_ports;
    if (!strcmp(id, CLAP_EXT_PARAMS))
        return &s_retune_plug_params;
    if (!strcmp(id, CLAP_EXT_STATE))
        return &s_retune_plug_state;
//    if (!strcmp(id, CLAP_EXT_GUI))
//        return &s_retune_plug_gui;
    return NULL;
}

static void retune_plug_on_main_thread(const struct clap_plugin *plugin) {}

static clap_plugin_t *retune_plug_create(const clap_host_t *host) {
    retune_plug_t *p = ( retune_plug_t *)calloc(1, sizeof(*p));
    p->host = host;
    p->plugin.desc = &s_retune_plug_desc;
    p->plugin.plugin_data = p;
    p->plugin.init = retune_plug_init;
    p->plugin.destroy = retune_plug_destroy;
    p->plugin.activate = retune_plug_activate;
    p->plugin.deactivate = retune_plug_deactivate;
    p->plugin.start_processing = retune_plug_start_processing;
    p->plugin.stop_processing = retune_plug_stop_processing;
    p->plugin.reset = retune_plug_reset;
    p->plugin.process = retune_plug_process;
    p->plugin.get_extension = retune_plug_get_extension;
    p->plugin.on_main_thread = retune_plug_on_main_thread;


    p->center = 261.6256;
    p->edo = 16;
    p->pitchRange = 2;
    FOR(i, 16) {
        p->channelKeys[i] = 0;
    }
    // Don't call into the host here

    return &p->plugin;
}

/////////////////////////
// clap_plugin_factory //
/////////////////////////

static struct {
    const clap_plugin_descriptor_t *desc;
    clap_plugin_t *(CLAP_ABI *create)(const clap_host_t *host);
} s_plugins[] = {
{
    .desc = &s_retune_plug_desc,
    .create = retune_plug_create,
},
};

static uint32_t plugin_factory_get_plugin_count(const struct clap_plugin_factory *factory) {
    return sizeof(s_plugins) / sizeof(s_plugins[0]);
}

static const clap_plugin_descriptor_t *
plugin_factory_get_plugin_descriptor(const struct clap_plugin_factory *factory, uint32_t index) {
    return s_plugins[index].desc;
}

static const clap_plugin_t *plugin_factory_create_plugin(const struct clap_plugin_factory *factory,
                                                         const clap_host_t                *host,
                                                         const char *plugin_id) {
    if (!clap_version_is_compatible(host->clap_version)) {
        return NULL;
    }

    const int N = sizeof(s_plugins) / sizeof(s_plugins[0]);
    for (int i = 0; i < N; ++i)
        if (!strcmp(plugin_id, s_plugins[i].desc->id))
            return s_plugins[i].create(host);

    return NULL;
}

static const clap_plugin_factory_t s_plugin_factory = {
    .get_plugin_count = plugin_factory_get_plugin_count,
    .get_plugin_descriptor = plugin_factory_get_plugin_descriptor,
    .create_plugin = plugin_factory_create_plugin,
};

////////////////
// clap_entry //
////////////////

static bool entry_init(const char *plugin_path) {
    // called only once, and very first
    return true;
}

static void entry_deinit(void) {
    // called before unloading the DSO
}

static const void *entry_get_factory(const char *factory_id) {
    if (!strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID))
        return &s_plugin_factory;
    return NULL;
}

// This symbol will be resolved by the host
CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    .clap_version = CLAP_VERSION_INIT,
    .init = entry_init,
    .deinit = entry_deinit,
    .get_factory = entry_get_factory,
};
//}
