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

extern int fluid_synth_all_notes_off_LOCAL(fluid_synth_t* synth, int chan);
extern int fluid_is_number(char* a);

/* Macros interface to poly/mono mode variables */
#define MASK_BASICCHANINFOS  (MASKMODE|BASIC_CHANNEL|ENABLED)
/* access to channel mode */
/* Set the basic channel infos for a MIDI basic channel */
#define set_fluid_channel_basic_channel_infos(chan,Infos) \
(chan->mode = (chan->mode & ~MASK_BASICCHANINFOS) | (Infos & MASK_BASICCHANINFOS))
/* Reset the basic channel infos for a MIDI basic channel */
#define reset_fluid_channel_basic_channel_infos(chan) (chan->mode &=  ~MASK_BASICCHANINFOS)
/* End of macros interface to poly/mono mode variables */

/* Macros interface to breath variables */
#define MASK_BREATH_MODE  (BREATH_POLY|BREATH_MONO|BREATH_SYNC)
/* access to default breath infos */
/* Set the breath infos for a MIDI  channel */
#define set_fluid_channel_breath_infos(chan,BreathInfos) \
(chan->mode = (chan->mode & ~MASK_BREATH_MODE) | (BreathInfos & MASK_BREATH_MODE))
/* Get the breath infos for a MIDI  channel */
#define get_fluid_channel_breath_infos(chan) (chan->mode & MASK_BREATH_MODE)

/**  API Poly/mono mode ******************************************************/
/**
 * Gets the list of basic channel informations from the synth instance.
 *
 * @param synth the synth instance.
 * @param basicChannelInfos, 
 * - If non NULL the function returns a pointer to an allocated table of 
 * fluid_basic_channels_infos or NULL on allocation error.
 * The caller must free this table when finished with it.
 * 
 * - If NULL the function return only the count of basic channel.
 *      
 * - Each entry in the table is a fluid_basic_channels_infos_t information: 
 *     -basicchan the Basic Channel number (0 to MIDI channel count-1).
 *     -mode the MIDI mode to use for basicchan (0 to 3).
 *     -val Number of channels (0 to MIDI channel count).
 * 
 * @return 
 *  - Count of basic channel informations in the returned table. 
 *  - FLUID_FAILED.
 *    - synth is NULL.
 *    - allocation error.
 *
 * Remark: by default a synth instance have only one basic channel
 * on MIDI channel 0 in Poly Omni On (i.e all MIDI channels are polyphonic).
 * 
 * Note: default shell has an equivalent command "basicchannels" to display
 * the list of basics channels. 
 */
int fluid_synth_get_basic_channels(	fluid_synth_t* synth,
					fluid_basic_channel_infos_t **basicChannelInfos)
{
	int i,nChan;	/* MIDI channel index and number */
	int nBasicChan; /* Basic Channel number to return */
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	fluid_synth_api_enter(synth);
	nChan = synth->midi_channels; /* MIDI Channels number */

	/* count basic channels */
	for(i = 0, nBasicChan = 0; i <  nChan; i++)
	{
		if (synth->channel[i]->mode &  BASIC_CHANNEL) nBasicChan++;
	}

	if (basicChannelInfos && nBasicChan) 
	{	
		/* allocate table for Basic Channel only */
		fluid_basic_channel_infos_t * bci;	/* basics channels information table */
		int b; /* index in bci */
		bci = FLUID_ARRAY(fluid_basic_channel_infos_t, nBasicChan );
		/* fill table */
		if (bci) for(i = 0, b=0; i <  nChan; i++)
		{	
			fluid_channel_t* chan = synth->channel[i];
			if (chan->mode &  BASIC_CHANNEL)
			{	/* This channel is a basic channel */
				bci[b].basicchan = i;	/* channel number */
				bci[b].mode = chan->mode &  MASKMODE; /* MIDI mode:0,1,2,3 */
				bci[b].val = chan->mode_val;	/* value (for mode 3 only) */
				b++;
			}
		}
		else 
		{
			nBasicChan = FLUID_FAILED; /* allocation error */
			FLUID_LOG(FLUID_ERR, "Out of memory");
		}
		*basicChannelInfos = bci; /* return table */
	}
	fluid_synth_api_exit(synth);
	return nBasicChan;
}

int fluid_synth_set_basic_channel_LOCAL(fluid_synth_t* synth, 
					int basicchan,int mode, int val);
