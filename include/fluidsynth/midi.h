/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef _FLUIDSYNTH_MIDI_H
#define _FLUIDSYNTH_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup midi_input MIDI Input
 *
 * MIDI Input Subsystem
 *
 * There are multiple ways to send MIDI events to the synthesizer. They can come
 * from MIDI files, from external MIDI sequencers or raw MIDI event sources,
 * can be modified via MIDI routers and also generated manually.
 *
 * The interface connecting all sources and sinks of MIDI events in libfluidsynth
 * is \ref handle_midi_event_func_t.
 *
 * @{
 */

/**
 * Generic callback function for MIDI event handler.
 *
 * @param data User defined data pointer
 * @param event The MIDI event
 * @return Should return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * This callback is used to pass MIDI events
 * - from \ref midi_player, \ref midi_router or \ref midi_driver
 * - to  \ref midi_router via fluid_midi_router_handle_midi_event()
 * - or to \ref synth via fluid_synth_handle_midi_event().
 *
 * Additionally, there is a translation layer to pass MIDI events to
 * a \ref sequencer via fluid_sequencer_add_midi_event_to_buffer().
 */
typedef int (*handle_midi_event_func_t)(void *data, fluid_midi_event_t *event);

/**
 * Generic callback function fired once by MIDI tick change.
 *
 * @param data User defined data pointer
 * @param tick The current (zero-based) tick, which triggered the callback
 * @return Should return #FLUID_OK on success, #FLUID_FAILED otherwise
 *
 * This callback is fired at a constant rate depending on the current BPM and PPQ.
 * e.g. for PPQ = 192 and BPM = 140 the callback is fired 192 * 140 times per minute (448/sec).
 *
 * It can be used to sync external elements with the beat,
 * or stop / loop the song on a given tick.
 * Ticks being BPM-dependent, you can manipulate values such as bars or beats,
 * without having to care about BPM.
 *
 * For example, this callback loops the song whenever it reaches the 5th bar :
 *
 * @code{.cpp}
int handle_tick(void *data, int tick)
{
    fluid_player_t *player = (fluid_player_t *)data;
    int ppq = 192; // From MIDI header
    int beatsPerBar = 4; // From the song's time signature
    int loopBar = 5;
    int loopTick = (loopBar - 1) * ppq * beatsPerBar;

    if (tick == loopTick)
    {
        return fluid_player_seek(player, 0);
    }

    return FLUID_OK;
}
 * @endcode
 */
typedef int (*handle_midi_tick_func_t)(void *data, int tick);
/** @} */

/**
 * @defgroup midi_events MIDI Events
 * @ingroup midi_input
 *
 * Functions to create, modify, query and delete MIDI events.
 *
 * These functions are intended to be used in MIDI routers and other filtering
 * and processing functions in the MIDI event path. If you want to simply
 * send MIDI messages to the synthesizer, you can use the more convenient
 * \ref midi_messages interface.
 *
 * @{
 */
/** @startlifecycle{MIDI Event} */
FLUIDSYNTH_API fluid_midi_event_t *new_fluid_midi_event(void);
FLUIDSYNTH_API void delete_fluid_midi_event(fluid_midi_event_t *event);
/** @endlifecycle */

FLUIDSYNTH_API int fluid_midi_event_set_type(fluid_midi_event_t *evt, int type);
FLUIDSYNTH_API int fluid_midi_event_get_type(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_channel(fluid_midi_event_t *evt, int chan);
FLUIDSYNTH_API int fluid_midi_event_get_channel(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_get_key(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_key(fluid_midi_event_t *evt, int key);
FLUIDSYNTH_API int fluid_midi_event_get_velocity(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_velocity(fluid_midi_event_t *evt, int vel);
FLUIDSYNTH_API int fluid_midi_event_get_control(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_control(fluid_midi_event_t *evt, int ctrl);
FLUIDSYNTH_API int fluid_midi_event_get_value(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_value(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_program(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_program(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_pitch(const fluid_midi_event_t *evt);
FLUIDSYNTH_API int fluid_midi_event_set_pitch(fluid_midi_event_t *evt, int val);
FLUIDSYNTH_API int fluid_midi_event_set_sysex(fluid_midi_event_t *evt, void *data,
        int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_set_text(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_get_text(fluid_midi_event_t *evt,
        void **data, int *size);
FLUIDSYNTH_API int fluid_midi_event_set_lyrics(fluid_midi_event_t *evt,
        void *data, int size, int dynamic);
FLUIDSYNTH_API int fluid_midi_event_get_lyrics(fluid_midi_event_t *evt,
        void **data, int *size);
/** @} */

/**
 * @defgroup midi_router MIDI Router
 * @ingroup midi_input
 *
 * Rule based transformation and filtering of MIDI events.
 *
 * @{
 */

/**
 * MIDI router rule type.
 *
 * @since 1.1.0
 */
typedef enum
{
    FLUID_MIDI_ROUTER_RULE_NOTE,                  /**< MIDI note rule */
    FLUID_MIDI_ROUTER_RULE_CC,                    /**< MIDI controller rule */
    FLUID_MIDI_ROUTER_RULE_PROG_CHANGE,           /**< MIDI program change rule */
    FLUID_MIDI_ROUTER_RULE_PITCH_BEND,            /**< MIDI pitch bend rule */
    FLUID_MIDI_ROUTER_RULE_CHANNEL_PRESSURE,      /**< MIDI channel pressure rule */
    FLUID_MIDI_ROUTER_RULE_KEY_PRESSURE,          /**< MIDI key pressure rule */
    FLUID_MIDI_ROUTER_RULE_COUNT                  /**< @internal Total count of rule types. This symbol
                                                    is not part of the public API and ABI stability
                                                    guarantee and may change at any time!*/
} fluid_midi_router_rule_type;


/** @startlifecycle{MIDI Router} */
FLUIDSYNTH_API fluid_midi_router_t *new_fluid_midi_router(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);
FLUIDSYNTH_API void delete_fluid_midi_router(fluid_midi_router_t *handler);
/** @endlifecycle */

FLUIDSYNTH_API int fluid_midi_router_set_default_rules(fluid_midi_router_t *router);
FLUIDSYNTH_API int fluid_midi_router_clear_rules(fluid_midi_router_t *router);
FLUIDSYNTH_API int fluid_midi_router_add_rule(fluid_midi_router_t *router,
        fluid_midi_router_rule_t *rule, int type);


/** @startlifecycle{MIDI Router Rule} */
FLUIDSYNTH_API fluid_midi_router_rule_t *new_fluid_midi_router_rule(void);
FLUIDSYNTH_API void delete_fluid_midi_router_rule(fluid_midi_router_rule_t *rule);
/** @endlifecycle */

FLUIDSYNTH_API void fluid_midi_router_rule_set_chan(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API void fluid_midi_router_rule_set_param1(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API void fluid_midi_router_rule_set_param2(fluid_midi_router_rule_t *rule,
        int min, int max, float mul, int add);
FLUIDSYNTH_API int fluid_midi_router_handle_midi_event(void *data, fluid_midi_event_t *event);
FLUIDSYNTH_API int fluid_midi_dump_prerouter(void *data, fluid_midi_event_t *event);
FLUIDSYNTH_API int fluid_midi_dump_postrouter(void *data, fluid_midi_event_t *event);
/** @} */

/**
 * @defgroup midi_driver MIDI Driver
 * @ingroup midi_input
 *
 * Functions for managing MIDI drivers.
 *
 * The available MIDI drivers depend on your platform. See \ref settings_midi for all
 * available configuration options.
 *
 * To create a MIDI driver, you need to specify a source for the MIDI events to be
 * forwarded to via the \ref fluid_midi_event_t callback. Normally this will be
 * either a \ref midi_router via fluid_midi_router_handle_midi_event() or the synthesizer
 * via fluid_synth_handle_midi_event().
 *
 * But you can also write your own handler function that preprocesses the events and
 * forwards them on to the router or synthesizer instead.
 *
 * @{
 */

/** @startlifecycle{MIDI Driver} */
FLUIDSYNTH_API
fluid_midi_driver_t *new_fluid_midi_driver(fluid_settings_t *settings,
        handle_midi_event_func_t handler,
        void *event_handler_data);

FLUIDSYNTH_API void delete_fluid_midi_driver(fluid_midi_driver_t *driver);
/** @endlifecycle */

/** @} */

/**
 * @defgroup midi_player MIDI File Player
 * @ingroup midi_input
 *
 * Parse standard MIDI files and emit MIDI events.
 *
 * @{
 */

/**
 * MIDI File Player status enum.
 * @since 1.1.0
 */
enum fluid_player_status
{
    FLUID_PLAYER_READY,           /**< Player is ready */
    FLUID_PLAYER_PLAYING,         /**< Player is currently playing */
    FLUID_PLAYER_STOPPING,        /**< Player is stopping, but hasn't finished yet (currently unused) */
    FLUID_PLAYER_DONE             /**< Player is finished playing */
};

/**
 * MIDI File Player tempo enum.
 * @since 2.2.0
 */
enum fluid_player_set_tempo_type
{
    FLUID_PLAYER_TEMPO_INTERNAL,      /**< Use midi file tempo set in midi file (120 bpm by default). Multiplied by a factor */
    FLUID_PLAYER_TEMPO_EXTERNAL_BPM,  /**< Set player tempo in bpm, supersede midi file tempo */
    FLUID_PLAYER_TEMPO_EXTERNAL_MIDI, /**< Set player tempo in us per quarter note, supersede midi file tempo */
    FLUID_PLAYER_TEMPO_NBR        /**< @internal Value defines the count of player tempo type (#fluid_player_set_tempo_type) @warning This symbol is not part of the public API and ABI stability guarantee and may change at any time! */
};

enum fluid_midi_control_change
{
    BANK_SELECT_MSB = 0x00,
    MODULATION_MSB = 0x01,
    BREATH_MSB = 0x02,
    FOOT_MSB = 0x04,
    PORTAMENTO_TIME_MSB = 0x05,
    DATA_ENTRY_MSB = 0x06,
    VOLUME_MSB = 0x07,
    BALANCE_MSB = 0x08,
    PAN_MSB = 0x0A,
    EXPRESSION_MSB = 0x0B,
    EFFECTS1_MSB = 0x0C,
    EFFECTS2_MSB = 0x0D,
    GPC1_MSB = 0x10, /* general purpose controller */
    GPC2_MSB = 0x11,
    GPC3_MSB = 0x12,
    GPC4_MSB = 0x13,
    BANK_SELECT_LSB = 0x20,
    MODULATION_WHEEL_LSB = 0x21,
    BREATH_LSB = 0x22,
    FOOT_LSB = 0x24,
    PORTAMENTO_TIME_LSB = 0x25,
    DATA_ENTRY_LSB = 0x26,
    VOLUME_LSB = 0x27,
    BALANCE_LSB = 0x28,
    PAN_LSB = 0x2A,
    EXPRESSION_LSB = 0x2B,
    EFFECTS1_LSB = 0x2C,
    EFFECTS2_LSB = 0x2D,
    GPC1_LSB = 0x30,
    GPC2_LSB = 0x31,
    GPC3_LSB = 0x32,
    GPC4_LSB = 0x33,
    SUSTAIN_SWITCH = 0x40,
    PORTAMENTO_SWITCH = 0x41,
    SOSTENUTO_SWITCH = 0x42,
    SOFT_PEDAL_SWITCH = 0x43,
    LEGATO_SWITCH = 0x44,
    HOLD2_SWITCH = 0x45,
    SOUND_CTRL1 = 0x46,
    SOUND_CTRL2 = 0x47,
    SOUND_CTRL3 = 0x48,
    SOUND_CTRL4 = 0x49,
    SOUND_CTRL5 = 0x4A,
    SOUND_CTRL6 = 0x4B,
    SOUND_CTRL7 = 0x4C,
    SOUND_CTRL8 = 0x4D,
    SOUND_CTRL9 = 0x4E,
    SOUND_CTRL10 = 0x4F,
    GPC5 = 0x50,
    GPC6 = 0x51,
    GPC7 = 0x52,
    GPC8 = 0x53,
    PORTAMENTO_CTRL = 0x54,
    EFFECTS_DEPTH1 = 0x5B,
    EFFECTS_DEPTH2 = 0x5C,
    EFFECTS_DEPTH3 = 0x5D,
    EFFECTS_DEPTH4 = 0x5E,
    EFFECTS_DEPTH5 = 0x5F,
    DATA_ENTRY_INCR = 0x60,
    DATA_ENTRY_DECR = 0x61,
    NRPN_LSB = 0x62,
    NRPN_MSB = 0x63,
    RPN_LSB = 0x64,
    RPN_MSB = 0x65,
    ALL_SOUND_OFF = 0x78,
    ALL_CTRL_OFF = 0x79,
    LOCAL_CONTROL = 0x7A,
    ALL_NOTES_OFF = 0x7B,
    OMNI_OFF = 0x7C,
    OMNI_ON = 0x7D,
    POLY_OFF = 0x7E,
    POLY_ON = 0x7F
};

/** @startlifecycle{MIDI File Player} */
FLUIDSYNTH_API fluid_player_t *new_fluid_player(fluid_synth_t *synth);
FLUIDSYNTH_API void delete_fluid_player(fluid_player_t *player);
/** @endlifecycle */

FLUIDSYNTH_API int fluid_player_add(fluid_player_t *player, const char *midifile);
FLUIDSYNTH_API int fluid_player_add_mem(fluid_player_t *player, const void *buffer, size_t len);
FLUIDSYNTH_API int fluid_player_play(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_stop(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_join(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_set_loop(fluid_player_t *player, int loop);
FLUIDSYNTH_API int fluid_player_set_tempo(fluid_player_t *player, int tempo_type, double tempo);
FLUID_DEPRECATED FLUIDSYNTH_API int fluid_player_set_midi_tempo(fluid_player_t *player, int tempo);
FLUID_DEPRECATED FLUIDSYNTH_API int fluid_player_set_bpm(fluid_player_t *player, int bpm);
FLUIDSYNTH_API int fluid_player_set_playback_callback(fluid_player_t *player, handle_midi_event_func_t handler, void *handler_data);
FLUIDSYNTH_API int fluid_player_set_tick_callback(fluid_player_t *player, handle_midi_tick_func_t handler, void *handler_data);

FLUIDSYNTH_API int fluid_player_get_status(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_current_tick(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_total_ticks(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_bpm(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_division(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_get_midi_tempo(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_seek(fluid_player_t *player, int ticks);
FLUIDSYNTH_API int fluid_player_set_track_mute(fluid_player_t *player, unsigned char track, char boolean);
FLUIDSYNTH_API int fluid_player_set_channel_mute(fluid_player_t *player, unsigned char chan, char boolean);
FLUIDSYNTH_API int fluid_player_set_track_solo(fluid_player_t *player, unsigned char track, char boolean);
FLUIDSYNTH_API int fluid_player_set_channel_solo(fluid_player_t *player, unsigned char chan, char boolean);
FLUIDSYNTH_API int fluid_player_reset_channel_transpose_status(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_reset_channel_mute(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_reset_track_mute(fluid_player_t *player);
FLUIDSYNTH_API int fluid_player_set_transpose_amount(fluid_player_t *player, char amount);
FLUIDSYNTH_API int fluid_player_set_channel_transpose_status(fluid_player_t *player, unsigned char chan, char boolean);
FLUIDSYNTH_API int fluid_player_get_channel_transpose_status(fluid_player_t *player, unsigned char chan);
FLUIDSYNTH_API int fluid_player_get_transpose_amount(fluid_player_t *player, int* val);
FLUIDSYNTH_API int fluid_player_get_channel_mute(fluid_player_t *player, unsigned char chan);
FLUIDSYNTH_API int fluid_player_get_track_mute(fluid_player_t *player, unsigned char track);
FLUIDSYNTH_API int fluid_player_get_track_solo(fluid_player_t *player, unsigned char track);
FLUIDSYNTH_API int fluid_player_get_channel_solo(fluid_player_t *player, unsigned char chan);
FLUIDSYNTH_API int fluid_player_get_track_count(fluid_player_t* player);
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _FLUIDSYNTH_MIDI_H */
