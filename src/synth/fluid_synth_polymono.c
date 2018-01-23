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
 * Resets a basic channel group designed by basicchan.
 *
 * @param synth the synth instance.
 * @param chan the basic channel of the group to reset. -1 means reset all basic channels. 
 * @Note: Be aware than when a synth instance has no basic channel, all channels are disabled.
 * In the intend to get some MIDI channels enabled, the application have to set at least 
 * one basic channel using fluid_synth_set_basic_channel API.
 *  
 * @return
 *  - FLUID_OK on success.
 *  - FLUID_FAILED 
 *    - synth is NULL.
 *    - chan is outside MIDI channel count.
 *    - chan isn't a basic channel. 
 */
int fluid_synth_reset_basic_channels(fluid_synth_t* synth, int chan)
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
 * Changes the mode of an existing basic channel or sets a new basic channel group.
 *
 * - If chan is already a basic channel, the mode is changed.
 * - If chan is not a basic channel, a new basic channel group is set.
 * In all case the function fails if any channels overlaps existing neighbour basic 
 * channel groups. To make room if necessary, existing basic channel groups can be
 * cleared using fluid_synth_reset_basic_channels API.
 * 
 * @param synth the synth instance.
 * @param chan the basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for chan (0 to 3).
 * @param val number of channels in the group(for mode 0,1,3 only).
 * @ Note:
 *   - val is relevant only for mode poly omnion (0), mono omnion (1), mono omni off (3). 
 *     a value -1 (or 0) means all channels from basicchan to MIDI channel count -1.
 *   - val is ignored for mode poly omnioff (2) as this mode implies a group of only
 *     one channel.
 * @return 
 * - FLUID_OK on success.
 * - FLUID_FAILED
 *   - chan is outside MIDI channel count.
 *   - mode is invalid.
 *   - val has a number of channels overlapping another basic channel group or been
 *     above MIDI channel count.
 */
int fluid_synth_set_basic_channel(fluid_synth_t* synth, int chan, int mode, int val)
{
    int result;
	/* checks parameters */
    fluid_return_val_if_fail (mode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail (mode < FLUID_CHANNEL_MODE_LAST, FLUID_FAILED);
    fluid_return_val_if_fail (val >= -1, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    if (val > 0 && chan + val > synth->midi_channels)
	{
        FLUID_API_RETURN(FLUID_FAILED);
	}

    result = fluid_synth_set_basic_channel_LOCAL(synth,chan,mode,val);
    /**/

    FLUID_API_RETURN(result);
}

/**
 * Changes the mode of an existing basic channel or set a new basic channel group.
 *
 * - If basicchan is already a basic channel, the mode is changed.
 * - If basicchan is not a basic channel, a new basic channel part is set.
 * In all case the function fail if any channels overlaps existing neighbour basic 
 * channel groups. To make room if necessary, existing basic channel groups can be
 * cleared using resetbasicchannels API.
 * 
 * The function is used internally by fluid_synth_set_basic_channel().
 * 
 * @param synth the synth instance.
 * @param basicchan the Basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for basicchan (0 to 3).
 * @param val number of monophonic channels.(for mode 0,1,3 only)
 * @ Note:
 *   - val is relevant only for mode poly omnion (0), mono omnion (1), mono omni off (3). 
 *     a value 0 (or -1) means all channels from basicchan to MIDI channel count -1.
 *   - val is ignored for mode poly omnioff (2) as this mode implies a group of only
 *     one channel.
 * @return 
 * - FLUID_OK on success.
 * - FLUID_FAILED
 *   - val has a number of channels overlapping another basic channel group or above MIDI channel count.
 */
int fluid_synth_set_basic_channel_LOCAL(fluid_synth_t* synth, int basicchan, int mode, int val)
{
	static const char * warning_msg = "channel %d overlaps other channel";
	int i, n_chan = synth->midi_channels; /* MIDI Channels number */
	mode = mode &  FLUID_CHANNEL_MODE_MASK;

	/* adjusts val range */
	if (mode == FLUID_CHANNEL_MODE_OMNIOFF_POLY)
	{
		val = 1; /* mode poly ominioff implies a group of only one channel.*/
	}
	else if (val <= 0)
	{  
		/* mode poly omnion (0), mono omnion (1), mono omni off (3) */
		/* value 0 (or -1) means all channels from basicchan to MIDI channel count -1.*/
		val = n_chan - basicchan;
	}
	/* checks val range */
	if ( basicchan + val > n_chan)
	{
		return FLUID_FAILED;
	}

	/* if basicchan is an existing basic channel group, it is cleared */
	if ( synth->channel[basicchan]->mode &  FLUID_CHANNEL_BASIC)
	{
		int nbr_chan = synth->channel[basicchan]->mode_val;
		fluid_synth_reset_basic_channel_LOCAL(synth, basicchan, nbr_chan);
	}

	/* checks if this basic channel group overlaps another basic channel group */
	for (i = basicchan; i < basicchan + val; i++)
	{
		if (synth->channel[i]->mode &  FLUID_CHANNEL_ENABLED)
		{
			FLUID_LOG(FLUID_INFO,warning_msg,i);
			return FLUID_FAILED;
		}
	}
	/* sets the basic channel group */
	for (i = basicchan; i < basicchan + val; i++)
	{
		int new_mode = mode; /* OMNI_OFF/ON, MONO/POLY ,others bits are zero */
		int new_val;
		/* MIDI specs: when mode is changed, channel must receive ALL_NOTES_OFF */
		fluid_synth_all_notes_off_LOCAL (synth, i);

		if (i == basicchan)
		{
			new_mode |= FLUID_CHANNEL_BASIC;
			new_val = val;
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
 * Returns poly mono mode informations from any MIDI channel.
 *
 * @param synth the synth instance
 * @param chan MIDI channel number (0 to MIDI channel count - 1)
 * @param basic_chan_out pointer to returned basic channel chan belongs to ( -1
 *        if chan is disabled).
 * @param mode_chan_out pointer to the returned mode of chan.
 * @param basic_val_out pointer to the returned val of basic channel.
 * @note if any of basic_chan_out, mode_chan_out, basic_val_out pointer is NULL
 *  the corresponding information isn't returned. 
 * 
 * @return
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 */
int fluid_synth_get_basic_channel(fluid_synth_t* synth, int chan,
					int *basic_chan_out, 
					int *mode_chan_out,
					int *basic_val_out )
{
	int basic_chan;
	/* checks parameters first */
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	basic_chan = FLUID_FAILED;
	if (synth->channel[chan]->mode &  FLUID_CHANNEL_ENABLED)
	{ /* chan is enabled , we search the basic channel chan belongs to */
		basic_chan = fluid_synth_get_previous_basic_channel(synth, chan);
	}	
	/* returns basic channel */ 
	if (basic_chan_out)
	{
		* basic_chan_out = basic_chan;
	}
	/* returns mode of chan */ 
	if (mode_chan_out)
	{
		* mode_chan_out = synth->channel[chan]->mode;
	}
	/* returns val of basic channel */ 
	if (basic_val_out)
	{
		* basic_val_out = 0;
		if (basic_chan >= 0)
		{
			* basic_val_out = synth->channel[basic_chan]->mode_val;
		}
	}
	/**/
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - legatomode is invalid.
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - legatomode is NULL.
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - portamentomode is invalid.
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - portamentomode is NULL.
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
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
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - breathmode is NULL.
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