/**
 * Sets a new list of basic channel informations into the synth instance.
 * This list replace the previous list. 
 *
 * @param synth the synth instance.
 * @param n Number of entries in basicChannelInfos.
 * @param basicChannelInfos the list of basic channel infos to set. 
 *      
 * If n is 0 or basicChannelInfos is NULL the function set one channel basic at
 * basicchan 0 in Omni On Poly (i.e all the MIDI channels are polyphonic).
 *      
 * Each entry in the table is a fluid_basic_channels_infos_t information: 
 *     -basicchan the Basic Channel number (0 to MIDI channel count-1).
 *     -mode the MIDI mode to use for basicchan (0 to 3).
 *     -val the value (for mode 3 only) (0 to MIDI channel count).
 * 
 * @return
 *  - FLUID_OK on success.
 *  - FLUID_FAILED 
 *    - synth is NULL.
 *    - n, basicchan or val is outside MIDI channel count.
 *    - mode is invalid. 
 *    - val has a number of channels overlapping the next basic channel.
 * 
 * Note:This API is the only one to replace all the basics channels in the 
 * synth instance.
 * The default shell has an equivalent command "resetbasicchannels" to set one
 * or more basic channels.
 */
char * WarningMsg ="resetbasicchannels: Different entries have the same basic channel.\n\
An entry supersedes a previous entry with the same basic channel.\n";

int fluid_synth_reset_basic_channels(fluid_synth_t* synth, 
                             int n, 
                             fluid_basic_channel_infos_t *basicChannelInfos)
{
    int i,nChan;
    int result;
    fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
    fluid_return_val_if_fail (n >= 0, FLUID_FAILED);
    fluid_synth_api_enter(synth);
    
    nChan = synth->midi_channels; /* MIDI Channels number */
    if (n > nChan)
    {
        FLUID_API_RETURN(FLUID_FAILED);
    }
    
    /* Check if information are valid  */
    if(n && basicChannelInfos ) for (i = 0; i < n; i++)
    {
	if (	basicChannelInfos[i].basicchan < 0 || 
		basicChannelInfos[i].basicchan >= nChan ||
		basicChannelInfos[i].mode < 0 ||
		basicChannelInfos[i].mode >= MODE_NBR ||
		basicChannelInfos[i].val < 0 ||
		basicChannelInfos[i].basicchan + basicChannelInfos[i].val > nChan)
        {
            FLUID_API_RETURN(FLUID_FAILED);
        }
    }

    /* Clear previous list of basic channel */
    for(i = 0; i <  nChan; i++) {
	reset_fluid_channel_basic_channel_infos(synth->channel[i]);
	synth->channel[i]->mode_val = 0; 
    }
    if(n && basicChannelInfos)
    {
	result = FLUID_OK;

	/* Set the new list of basic channel */
	for (i = 0; i < n; i++)
	{
	    int bchan = basicChannelInfos[i].basicchan;
            if (synth->channel[bchan]->mode &  BASIC_CHANNEL)
		/* Different entries have the same basic channel. 
		 An entry supersedes a previous entry with the same 
		 basic channel.*/
		FLUID_LOG(FLUID_INFO, WarningMsg);
	    /* Set Basic channel first */
            else synth->channel[bchan]->mode |= BASIC_CHANNEL;
	}

	for (i = 0; i < n; i++)
	{
	    int r =fluid_synth_set_basic_channel_LOCAL( synth, 
				basicChannelInfos[i].basicchan,
				basicChannelInfos[i].mode,
				basicChannelInfos[i].val);
	    if (result == FLUID_OK) result = r;
	}
    }
    else result = fluid_synth_set_basic_channel_LOCAL( synth, 0, OMNION_POLY,0);
    FLUID_API_RETURN(result);
}

/**
 * Changes the mode of an existing basic channel or inserts a new basic channel part.
 *
 * - If basicchan is already a basic channel, the mode is changed.
 * - If basicchan is not a basic channel, a new basic channel part is inserted between
 * the previous basic channel and the next basic channel. val value of the previous
 * basic channel will be narrowed if necessary. 
 * 
 * @param synth the synth instance.
 * @param basicchan the Basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for basicchan (0 to 3).
 * @param val Number of monophonic channels (for mode 3 only) (0 to MIDI channel count).
 * 
 * @return 
 * - FLUID_OK on success.
 * - FLUID_FAILED
 *   - synth is NULL.
 *   - chan or val is outside MIDI channel count.
 *   - mode is invalid.
 *   - val has a number of channels overlapping the next basic channel.
 */
