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

#ifndef _FLUIDSYNTH_VOICE_H
#define _FLUIDSYNTH_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup voices Voice Manipulation
 * @ingroup soundfonts
 *
 * Synthesis voice manipulation functions.
 *
 * The interface to the synthesizer's voices.
 * Examples on using them can be found in the source code of the default SoundFont
 * loader (fluid_defsfont.c).
 *
 * Most of these functions should only be called from within synthesis context,
 * such as the SoundFont loader's noteon method.
 *
 * @{
 */

/**
 * Enum used with fluid_voice_add_mod() to specify how to handle duplicate modulators.
 */
enum fluid_voice_add_mod
{
    FLUID_VOICE_OVERWRITE,        /**< Overwrite any existing matching modulator */
    FLUID_VOICE_ADD,              /**< Add (sum) modulator amounts */
    FLUID_VOICE_DEFAULT           /**< For default modulators only, no need to check for duplicates */
};

/**
 * Enum indicating the reason a voice callback was invoked.
 *
 * @since 2.5.4
 */
enum fluid_voice_callback_reason
{
    FLUID_VOICE_CALLBACK_NOTEOFF,   /**< A true noteoff was processed for this voice, i.e. the voice
                                         is neither sustained nor sostenutoed and has entered
                                         its release phase. */
    FLUID_VOICE_CALLBACK_FINISHED   /**< The voice has finished playing and is about to be
                                         removed from the DSP loop. */
};

/**
 * Callback function type for voice events.
 *
 * @param voice The voice instance that triggered the callback.
 * @param reason The reason why the callback was invoked (see #fluid_voice_callback_reason).
 * @param data User-defined data pointer as passed to fluid_voice_set_callback().
 *
 * @note This callback is invoked from the synthesis thread context.
 *       The callback implementation must not call any FluidSynth API function
 *       that could trigger voice or synth modifications, as this may lead to
 *       deadlocks or data corruption. The callback should be kept as short as
 *       possible.
 *
 * @since 2.5.4
 */
typedef void (*fluid_voice_callback_t)(fluid_voice_t *voice, enum fluid_voice_callback_reason reason, void *data);

FLUIDSYNTH_API void fluid_voice_add_mod(fluid_voice_t *voice, fluid_mod_t *mod, int mode);
FLUIDSYNTH_API float fluid_voice_gen_get(fluid_voice_t *voice, int gen);
FLUIDSYNTH_API void fluid_voice_gen_set(fluid_voice_t *voice, int gen, float val);
FLUIDSYNTH_API void fluid_voice_gen_incr(fluid_voice_t *voice, int gen, float val);

FLUIDSYNTH_API unsigned int fluid_voice_get_id(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_get_channel(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_get_key(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_get_actual_key(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_get_velocity(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_get_actual_velocity(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_is_playing(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_is_on(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_is_sustained(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_is_sostenuto(const fluid_voice_t *voice);
FLUIDSYNTH_API int fluid_voice_optimize_sample(fluid_sample_t *s);
FLUIDSYNTH_API void fluid_voice_update_param(fluid_voice_t *voice, int gen);
FLUIDSYNTH_API void fluid_voice_set_callback(fluid_voice_t *voice, fluid_voice_callback_t callback, void *data);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_VOICE_H */

