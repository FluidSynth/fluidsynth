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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_mod.h"
#include "fluid_chan.h"
#include "fluid_voice.h"

/**
 * Clone the modulators destination, sources, flags and amount.
 * @param mod the modulator to store the copy to
 * @param src the source modulator to retrieve the information from
 * @note The \c next member of \c mod will be left unchanged.
 */
void
fluid_mod_clone(fluid_mod_t *mod, const fluid_mod_t *src)
{
    mod->dest = src->dest;
    mod->src1 = src->src1;
    mod->flags1 = src->flags1;
    mod->src2 = src->src2;
    mod->flags2 = src->flags2;
    mod->amount = src->amount;
}

/**
 * Set a modulator's primary source controller and flags.
 * @param mod The modulator instance
 * @param src Modulator source (#fluid_mod_src or a MIDI controller number)
 * @param flags Flags determining mapping function and whether the source
 *   controller is a general controller (#FLUID_MOD_GC) or a MIDI CC controller
 *   (#FLUID_MOD_CC), see #fluid_mod_flags.
 */
void
fluid_mod_set_source1(fluid_mod_t *mod, int src, int flags)
{
    mod->src1 = src;
    mod->flags1 = flags;
}

/**
 * Set a modulator's secondary source controller and flags.
 * @param mod The modulator instance
 * @param src Modulator source (#fluid_mod_src or a MIDI controller number)
 * @param flags Flags determining mapping function and whether the source
 *   controller is a general controller (#FLUID_MOD_GC) or a MIDI CC controller
 *   (#FLUID_MOD_CC), see #fluid_mod_flags.
 */
void
fluid_mod_set_source2(fluid_mod_t *mod, int src, int flags)
{
    mod->src2 = src;
    mod->flags2 = flags;
}

/**
 * Set the destination effect of a modulator.
 * @param mod The modulator instance
 * @param dest Destination generator (#fluid_gen_type)
 */
void
fluid_mod_set_dest(fluid_mod_t *mod, int dest)
{
    mod->dest = dest;
}

/**
 * Set the scale amount of a modulator.
 * @param mod The modulator instance
 * @param amount Scale amount to assign
 */
void
fluid_mod_set_amount(fluid_mod_t *mod, double amount)
{
    mod->amount = (double) amount;
}

/**
 * Get the primary source value from a modulator.
 * @param mod The modulator instance
 * @return The primary source value (#fluid_mod_src or a MIDI CC controller value).
 */
int
fluid_mod_get_source1(const fluid_mod_t *mod)
{
    return mod->src1;
}

/**
 * Get primary source flags from a modulator.
 * @param mod The modulator instance
 * @return The primary source flags (#fluid_mod_flags).
 */
int
fluid_mod_get_flags1(const fluid_mod_t *mod)
{
    return mod->flags1;
}

/**
 * Get the secondary source value from a modulator.
 * @param mod The modulator instance
 * @return The secondary source value (#fluid_mod_src or a MIDI CC controller value).
 */
int
fluid_mod_get_source2(const fluid_mod_t *mod)
{
    return mod->src2;
}

/**
 * Get secondary source flags from a modulator.
 * @param mod The modulator instance
 * @return The secondary source flags (#fluid_mod_flags).
 */
int
fluid_mod_get_flags2(const fluid_mod_t *mod)
{
    return mod->flags2;
}

/**
 * Get destination effect from a modulator.
 * @param mod The modulator instance
 * @return Destination generator (#fluid_gen_type)
 */
int
fluid_mod_get_dest(const fluid_mod_t *mod)
{
    return mod->dest;
}

/**
 * Get the scale amount from a modulator.
 * @param mod The modulator instance
 * @return Scale amount
 */
double
fluid_mod_get_amount(const fluid_mod_t *mod)
{
    return (double) mod->amount;
}

/*
 * returns the number of member modulators inside a simple or 
 * complex modulator (i.e linked modulators).
 * @param mod, pointer on modulator. 
 * @return  number of modulators.
 *  Must be > 1 for complex modulator and 1 for unlinked modulator.
 */
int fluid_mod_get_linked_count(const fluid_mod_t *mod)
{
    int count = 0;
    do
    {
        mod = mod->next;
        count++;
    }
    while(mod && (mod->dest & FLUID_MOD_LINK_DEST));
    return count;
}

/*
 * returns the number of modulators inside a list.
 * @param mod, pointer on modulator list.
 * @return  number of modulators.
 */
int fluid_mod_get_list_count(const fluid_mod_t *mod)
{
    int count = 0;
    while(mod)
    {
        count++;
        mod = mod->next;
    }
    return count;
}

/*
 * returns next modulator following current modulator.
 * If mod is a complex linked modulator all members are skipped

 * @param mod, pointer on modulator. 
 * @return  next modulator or NULL if mod is the last modulator.
 */
fluid_mod_t *fluid_mod_get_next(fluid_mod_t *mod)
{
    do
    {
        mod = mod->next;
    }
    while(mod && (mod->dest & FLUID_MOD_LINK_DEST));
    return mod;
}

/*
 * returns TRUE if modulator source src1 is linked, FALSE otherwise.
 */
int fluid_mod_has_linked_src1 (const fluid_mod_t * mod)
{
    return(((mod->flags1 & FLUID_MOD_CC) == 0)
             /* SF2.04 section 8.2.1: Constant value */
             &&  (mod->src1 == FLUID_MOD_LINK_SRC)) ;   
}

/*
 * returns TRUE if the modulator is linked (i.e member of a complex modulator),
 * FALSE otherwise (i.e the modulator is simple).
 * A modulator is linked when source src1 is linked or destination is linked.
 */
int fluid_mod_is_linked (const fluid_mod_t * mod)
{
    return (fluid_mod_has_linked_src1(mod) || (mod->dest & FLUID_MOD_LINK_DEST));
}

/*
 * retrieves the initial value from the given source of the modulator
 */
static fluid_real_t
fluid_mod_get_source_value(const unsigned char mod_src,
                           const unsigned char mod_flags,
                           fluid_real_t *range,
                           const fluid_voice_t *voice
                          )
{
    const fluid_channel_t *chan = voice->channel;
    fluid_real_t val;

    if(mod_flags & FLUID_MOD_CC)
    {
        /* From MIDI Recommended Practice (RP-036) Default Pan Formula:
         * "Since MIDI controller values range from 0 to 127, the exact center
         * of the range, 63.5, cannot be represented. Therefore, the effective
         * range for CC#10 is modified to be 1 to 127, and values 0 and 1 both
         * pan hard left. The recommended method is to subtract 1 from the
         * value of CC#10, and saturate the result to be non-negative."
         *
         * We treat the balance control in exactly the same way, as the same
         * problem applies here as well.
         */
        if(mod_src == PAN_MSB || mod_src == BALANCE_MSB)
        {
            *range = 126;
            val = fluid_channel_get_cc(chan, mod_src) - 1;

            if(val < 0)
            {
                val = 0;
            }
        }
        else
        {
            val = fluid_channel_get_cc(chan, mod_src);
        }
    }
    else
    {
        switch(mod_src)
        {
        case FLUID_MOD_NONE:         /* SF 2.01 8.2.1 item 0: src enum=0 => value is 1 */
            val = *range;
            break;

        case FLUID_MOD_VELOCITY:
            val = fluid_voice_get_actual_velocity(voice);
            break;

        case FLUID_MOD_KEY:
            val = fluid_voice_get_actual_key(voice);
            break;

        case FLUID_MOD_KEYPRESSURE:
            val = fluid_channel_get_key_pressure(chan, voice->key);
            break;

        case FLUID_MOD_CHANNELPRESSURE:
            val = fluid_channel_get_channel_pressure(chan);
            break;

        case FLUID_MOD_PITCHWHEEL:
            val = fluid_channel_get_pitch_bend(chan);
            *range = 0x4000;
            break;

        case FLUID_MOD_PITCHWHEELSENS:
            val = fluid_channel_get_pitch_wheel_sensitivity(chan);
            break;

        default:
            FLUID_LOG(FLUID_ERR, "Unknown modulator source '%d', disabling modulator.", mod_src);
            val = 0.0;
        }
    }

    return val;
}