int fluid_synth_set_basic_channel(fluid_synth_t* synth, int basicchan, int mode, int val)
{
    int result;
    const int chan = basicchan;

    fluid_return_val_if_fail (mode >= 0, FLUID_FAILED);
    fluid_return_val_if_fail (mode < MODE_NBR, FLUID_FAILED);
    fluid_return_val_if_fail (val >= 0, FLUID_FAILED);
    FLUID_API_ENTRY_CHAN(FLUID_FAILED);
    /**/
    if (basicchan + val > synth->midi_channels)
        FLUID_API_RETURN(FLUID_FAILED);

    result = fluid_synth_set_basic_channel_LOCAL(synth, basicchan,mode,val);
    /**/
    FLUID_API_RETURN(result);
}

/**
 * Changes the mode of an existing basic channel or inserts a new basic channel part.
 *
 * - If basicchan is already a basic channel, the mode is changed.
 * - If basicchan is not a basic channel, a new basic channel part is inserted between
 * the previous basic channel and the next basic channel. val value of the previous
 * basic channel will be narrowed if necessary. 
 * 
 * The function is used internally by API  fluid_synth_reset_basic_channels() and
 * fluid_synth_set_basic_channel().
 * 
 * @param synth the synth instance.
 * @param basicchan the Basic Channel number (0 to MIDI channel count-1).
 * @param mode the MIDI mode to use for basicchan (0 to 3).
 * @param val Number of monophonic channels (for mode 3 only) (0 to MIDI channel count).
 * 
 * @return 
 * - FLUID_OK on success.
 * - FLUID_FAILED
 *   - basicchan is outside MIDI channel count.
 *   - val has a number of channels overlapping the next basic channel.
 * 
 * Note: default shell has an equivalent command "setbasicchannels".
 */
char * WarningMsg1 = "Basic channel %d has been narrowed to %d channels.";
char * WarningMsg2 = "Basic channel %d have number of channels that overlaps.\n\
the next basic channel\n";

int fluid_synth_set_basic_channel_LOCAL(fluid_synth_t* synth, 
					int basicchan,int mode, int val)
{
	int nChan = synth->midi_channels; /* MIDI Channels number */
	int result = FLUID_FAILED; /* default return */
	if (basicchan < nChan)
	{
		int LastBeginRange; /* Last channel num inside the beginning range + 1. */
		int LastEndRange; /* Last channel num inside the ending range + 1. */
		int prevbasicchan = -1 ; // Previous basic channel
		int i;
		result = FLUID_OK;
		if ( !(synth->channel[basicchan]->mode &  BASIC_CHANNEL))
		{	/* a new basic channel is inserted between previous basic channel 
			and the next basic channel.	*/
			if (synth->channel[basicchan]->mode &  ENABLED)
			{ /* val value of the previous basic channel need to be narrowed */
				for (i = basicchan - 1; i >=0; i--)
				{	/* search previous basic channel */
					if (synth->channel[i]->mode &  BASIC_CHANNEL)
					{	/* i is the previous basic channel */
						prevbasicchan = i;
						break;
					}
				}
			}	
		}

		/* LastEndRange: next basic channel  or midi_channels count  */
		for (LastEndRange = basicchan +1; LastEndRange < nChan; LastEndRange++)
		{
			if (synth->channel[LastEndRange]->mode &  BASIC_CHANNEL) break;
		}
		/* Now LastBeginRange is set */
		switch (mode = mode &  MASKMODE)
		{
			case OMNION_POLY:	/* Mode 0 and 1 */
			case OMNION_MONO:
				LastBeginRange = LastEndRange;
				break;
			case OMNIOFF_POLY:		/* Mode 2 */
				LastBeginRange = basicchan + 1;
				break;
			case OMNIOFF_MONO:		/* Mode 3 */
				if (val) LastBeginRange = basicchan + val;
				else LastBeginRange = LastEndRange;
		}
		/* Check if val overlaps the next basic channel */
		if (LastBeginRange > LastEndRange)
		{	
			/* val have number of channels that overlaps the next basic channel */
			FLUID_LOG(FLUID_INFO,WarningMsg2,basicchan);
			return FLUID_FAILED;
		}
		/* if previous basic channel exists, val is narrowed */
		if(prevbasicchan >= 0)
		{
			synth->channel[prevbasicchan]->mode_val = basicchan - prevbasicchan;
			FLUID_LOG(FLUID_INFO,WarningMsg1,prevbasicchan,basicchan - prevbasicchan);
		}

		/* val is limited up to LastBeginRange */
		val = LastBeginRange - basicchan;
		/* Set the Mode to the range zone: Beginning range + Ending range */
		for (i = basicchan; i < LastEndRange; i++)
		{	
			int newmode = mode; /* OMNI_OFF/ON, MONO/POLY ,others bits are zero */
			/* MIDI specs: when mode is changed, channel must receive
			 ALL_NOTES_OFF */
			fluid_synth_all_notes_off_LOCAL (synth, i);
			/* basicchan only is marked Basic Channel */
			if (i == basicchan)	newmode |= BASIC_CHANNEL; 
			else val =0; /* val is 0 for other channel than basic channel */
			/* Channel in beginning zone are enabled */
			if (i < LastBeginRange) newmode |= ENABLED; 
			/* Channel in ending zone are disabled */
			else newmode = 0;
			/* Now mode is OMNI OFF/ON,MONO/POLY, BASIC_CHANNEL or not
			   ENABLED or not */	
			set_fluid_channel_basic_channel_infos(synth->channel[i],newmode);
			synth->channel[i]->mode_val = val;
		}
	}
	return result;
}

