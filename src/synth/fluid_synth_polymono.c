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


/**  API Poly/mono mode ******************************************************/
/**
 The API function get the list of basic channel informations in the synthesizer.

 * @param synth, FluidSynth instance
 * @param  basicChannelInfos, 
 * @  If non NULL the function returns a pointer to allocated table of 
 * @    fluid_basic_channels_infos  or NULL on allocation error.
 * @    The caller must free this table when finished with it.
 * @
 * @  If NULL the function return only the count of basic channel.
 * @     
 * @  Each entry in the table is a fluid_basic_channels_infos_t 
 * @    -basicchan is the Basic Channel number (0 to MIDI channel count-1)
 * @    -mode is MIDI mode infos for basicchan (0 to 3)
 * @    -val is the number of channels (0 to MIDI channel count)

 * @return, Count of basic channel informations in the returned table or 
 * @ FLUID_FAILED if synth is NULL or allocation error.
 
 * @ Remark: By default a FluidSynth instance have only one basic channel
 * @ on MIDI channel 0 in Poly Omni On (i.e all MIDI channels are polyphonic).
 
 * @ Note: The default shell have equivalent command "basicchannels" to display
 * @ basics channels 

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
		if (IsChanBasicChannel(synth->channel[i])) nBasicChan++;
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
			if (IsChanBasicChannel(chan))
			{	/* This channel is a basic channel */
				bci[b].basicchan = i;	/* channel number */
				bci[b].mode = GetChanMode(chan); /* MIDI mode:0,1,2,3 */
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
 The API function set a new list of basic channel informations in the synthesizer.
 This list replace the previous list. 

 * @param synth, FluidSynth instance.
 * @param n, number of entry in basicChannelInfos.
 * @param  basicChannelInfos, the list of basic channel infos  to set. 
 * @     
 * @ If n is 0 or basicChannelInfos is NULL, the function set one channel basic at
 * @ basicchan 0 in Omni On Poly (i.e all the MIDI channel are polyphonic).
 * @     
 * @ Each entry in the table is a fluid_basic_channels_infos_t 
 * @    -basicchan is the Basic Channel number (0 to MIDI channel count-1)
 * @    -mode is MIDI mode infos for basicchan (0 to 3)
 * @    -val is the value (for mode 3 only) (0 to MIDI channel count)
 * @
 * @return
 * @ FLUID_OK if success
 * @ FLUID_POLYMONO_WARNING prevent about entries coherence in the table.
 * @    -Different entries have the same basic channel.An entry supersedes
 * @     a previous entry with the same basic channel.
 * @    -Val have a number of channels that overlaps the next basic channel.
 * @     Anyway, the function does the job and restricts val to the right value.
 * @ FLUID_FAILED 
 * @  synth is NULL.
 * @  n, basicchan or val is outside MIDI channel count.
 * @  mode is invalid. 
 * @
 * @ Note:This API is the only one to replace all the basics channels in one 
 * @ synth instance.
 * @ The default shell have equivalent command "resetbasicchannels" to set one
 * @ or more basic channels.
