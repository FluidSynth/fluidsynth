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

#ifndef _FLUIDSYNTH_MOD_H
#define _FLUIDSYNTH_MOD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup modulators SoundFont Modulators
 * @ingroup soundfonts
 *
 * SoundFont modulator functions and constants.
 *
 * @{
 */

/**
 * Flags defining the polarity, mapping function and type of a modulator source.
 * Compare with SoundFont 2.04 PDF section 8.2.
 *
 * Note: Bit values do not correspond to the SoundFont spec!  Also note that
 * #FLUID_MOD_GC and #FLUID_MOD_CC are in the flags field instead of the source field.
 */
enum fluid_mod_flags
{
    FLUID_MOD_POSITIVE = 0,       /**< Mapping function is positive */
    FLUID_MOD_NEGATIVE = 1,       /**< Mapping function is negative */
    FLUID_MOD_UNIPOLAR = 0,       /**< Mapping function is unipolar */
    FLUID_MOD_BIPOLAR = 2,        /**< Mapping function is bipolar */
    FLUID_MOD_LINEAR = 0,         /**< Linear mapping function */
    FLUID_MOD_CONCAVE = 4,        /**< Concave mapping function */
    FLUID_MOD_CONVEX = 8,         /**< Convex mapping function */
    FLUID_MOD_SWITCH = 12,        /**< Switch (on/off) mapping function */
    FLUID_MOD_GC = 0,             /**< General controller source type (#fluid_mod_src) */
    FLUID_MOD_CC = 16,             /**< MIDI CC controller (source will be a MIDI CC number) */

    FLUID_MOD_CUSTOM = 0x40,       /**< Custom mapping function */
    FLUID_MOD_SIN = 0x80,          /**< Custom non-standard sinus mapping function @deprecated Deprecated and non-functional since 2.5.0, use #FLUID_MOD_CUSTOM and fluid_mod_set_custom_mapping() instead. */
};

/**
 * Transform types for the SoundFont2 modulators as defined by SoundFont 2.04 section 8.3.
 */
enum fluid_mod_transforms
{
    FLUID_MOD_TRANSFORM_LINEAR = 0, /**< Linear: directly add the computed value to summing node */
    FLUID_MOD_TRANSFORM_ABS = 2     /**< Abs: add the absolute value of the computed to summing node */
};

/**
 * General controller (if #FLUID_MOD_GC in flags).  This
 * corresponds to SoundFont 2.04 PDF section 8.2.1
 */
enum fluid_mod_src
{
    FLUID_MOD_NONE = 0,                   /**< No source controller */
    FLUID_MOD_VELOCITY = 2,               /**< MIDI note-on velocity */
    FLUID_MOD_KEY = 3,                    /**< MIDI note-on note number */
    FLUID_MOD_KEYPRESSURE = 10,           /**< MIDI key pressure */
    FLUID_MOD_CHANNELPRESSURE = 13,       /**< MIDI channel pressure */
    FLUID_MOD_PITCHWHEEL = 14,            /**< Pitch wheel */
    FLUID_MOD_PITCHWHEELSENS = 16         /**< Pitch wheel sensitivity */
};

/**
 * This function transforms or maps a modulator source value into a normalized range of <code>[-1.0;+1.0]</code>.
 *
 * See fluid_mod_set_custom_mapping().
 *
 * @param mod The modulator instance. The behavior is undefined if you modify @p mod through any of the <code>fluid_mod_set*()</code> functions from within the callback.
 * @param value The input value from the modulator source, which will be in range <code>[0;16383]</code> if the input source value is #FLUID_MOD_PITCHWHEEL, or <code>[0;127]</code> otherwise.
 * @param range The value-range of the modulator source, i.e. <code>16384</code>, if the input source value is #FLUID_MOD_PITCHWHEEL, otherwise <code>128</code>.
 * @param data Custom data pointer, as supplied via fluid_mod_set_custom_mapping().
 * @param is_src1 A boolean, which, if true, indicates that the mapping function is called for source1. Otherwise, it's called for source2. Only useful if two sources have been specified with the #FLUID_MOD_CUSTOM flag set.
 * @return A value mapped into range <code>[-1.0;+1.0]</code>. For return values that exceed the mentioned range, the behavior is unspecified
 * (i.e. it may be honored, it may be clipped, ignored, the entire modulator may be disabled, etc.).
 * @since 2.5.0
 */
typedef double (*fluid_mod_mapping_t)(const fluid_mod_t *mod, int value, int range, int is_src1, void *data);

/** @startlifecycle{Modulator} */
FLUIDSYNTH_API fluid_mod_t *new_fluid_mod(void);
FLUIDSYNTH_API void delete_fluid_mod(fluid_mod_t *mod);
/** @endlifecycle */

FLUIDSYNTH_API size_t fluid_mod_sizeof(void);

FLUIDSYNTH_API void fluid_mod_set_source1(fluid_mod_t *mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_source2(fluid_mod_t *mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_dest(fluid_mod_t *mod, int dst);
FLUIDSYNTH_API void fluid_mod_set_amount(fluid_mod_t *mod, double amount);
FLUIDSYNTH_API void fluid_mod_set_transform(fluid_mod_t *mod, int type);
FLUIDSYNTH_API void fluid_mod_set_custom_mapping(fluid_mod_t *mod, fluid_mod_mapping_t mapping_function, void* data);

FLUIDSYNTH_API int fluid_mod_get_source1(const fluid_mod_t *mod);
FLUIDSYNTH_API int fluid_mod_get_flags1(const fluid_mod_t *mod);
FLUIDSYNTH_API int fluid_mod_get_source2(const fluid_mod_t *mod);
FLUIDSYNTH_API int fluid_mod_get_flags2(const fluid_mod_t *mod);
FLUIDSYNTH_API int fluid_mod_get_dest(const fluid_mod_t *mod);
FLUIDSYNTH_API double fluid_mod_get_amount(const fluid_mod_t *mod);
FLUIDSYNTH_API int fluid_mod_get_transform(const fluid_mod_t *mod);

FLUIDSYNTH_API int fluid_mod_test_identity(const fluid_mod_t *mod1, const fluid_mod_t *mod2);
FLUIDSYNTH_API int fluid_mod_has_source(const fluid_mod_t *mod, int cc, int ctrl);
FLUIDSYNTH_API int fluid_mod_has_dest(const fluid_mod_t *mod, int gen);

FLUIDSYNTH_API void fluid_mod_clone(fluid_mod_t *mod, const fluid_mod_t *src);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_MOD_H */

