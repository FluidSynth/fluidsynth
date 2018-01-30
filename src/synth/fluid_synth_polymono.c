/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "fluid_synth.h"
#include "fluid_chan.h"



/**  API Poly/mono mode ******************************************************/

/*
 * Resets a basic channel group of MIDI channels.
 * @param synth the synth instance.
 * @param chan the beginning channel of the group.
 * @param nbr_chan the number of channel in the group. 
*/
static void fluid_synth_reset_basic_channel_LOCAL(fluid_synth_t* synth, int chan, int nbr_chan)
{
	int i;
	for (i = chan; i < chan + nbr_chan; i++)
	{
		fluid_channel_reset_basic_channel_info(synth->channel[i]);
		synth->channel[i]->mode_val = 0; 
	}
}

/**
 * Disables and unassigns all channels from a basic channel group.
 *
 * @param synth The synth instance.
 * @param chan The basic channel of the group to reset or -1 to reset all channels.
 * @note By default (i.e. on creation after new_fluid_synth() and after fluid_synth_system_reset())
 * a synth instance has one basic channel at channel 0 in mode #FLUID_CHANNEL_MODE_OMNION_POLY.
 * All other channels belong to this basic channel group.
 *  
 * @return
 *  - #FLUID_OK on success.
 *  - #FLUID_FAILED 
 *    - \a synth is NULL.
 *    - \a chan is outside MIDI channel count.
 *    - \a chan isn't a basic channel. 
 */
int fluid_synth_reset_basic_channel(fluid_synth_t* synth, int chan)
{
    int nbr_chan;
  
    /* checks parameters first */
	if (chan < 0)
	{
		fluid_return_val_if_fail (synth!= NULL, FLUID_FAILED);
		fluid_synth_api_enter(synth);
		/* The range is all MIDI channels from 0 to MIDI channel count -1 */
		chan = 0; /* beginning chan */
		nbr_chan =  synth->midi_channels; /* MIDI Channels number */
	}
	else
	{
	    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
		/* checks if chan is a basic channel */
		if ( ! (synth->channel[chan]->mode &  FLUID_CHANNEL_BASIC) )
		{
			FLUID_API_RETURN(FLUID_FAILED);
		}
		/* The range is all MIDI channels in the group from chan */
		nbr_chan = synth->channel[chan]->mode_val; /* nbr of channels in the group */
	}
	/* resets the range of MIDI channels */
	fluid_synth_reset_basic_channel_LOCAL(synth, chan, nbr_chan);
    FLUID_API_RETURN(FLUID_OK);
}

/**
 * Sets a new basic channel group only. The function doesn't allow to change an
 * existing basic channel.
 *
 * The function fails if any channel overlaps any existing basic channel group. 
 * To make room if necessary, basic channel groups can be cleared using
 * fluid_synth_reset_basic_channel().
 * 
 * @param synth the synth instance.
 * @param chan the basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for chan (see #fluid_basic_channel_modes).
 * @param val number of channels in the group.
 * @note \a val is only relevant for mode #FLUID_CHANNEL_MODE_OMNION_POLY, #FLUID_CHANNEL_MODE_OMNION_MONO
 * and #FLUID_CHANNEL_MODE_OMNIOFF_MONO, i.e. it is ignored for #FLUID_CHANNEL_MODE_OMNIOFF_POLY as this 
 * mode implies a group of only one channel. A value of 0 means all possible channels from \a chan to 
 * to next basic channel minus 1 (if any) or to MIDI channel count minus 1.
 * @return 
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a mode is invalid.
 *   - \a val has a number of channels overlapping another basic channel group or been
 *     above MIDI channel count.
 *   - When the function fails, any existing basic channels aren't modified.
 */