*/
int fluid_synth_reset_basic_channels(fluid_synth_t* synth, 
                             int n, 
                             fluid_basic_channel_infos_t *basicChannelInfos)
{
	int i,nChan;
	int result;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */
	if( n < 0 || n > nChan) return FLUID_FAILED;
	/* Check if information are valid  */
	if(n && basicChannelInfos ) for (i = 0; i < n; i++)
	{
		if (basicChannelInfos[i].basicchan < 0 || 
			basicChannelInfos[i].basicchan >= nChan ||
			basicChannelInfos[i].mode < 0 ||
			basicChannelInfos[i].mode >= MODE_NBR ||
			basicChannelInfos[i].val < 0 ||
			basicChannelInfos[i].basicchan + basicChannelInfos[i].val > nChan)
			return FLUID_FAILED;
	}

	fluid_synth_api_enter(synth);
	/* Clear previous list of basic channel */
	for(i = 0; i <  nChan; i++) {
		ResetBasicChanInfos(synth->channel[i]);
		synth->channel[i]->mode_val = 0; 
	}
	if(n && basicChannelInfos)
	{
		result = FLUID_OK;
		
		/* Set the new list of basic channel */
		for (i = 0; i < n; i++)
		{
			
			int bchan = basicChannelInfos[i].basicchan;
			if (IsChanBasicChannel(synth->channel[bchan]))
				/* Different entries have the same basic channel 
				 An entry supersedes a previous entry with the same 
				 basic channel.*/
				result = FLUID_POLYMONO_WARNING;
			/* Set Basic channel first */
			else SetModeBasicChan(synth->channel[bchan]->mode);
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
	fluid_synth_api_exit(synth);
	return result;
}

/**
 The API function change the mode of a an existing basic channel or insert a new
 basic channel part.
 -if basicchan is already a basic channel, his mode is changed.
 -If basicchan is not a basic channel, a new basic channel part is inserted
  between the previous basic channel and the next basic channel.
  val value of the previous basic channel will be narrowed if necessary.
 
 * @param synth, FluidSynth instance.
 * @param basicchan is the Basic Channel number (0 to MIDI channel count-1).
 * @param mode is MIDI mode infos for basichan (0 to 3).
 * @param val Number of monophonic channels (for mode 3 only) (0 to MIDI channel count).

 * @return
 * @ FLUID_OK if success
 * @ FLUID_POLYMONO_WARNING 
 * @  1)val of the previous basic channel has been narrowed or
 * @  2)val have a number of channels that overlaps the next basic channel part.
 * @  Anyway, the function does the job and restricts val to the right value.
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan or val is outside MIDI channel count.
 * @   mode is invalid.
*/
int fluid_synth_set_basic_channel(fluid_synth_t* synth, int basicchan, int mode, int val)
{
	int nChan;
	int result;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */

	if (basicchan < 0 || basicchan >= nChan || 
		mode < 0 ||mode >= MODE_NBR ||
		val < 0 || basicchan + val > nChan)
		return FLUID_FAILED;

	fluid_synth_api_enter(synth);
	/**/
	result = fluid_synth_set_basic_channel_LOCAL(synth, basicchan,mode,val);
	/**/
	fluid_synth_api_exit(synth);
	return result;
}

/**
 The internal function change the mode of a an existing basic channel or insert
 a new basic channel part.
 -if basicchan is already a basic channel, his mode is changed.
 -If basicchan is not a basic channel, a new basic channel part is inserted
  between the previous basic channel and the next basic channel.
  val value of the previous basic channel will be narrowed if necessary.
 
 The function is used internally by API  fluid_synth_reset_basic_channels(),
										fluid_synth_set_basic_channel()

 * @param synth, FluidSynth instance.
 * @param basicchan is the Basic Channel number (0 to MIDI channel count-1).
 * @param mode is MIDI mode infos for basichan (0 to 3).
 * @param val is the value (for mode 3 only) (0 to MIDI channel count).

 * @return
 * @ FLUID_OK if success
 * @ FLUID_POLYMONO_WARNING 
 * @  1)val of the previous basic channel has been narrowed or
 * @  2)val have a number of channels that overlaps the next basic
 * @  channel part or val is outside MIDI channel count.
 * @  Anyway, the function does the job and restricts val to the right value.
 * @ FLUID_FAILED basicchan is outside MIDI channel count.
 * @
 * @ Note: The default shell have equivalent command "setbasicchannels".
*/
int fluid_synth_set_basic_channel_LOCAL(fluid_synth_t* synth, 
											 int basicchan,int mode, int val)
{
	int nChan = synth->midi_channels; /* MIDI Channels number */
	int result = FLUID_FAILED; /* default return */
	if (basicchan < nChan)
	{
		int LastBeginRange; /* Last channel num inside the beginning range + 1. */
		int LastEndRange; /* Last channel num inside the ending range + 1. */
		int i;
		result = FLUID_OK;
		if ( !IsChanBasicChannel(synth->channel[basicchan]))
		{	/* a new basic channel is inserted between previous basic channel 
			and the next basic channel.	*/
			if ( IsChanEnabled(synth->channel[basicchan]))
			{ /* val value of the previous basic channel need to be narrowed */
				for (i = basicchan - 1; i >=0; i--)
				{	/* search previous basic channel */
					if (IsChanBasicChannel(synth->channel[i]))
					{	/* i is the previous basic channel */
						/* val of previous is narrowed */
						synth->channel[i]->mode_val = basicchan - i;
						result = FLUID_POLYMONO_WARNING;
						break;
					}
				}
			}	
		}

		/* LastEndRange: next basic channel  or midi_channels count  */
		for (LastEndRange = basicchan +1; LastEndRange < nChan; LastEndRange++)
		{
			if (IsChanBasicChannel(synth->channel[LastEndRange])) break;
		}
		/* Now LastBeginRange is set */
		switch (mode = GetModeMode(mode))
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
		/* LastBeginRange limited up to LastEndRange */
		if (LastBeginRange > LastEndRange)
		{	
			LastBeginRange = LastEndRange;
			/* val have number of channel that overlaps the next basic channel */
			result = FLUID_POLYMONO_WARNING;
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
			if (i == basicchan)	SetModeBasicChan(newmode); 
			else val =0; /* val is 0 for other channel than basic channel */
			/* Channel in beginning zone are enabled */
			if (i < LastBeginRange) SetModeChanEn(newmode); 
			/* Channel in ending zone are disabled */
			else newmode = 0;
			/* Now mode is OMNI OFF/ON,MONO/POLY, BASIC_CHANNEL or not
			   ENABLED or not */	
			SetBasicChanInfos(synth->channel[i],newmode);
			synth->channel[i]->mode_val = val;
		}
	}
	return result;
}

/**
 The API function returns poly mono mode informations about any
 MIDI channel.

 * @param synth, FluidSynth instance.
 * @param chan, any MIDI channel number to get mode (0 to MIDI channel count - 1).
 * @param  modeInfos, pointer to a fluid_basic_channels_infos_t. 
 * @    -basicchan , chan.
 * @    -mode is MIDI mode infos of chan:
 * @     bit 0: MONO:	0,Polyphonique;	1,Monophonique.
 * @     bit 1: OMNI:	0,Omni  On;		1,Omni Off.
 * @     bit 2: BASIC_CHANNEL:	1, this channel is a Basic Channel.
 * @     bit 3: ENABLED: 1,chan is listened; 
 * @                     0, voices messages (MIDI note on/of, cc) are ignored on chan.
 * @    -val, number of channels in the group from basic channel (if bit 2 is set),
 * @          or 0 if bit 2 is 0.
 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @  synth s NULL.
 * @  chan is outside MIDI channel count or modeInfos is NULL.
 
 * @ Note: The default shell have equivalent command "channelsmode".

*/
int fluid_synth_get_channel_mode(fluid_synth_t* synth, int chan,
									 fluid_basic_channel_infos_t  *modeInfos)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (modeInfos!= NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */
	if(chan < 0 || chan >= nChan) return FLUID_FAILED;
	fluid_synth_api_enter(synth);
	/**/
	modeInfos->basicchan= chan;
	modeInfos->mode = synth->channel[chan]->mode;
	modeInfos->val = synth->channel[chan]->mode_val;
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}
/**  API legato mode *********************************************************/
/**
 The API function set the legato mode for a channel.
 
 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param legatomode .
 *  0: RETRIGGER_0 (fast release)	1: RETRIGGER_1 (normal release)
 *  2: MULTI_RETRIGGER 		        3: SINGLE_TRIGGER_0		4 : SINGLE_TRIGGER_1

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
 * @   legatomode is invalid.
*/
int fluid_synth_set_legato_mode(fluid_synth_t* synth, int chan, int legatomode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */

	if (chan < 0 || chan >= nChan || 
		legatomode < 0 ||legatomode >= LEGATOMODE_NBR )
		return FLUID_FAILED;

	fluid_synth_api_enter(synth);
	/**/
	SetChanLegatoMode(synth->channel[chan],legatomode);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**
 The API function get the legato mode for a channel.
 
 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param legatomode, pointer to returned mode .
 *  0: RETRIGGER_0 (fast release)	1: RETRIGGER_1 (normal release)
 *  2: MULTI_RETRIGGER 		        3: SINGLE_TRIGGER_0		4 : SINGLE_TRIGGER_1

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
 * @   legatomode is NULL.
*/
int fluid_synth_get_legato_mode(fluid_synth_t* synth, int chan, int *legatomode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (legatomode!= NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */
	if(chan < 0 || chan >= nChan) return FLUID_FAILED;
	fluid_synth_api_enter(synth);
	/**/
	* legatomode = GetChanLegatoMode(synth->channel[chan]);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**  API portamento mode *********************************************************/
/**
 The API function set the portamento mode for a channel.
 
 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param portamentomode .
 *  0: EACH_NOTE	 	1: LEGATO_ONLY	 
 *  2: STACCATO_ONLY

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
 * @   portamentomode is invalid.
*/
int fluid_synth_set_portamento_mode(fluid_synth_t* synth, int chan,
									int portamentomode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */

	if (chan < 0 || chan >= nChan || 
		portamentomode < 0 ||portamentomode >= PORTAMENTOMODE_NBR )
		return FLUID_FAILED;

	fluid_synth_api_enter(synth);
	/**/
	SetChanPortamentoMode(synth->channel[chan],portamentomode);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**
 The API function get the portamento mode for a channel.
 
 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param portamentomode pointer to returned mode.
 *  0: EACH_NOTE	 	1: LEGATO_ONLY	 
 *  2: STACCATO_ONLY

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
 * @   portamentomode is NULL.
*/
int fluid_synth_get_portamento_mode(fluid_synth_t* synth, int chan,
									int *portamentomode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (portamentomode!= NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */
	if(chan < 0 || chan >= nChan) return FLUID_FAILED;
	fluid_synth_api_enter(synth);
	/**/
	* portamentomode = GetChanPortamentoMode(synth->channel[chan]);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**  API breath mode *********************************************************/
/**
 The API function set the breath mode for a channel. 

 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param breathmode bits
 *        BREATH_POLY       default breath poly On/Off 
 *        BREATH_MONO       default breath mono On/Off 
 *		  BREATH_SYNC       breath noteOn/noteOff triggering On/Off

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
*/
int fluid_synth_set_breath_mode(fluid_synth_t* synth, int chan, int breathmode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */

	if (chan < 0 || chan >= nChan )
		return FLUID_FAILED;

	fluid_synth_api_enter(synth);
	/**/
	SetBreathInfos(synth->channel[chan],breathmode);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**
 The API function get the breath mode option for a channel.
 
 * @param synth, FluidSynth instance.
 * @param chan,  MIDI Channel number (0 to MIDI channel count-1).
 * @param breathmode, pointer to returned breath infos .
 *        BREATH_POLY       default breath poly On/Off 
 *        BREATH_MONO       default breath mono On/Off
 *		  BREATH_SYNC       breath noteOn/noteOff triggering On/Off

 * @return
 * @ FLUID_OK if success
 * @ FLUID_FAILED 
 * @   synth is NULL.
 * @   chan outside MIDI channel count.
 * @   breathmode is NULL.
*/
int fluid_synth_get_breath_mode(fluid_synth_t* synth, int chan, int *breathmode)
{
	int nChan;
	/* check parameters first */
	fluid_return_val_if_fail (synth != NULL, FLUID_FAILED);
	fluid_return_val_if_fail (breathmode!= NULL, FLUID_FAILED);
	nChan = synth->midi_channels; /* MIDI Channels number */
	if(chan < 0 || chan >= nChan) return FLUID_FAILED;
	fluid_synth_api_enter(synth);
	/**/
	* breathmode = GetBreathInfos(synth->channel[chan]);
	/**/
	fluid_synth_api_exit(synth);
	return FLUID_OK;
}

/**  commands Poly/mono mode *************************************************/

/*-----------------------------------------------------------------------------
  basicchannels 
   Print the list of all MIDI basic channels informations 
   example:

	"Basic channel:  0, poly omni on (0), nbr:  3"
	"Basic channel:  3, poly omni off(2), nbr:  1"
	"Basic channel:  8, mono omni off(3), nbr:  2"
	"Basic channel: 13, mono omni on (1), nbr:  3"
*/
char * modename[]={	"poly omni on (0)","mono omni on (1)",
					"poly omni off(2)","mono omni off(3)"};
int fluid_handle_basicchannels (fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
	fluid_basic_channel_infos_t *bci; /* basic channels table */
    /* get list of basic channels */
	int n = fluid_synth_get_basic_channels(synth, &bci);
	if (n > 0)
	{	int i;
		/* display all basic channels */
		for (i =0; i< n; i++)
		{
			fluid_ostream_printf(out,
		            "Basic channel:%3d, %s, nbr:%3d\n", bci[i].basicchan, 
					modename[bci[i].mode], 
					bci[i].val);
		}
		/* bci has been allocated by fluid_synth_get_basic_channels() */ 
        free(bci); 
	}
	/* n is 1 or more basic channel number (never 0)*/
	else if(n < 0) return -1; /* error */
	if (n == 0) fluid_ostream_printf(out,"no basic channels\n");
	return 0;
}

char * WarningMsg ="resetbasicchannels: warning:\n\
-(1) Different entries have the same basic channel.An entry supersedes\n\
 a previous entry with the same basic channel.\n\
-(2) Number of channels in one entry overlaps the next basic channel.\n\
Anyway, the command succeeds and restricts nbr to the right value.\n";

char *InvalidArg =" invalid argument";
char *TooFewArg = " too few argument, chan mode val [chan mode val]...";
/*-----------------------------------------------------------------------------
  resetbasicchannels [chan1 Mode1 nbr1   chan2 Mode2 nbr2 ...]
  
  Set the list of MIDI basic channels with mode
  This list replace any previous basic channels list.

  With no parameters the function set one channel basic at
  basicchan 0 mode 0 (Omni On Poly) (i.e all the MIDI channel are polyphonic).
*/
int fluid_handle_resetbasicchannels (fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
    int result;
	int i,n = 0;
	fluid_basic_channel_infos_t * bci = NULL;

	if (ac )
	{	/* parameters for list entries */
		for (i = 0; i < ac; i++)
		{
			if (!fluid_is_number(av[i]))
			{
				fluid_ostream_printf(out, "resetbasicchannels:%s\n",InvalidArg);
				return -1;	
			}
		}
		n = ac / 3; /* number of basic channel information */
		if (ac % 3)
		{	/* each entry needs 3 parameters: basicchan,mode,val */
			fluid_ostream_printf(out, "resetbasicchannels:chan %d,%s\n",
						atoi(av[(n * 3)]),TooFewArg);
			return -1;	
		}
		/* alloc bci table and fill  */
		bci = FLUID_ARRAY(fluid_basic_channel_infos_t, n );
		for (i = 0; i < n; i++)
		{
			bci[i].basicchan = atoi(av[(i * 3)]); 
			bci[i].mode = atoi(av[(i * 3)+1]); 
			bci[i].val = atoi(av[(i * 3)+2]); 
		}
	}
	/* set list of basic channels */
	result = fluid_synth_reset_basic_channels(synth,n, bci);
	if(bci) free(bci);
	if (result==FLUID_POLYMONO_WARNING)
		fluid_ostream_printf(out, WarningMsg);
	if (result == FLUID_FAILED)  
		fluid_ostream_printf(out, "resetbasicchannels:%s\n",InvalidArg);
	return 0;
}

char * WarningMsg1 ="warning:\n\
-(1) val of the previous basic channel has been narrowed or\n\
-(2) Number of channels in one entry overlaps the next basic channel.\n\
Anyway, the command succeeds and restricts nbr to the right value.\n";

/*-----------------------------------------------------------------------------
  setbasicchannels chan1 Mode1 nbr1    [chan2 Mode2 nbr2..]
  
  Change or add basic channel 1 and 2
  
  -if chan is already a basic channel, his mode is changed.
  -If chan is not a basic channel, a new basic channel part is inserted
  between the previous basic channel and the next basic channel.
  val value of the previous basic channel will be narrowed if necessary.

*/
int fluid_handle_setbasicchannels (fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
    int result;
	int i,n ;

	if (ac )
	{	/* parameters for list entries */
		for (i = 0; i < ac; i++)
		{
			if (!fluid_is_number(av[i]))
			{
				fluid_ostream_printf(out, "setbasicchannels:%s\n",InvalidArg);
				return -1;	
			}
		}
	}
	n = ac / 3; /* number of basic channel information */
	if(!ac || ac % 3) 
	{	/* each entry needs 3 parameters: basicchan,mode,val */
		fluid_ostream_printf(out, "setbasicchannels:chan %d,%s\n",
					atoi(av[(n * 3)]),TooFewArg);
		return -1;	
	}

	for (i = 0; i < n; i++)
	{
		int basicchan = atoi(av[(i * 3)]); 
		int mode = atoi(av[(i * 3)+1]); 
		int val = atoi(av[(i * 3)+2]); 
		/* change basic channels */
	
		result = fluid_synth_set_basic_channel(synth,basicchan,mode,val);
		if (result==FLUID_POLYMONO_WARNING)
			fluid_ostream_printf(out,"channel:%3d, mode:%3d, nbr:%3d, %s",
				basicchan,mode, val, WarningMsg1);
		if (result == FLUID_FAILED)  
			fluid_ostream_printf(out,"channel:%3d, mode:%3d, nbr:%3d, %s\n",
				basicchan,mode, val, InvalidArg);
	}
	return 0;
}

/*-----------------------------------------------------------------------------
  channelsmode
     Print channel mode of all MIDI channels (Poly/mono, Enabled, Basic Channel)
     example 

     channel:  0, disabled
     channel:  1, disabled
     channel:  2, disabled
     channel:  3, disabled
     channel:  4, disabled
     channel:  5, enabled, basic channel, mono omni off(3), nbr:  2
     channel:  6, enabled, --           , mono            , --
     channel:  7, disabled
     channel:  8, disabled
     channel:  9, disabled
     channel: 10, enabled, basic channel, mono omni off(3), nbr:  4
     channel: 11, enabled, --           , mono            , --
     channel: 12, enabled, --           , mono            , --
     channel: 13, enabled, --           , mono            , --
     channel: 14, disabled
     channel: 15, disabled
  
 channelsmode chan1 chan2
     Print only channel mode of MIDI channel chan1, chan2
*/
int fluid_handle_channelsmode (fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
	fluid_basic_channel_infos_t bci; /* basic channels infos */
	int i,result;
    int n,nChan= synth->midi_channels; 
	
	for (i = 0; i < ac; i++)
	{
		if (!fluid_is_number(av[i]))
		{
			fluid_ostream_printf(out, "channelsmode:%s\n",InvalidArg);
			return -1;
		}
	}
	if (ac ) n = ac; /* print ac MIDI channels number */
	else n= nChan; /* print all MIDI channels number */
	/* print header */	
	fluid_ostream_printf(out,"Channel    , Status , Type         , Mode            , Nbr of channels\n");
	for (i = 0; i < n; i++)
	{
		int chan = ac ? atoi(av[i]): i;
		result = fluid_synth_get_channel_mode(synth, chan, &bci);
		if (result == FLUID_OK)
		{
			if(IsModeChanEn(bci.mode))
			{	/* This channel is enabled */
				char * basicchannel1="basic channel"; /* field basic channel */
				char * bcmsg; /* field basic channel */
				char * polymsg ="poly"; /* field mode */
				char * monomsg ="mono"; /* field mode */
				char * pMode; /* field mode */
				char * blank="--";
				char nbr1[10]; /* field Nbr */
				char *pNbr; /* field Nbr */
				int mode = GetModeMode(bci.mode);
				if (IsModeBasicChan(bci.mode))
				{	/* This channel is a basic channel */
					bcmsg = basicchannel1;
					sprintf(nbr1,"nbr:%3d",bci.val);
					pNbr = nbr1;
					pMode = modename[mode];
				}
				else
				{	/* This channel is member of a part */
					bcmsg = blank;		pNbr = blank;
					if(IsModeMono(mode)) pMode = monomsg;
					else pMode = polymsg;
				}
				fluid_ostream_printf(out,
						"channel:%3d, enabled, %-13s, %-16s, %s\n", 
						bci.basicchan,
						bcmsg,
						pMode,
						pNbr);
			}
			else fluid_ostream_printf(out, "channel:%3d, disabled\n",bci.basicchan);
		}
		else fluid_ostream_printf(out,
							"channel:%3d is is outside MIDI channel count(%d)\n",
							chan,nChan); 
	}
	return 0;
}

/**  commands mono legato mode ***********************************************/
/*-----------------------------------------------------------------------------
 legatomode
     Print legato mode of all MIDI channels
     example 

     channel:  0, (2)single-trigger_0
     channel:  1, (1)multi-retrigger
     channel:  2, (0)retrigger_0
     channel:  3, (3)single-trigger_1
     .....
  
 legatomode chan1 chan2
     Print only legato mode of MIDI channel chan1, chan2
*/
char * nameLegatomode[LEGATOMODE_NBR]={
	"(0)retrigger_0 (fast release)","(1)retrigger_1 (normal release)",
	"(2)multi-retrigger","(3)single-trigger_0","(4)single-trigger_1"
};

int fluid_handle_legatomode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
	int mode;
	int i,result;
    int n,nChan= synth->midi_channels; 
	
	for (i = 0; i < ac; i++)	{
		if (!fluid_is_number(av[i]))		{
			fluid_ostream_printf(out, "legatomode:%s\n",InvalidArg);
			return -1;
		}
	}
	if (ac ) n = ac; /* print ac MIDI channels number */
	else n= nChan; /* print all MIDI channels number */
	/* print header */	
	fluid_ostream_printf(out,"Channel    , legato mode\n");
	for (i = 0; i < n; i++)
	{
		int chan = ac ? atoi(av[i]): i;
		result = fluid_synth_get_legato_mode(synth, chan, &mode);
		if (result == FLUID_OK)
				fluid_ostream_printf(out,"channel:%3d, %s\n",chan,
										nameLegatomode[mode]);
		else fluid_ostream_printf(out,
							"channel:%3d is is outside MIDI channel count(%d)\n",
							chan,nChan); 
	}
	return 0;
}


/*-----------------------------------------------------------------------------
  setlegatomode chan1 Mode1 [chan2 Mode2 ..]
  
  Change legato mode for channels chan1 and [chan2]
*/
char *TooFewArgChanMode = " too few argument, chan mode [chan mode]...";
int fluid_handle_setlegatomode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
    int result;
	int i,n ;

	if (ac )
	{	/* parameters for list entries */
		for (i = 0; i < ac; i++)
		{
			if (!fluid_is_number(av[i]))
			{
				fluid_ostream_printf(out, "setlegatomode:%s\n",InvalidArg);
				return -1;	
			}
		}
	}
	n = ac / 2; /* number of legato information */
	if(!ac || ac % 2) 
	{	/* each entry needs 2 parameters: chan,mode */
		fluid_ostream_printf(out, "setlegatomode:chan %d,%s\n",
					atoi(av[(n * 2)]),TooFewArgChanMode);
		return -1;	
	}

	for (i = 0; i < n; i++)
	{
		int chan = atoi(av[(i * 2)]); 
		int mode = atoi(av[(i * 2)+1]); 
		/* change legato mode */
	
		result = fluid_synth_set_legato_mode(synth,chan,mode);
		if (result == FLUID_FAILED)  
			fluid_ostream_printf(out,"chan:%3d, mode:%3d, %s\n",
									chan,mode, InvalidArg);
	}
	return 0;
}

/*-----------------------------------------------------------------------------
 portamentomode
     Print portamento mode of all MIDI channels
     example 

     channel:  0, (2)staccato only
     channel:  1, (1)legato only
     channel:  2, (0)each note
     channel:  3, (1)legato only
     .....
  
 portamentotomode chan1 chan2
     Print only portamentoto mode of MIDI channel chan1, chan2
*/
char * namePortamentomode[PORTAMENTOMODE_NBR]={
	"(0)each note","(1)legato only",
	"(2)staccato only"
};

int fluid_handle_portamentomode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
	int mode;
	int i,result;
    int n,nChan= synth->midi_channels; 
	
	for (i = 0; i < ac; i++)	{
		if (!fluid_is_number(av[i]))		{
			fluid_ostream_printf(out, "portamentomode:%s\n",InvalidArg);
			return -1;
		}
	}
	if (ac ) n = ac; /* print ac MIDI channels number */
	else n= nChan; /* print all MIDI channels number */
	/* print header */	
	fluid_ostream_printf(out,"Channel    , portamentoto mode\n");
	for (i = 0; i < n; i++)
	{
		int chan = ac ? atoi(av[i]): i;
		result = fluid_synth_get_portamento_mode(synth, chan, &mode);
		if (result == FLUID_OK)
				fluid_ostream_printf(out,"channel:%3d, %s\n",chan,
										namePortamentomode[mode]);
		else fluid_ostream_printf(out,
							"channel:%3d is is outside MIDI channel count(%d)\n",
							chan,nChan); 
	}
	return 0;
}


/*-----------------------------------------------------------------------------
  setportamentomode chan1 Mode1 [chan2 Mode2 ..]
  
  Change portamento mode for channels chan1 and [chan2]
*/
int fluid_handle_setportamentomode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
    int result;
	int i,n ;

	if (ac )
	{	/* parameters for list entries */
		for (i = 0; i < ac; i++)
		{
			if (!fluid_is_number(av[i]))
			{
				fluid_ostream_printf(out, "setportamentomode:%s\n",InvalidArg);
				return -1;	
			}
		}
	}
	n = ac / 2; /* number of portamento information */
	if(!ac || ac % 2) 
	{	/* each entry needs 2 parameters: chan,mode */
		fluid_ostream_printf(out, "setportamentomode:chan %d,%s\n",
					atoi(av[(n * 2)]),TooFewArgChanMode);
		return -1;	
	}

	for (i = 0; i < n; i++)
	{
		int chan = atoi(av[(i * 2)]); 
		int mode = atoi(av[(i * 2)+1]); 
		/* change portamento mode */
	
		result = fluid_synth_set_portamento_mode(synth,chan,mode);
		if (result == FLUID_FAILED)  
			fluid_ostream_printf(out,"chan:%3d, mode:%3d, %s\n",
									chan,mode, InvalidArg);
	}
	return 0;
}


/*-----------------------------------------------------------------------------
 breathmode
     Print breath options of all MIDI channels (poly on/off, mono on/off
	 breath
     example 

	Channel    , poly breath , mono breath , breath sync
	channel:  0, off         , off         , off
	channel:  1, off         , off         , off
	channel:  2, off         , off         , off
	.....
  
 breathmode chan1 chan2
     Print only breath mode of MIDI channel chan1, chan2
*/
char * Onmsg ="on";
char * Offmsg ="off";
int fluid_handle_breathmode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
	int breathmode;
	int i,result;
    int n,nChan= synth->midi_channels; 
	
	for (i = 0; i < ac; i++)	{
		if (!fluid_is_number(av[i]))		{
			fluid_ostream_printf(out, "breathmode:%s\n",InvalidArg);
			return -1;
		}
	}
	if (ac ) n = ac; /* print ac MIDI channels number */
	else n= nChan; /* print all MIDI channels number */
	/* print header */	
	fluid_ostream_printf(out,"Channel    , poly breath , mono breath , breath sync\n");
	for (i = 0; i < n; i++)
	{
		int chan = ac ? atoi(av[i]): i;
		result = fluid_synth_get_breath_mode(synth, chan, &breathmode);
		if (result == FLUID_OK)
		{
				char * msgPolyBreath, * msgMonoBreath, * msgBreathSync; 
				if (IsPolyDefaultBreath(breathmode))
					msgPolyBreath =Onmsg;
				else msgPolyBreath = Offmsg;
				if (IsMonoDefaultBreath(breathmode))
					msgMonoBreath =Onmsg;
				else msgMonoBreath = Offmsg;
				if (IsBreathSync(breathmode))
					msgBreathSync =Onmsg;
				else msgBreathSync = Offmsg;
				fluid_ostream_printf(out,"channel:%3d, %-12s, %-12s, %-11s\n",chan,
										msgPolyBreath, msgMonoBreath, msgBreathSync);
		}
		else fluid_ostream_printf(out,
							"channel:%3d is is outside MIDI channel count(%d)\n",
							chan,nChan); 
	}
	return 0;
}

/*-----------------------------------------------------------------------------
  setbreathmode chan1 poly_breath_mod(1/0) mono_breath_mod mono_breath_sync(1/0)
  
  Change breath options for channels chan1 and [chan2...]
*/
char *TooFewArgBreath = 
" too few argument:\nchan 1/0(breath poly) 1/0(breath mono) 1/0(breath sync mono)[..]";
int fluid_handle_setbreathmode(fluid_synth_t* synth, int ac, char** av, 
								fluid_ostream_t out)
{
    int result;
	int i,n, nChan= synth->midi_channels; 

	if (ac )
	{	/* parameters for list entries */
		for (i = 0; i < ac; i++)
		{
			if (!fluid_is_number(av[i]))
			{
				fluid_ostream_printf(out, "setbreathmode:%s\n",InvalidArg);
				return -1;	
			}
		}
	}
	n = ac / 4; /* number of default breath informations */
	if(!ac || ac % 4) 
	{	/* each entry needs 3 parameters: chan,chan1 poly_breath(1/0) mono_breath(1/0) */
		fluid_ostream_printf(out, "setbreathmode:chan %d,%s\n",
					atoi(av[(n * 3)]),TooFewArgBreath);
		return -1;	
	}

	for (i = 0; i < n; i++)
	{
		int chan = atoi(av[(i * 4)]); 
		int poly_breath = atoi(av[(i * 4)+1]); 
		int mono_breath = atoi(av[(i * 4)+2]);
		int breath_sync = atoi(av[(i * 4)+3]);
		int breath_infos = 0;
		/* change ldefault breath  */
		if(poly_breath) SetPolyDefaultBreath(breath_infos);
		if(mono_breath) SetMonoDefaultBreath(breath_infos);
		if(breath_sync) SetBreathSync(breath_infos);
		result = fluid_synth_set_breath_mode(synth,chan,breath_infos);
		if (result == FLUID_FAILED)  
				fluid_ostream_printf(out,
							"channel:%3d is is outside MIDI channel count(%d)\n",
							chan,nChan); 
	}
	return 0;
}