/**
 * Returns poly mono mode informations from any MIDI channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param  modeInfos, pointer on a fluid_basic_channels_infos_t informations. 
 *     - basicchan , chan.
 *     - mode MIDI mode infos of chan:
 *       bit 0: MONO:	0,Polyphonic;	1,Monophonic.
 *       bit 1: OMNI:	0,Omni  On;		1,Omni Off.
 *       bit 2: BASIC_CHANNEL:	1, chan is a Basic Channel.
 *       bit 3: ENABLED: 1,chan is listening. 
 *                       0,chan ignores voices messages (MIDI note on/of, cc).
 *     - val, Number of channels in the group from basic channel (if bit 2 is set)
 *     or 0 if bit 2 is 0.
 *
 * @return
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 *   - modeInfos is NULL.
 *
 * Note: default shell has an equivalent command "channelsmode".
 */
int fluid_synth_get_channel_mode(fluid_synth_t* synth, int chan,
				fluid_basic_channel_infos_t  *modeInfos)
{
	/* check parameters first */
	fluid_return_val_if_fail (modeInfos!= NULL, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	modeInfos->basicchan= chan;
	modeInfos->mode = synth->channel[chan]->mode;
	modeInfos->val = synth->channel[chan]->mode_val;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}
/**  API legato mode *********************************************************/
/**
 * Sets the legato mode of a channel.
 * 
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param legatomode
 *    0: RETRIGGER    1: MULTI_RETRIGGER.
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
	/* check parameters first */
	fluid_return_val_if_fail (legatomode >= 0, FLUID_FAILED);
	fluid_return_val_if_fail (legatomode < LEGATOMODE_NBR, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	synth->channel[chan]->legatomode = legatomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the legato mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param legatomode, pointer to the returned mode .
 *    0: RETRIGGER    1: MULTI_RETRIGGER.
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
	/* check parameters first */
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
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param portamentomode .
 *    0: EACH_NOTE.	    1: LEGATO_ONLY.
 *    2: STACCATO_ONLY
 *
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
	/* check parameters first */
	fluid_return_val_if_fail (portamentomode >= 0, FLUID_FAILED);
	fluid_return_val_if_fail (portamentomode < PORTAMENTOMODE_NBR, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	synth->channel[chan]->portamentomode = portamentomode;
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the portamento mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param portamentomode pointer to the returned mode.
 *    0: EACH_NOTE.     1: LEGATO_ONLY.
 *    2: STACCATO_ONLY.
 *
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
	/* check parameters first */
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
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param breathmode bits:
 *    BREATH_POLY   default breath poly On/Off.
 *    BREATH_MONO   default breath mono On/Off.
 *    BREATH_SYNC   breath noteOn/noteOff triggering On/Off.
 *
 * @return
 * - FLUID_OK on success.
 * - FLUID_FAILED 
 *   - synth is NULL.
 *   - chan is outside MIDI channel count.
 */
int fluid_synth_set_breath_mode(fluid_synth_t* synth, int chan, int breathmode)
{
	/* check parameters first */
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	set_fluid_channel_breath_infos(synth->channel[chan],breathmode);
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

/**
 * Gets the breath mode of a channel.
 *
 * @param synth the synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param breathmode, pointer to the returned breath mode.
 *    BREATH_POLY   default breath poly On/Off. 
 *    BREATH_MONO   default breath mono On/Off.
 *    BREATH_SYNC   breath noteOn/noteOff triggering On/Off.
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
	/* check parameters first */
	fluid_return_val_if_fail (breathmode!= NULL, FLUID_FAILED);
	FLUID_API_ENTRY_CHAN(FLUID_FAILED);
	/**/
	* breathmode = get_fluid_channel_breath_infos(synth->channel[chan]);
	/**/
	FLUID_API_RETURN(FLUID_OK);
}