int fluid_synth_set_basic_channel(fluid_synth_t* synth, int chan, int mode, int val)
{
    int result;
	/* checks parameters */
    fluid_return_val_if_fail (mode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail (mode < FLUID_CHANNEL_MODE_LAST, FLUID_FAILED);
    fluid_return_val_if_fail (val >= 0, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    if (val > 0 && chan + val > synth->midi_channels)
	{
        FLUID_API_RETURN(FLUID_FAILED);
	}
	/* set a new basic channel group (changing an existing basic channel is disabled */
    result = fluid_synth_set_basic_channel_LOCAL(synth,chan,mode,val,0);
    /**/

    FLUID_API_RETURN(result);
}
/*
 * Local version of fluid_synth_set_basic_channel(), called internally:
 * - by fluid_synth_set_basic_channel() to set a new basic channel group (changing
 *   an existing basic channel is disabled).
 * - during creation new_fluid_synth() or on CC reset to set a default basic channel group.
 * - on CC ominoff, CC omnion, CC poly , CC mono to change an existing basic channel group.
 *
 * @param synth , chan, mode, val see fluid_synth_set_basic_channel()
 * @param enable_change, when true the function is enabled to change an existing basic channel.
 * @return 
 * - #FLUID_OK on success.
 * - #FLUID_FAILED
 *   - basichan is a group that overlaps existing basic channel group or been
 *     above MIDI channel count.
 *   - When the function fails, any existing basic channels aren't modified.
*/
int fluid_synth_set_basic_channel_LOCAL(fluid_synth_t* synth, 
                                        int basicchan, int mode, int val,
										char enable_change)
{
	static const char * warning_msg = "channel %d overlaps other channel";
	int i, n_chan = synth->midi_channels; /* MIDI Channels number */
	int real_val = val; /* real number of channels in the group */
	mode = mode &  FLUID_CHANNEL_MODE_MASK;

	/* adjusts val range */
	if (mode == FLUID_CHANNEL_MODE_OMNIOFF_POLY)
	{
		real_val = 1; /* mode poly ominioff implies a group of only one channel.*/
	}
	else if (val == 0)
	{  
		/* mode poly omnion (0), mono omnion (1), mono omni off (3) */
		/* value 0 means all possible channels from basicchan to MIDI channel count -1.*/
		real_val = n_chan - basicchan;
	}
	/* checks val range */
	else if ( basicchan + val > n_chan)
	{
		return FLUID_FAILED;
	}

	/* checks if this basic channel group overlaps a previous basic channel group 
	   or checks if the change of an existing basic channel is disabled */
	if ( !(synth->channel[basicchan]->mode &  FLUID_CHANNEL_BASIC)|| !enable_change)
	{
		if (synth->channel[basicchan]->mode &  FLUID_CHANNEL_ENABLED)
		{
			/* overlap with the previous basic channel group 
			or basicchan is an existing basic channel not allowed to be changed*/
			FLUID_LOG(FLUID_INFO,warning_msg,basicchan);
			return FLUID_FAILED;
		}
	}
	/* checks if this basic channel group overlaps next basic channel group */
	for (i = basicchan + 1; i < basicchan + real_val; i++)
	{
		if (synth->channel[i]->mode &  FLUID_CHANNEL_BASIC)
		{
			/* A value of 0 for val means all possible channels from basicchan to 
			to the next basic channel -1 (if any).
			When i reachs the next basic channel group, real_val will be
			limited if it is possible */
			if (val == 0)
			{	/* limitation of real_val */
				real_val = i - basicchan;
				break;
			}
			/* overlap with the next basic channel group */
			FLUID_LOG(FLUID_INFO,warning_msg,i);
			return FLUID_FAILED;
		}
	}

	/* if basicchan is an existing basic channel group, it is cleared */
	if ( synth->channel[basicchan]->mode &  FLUID_CHANNEL_BASIC)
	{
		int nbr_chan = synth->channel[basicchan]->mode_val;
		fluid_synth_reset_basic_channel_LOCAL(synth, basicchan, nbr_chan);
	}

	/* sets the basic channel group */
	for (i = basicchan; i < basicchan + real_val; i++)
	{
		int new_mode = mode; /* OMNI_OFF/ON, MONO/POLY ,others bits are zero */
		int new_val;
		/* MIDI specs: when mode is changed, channel must receive ALL_NOTES_OFF */
		fluid_synth_all_notes_off_LOCAL (synth, i);

		if (i == basicchan)
		{
			new_mode |= FLUID_CHANNEL_BASIC;
			new_val = real_val;
		}
		else
		{
			new_val =0; /* val is 0 for other channel than basic channel */
		}
		/* Channel is enabled */
		new_mode |= FLUID_CHANNEL_ENABLED;
		/* Now new_mode is OMNI OFF/ON,MONO/POLY, BASIC_CHANNEL or not and enabled */
		fluid_channel_set_basic_channel_info(synth->channel[i],new_mode);
		synth->channel[i]->mode_val = new_val;
	}
	return FLUID_OK;
}

/**
 * Searchs a previous basic channel.
 * 
 * @param synth the synth instance.
 * @param chan starting index of the search (including chan).
 * @return index of the basic channel if found , FLUID_FAILED otherwise 
 */
static int fluid_synth_get_previous_basic_channel(fluid_synth_t* synth, int chan)
{
	for (; chan >=0; chan--)
	{	/* searchs previous basic channel */
		if (synth->channel[chan]->mode &  FLUID_CHANNEL_BASIC)
		{	/* chan is the previous basic channel */
			return chan;
		}
	}
	return FLUID_FAILED;
}

/**
 * Returns poly mono mode information of any MIDI channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param basic_chan_out Buffer to store the basic channel \a chan belongs to or #FLUID_FAILED if \a chan is disabled.
 * @param mode_out Buffer to store the mode of \a chan (see #fluid_basic_channel_modes) or #FLUID_FAILED if \a chan is disabled.
 * @param val_out Buffer to store the total number of channels in this basic channel group or #FLUID_FAILED if \a chan is disabled.
 * @note If any of \a basic_chan_out, \a mode_out, \a val_out pointer is NULL
 *  the corresponding information isn't returned.
 * 
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 */
int fluid_synth_get_basic_channel(fluid_synth_t* synth, int chan,
					int *basic_chan_out, 
					int *mode_out,
					int *val_out )
{
    int basic_chan = FLUID_FAILED;
    int mode = FLUID_FAILED;
    int val = FLUID_FAILED;
    
	/* checks parameters first */
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    
	if ((synth->channel[chan]->mode &  FLUID_CHANNEL_ENABLED) &&
        /* chan is enabled , we search the basic channel chan belongs to */
        (basic_chan = fluid_synth_get_previous_basic_channel(synth, chan)) != FLUID_FAILED)
	{ 
        mode = synth->channel[chan]->mode & FLUID_CHANNEL_MODE_MASK;
        val = synth->channel[basic_chan]->mode_val;
	}
	
	if (basic_chan_out)
	{
		* basic_chan_out = basic_chan;
	}
	
	if (mode_out)
	{
		* mode_out = mode;
	}
	
	if (val_out)
	{
		* val_out = val;
	}
	
	FLUID_API_RETURN(FLUID_OK);
}

/**  API legato mode *********************************************************/

/**
 * Sets the legato mode of a channel.
 * 
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param legatomode The legato mode as indicated by #fluid_channel_legato_mode
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a legatomode is invalid.
 */
int fluid_synth_set_legato_mode(fluid_synth_t* synth, int chan, int legatomode)
{
	/* checks parameters first */
	fluid_return_val_if_fail (legatomode >= 0, FLUID_FAILED);
	fluid_return_val_if_fail (legatomode < FLUID_CHANNEL_LEGATO_MODE_LAST, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	synth->channel[chan]->legatomode = legatomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the legato mode of a channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param legatomode The legato mode as indicated by #fluid_channel_legato_mode
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a legatomode is NULL.
 */
int fluid_synth_get_legato_mode(fluid_synth_t* synth, int chan, int *legatomode)
{
	/* checks parameters first */
	fluid_return_val_if_fail (legatomode!= NULL, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	* legatomode = synth->channel[chan]->legatomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**  API portamento mode *********************************************************/

/**
 * Sets the portamento mode of a channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param portamentomode The portamento mode as indicated by #fluid_channel_portamento_mode
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a portamentomode is invalid.
 */
int fluid_synth_set_portamento_mode(fluid_synth_t* synth, int chan,
					int portamentomode)
{
	/* checks parameters first */
	fluid_return_val_if_fail (portamentomode >= 0, FLUID_FAILED);
	fluid_return_val_if_fail (portamentomode < FLUID_CHANNEL_PORTAMENTO_MODE_LAST, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	synth->channel[chan]->portamentomode = portamentomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the portamento mode of a channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param portamentomode Pointer to the portamento mode as indicated by #fluid_channel_portamento_mode
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a portamentomode is NULL.
 */
int fluid_synth_get_portamento_mode(fluid_synth_t* synth, int chan,
					int *portamentomode)
{
	/* checks parameters first */
	fluid_return_val_if_fail (portamentomode!= NULL, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	* portamentomode = synth->channel[chan]->portamentomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**  API breath mode *********************************************************/

/**
 * Sets the breath mode of a channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param breathmode The breath mode as indicated by #fluid_channel_breath_flags
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 */
int fluid_synth_set_breath_mode(fluid_synth_t* synth, int chan, int breathmode)
{
	/* checks parameters first */
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	fluid_channel_set_breath_info(synth->channel[chan],breathmode);
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the breath mode of a channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param breathmode Pointer to the returned breath mode as indicated by #fluid_channel_breath_flags
 *
 * @return
 * - #FLUID_OK on success.
 * - #FLUID_FAILED 
 *   - \a synth is NULL.
 *   - \a chan is outside MIDI channel count.
 *   - \a breathmode is NULL.
 */
int fluid_synth_get_breath_mode(fluid_synth_t* synth, int chan, int *breathmode)
{
	/* checks parameters first */
	fluid_return_val_if_fail (breathmode!= NULL, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	* breathmode = fluid_channel_get_breath_info(synth->channel[chan]);
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