/**
 * transforms the initial value retrieved by \c fluid_mod_get_source_value into [0.0;1.0]
 */
static fluid_real_t
fluid_mod_transform_source_value(fluid_real_t val, unsigned char mod_flags, const fluid_real_t range)
{
    /* normalized value, i.e. usually in the range [0;1] */
    const fluid_real_t val_norm = val / range;

    /* we could also only switch case the lower nibble of mod_flags, however
     * this would keep us from adding further mod types in the future
     *
     * instead just remove the flag(s) we already took care of
     */
    mod_flags &= ~FLUID_MOD_CC;

    switch(mod_flags/* & 0x0f*/)
    {
    case FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =0 */
        val = val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =1 */
        val = 1.0f - val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =2 */
        val = -1.0f + 2.0f * val_norm;
        break;

    case FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =3 */
        val = 1.0f - 2.0f * val_norm;
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =4 */
        val = fluid_concave(127 * (val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =5 */
        val = fluid_concave(127 * (1.0f - val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =6 */
        val = (val_norm > 0.5f) ?  fluid_concave(127 * 2 * (val_norm - 0.5f))
              : -fluid_concave(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONCAVE | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =7 */
        val = (val_norm > 0.5f) ? -fluid_concave(127 * 2 * (val_norm - 0.5f))
              :  fluid_concave(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =8 */
        val = fluid_convex(127 * (val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =9 */
        val = fluid_convex(127 * (1.0f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =10 */
        val = (val_norm > 0.5f) ?  fluid_convex(127 * 2 * (val_norm - 0.5f))
              : -fluid_convex(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_CONVEX | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =11 */
        val = (val_norm > 0.5f) ? -fluid_convex(127 * 2 * (val_norm - 0.5f))
              :  fluid_convex(127 * 2 * (0.5f - val_norm));
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* =12 */
        val = (val_norm >= 0.5f) ? 1.0f : 0.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* =13 */
        val = (val_norm >= 0.5f) ? 0.0f : 1.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* =14 */
        val = (val_norm >= 0.5f) ? 1.0f : -1.0f;
        break;

    case FLUID_MOD_SWITCH | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* =15 */
        val = (val_norm >= 0.5f) ? -1.0f : 1.0f;
        break;

    /*
     * MIDI CCs only have a resolution of 7 bits. The closer val_norm gets to 1,
     * the less will be the resulting change of the sinus. When using this sin()
     * for scaling the cutoff frequency, there will be no audible difference between
     * MIDI CCs 118 to 127. To avoid this waste of CCs multiply with 0.87
     * (at least for unipolar) which makes sin() never get to 1.0 but to 0.98 which
     * is close enough.
     */
    case FLUID_MOD_SIN | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE: /* custom sin(x) */
        val = FLUID_SIN((FLUID_M_PI / 2.0f * 0.87f) * val_norm);
        break;

    case FLUID_MOD_SIN | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE: /* custom */
        val = FLUID_SIN((FLUID_M_PI / 2.0f * 0.87f) * (1.0f - val_norm));
        break;

    case FLUID_MOD_SIN | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE: /* custom */
        val = (val_norm > 0.5f) ?  FLUID_SIN(FLUID_M_PI * (val_norm - 0.5f))
              : -FLUID_SIN(FLUID_M_PI * (0.5f - val_norm));
        break;

    case FLUID_MOD_SIN | FLUID_MOD_BIPOLAR | FLUID_MOD_NEGATIVE: /* custom */
        val = (val_norm > 0.5f) ? -FLUID_SIN(FLUID_M_PI * (val_norm - 0.5f))
              :  FLUID_SIN(FLUID_M_PI * (0.5f - val_norm));
        break;

    default:
        FLUID_LOG(FLUID_ERR, "Unknown modulator type '%d', disabling modulator.", mod_flags);
        val = 0.0f;
        break;
    }

    return val;
}

/*
 * fluid_mod_get_value.
 * Computes and return modulator output following SF2.01
 * (See SoundFont Modulator Controller Model Chapter 9.5).
 *
 * Output = Transform(Amount * Map(primary source input) * Map(secondary source input))
 *
 * Notes:
 * 1)fluid_mod_get_value, ignores the Transform operator. The result is:
 *
 *   Output = Amount * Map(primary source input) * Map(secondary source input)
 *
 * 2)When primary source input (src1) is set to General Controller 'No Controller',
 *   output is forced to 0.
 *
 * 3)When secondary source input (src2) is set to General Controller 'No Controller',
 *   output is forced to +1.0 
 */
fluid_real_t
fluid_mod_get_value(fluid_mod_t *mod, fluid_voice_t *voice)
{
    extern fluid_mod_t default_vel2filter_mod;

    fluid_real_t v1 = 0.0, v2 = 1.0;
    fluid_real_t range1 = 127.0, range2 = 127.0;

    /* 'special treatment' for default controller
     *
     *  Reference: SF2.01 section 8.4.2
     *
     * The GM default controller 'vel-to-filter cut off' is not clearly
     * defined: If implemented according to the specs, the filter
     * frequency jumps between vel=63 and vel=64.  To maintain
     * compatibility with existing sound fonts, the implementation is
     * 'hardcoded', it is impossible to implement using only one
     * modulator otherwise.
     *
     * I assume here, that the 'intention' of the paragraph is one
     * octave (1200 cents) filter frequency shift between vel=127 and
     * vel=64.  'amount' is (-2400), at least as long as the controller
     * is set to default.
     *
     * Further, the 'appearance' of the modulator (source enumerator,
     * destination enumerator, flags etc) is different from that
     * described in section 8.4.2, but it matches the definition used in
     * several SF2.1 sound fonts (where it is used only to turn it off).
     * */
    if(fluid_mod_test_identity(mod, &default_vel2filter_mod))
    {
// S. Christian Collins' mod, to stop forcing velocity based filtering
        /*
            if (voice->vel < 64){
              return (fluid_real_t) mod->amount / 2.0;
            } else {
              return (fluid_real_t) mod->amount * (127 - voice->vel) / 127;
            }
        */
        return 0; // (fluid_real_t) mod->amount / 2.0;
    }

// end S. Christian Collins' mod

    /* get the initial value of the first source */
    if(mod->src1 > 0)
    {
        if(fluid_mod_has_linked_src1(mod))
        {
            /* src1 link source isn't mapped (i.e transformed) */
            v1 = mod->link;
        }
        else
        {
            v1 = fluid_mod_get_source_value(mod->src1, mod->flags1, &range1, voice);

            /* transform the input value */
            v1 = fluid_mod_transform_source_value(v1, mod->flags1, range1);
        }
    }
    /* When primary source input (src1) is set to General Controller 'No Controller',
       output is forced to 0.0
    */
    else
    {
        return 0.0;
    }

    /* no need to go further */
    if(v1 == 0.0f)
    {
        return 0.0f;
    }

    /* get the second input source */
    if(mod->src2 > 0)
    {
        v2 = fluid_mod_get_source_value(mod->src2, mod->flags2, &range2, voice);

        /* transform the second input value */
        v2 = fluid_mod_transform_source_value(v2, mod->flags2, range2);
    }
    /* When secondary source input (src2) is set to General Controller 'No Controller',
       output is forced to +1.0
    */
    else
    {
        v2 = 1.0f;
    }

    /* it's as simple as that: */
    return (fluid_real_t) mod->amount * v1 * v2;
}

/**
 * Create a new uninitialized modulator structure.
 * Structure's fields are left uninitialized except 'next' field that is 
 * initialized to NULL.
 * @return New allocated modulator or NULL if out of memory
 */
fluid_mod_t *
new_fluid_mod()
{
    fluid_mod_t *mod = FLUID_NEW(fluid_mod_t);

    if(mod == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }
    mod->next = NULL;

    return mod;
}

/**
 * Free a modulator structure.
 * @param mod Modulator to free
 */
void
delete_fluid_mod(fluid_mod_t *mod)
{
    FLUID_FREE(mod);
}

/*
 * delete list of modulators.
 */
void delete_fluid_mod_list(fluid_mod_t *mod)
{
    fluid_mod_t *tmp;

    while(mod)	/* delete the modulators */
    {
        tmp = mod;
        mod = mod->next;
        delete_fluid_mod(tmp);
    }
}

/**
 * Returns the size of the fluid_mod_t structure.
 *
 * Useful in low latency scenarios e.g. to allocate a modulator on the stack.
 *
 * @return Size of fluid_mod_t in bytes
 */
size_t fluid_mod_sizeof()
{
    return sizeof(fluid_mod_t);
}

/**
 * Checks if modulator with source other than CC is FLUID_MOD_NONE.
 *
 * @param mod, modulator.
 * @param src1_select, source input selection to check.
 *   1 to check src1 source or
 *   0 to check src2 source.
 * @return TRUE if modulator source other than cc is FLUID_MOD_NONE, FALSE otherwise.
 */
static int
fluid_mod_is_src_none(const fluid_mod_t *mod, unsigned char src1_select)
{
    unsigned char flags, src;

    if(src1_select)
    {
        flags = mod->flags1;
        src = mod->src1;
    }
    else
    {
        flags = mod->flags2;
        src = mod->src2;
    }
    return(((flags & FLUID_MOD_CC) == FLUID_MOD_GC) &&
            (src == FLUID_MOD_NONE));
}

/**
 * Checks if modulators source other than CC source is invalid.
 * (specs SF 2.01  7.4, 7.8, 8.2.1)
 *
 * @param mod, modulator.
 * @param src1_select, source input selection to check.
 *   1 to check src1 source.
 *   0 to check src2 source.
 * @return FALSE if selected modulator source other than cc is invalid, TRUE otherwise.
 */
static int
fluid_mod_check_non_cc_source(const fluid_mod_t *mod, unsigned char src1_select)
{
    unsigned char flags, src;

    if(src1_select)
    {
        flags = mod->flags1;
        src = mod->src1;
    }
    else
    {
        flags = mod->flags2;
        src = mod->src2;
    }

    return(((flags & FLUID_MOD_CC) != 0)  /* src is a CC */
           /* SF2.01 section 8.2.1: Constant value */
           || ((src == FLUID_MOD_NONE)
               || (src == FLUID_MOD_VELOCITY)        /* Note-on velocity */
               || (src == FLUID_MOD_KEY)             /* Note-on key number */
               || (src == FLUID_MOD_KEYPRESSURE)     /* Poly pressure */
               || (src == FLUID_MOD_CHANNELPRESSURE) /* Channel pressure */
               || (src == FLUID_MOD_PITCHWHEEL)      /* Pitch wheel */
               || (src == FLUID_MOD_PITCHWHEELSENS)  /* Pitch wheel sensitivity */
               || (src1_select && (src == FLUID_MOD_LINK_SRC)) /* linked source */
              ));
}

/**
 * Checks if modulator CC source is invalid (specs SF 2.01  7.4, 7.8, 8.2.1).
 * @param mod, modulator.
 * @src1_select, source input selection:
 *   1 to check src1 source or
 *   0 to check src2 source.
 * @return FALSE if selected modulator's source CC is invalid, TRUE otherwise.
 */
static int
fluid_mod_check_cc_source(const fluid_mod_t *mod, unsigned char src1_select)
{
    unsigned char flags, src;

    if(src1_select)
    {
        flags = mod->flags1;
        src = mod->src1;
    }
    else
    {
        flags = mod->flags2;
        src = mod->src2;
    }

    return(((flags & FLUID_MOD_CC) == 0)  /* src is non CC */
           || ((src != BANK_SELECT_MSB)
               && (src != BANK_SELECT_LSB)
               && (src != DATA_ENTRY_MSB)
               && (src != DATA_ENTRY_LSB)
               /* is src not NRPN_LSB, NRPN_MSB, RPN_LSB, RPN_MSB */
               && ((src < NRPN_LSB) || (RPN_MSB < src))
               /* is src not ALL_SOUND_OFF, ALL_CTRL_OFF, LOCAL_CONTROL, ALL_NOTES_OFF ? */
               /* is src not OMNI_OFF, OMNI_ON, POLY_OFF, POLY_ON ? */
               && (src < ALL_SOUND_OFF)
               /* CC lsb shouldn't allowed to modulate (spec SF 2.01 - 8.2.1)
                  However, as long fluidsynth will use only CC 7 bits resolution,
                  it is safe to ignore these SF recommendations on CC receive.
                  See explanations in fluid_synth_cc_LOCAL() */
               /* uncomment next line to forbid CC lsb  */
               /* && ((src < 32) || (63 < src)) */
              ));
}

/**
 * Checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1)
 * @param mod, modulator.
 * @param name,if not NULL, pointer on a string displayed as a warning.
 * @return TRUE if modulator sources src1, src2 are valid, FALSE otherwise.
 */
int fluid_mod_check_sources(const fluid_mod_t *mod, char *name)
{
    static const char invalid_non_cc_src[] =
        "Invalid modulator, using non-CC source %s.src%d=%d";
    static const char invalid_cc_src[] =
        "Invalid modulator, using CC source %s.src%d=%d";
    static const char src1_is_none[] =
        "Modulator with source 1 none %s.src1=%d";

    /* checks valid non cc sources */
    if(!fluid_mod_check_non_cc_source(mod, 1)) /* check src1 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_non_cc_src, name, 1, mod->src1);
        }

        return FALSE;
    }

    /* Note: When primary source input src1 is set to General Controller 'No Controller',
      1)The output of this modulator will be forced to 0 at synthesis time (see fluid_mod_get_value()).
      2)Also this modulator cannot be used to overwrite a default modulator (as
      there is no default modulator with src1 source equal to FLUID_MOD_NONE).
      Consequently it is useful to return FALSE to indicate this modulator
      being useless. It will be removed later with others invalid modulators.
	*/
    if(fluid_mod_is_src_none(mod, 1))  /* check src1 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, src1_is_none, name, mod->src1);
        }

        /* Indicate this modulator is useless */
        return FALSE;
    }

    if(!fluid_mod_check_non_cc_source(mod, 0)) /* check src2 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_non_cc_src, name, 2, mod->src2);
        }

        return FALSE;
    }

    /* Note: When secondary source input src2 is set to General Controller 'No Controller',
       output will be forced to +1.0 at synthesis time (see fluid_mod_get_value()).
       That means that this source should behave unipolar only. We need to have the
       unipolar flag to ensure to ensure a correct evaluation of the minimum
       value later (see fluid_voice_get_lower_boundary_for_attenuation()).
    */
    if(fluid_mod_is_src_none(mod, 0)) /* check src2 */
    {
        if (mod->flags2 & FLUID_MOD_BIPOLAR)
        {
            if(name)
            {
                FLUID_LOG(FLUID_WARN, invalid_non_cc_src, name, 2, mod->src2);
            }
            return FALSE;
        }
    }

    /* checks valid cc sources */
    if(!fluid_mod_check_cc_source(mod, 1)) /* check src1 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_cc_src, name, 1, mod->src1);
        }

        return FALSE;
    }

    if(!fluid_mod_check_cc_source(mod, 0)) /* check src2 */
    {
        if(name)
        {
            FLUID_LOG(FLUID_WARN, invalid_cc_src, name, 2, mod->src2);
        }

        return FALSE;
    }

    return TRUE;
}

/*
 * Test if both complex linked modulator branches are identical.
 * The first member of complex modulator is the ending modulator connected
 * to a generator. Other members are branches connected to this first
 * member. The function checks branches only, it doesn't check ending modulator.
 *
 * Note that testing branches identity is based on the internal modulator
 * ordering rule described in fluid_mod_copy_linked_mod().
 * 
 * Example of complex modulator cm0:
 *   gen <-- cm0.m0 <-- cm0.m1 <-- cm0.m2 <-- CC1
 *                  <-- cm0.m3 <-- CC2
 *
 * cm0.m0 is the ending modulator of cm0.
 * cm0.m1 is the modulator following cm0.m1. This is the first modulator of all
 * branches connected to cm0.m0. These branches are cm0.b0, cm0.b1, cm0.b2
 *  (cm0.b0) cm0.m0 <-- cm0.m1 
 *  (cm0.b1)            cm0.m1 <-- cm0.m2 <-- CC1
 *  (cm0.b2) cm0.m0 <-- cm0.m3 <-- CC2
 *
 * Example of complex modulator cm1:
 *   gen <-- cm1.m0 <-- cm1.m1 <-- CC2
 *                  <-- cm1.m2 <-- cm1.m3 <-- CC1
 *          
 * cm1.m0 is the ending modulator of cm1.
 * cm1.m1 is the modulator following cm1.m0. This the first modulator of all
 * branches connected to cm1.m0. These branches are cm1.b0, cm1.b1, cm1.b2
 *
 *  (cm1.b0) cm1.m0 <-- cm1.m1 <-- CC2
 *  (cm1.b1) cm1.m0 <-- cm1.m2
 *  (cm1.b2)            cm1.m2 <-- cm1.m3 <-- CC1
 *
 * Branch cm1.b1 and cm1.b2 are the only one similar to cm0.b0, cm0.b1 respectively.
 * This is true if:
 *   - cm1.m2 has same sources then cm0.m1
 *   - cm1.m3 has same sources then cm0.m2
 *
 * Branch cm1.b0 is the only one similar to cm0.b2. This is true if:
 *   - cm1.m1 has same sources then cm0.m3
 *
 * Note that similar branches in cm0 and cm1 are not necessarly ordered the same way.
 * Branches ordering need not to be same to get identical complex modulators. This
 * ordering comes from the soundfont and is irrelevant (SF 2.01 9.5.4 p 58).
 *
 * @param cm0_mod, pointer on modulator branch 0. Must be the modulator following
 *  cm0 ending modulator. In the figure above this is cm0.m1.
 *
 * @param cm1_mod, pointer on modulator branch 1. Must be the modulator following
 *  cm1 ending modulator. In the figure above this is cm1.m1.
 *
 * @param test_mode,
 *   FLUID_LINKED_MOD_TEST_ONLY,  test identity only.
 *   FLUID_LINKED_MOD_TEST_ADD, modulators amounts on branch1 are added
 *    to identical  modulators on branche0.
 *   FLUID_LINKED_MOD_TEST_OVERWRITE, modulators amounts on branch1 overwrite
 *    identical modulators on branch0.
 *
 * @return TRUE if complex modulators branches are identical, FALSE otherwise.
 */
static int fluid_mod_test_branch_identity(fluid_mod_t *cm0_mod, 
                                             fluid_mod_t *cm1_mod,
                                             unsigned char test_mode)
{
    int r = 1;				 /* result of test */
    struct
    {
        fluid_mod_t *cm1_branch; /* branch modulator for cm1 */
        unsigned char dest1_idx; /* destination index of cm1_branch */
        unsigned char dest0_idx; /* destination index of cm0 branch */
    }branch_level[FLUID_NUM_MOD];
    int state_idx = 0; /* branch state index */

    unsigned char mod0_idx, mod1_idx;  /* index of cm0_mod and cm1_mod */

    /* initialize branches stack state at index 0 */
    branch_level[0].cm1_branch = cm1_mod; /* branch modulator for cm1 */
    /* destination index of cm1 branch */
    branch_level[0].dest1_idx = FLUID_MOD_LINK_DEST;
    /* destination index of cm0 branch  0*/
    mod0_idx = cm0_mod->dest;
    branch_level[0].dest0_idx = mod0_idx;

    /* verify identity of each member of branches */
    mod0_idx++; /* current cm0_mod index */
    while(r)
    {
        unsigned char dest1_idx = branch_level[state_idx].dest1_idx;
        /* Search any modulator cm1_mod identic to cm0_mod */
        r = 0; /* 0, indicates modulators not identic */
        mod1_idx = dest1_idx + 1; /* current cm1_mod index */
        cm1_mod = branch_level[state_idx].cm1_branch;  /* current modulator cm1_mod */
        while(cm1_mod && (cm1_mod->dest >= dest1_idx))
        {   /* is cm1_mod destination equal to ancestor index ? */
            if((cm1_mod->dest == dest1_idx)
            /* is cm0_mod identical to cm1_mod ? */
               && (cm0_mod->flags1 == cm1_mod->flags1)
               && (cm0_mod->src1 == cm1_mod->src1)
               && (cm0_mod->flags2 == cm1_mod->flags2)
               && (cm0_mod->src2 == cm1_mod->src2))
            {
                /* cm0_mod is identic to cm1_mod */
                /* does amount need to be added ? */
                if (test_mode == FLUID_LINKED_MOD_TEST_ADD)
                {
                    cm0_mod->amount += cm1_mod->amount;
                }
                /* does amount need to be overwrited ? */
                else if (test_mode == FLUID_LINKED_MOD_TEST_OVERWRITE)
                {
                    cm0_mod->amount = cm1_mod->amount;
                }
                /* does cm0_mod and cm1_mod have linked source ? */
                if(fluid_mod_has_linked_src1(cm0_mod))
                {
                    /* both cm0_mod and cm1_mod have sources linked, so we go forward
                       to the next branches connected to cm0_mod and cm1_mod inputs
                       respectively */
                    /* goes on next branches */
                    state_idx++;
                    if(state_idx >= FLUID_NUM_MOD)
                    {
                        /* internal limit FLUID_NUM_MOD is exceeded */
                        FLUID_LOG(FLUID_WARN, "branch stack overflow.");
                        return FALSE;
                    }

                    /* next cm1 member on cm1 branch */
                    branch_level[state_idx].cm1_branch = cm1_mod->next;
                    branch_level[state_idx].dest1_idx = mod1_idx;
					
                    /* next cm0 member on cm0 branch */
                    branch_level[state_idx].dest0_idx = mod0_idx;
                    mod0_idx++;	
                    cm0_mod = cm0_mod->next; 

                    /* continues on the new branch */
                    r = 1;
                    break;				 
                 }
                 /* continues on current branches (cm0 and cm1) */ 
			     mod0_idx++;       /* next modulator cm0_mod in cm0 */
                 cm0_mod = cm0_mod->next;
                 /* does last member of cm0 is reached ? */
                 if(!(cm0_mod && (cm0_mod->dest & FLUID_MOD_LINK_DEST)))
                 {                   
                     return TRUE; /* all members are identical */
                 }
                 /* get cmo_mod destination in r */
                 r = cm0_mod->dest;
                 /* unstack branches stack (from state_idx to 0) until we find a modulator
                    index (branch_level[state_idx].dest0_idx) equal to cm0_mod destination.
                    Note: As far the branch_level[0].dest0_idx field have been properly
                    initialized, there is no risk that state_idx become negative
                 */
                 while(r < branch_level[state_idx].dest0_idx)
                 {
                     state_idx--;
                     if(state_idx < 0)
                     {
                         /* internal error: this should never happen */
                         FLUID_LOG(FLUID_ERR, "branch stack underflow.");
                         return FALSE;
                     }
                 }
                 /* continues on the current branch */
                 break;				 
            }
            mod1_idx++; /* next modulator cm1_mod in cm1 */
            cm1_mod = cm1_mod->next;
        }
    }
    /* at least one member is not identical */
    return FALSE;
}

/*
 * Test if both complex linked modulators cm0, cm1 are identical.
 * Both complex modulators are identical when:
 * - the modulators number of cm0 is equal to the modulators number of cm1.
 * - branches of cm0 are identical to branches of cm1.
 *
 * @param cm0, final modulator of a complex linked modulators. 
 *  cm0 must be connected to a generator.
 *
 * @param cm1, final modulator of complex linked modulators.
 *  cm1 must be connected to a generator.
 *
 * @param test_mode,
 *   FLUID_LINKED_MOD_TEST_ONLY,  test identity only.
 *   FLUID_LINKED_MOD_TEST_ADD, modulators amounts of cm1 are added
 *    to identical  modulators of cm0.
 *   FLUID_LINKED_MOD_TEST_OVERWRITE, modulators amounts of cm1 overwrite
 *    identical modulators of cm0.
 *
 * @return TRUE if complex modulators are identical, FALSE otherwise.
 */
int fluid_mod_test_linked_identity(fluid_mod_t *cm0,
                                   fluid_mod_t *cm1, 
                                   unsigned char test_mode)
{
    int count0 = fluid_mod_get_linked_count(cm0);
    int count1 = fluid_mod_get_linked_count(cm1);

    /* test of count and identity of final modulators cm0 and cm1 */
    if((count0 == count1) && (count0 > 1)
        && fluid_mod_test_identity(cm0, cm1))
    {
        /* does amount need to be added ? */
        if (test_mode == FLUID_LINKED_MOD_TEST_ADD)
        {
            cm0->amount += cm1->amount;
        }
        /* does amount need to be overwrited ? */
        else if (test_mode == FLUID_LINKED_MOD_TEST_OVERWRITE)
        {
            cm0->amount = cm1->amount;
        }
        /* identity test of branches of cm0 and cm1 */
        {
            return fluid_mod_test_branch_identity(cm0->next, cm1->next,
                                                     test_mode);
        }
    }
    return FALSE;
}


/**
 * Checks if two modulators are identical in sources, flags and destination.
 * @param mod1 First modulator
 * @param mod2 Second modulator
 * @return TRUE if identical, FALSE otherwise
 *
 * SF2.01 section 9.5.1 page 69, 'bullet' 3 defines 'identical'.
 */
int
fluid_mod_test_identity(const fluid_mod_t *mod1, const fluid_mod_t *mod2)
{
    return mod1->dest == mod2->dest
           && mod1->src1 == mod2->src1
           && mod1->src2 == mod2->src2
           && mod1->flags1 == mod2->flags1
           && mod1->flags2 == mod2->flags2;
}

/**
 * Check if the modulator has the given source.
 *
 * @param mod The modulator instance
 * @param cc Boolean value indicating if ctrl is a CC controller or not
 * @param ctrl The source to check for (if \c cc == FALSE : a value of type #fluid_mod_src, else the value of the MIDI CC to check for)
 *
 * @return TRUE if the modulator has the given source, FALSE otherwise.
 */
int fluid_mod_has_source(const fluid_mod_t *mod, int cc, int ctrl)
{
    return
        (
            (
                ((mod->src1 == ctrl) && ((mod->flags1 & FLUID_MOD_CC) != 0) && (cc != 0))
                || ((mod->src1 == ctrl) && ((mod->flags1 & FLUID_MOD_CC) == 0) && (cc == 0))
            )
            ||
            (
                ((mod->src2 == ctrl) && ((mod->flags2 & FLUID_MOD_CC) != 0) && (cc != 0))
                || ((mod->src2 == ctrl) && ((mod->flags2 & FLUID_MOD_CC) == 0) && (cc == 0))
            )
        );
}

/**
 * Check if the modulator has the given destination.
 * @param mod The modulator instance
 * @param gen The destination generator of type #fluid_gen_type to check for
 * @return TRUE if the modulator has the given destination, FALSE otherwise.
 */
int fluid_mod_has_dest(const fluid_mod_t *mod, int gen)
{
    return mod->dest == gen;
}

/**
 * Checks if modulator mod is identic to another modulator in the list
 * (specs SF 2.0X  7.4, 7.8).
 * @param mod, modulator list.
 * @param name, if not NULL, pointer on a string displayed as warning.
 * @return TRUE if mod is identic to another modulator, FALSE otherwise.
 */
static int
fluid_mod_is_identic_in_list(const fluid_mod_t *mod, char *name)
{
    fluid_mod_t *next = mod->next;

    while(next)
    {
        /* is mod identic to next ? */
        if(fluid_mod_test_identity(mod, next))
        {
            if(name)
            {
                FLUID_LOG(FLUID_WARN, "Ignoring identic modulator %s", name);
            }

            return TRUE;
        }

        next = next->next;
    }

    return FALSE;
}

/**
 * Checks and remove invalid modulators from a zone modulators list.
 * - remove linked modulators.
 * - remove modulators with invalid sources (specs SF 2.01  7.4, 7.8, 8.2.1).
 * - remove identic modulators in the list (specs SF 2.01  7.4, 7.8).
 * On output, the list contains only valid unlinked modulators.
 *
 * @param list_mod, address of pointer on modulator list.
 */
void fluid_mod_remove_invalid_from_list(fluid_mod_t **list_mod)
{
    fluid_mod_t *prev_mod = NULL; /* previous modulator in list_mod */
    fluid_mod_t *mod = *list_mod; /* first modulator in list_mod */
    while(mod)
    {	
        fluid_mod_t *next = mod->next;
        if(   /* Is mod a linked modulator ? */
              fluid_mod_is_linked(mod)
              /* or has mod invalid sources ? */
              || !fluid_mod_check_sources(mod, NULL)
              /* or is mod identic to any following modulator ? */
              || fluid_mod_is_identic_in_list(mod, NULL))
        {  
            /* the modulator is useless so we remove it */
            if (prev_mod)
            {
                prev_mod->next =next;
			}
            else
            {
                *list_mod = next;
			}

            delete_fluid_mod(mod); /* freeing */
        }
        else 
        {
            prev_mod = mod; 
        }
        mod = next;
    }
}

/* debug function: Prints the contents of a modulator */
void fluid_dump_modulator(fluid_mod_t * mod)
{
    int src1=mod->src1;
    int dest=mod->dest;
    int src2=mod->src2;
    int flags1=mod->flags1;
    int flags2=mod->flags2;
    fluid_real_t amount=(fluid_real_t)mod->amount;

    printf("Src: ");
    if(flags1 & FLUID_MOD_CC)
    {
        printf("MIDI CC=     %3i",src1);
    } 
    else
    {
        switch(src1)
        {
            case FLUID_MOD_NONE:                printf("None            "); break;
            case FLUID_MOD_VELOCITY:            printf("note-on velocity"); break;
            case FLUID_MOD_KEY:                 printf("Key nr          "); break;
            case FLUID_MOD_KEYPRESSURE:         printf("Poly pressure   "); break;
            case FLUID_MOD_CHANNELPRESSURE:     printf("Chan pressure   "); break;
            case FLUID_MOD_PITCHWHEEL:          printf("Pitch Wheel     "); break;
            case FLUID_MOD_PITCHWHEELSENS:      printf("Pitch wheel sens"); break;
            case FLUID_MOD_LINK_SRC:            printf("link                    "); break;
            default:                            printf("unknown:     %3i", src1);
        }; /* switch src1 */
    }; /* if not CC */

    if (src1 != FLUID_MOD_LINK_SRC)
    {
        if (flags1 & FLUID_MOD_NEGATIVE){printf(" - ");} 
        else                            {printf(" + ");};
        if (flags1 & FLUID_MOD_BIPOLAR) {printf("bip  ");}
        else                            {printf("unip ");};
    }
    printf("-> ");
    switch(dest)
    {
        case GEN_FILTERQ:         printf("Q              "); break;
        case GEN_FILTERFC:        printf("fc             "); break;

        case GEN_CUSTOM_FILTERQ:  printf("custom-Q       "); break;
        case GEN_CUSTOM_FILTERFC: printf("custom-fc      "); break;

        case GEN_VIBLFOTOPITCH:	  printf("VibLFO-to-pitch"); break;
        case GEN_MODENVTOPITCH:	  printf("ModEnv-to-pitch"); break;
        case GEN_MODLFOTOPITCH:	  printf("ModLFO-to-pitch"); break;
        case GEN_CHORUSSEND:      printf("Chorus send    "); break;
        case GEN_REVERBSEND:      printf("Reverb send    "); break;
        case GEN_PAN:             printf("pan            "); break;

        case GEN_CUSTOM_BALANCE:  printf("balance        "); break;

        case GEN_ATTENUATION:     printf("att            "); break;
        default:
            if(dest & FLUID_MOD_LINK_DEST)
            {
			                      printf("link-dest    %2i",dest &~FLUID_MOD_LINK_DEST);
            }
            else
            {
	                              printf("dest         %2i",dest);
            }

    }; /* switch dest */
    printf(", amount %9.2f, flags %3i, src2 %3i, flags2 %3i\n",amount, flags1, src2, flags2);
};

/*
 Print a simple modulator or all modulator members of a complex modulator.
 @param mod, pointer on first member.
 @param mod_idx, modulator index (displayed in the header).
 @param offset, offset to add to each index member.
*/
void fluid_dump_linked_mod(fluid_mod_t *mod, int mod_idx, int offset)
{
	int i, num = fluid_mod_get_linked_count(mod);

	printf("modulator #%d, member count:%d\n",mod_idx, num);
	for (i = 0; i < num; i++)
	{
        printf("mod%02d ", i + offset);
		fluid_dump_modulator(mod);
		mod = mod->next;
	}
}

/* description of bit flags set in modulator's path field by fluid_mod_check_linked_mod_LOCAL()
   These flags indicates if a modulator belongs to a linked path.

   FLUID_PATH_CURRENT | FLUID_PATH_VALID | Modulator state
   -------------------|------------------|--------------------------------------
         0            |     0            | doesn't belong to any linked path
   -------------------|------------------|--------------------------------------
         1            |     0            | belongs to a linked path not yet complete
   -------------------|------------------|--------------------------------------
         1            |     1            | belongs to a complete linked path
*/

/**
 * Check linked modulator paths without destination and circular linked modulator
 * paths (specif SF 2.0  7.4, 7.8  and 9.5.4).
 *
 * Warning: This function must be called before calling
 * fluid_mod_copy_linked_mod().
 *
 * Any linked modulator path from the start to the end are checked and returned
 * in path table.
 *
 * Let a linked path     CC-->m2-->m6-->m3-->gen
 *
 * - A linked path begins from a modulator with source scr1 not linked and
 *   destination linked to a modulator (e.g m2).
 * - A linked path ends on a modulator with source scr1 linked and destination
 *   connected to a generator (e.g m3).
 *
 * - Path without destination:
 *   When a destination cannot be reached inside a path, this path is said to be
 *   "without destination". The following message displays this situation:
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   with, mod2 being the modulator at the beginning of the path.
 *	 This case occurs when a modulator doesn't exist at m6 destination index
 *	 for example (CC->m2-->m6-->?).
 *	 This case occurs also if a modulator exists at m6 destination index
 *	 (e.g CC->m2-->m6-->m3->...) and this modulator (e.g m3) have source src1 not
 *   linked. Two messages are displayed to show the later case:
 *      fluidsynth: warning: invalid destination zone-name/mod3.
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   First message indicates that m3 is invalid (because source src1 isn't linked
 *   or mod is invalid).
 *   When a path is without destination, all modulators from the beginning to the one
 *   without destination are marked invalid (FLUID_PATH_VALID  = 0, amount = 0)
 *   (e.g  m2,m6).
 *
 * - Circular path:
 *   When a destination is a modulator already encountered this is a circular path
 *   (e.g: CC-->m2-->m6-->m3-->m8-->m6). Two messages are displayed:
 *      fluidsynth: warning: invalid circular path zone-name/mod6.
 *      fluidsynth: warning: path without destination zone-name/mod2.
 *   First message indicates that m6 is a modulator already encountered.
 *   Second message indicates the modulator at the beginning of the path (e.g m2).
 *   When a path is circular, all modulators from the beginning to the one
 *   already encontered are marked invalid (FLUID_PATH_VALID  = 0, amount = 0)
 *   (e.g  m2,m6,m3,m8).
 *
 * Other incomplete linked modulator paths are isolated.
 * Isolated path begins with modulator mx having source src1 linked, with no
 * others modulators connected to mx.
 * These isolated modulator paths are still in list_mod but not registered in
 * path table. They should be marked invalid later.
 *
 * The function searchs all linked path starting from the beginning of the path
 * (ie. a modulator with source not linked) forward to the endind of the path
 * (ie. a modulator connected to a generator).
 * Search direction is the reverse that the one done in fluid_mod_copy_linked_mod().
 * The function is recursive and intended to be called the first time to
 * start the search from the beginning of any path (see dest_idx, path_idx).
 *
 * @param list_name, list name used to prefix warning messages displayed.
 *  if NULL, no message are displayed.
 * @param list_mod, pointer on modulators list.
 *  On input,  modulators's path field must be initialized:
 *  - FLUID_PATH_CURRENT , FLUID_PATH_VALID must be initialized to 0.
 *  - FLUID_MOD_VALID to 1 indicates that the modulator is valid.
 *  On output,   modulators's path field indicates if the modulator belongs to
 *  a linked path:
 *  - no path (FLUID_PATH_CURRENT set to 0, FLUID_PATH_VALID set to 0) or
 *  - valid complete paths (FLUID_PATH_CURRENT, FLUID_PATH_VALID set to 1) or
 *  - invalid incomplete paths (FLUID_PATH_CURRENT set to 1, FLUID_PATH_VALID
 *    set to 0).
 *  - invalid linked modulator are marqued with FLUID_MOD_VALID to 0.
 * @param dest_idx, index of the destination linked modulator to search.
 *  Must be - 1 at first call.
 *   if < 0, search first modulator (i.e first linked modulateur).
 *   if >= 0 index of the destination linked modulator to search.
 *
 * @return
 *  - the number of linked modulators if any valid linked path exists.
 *  - 0 if no linked path exists.
*/
static int
fluid_mod_check_linked_mod_LOCAL(char *list_name, fluid_mod_t *list_mod,
                                 int dest_idx)
{
    int linked_count = 0; /* number of linked modulators */
    int mod_idx = 0; /* index of current mod in list */
    fluid_mod_t *mod = list_mod; /* first modulator in list_mod */
    while(mod)
    {
        /* is it a request to search first linked modulator of a path ? */
        if (dest_idx < 0)
        {
            /* checks if mod source isn't linked and mod destination is linked */
            if (!fluid_mod_has_linked_src1(mod) && (mod->dest & FLUID_MOD_LINK_DEST)
                 && (mod->path & FLUID_MOD_VALID))
            {
                int count;
                /* memorizes mod state: in current linked path */
                mod->path |= FLUID_PATH_CURRENT;

                /* search and check the full path to the end. */
                count = fluid_mod_check_linked_mod_LOCAL(list_name, list_mod, mod->dest);
                if (count < 0)
                {   /* no final destination found for mod */
                    mod->path &= ~FLUID_MOD_VALID; /* mod marked invalid */
                    /* warning: path is without destination */
                    if(list_name != NULL)
                    {
                        FLUID_LOG(FLUID_WARN, "Path without destination %s/mod%d",
                                  list_name, mod_idx);
                    }
                }
                else
                {
                    mod->path |= FLUID_PATH_VALID; /* current path is valid */
                    linked_count += (count + 1);
                }
            }
        }
        /* request to search next modulator in the current path */
        else if((mod_idx | FLUID_MOD_LINK_DEST) == dest_idx) /* is mod a destination ? */
        { 
            /* mod is destination of a previous modulator in path */
            /* is this modulator destination valid ? */
            if (!fluid_mod_has_linked_src1(mod) || !(mod->path & FLUID_MOD_VALID))
            {
                /* warning: path have an invalid destination */
                if(list_name != NULL)
                {
                    FLUID_LOG(FLUID_WARN, "Invalid destination %s/mod%d",
                              list_name, mod_idx);
                }
                return -1; /* current path is invalid */
            }
 
            /* mod is a valid destination modulator */
            /* Checks if mod belongs to a path already discovered */
            if (mod->path & FLUID_PATH_VALID)
            {
                return 0; /* current path is valid */
            }

            /* Checks if mod belongs to current path */
            if (mod->path & FLUID_PATH_CURRENT)
            {
                /* warning: invalid circular path */
                if(list_name != NULL)
                {
                    FLUID_LOG(FLUID_WARN, "Invalid circular path %s/mod%d",
                              list_name, mod_idx);
                }
                return -1; /* current path is invalid */
            }

            /* memorizes mod state: in current linked path */
            mod->path |= FLUID_PATH_CURRENT;

            /* does mod destination linked ? */
            if(mod->dest & FLUID_MOD_LINK_DEST)
            {
                linked_count = fluid_mod_check_linked_mod_LOCAL(list_name, list_mod,
                                                           mod->dest);
                if (linked_count < 0)
                {
                    mod->path &= ~FLUID_MOD_VALID; /* mod marked invalid */
                    return -1;       /* current path is invalid */
                }
            }
            linked_count++;
            mod->path |= FLUID_PATH_VALID; /* current path is valid */
            return linked_count;
        }
        mod = mod->next;
        mod_idx++;
    }
    return ((dest_idx < 0)?  linked_count : -1);
}

/**
 * Valid linked modulators paths are searched and cloned from list_mod list to
 * linked_mod list.
 * When finished, modulators in linked_mod are grouped in complex modulator.
 * (cm0,cm1,cm2..).
 * The first member of any complex modulator is the ending modulator (connected
 * to a generator). Following members are chained to each other to reach the last
 * member. The destination index of modulator member following the first is
 * relative (0 based) to the first member index.
 *
 * The member ordering rule is implemented as this:
 *  If any member mx has src1 linked it must be immediatley followed by a member
 *  whose destination field is mx. This rule ensures:
 *  1) That at synthesis time (noteon or CC modulation), any modulator mod_src
 *    (connected to another modulators mod_dst) are computed before this modulator mod_dst.
 *  2) That ordering is previsible in a way making test identity possible
 *     between two complex modulators (in fluid_mod_test_branch_identity()).
 * 
 * The function searchs all linked path starting from the end of the path 
 * (i.e modulator connected to a generator) backward to the beginning of 
 * the path (ie. a modulator with source not linked).
 * Search direction is the reverse that the one done in fluid_mod_check_linked_mod_LOCAL().
 * The function is recursive and intended to be called the first time to
 * start the search from ending linked modulator (see dest_idx, new_idx).
 *
 * @param list_mod, modulators list. Modulators in this list must be prepared
 *  by fluid_mod_check_linked_mod_LOCAL() before calling this function.
 *  Only modulators with path field set to FLUID_PATH_VALID will be cloned.
 *
 * @param dest_idx, initial index of linked destination modulator to search.
 *  Must be set to -1 at first call.
 *  -1, to search ending linked modulator.
 *  >= 0, to search a modulator with linked destination equal to dest_idx index.
 *
 * @param new_idx, index (1 based) of the most recent modulator at the end
 *  of linked_mod. Must be set to 0 at first call.
 *
 * @param linked_mod, address of pointer on linked modulators list returned
 *  if any linked modulators exist.
 * @param linked_count, number of modulators in linked_mod:
 *  - If > 0, the function assumes that linked_mod contains a table provided
 *    by the caller. The function returns linked modulators directly in this table.
 *  - If 0, the function makes internal allocation and returns the list in
 *    linked_mod.
 *
 * @return
 *  - number of linked modulators returned in linked_mod if any valid
 *    linked path exists.
 *  - 0 if no linked path exists.
 *  FLUID_FAILED if failed (memory error).
*/
static int 
fluid_mod_copy_linked_mod(const fluid_mod_t *list_mod, int dest_idx, int new_idx,
                           fluid_mod_t **linked_mod,
                           int linked_count)
{
    int total_linked_count = 0; /* number of linked modulator to return */
    int linked_idx = new_idx; /* Last added modulator index in linked_mod */
    int mod_idx = 0; /* first modulator index in list mod*/
    const fluid_mod_t *mod = list_mod;
    while(mod)
    {
        if (mod->path & FLUID_PATH_VALID) /* ignores invalid path */
        {
            /* is_src_linked is true when modulator mod's input are linked */
            int is_src1_linked = fluid_mod_has_linked_src1(mod);

            /* is_mod_dst_only is true when mod is a linked ending modulator */
            int is_mod_dst_only = (dest_idx < 0) && is_src1_linked &&
                                  !(mod->dest & FLUID_MOD_LINK_DEST);

            /* is_mod_src is true when mod linked destination is equal to dest_idx */
            int is_mod_src = ((dest_idx >= 0) && (dest_idx == mod->dest));

            /* is mod any linked modulator of interest ? */
            if (is_mod_dst_only || is_mod_src)
            {
                /* Make a copy of this modulator */
                fluid_mod_t *mod_cpy;
                if(linked_count <= 0) /* linked_mod must be allocated internally */
                {
                    mod_cpy = new_fluid_mod(); /* next field is set to NULL */
                    if(mod_cpy == NULL)
                    {
                        delete_fluid_mod_list(*linked_mod); /* freeing */
                        *linked_mod = NULL;
                        return FLUID_FAILED;
                    }
                }
                /* adding mod_cpy in linked_mod */
                if (linked_idx == 0) /* the list is empty */
                {
                    if(linked_count <= 0) /* list is allocated internally */
                    {
                        /* puts mod_cpy at the begin of linked list */
                        *linked_mod = mod_cpy;
                    }
                    else /* list is external given by linked_mod table */
                    {
                        /* get first entry from external linked_mod table */
                        mod_cpy = *linked_mod;
                        mod_cpy->next = NULL;
                    }
                }
                else /* the list isn't empty */
                {
                    /* Find the last modulator in the list */
                    fluid_mod_t * last_mod = *linked_mod;
                    int count = 1;
                    while (last_mod->next != NULL)
                    {
                        last_mod = last_mod->next;
                        count++;
                    }

                    if(linked_count > 0) /* list is external */
                    {
                        /* check if external table length is exceeded */
                        if(count >= linked_count)
                        {
                            return FLUID_FAILED;
                        }
                        mod_cpy = last_mod + 1; /* next entry in table */
                        mod_cpy->next = NULL;
                    }
                    /* puts mod_cpy at the end of linked list */
                    last_mod->next = mod_cpy;
                }
                fluid_mod_clone(mod_cpy, mod);

                /* updates destination field of mod_cpy (except ending modulator) */
                if (is_mod_src)
                {
                    /* new destination field must be an index 0 based. */
                    mod_cpy->dest = FLUID_MOD_LINK_DEST | (new_idx - 1);
                }
                else /* mod is an ending modulator */
                {
                    linked_idx = 0; /* force index of ending modulator to 0 */
                }
                linked_idx++; /* updates count of linked mod */

                /* is mod's source src1 linked ? */
                if(is_src1_linked) 
                {	/* search a modulator with output linked to mod */
                    linked_idx = fluid_mod_copy_linked_mod(list_mod,
                                                 mod_idx | FLUID_MOD_LINK_DEST,
                                                 linked_idx,
                                                 linked_mod, linked_count);
                    if(linked_idx == FLUID_FAILED)
                    {
                        return FLUID_FAILED;
                    }
                }
                if (is_mod_dst_only)
                {
                    total_linked_count += linked_idx;
                }
            }
        }
        mod = mod->next;
        mod_idx++;
    }
    return ((new_idx == 0) ? total_linked_count : linked_idx);
}

/**
 * Checks all modulators from a modulator list list_mod and optionally clone
 * valid linked modulators from list_mod list to linked_mod list.
 * - check valid sources (if requested, see list_name).
 * - check identic modulators (if requested, see list_name).
 * - check linked modulators paths (linked path without destination, circulars).
 * - check "isolated" linked paths (if requested, see list_name).
 * - clone valid linked modulators paths to linked_mod.
 * The function does the same job that fluid_zone_check_mod() except that
 * modulators aren't removed from list_mod and lists length aren't
 * limited. The function is appropriate to be called by soundfont loader as well
 * by API not yet implemented:
 * - fluid_voice_add_mod2(), fluid_check_complex_mod()
 * - fluid_synth_add_default_mod2(), fluid_synth_remove_default_mod2().
 *
 * @param list_name, list name used to prefix warning messages displayed.
 *  If NULL, the function does minimum safe check for linked modulators.
 *   - no warning message are displayed.
 *   - checks not done are: valid source, identic modulator, isolated linked path.
 *  This is useful for performance reason, when the caller already know that
 *  modulators are valid. In this case only minimum safe checks are done
 *  (linked paths without destination, circulars).
 *  When called by the soundfont loader, it is a good pratice to do full check.
 *  (see fluid_zone_check_mod()).
 *
 * @param list_mod, pointer on table or modulators list.
 *  On input, the list may contains any unlinked or linked modulators.
 *  On output, invalid modulators are marked invalid with amount value forced
 *  to 0.
 * @param mod_count number of modulators in table list_mod:
 *  - If > 0, the function assumes that list_mod is a table and initializes it
 *    as a list of modulators chained by next field, so that the caller doesn't
 *    need to do this initialization. This is appropriate when the function is
 *    called from fluid_voice_add_mod2().
 *  - If 0, the function assumes that mod_list is a list of modulators with next
 *    field properly initalialized by the caller. This is appropriate when the
 *    function is called from the soundfont loader.
 *
 * @param linked_mod, if not NULL, address of pointer on linked modulators
 *  list returned.
 * @param linked_count, number of modulators in linked_mod:
 *  - If > 0, the function assumes that linked_mod contains a table provided
 *    by the caller. The function returns linked modulators directly in this table
 *    which is faster because it doesn't allocate memory.
 *    This is appropriate when the function is called from fluid_voice_add_mod2().
 *  - If 0, the function makes internal allocation and returns the list in
 *    linked_mod. This is appropriate when the function is called  from the
 *    soundfont loader as the list of linked modulators must exist during the
 *    life of the preset it belongs to. NULL is returned in linked_mod if there is
 *    no linked modulators in list_mod.
 * @return
 *  - the number of linked modulators if any valid linked path exists.
 *  - 0 if no linked path exists.
 *  - FLUID_FAILED if failed (memory error).
 *
 * See test_modulator_links.c.
 */
int
fluid_mod_check_linked_mod(char *list_name,
                            fluid_mod_t *list_mod, int mod_count,
                            fluid_mod_t **linked_mod, int linked_count)
{
    int result;
    fluid_mod_t *mod;

    if (mod_count > 0) /* list_mod is a table */
    {
        int i, count;
        /* intialize list_mod as a list */
        for (i = 0, count = mod_count-1; i < count; i++)
        {
            list_mod[i].next = &list_mod[i+1]; /* initialize next field */
            /* initialize path:
               - reset bits FLUID_PATH_VALID, FLUID_PATH_CURRENT
               - set bit FLUID_MOD_VALID
             */
            list_mod[i].path = FLUID_MOD_VALID;
        }
        list_mod[count].next = NULL; /* last next field must be NULL */
        list_mod[count].path = FLUID_MOD_VALID; /* initialize path */
    }
    else /* lis_mod is a list of modulators */
    {
        mod = list_mod; /* first modulator in list_mod */
        mod_count = 0;
        while(mod)
        {
            mod_count++;
            mod->path = FLUID_MOD_VALID; /* initialize path */
            mod = mod->next;
        }
    }

    if(!mod_count)
    { /* There are no modulators, no need to go further */
        return 0;
    }

    /* checks valid modulator sources (specs SF 2.01  7.4, 7.8, 8.2.1).*/
    /* checks identic modulators in the list (specs SF 2.01  7.4, 7.8). */
    if(list_name != NULL)
    {
        mod = list_mod; /* first modulator in list_mod */
        mod_count = 0;
        while(mod)
        {
            char list_mod_name[256];

            /* prepare modulator name: zonename/#modulator */
            FLUID_SNPRINTF(list_mod_name, sizeof(list_mod_name),"%s/mod%d",
                           list_name, mod_count);

            /* has mod invalid sources ? */
            if(!fluid_mod_check_sources (mod,  list_mod_name)
            /* or is mod identic to any following modulator ? */
               ||fluid_mod_is_identic_in_list(mod, list_mod_name))
            {   /* marks this modulator invalid for future checks */
                mod->path &= ~FLUID_MOD_VALID;
            }

            mod_count++;
            mod = mod->next;
        }
    }

    /* Now check linked modulator path */
    result = fluid_mod_check_linked_mod_LOCAL(list_name, list_mod, -1);

    /* Now path contains complete or partial discovered modulators paths.
       Other unreachable linked modulators path (isolated) are still in list_mod but
       not in path. These should now marked invalid and a message is displayed.
       (specifications SF 2.01  7.4, 7.8) */
    if(list_name != NULL)
    {
        mod_count = 0; /* number of modulators in list_mod. */
        mod = list_mod; /* first modulator in list_mod */
        while(mod)
        {
            if( /* Check linked mod only not in discovered paths */
                (mod->path & FLUID_MOD_VALID)
                && fluid_mod_has_linked_src1(mod)
                /* Check if mod doesn't belong to any discovered paths */
                && !(mod->path & FLUID_PATH_CURRENT) )
            {
                mod->path &= ~FLUID_MOD_VALID; /* mod marked invalid */
                FLUID_LOG(FLUID_WARN, "Invalid isolated path %s/mod%d",
                          list_name, mod_count);
            }
            mod = mod->next;
            mod_count++;
        }
    }

    /* clone of linked modulators if requested */
    if(linked_mod)
    {
        if(linked_count <= 0)
        {
            *linked_mod = NULL; /* Initialize linked modulator list to NULL */
        }
        /* does one or more valid linked modulators exists ? */
        if(result)
        {
            /* one or more linked modulators paths exists */
            /* clone valid linked modulator paths from list_mod to linked_mod.*/
            result = fluid_mod_copy_linked_mod(list_mod, -1, 0,
                                                linked_mod, linked_count);
        }
    }

    return result;
}
