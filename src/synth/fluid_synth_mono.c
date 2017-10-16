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
#include "fluid_defsfont.h"

extern void fluid_synth_release_voice_on_same_note_LOCAL(fluid_synth_t* synth,
                                                            int chan, int key);
/**  monophonic playing *******************************************************/

/******************************************************************************
  Monophonic list methods
******************************************************************************/

/**
 * Add a note to the monophonic list.
 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127, 0=noteoff).
 * @param onenote. When 1 the function add the note but the monophonic
 *                 keeps only one note.
 * Note: iLast index keeps a trace of the most recent note inserted.
 *       PrevNote keeps a trace of the note prior iLast note.
 *       ChanLegato bit keeps trace of legato/staccato playing.
 */
static void
fluid_channel_add_monolist(fluid_channel_t* chan, unsigned char key,
						   unsigned char vel, unsigned char onenote)
{
	unsigned char iLast = chan->iLast;
	if (chan->nNotes) SetChanLegato(chan); /* update Legato playing bit */
	else ResetChanLegato(chan); /* update Staccato playing bit */
	/* keep trace of the note prior last note */
	if(chan->nNotes) chan->PrevNote = chan->monolist[iLast].note;
	/* update iLast before writing new note */
	iLast = chan->monolist[iLast].next; 
	chan->iLast = iLast; 			/* now ilast is the last note */
	chan->monolist[iLast].note = key; /* we save note and velocity */
	chan->monolist[iLast].vel = vel; 	
	if (onenote) { /* clear monolist to one note addition */
		chan->iFirst = iLast;		chan->nNotes = 0;
	}
	if(chan->nNotes < maxNotes) chan->nNotes++; /* update nNotes */
	else { /* overflow situation. So circular motion for iFirst */
		chan->iFirst = chan->monolist[iLast].next;
		/* warning */
        FLUID_LOG(FLUID_INFO, "Overflow on monophonic list channel %d ",
								chan->channum);
	}
}

/**
 * Searching a note in the monophonic list.
 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127) to search.
 * @return index of the note if find, InvalidNote otherwise.
 */
static unsigned short
fluid_channel_search_monolist(fluid_channel_t* chan, unsigned char key)
{
	short n = chan->nNotes; /* number of notes in monophonic list */
	short i= chan->iFirst; /* searching starts from iFirst included */
	while(n) {
		if(chan->monolist[i].note == key) break; /* found */
		i = chan->monolist[i].next; /* next element */
		n--;
	}
	if (n) return i;/* found i */
	else return InvalidNote; /* not found */
}

/**
 * removing a note out of the monophonic list.
 * @param chan  fluid_channel_t.
 * @param i, index of the note to remove.
 * If i is invalid or the list is empty, the function do nothing and returns
 * InvalideNote.
 * return prev index  if the note is the last note in the list,
 *        InvalidNote otherwise.
 * Note: iLast index keeps a trace of the most recent note played even if
 *       the list is empty.
 *       PrevNote keeps a trace of the note removed.
 *       ChanLegato bit keeps trace of legato/staccato playing
 */
static unsigned char
fluid_channel_remove_monolist(fluid_channel_t* chan, short i)
{
	unsigned char iPrev = InvalidNote;
	unsigned char iLast = chan->iLast;
	/* check if index is valid */
	if(IsInvalidNote(i) || i >= maxNotes || !chan->nNotes) return InvalidNote;	
	/* The element is about to be removed and inserted between iLast and iNext */
	/* Note: when i is egal to iLast or egal to iFirst, Removing/Inserting
	   isn't necessary */
	if (i == iLast) { /* Removing/Inserting isn't necessary */
		/* keep trace of the note prior last note */
		chan->PrevNote= chan->monolist[iLast].note;
		/* update iLast to the previous  */
		iPrev = chan->monolist[i].prev;
		chan->iLast = iPrev;
	}
	else { /* i is before iLast */
		if(i == chan->iFirst)  chan->iFirst = chan->monolist[i].next;
		else { /* i is between iFirst ans iLast */
			/* Removing element i and inserting between iLast and iNext */
			unsigned char next,prev,nextend;
			/* removing by chaining prev and next */
			next = chan->monolist[i].next;		prev = chan->monolist[i].prev;
			chan->monolist[next].prev = prev;	chan->monolist[prev].next = next;
			/* inserting after iLast */
			nextend = chan->monolist[iLast].next;
			chan->monolist[i].next = nextend; 
			chan->monolist[nextend].prev = i;
			chan->monolist[i].prev = iLast;
			chan->monolist[iLast].next = i;
		}
	}
	chan->nNotes--;
	if (chan->nNotes) SetChanLegato(chan); /* update Legato playing bit */
	else ResetChanLegato(chan); /* update Staccato playing bit */
	return iPrev;
}

/**
 * Removing all notees out of the monophonic list.
 * @param chan  fluid_channel_t.
 * Note: iLast index keeps a trace of the most recent note played even when
 *       the list is empty.
 *       PrevNote keeps a trace of the note .
 *       ChanLegato bit keeps trace of legato/staccato playing
*/
void fluid_channel_clear_monolist(fluid_channel_t* chan)
{
	chan->iFirst = chan->monolist[chan->iLast].next;
	chan->PrevNote= chan->monolist[chan->iLast].note;
	chan->nNotes = 0;
	ResetChanLegato(chan);
}

/**
 * The function is called on legato off
 * The monophonic list is flushed keeping last note only.
 * @param chan  fluid_channel_t.
 * Note: iLast index keeps a trace of the most recent note played.
 *       PrevNote keeps a trace of the note .
 *       ChanLegato bit keeps trace of legato/staccato playing
*/
static void fluid_channel_keep_lastnote_monolist(fluid_channel_t* chan)
{
	chan->iFirst = chan->iLast;
	chan->nNotes = 1;
}


/**
 * Adds the note into the monophonic list , keeping only this note
 * @param chan  fluid_channel_t.
 * Note: iLast index keeps a trace of the most recent note inserted.
 *       PrevNote keeps a trace of the note prior iLast note.
 *       ChanLegato bit keeps trace of legato/staccato playing
 */
void fluid_channel_set_onenote_monolist(fluid_channel_t* chan, unsigned char key,
								            unsigned char vel)
{
	fluid_channel_add_monolist(chan, key, vel,1);
}



/*****************************************************************************
 Portamemto related functions in Poly or Mono mode
******************************************************************************/

/**
 * GetFromKeyPortamentoLegato return two informations:
 * 1)The function determines if a portamento must occur on next noteOn 
 * (PTC, or Portamento On).On portamento On, the function takes into account the
 * 'portamento mode'
 * 'fromkey portamento' which is the pitchstart key of a portamento is returned
 * in fluid_synth_t.fromkey_portamento to enable the portamento.
 *  -When CC PTC has been received its value supersedes the defaultFromkey and any
 *   Portamento pedal and portamento mode.
 *  -When CC PTC haven't received and Portamento is On ,'fromkey portamento' is
 *   the 'defaultFromkey' note if this parameter is valid, otherwise 
 *  'fromkey portamento' is determined from the portamento mode and the note prior
 *  the most  recent note played.
 *  Where portamento mode is: 
 *  - each note, the note prior the most recent note is any note played staccato
 *    or legato.
 *  - legato only, the note prior the most recent note is a note played legato
 *    with the next.Any staccato note and the first note of a legato passage will
 *	 be played without portamento.
 *
 * 2)The function determines if a legato playing must occurs on next noteOn.
 *  'fromkey legato note' is returned in the case of legato playing. 
 *	- When en CC PTC has been received its value supersedes the defaultFromkey
 *	  only if defaultFromkey is invalid.
 *  - When CC PTC haven't received, 'fromkey legato' is  'defaultFromkey'
 *	  if this parameter is valid, otherwise 'fromkey legato' is determined from
 *	  the mono/poly mode and the actual 'staccato/legato' playing state as this:
 *	   - in staccato (poly/Mono), fromkey legato is InvalidNote.
 *	   - in mono mode legato playing, fromkey legato is the note prior the most
 *	     recent note played.
 *	   - in poly mode legato playing, actually we don't want playing legato. So
 *        fromkey legato is InvalidNote.
 *
 * On input
 * @param chan  fluid_channel_t.
 * @param defaultFromkey, the defaut 'fromkey portamento' note or 'fromkey legato'
 *       note (see description above).
 *
 * @return
 *  1)'fromkey portamento' is returned in fluid_synth_t.fromkey_portamento.
 *  If valid,it means that portamento is enabled (by PTC receive or Portamento On).
 *  otherwise it means that portamento is disabled.
 *  During next voices staccato/legato starting process, if fromkey is valid the
 *  portamento will be started.
 *
 *  2) The 'fromkey legato' note is returned in the case of legato playing.
 *
 * The function is intended to be called when the following event occurs:
 * - On noteOn (Poly or Mono) after insertion in the monophonic list.
 * - On noteOff(mono legato playing). In this case, defaultFromkey must be valid.
 *
 * In poly, defaultFromkey must be InvalidNote.
 * In mono staccato playing,defaultFromkey must be InvalidNote. 
 * In mono when legato playing,defaultFromkey must be valid.
 */
static unsigned char GetFromKeyPortamentoLegato(fluid_channel_t* chan, 
						unsigned char defaultFromkey)
{
	unsigned char ptc =  portamentoCtrl(chan);
	if(IsValidNote(ptc))
	{	/* CC PTC has been received */
		clearPortamentoCtrl(chan);	/* clear the CC PTC receive */
		chan->synth->fromkey_portamento =  ptc;/* return fromkey portamento */
		/* return fromkey legato */
		if(IsInvalidNote(defaultFromkey)) defaultFromkey= ptc;
	}
	else 
	{	/* determine and return fromkey portamento */
		unsigned char fromkey_portamento = InvalidNote;
		if(fluid_channel_portamento(chan))
		{	/* Portamento when Portamento pedal is On */
			/* 'fromkey portamento'is determined from the portamento mode
			 and the most recent note played */
			unsigned char portamentomode = GetChanPortamentoMode(chan);
			if(IsValidNote(defaultFromkey)) 
				fromkey_portamento = defaultFromkey; /* on each note */
			else fromkey_portamento = ChanPrevNote(chan); /* on each note */
			if(portamentomode == LEGATO_ONLY)
			{   /* Mode portamento:legato only */
				if(IsChanStaccato(chan)) fromkey_portamento = InvalidNote;
			}
			else if(portamentomode == STACCATO_ONLY)
			{	/* Mode portamento:staccato only */
				if(IsChanLegato(chan)) fromkey_portamento = InvalidNote;
			}
			/* else Mode portamento: on each note (staccato/legato) */
		}
		/* Return fromkey portamento */
		chan->synth->fromkey_portamento = fromkey_portamento;
		/* Determine and return fromkey legato */
		if(IsInvalidNote(defaultFromkey))
		{
			/* in staccato (poly/Mono) return InvalidNote */
			/* In mono mode legato playing return the note prior most 
			   recent note played */
			if (IsChanPlayingMono(chan) && IsChanLegato(chan))
					defaultFromkey = ChanPrevNote(chan); /* note prior last note */
			/* In poly mode legato playing, actually we don't want playing legato.
			So return InvalidNote */
		}
	}
	return defaultFromkey; /* Return legato fromkey */
}

/**
 * The function changes the state (Valid/Invalid) of the previous note played in
 * a staccato manner (ChanPrevNote()).
 * When potamento mode: 'each note' or 'staccato only' is selected, on next 
 * noteOn a portamento will be started from the most recent note played 
 * staccato ChanLastNote.
 * It will be possible that it isn't appropriate. To give the musician the 
 * possibility to choose this note , the note will be marked valid on noteOff
 * if portamento pedalis On, otherwise the note  will be marked invalid.
 *
 * The function is intended to be called when the following event occurs:
 * - On noteOff (in poly or mono mode), to mark previous note valid/invalid.
 * - On Portamento Off(in poly or mono mode), to mark the previous note invalid.
 * @param chan  fluid_channel_t.
 */
void ValidInvalidPrevNoteStaccato(fluid_channel_t* chan)
{
	if(IsChanStaccato(chan)) /* the monophonic list is empty */ 
	if(! fluid_channel_portamento(chan))  ChanClearPrevNote(chan); 
	/* else PrevNote still remains valid for next fromkey portamento */
}

/*****************************************************************************
 noteon - noteoff functions in Mono mode
******************************************************************************/
/*-----------------------------------------------------------------------------
  noteon - noteoff on a channel in "monophonic playing".
  
  A channel needs to be played monophonic if this channel has been set
  monophonic by basic channel API.(see fluid_synth_polymono.c).
  A channel needs also to be played monophonic if it has been set
  polyphonic and legato pedal is On.
  When in "monophonic playing" state only one note at a time can be played in
  a staccato or legato manner.

------------------------------------------------------------------------------*/
static int fluid_synth_noteon_mono_staccato(fluid_synth_t* synth, int chan, 
									 int key, int vel);
int fluid_synth_noteoff_monopoly(fluid_synth_t* synth, int chan, int key,
							 char Mono);

int fluid_synth_noteon_mono_legato(fluid_synth_t* synth, int chan,
							   int fromkey, int tokey, int vel);

/**
 * Plays a note-on event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing".
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param fromkey MIDI note number (0-127). 
 *      previous note if legato playing.
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
int fluid_synth_noteon_mono_LOCAL(fluid_synth_t* synth, int chan, 
									int key,  int vel)
{
	fluid_channel_t* channel = synth->channel[chan];

	/* Adds note to the monophonic list */
	fluid_channel_add_monolist(channel,(unsigned char)key,(unsigned char)vel,0);
	if (!IsChanBreathSync(channel) || fluid_channel_breath_msb(channel) )
	{
		/* legato/staccato playing detection */
		if(IsChanLegato(channel)) { /* legato playing */
			/* legato from iPrev to key */
			/* the voices from iPrev key number are to be used to play key number */
			/* fromkey must be valid */
			return fluid_synth_noteon_mono_legato(synth, chan,
								   ChanPrevNote(channel), key, vel);
		}
		/* staccato playing */
		else return fluid_synth_noteon_mono_staccato(synth, chan, key, vel);
	}
	else return FLUID_OK;
}

/**
 * Plays a note-off event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing".
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
int fluid_synth_noteoff_mono_LOCAL(fluid_synth_t* synth, int chan, int key)
{
	int status;
	unsigned char i,iPrev;
	fluid_channel_t* channel = synth->channel[chan];
	/* search the note in monophonic list */
	i=fluid_channel_search_monolist(channel, (unsigned char)key);
	
	if (IsValidNote(i))	{ /* the note is in monophonic list */
		/* Remove note out the monophonic list */
		iPrev = fluid_channel_remove_monolist(channel,i);

		if (!IsChanBreathSync(channel) || fluid_channel_breath_msb(channel) )
		{
			/* legato playing detection */
			if(IsChanLegato(channel)) { /* the list contains others notes */
				if(IsValidNote(iPrev)) { /* legato playing detection */
					/* legato from key to iPrev key */
					/* the voices from key number are to be used to
					play iPrev key number. */
					status = fluid_synth_noteon_mono_legato(synth, chan,
								   key, channel->monolist[iPrev].note,
								   channel->monolist[iPrev].vel);
				}
				/* else the note doesn't need to be played off */
				else	status = FLUID_OK;
			}
			else { /* the monophonic list is empty */
				/* plays the monophonic note noteoff and eventually held
				by sustain/sostenuto */
				status = fluid_synth_noteoff_monopoly(synth, chan, key, 1);
			}
		}
		else	status = FLUID_OK;
	}
	else { /* the note is not found in the list so the note was
		   played On when we were in polyphonic playing */
			/* play the noteoff as for polyphonic  */
			status = fluid_synth_noteoff_monopoly(synth, chan, key, 0);
	}
	return status;
}

/*----------------------------------------------------------------------------
 staccato playing
-----------------------------------------------------------------------------*/

/**
 * Plays noteon for monophonic note.
 * Please see the description above about "monophonic playing".
 * @param synth FluidSynth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 */
static int
fluid_synth_noteon_mono_staccato(fluid_synth_t* synth,int chan,int key,int vel)
{
	fluid_channel_t* channel = synth->channel[chan];
	
	/* Before playing a new note, if a previous monophonic note is currently
	   sustained it needs to be released */
	fluid_synth_release_voice_on_same_note_LOCAL(synth,chan,
												channel->key_sustained);
	/* Get possible 'fromkey portamento'   */
	GetFromKeyPortamentoLegato( channel, InvalidNote);
	/* The note needs to be played by voices allocation  */
	return fluid_preset_noteon(channel->preset, synth, chan, key, vel);
}

/**
 * Plays noteoff for a polyphonic or monophonic note
 * Please see the description above about "monophonic playing".
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param Mono, 1 noteoff on monophonic note.
 *              0 noteoff on polyphonic note.
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 *
 * Note: On return, on monophonic, sustained note  needs to be remembered
 * in key_sustained.
 * On noteon for a monophonic note if a previous monophonic note is sustained
 * it will be released. Remembering is done here on noteOff.
 */
int fluid_synth_noteoff_monopoly(fluid_synth_t* synth, int chan, int key,
                            char Mono)
{
    int status = FLUID_FAILED;
    fluid_voice_t* voice;
    int i;
    fluid_channel_t* channel = synth->channel[chan];
    /* Key_sustained is prepared to return no note sustained (-1) */
    if (Mono) channel->key_sustained = -1; /* no mono note sustained */
    /* noteoff for all voices with same chan and same key */
    for (i = 0; i < synth->polyphony; i++) {
        voice = synth->voice[i];
        if (fluid_voice_is_on(voice) && (fluid_voice_get_channel(voice) == chan) && (fluid_voice_get_key(voice) == key)) {
            if (synth->verbose) {
                int used_voices = 0;
                int k;
                for (k = 0; k < synth->polyphony; k++) {
                    if (!_AVAILABLE(synth->voice[k])) {
                    used_voices++;
                    }
                }
                FLUID_LOG(FLUID_INFO, "noteoff\t%d\t%d\t%d\t%05d\t%.3f\t%d",
                    fluid_voice_get_channel(voice), fluid_voice_get_key(voice), 0, fluid_voice_get_id(voice),
                    (fluid_curtime() - synth->start) / 1000.0f,
                    used_voices);
            } /* if verbose */
            
            fluid_voice_noteoff(voice);
            /* noteoff on monophonic note */
            /* Key remembering if the note is sustained  */
            if(Mono &&
            (fluid_voice_is_sustained(voice) || fluid_voice_is_sostenuto(voice)))
            {
                channel->key_sustained = key;
            }
            
            status = FLUID_OK;
        } /* if voice on */
    } /* for all voices */
    return status;
}

/*----------------------------------------------------------------------------
 legato playing
-----------------------------------------------------------------------------*/
/**
 * Plays noteon for a monophonic note played legato.
 * Please see the description above about "monophonic playing".
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param fromkey MIDI note number (0-127).
 * @param tokey MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 * Note: The voices with key 'fromkey' are to be used to play key 'tokey'.
 * The fonction is able to play legato over Preset Zone(s) (PZ) and
 * Instrument Zone(s) (IZ).
 * as far as possible. When key tokey is outside the current IZ,PZ current
 * fromkey voices are released.If necessary new voices are restarted when tokey
 * enters in new IZ,PZ.
 */
int fluid_synth_noteon_mono_legato(fluid_synth_t* synth, int chan,
							   int fromkey, int tokey, int vel)
{
	fluid_channel_t* channel = synth->channel[chan];
	unsigned char legatomode = GetChanLegatoMode(channel);
	fluid_voice_t* voice;
	int i ;
	/* Get possible 'fromkey portamento' and possible 'fromkey legato' note  */
	fromkey = GetFromKeyPortamentoLegato( channel, (unsigned char)fromkey);

	if (IsValidNote(fromkey)) for (i = 0; i < synth->polyphony; i++) {
		/* search fromkey voices: only those who don't have 'note off' */
		voice = synth->voice[i];
		if (fluid_voice_is_on(voice) &&
            fluid_voice_get_channel(voice) == chan &&
            fluid_voice_get_key(voice) == fromkey)
            {
			/* Check if tokey is inside the range of the running voice */
			if (fluid_inst_zone_inside_range(voice->inst_zone, tokey, vel)) {
				switch (legatomode) 
				{
					case RETRIGGER_0: /* mode 0 */
						fluid_update_release(voice,0); /* fast release */
					break;
					case RETRIGGER_1: /* mode 1 */
						fluid_update_release(voice,1); /* normal release */
					break;
					case MULTI_RETRIGGER: /* mode 2 */
						/* Skip in attack section */
						fluid_update_multi_retrigger_attack(voice,tokey,vel);
					break;
					case SINGLE_TRIGGER_0: /* mode 3 */
						fluid_update_single_trigger0(voice,fromkey,tokey,vel);
					break;
					case SINGLE_TRIGGER_1: /* mode 4 */
						fluid_update_single_trigger1(voice,fromkey,tokey,vel);
					break;
					default: /* Invalid mode */
					FLUID_LOG(FLUID_WARN, 
						"Failed to execute legato mode: %d",legatomode);
					return FLUID_FAILED;
				}
				if (legatomode >= MULTI_RETRIGGER)	{
					/* Start portamento if enabled */
					if(	IsValidNote(synth->fromkey_portamento))
						/* Send portamento parameters to the voice dsp */
						fluid_voice_update_portamento(voice,
											synth->fromkey_portamento,tokey);
					/* The voice is now used to play tokey in legato manner */
					/* mark this IZ to be ignored during next fluid_preset_noteon() */
					SetIgnoreInstZone (voice->inst_zone);
				}
			}
			else  
			{ /* tokey note is outside the voice range, so the voice is released */
				fluid_update_release(voice,legatomode);
			}
		}
	}
	/* May be,tokey will enter in others IZ,PZ , in this case it needs to be
	played by voices allocation  */
	return fluid_preset_noteon(channel->preset,synth,chan,tokey,vel);
}


/**
 * The function handle Poly/mono commutation on Legato pedal On/Off.
 * @param chan  fluid_channel_t.
 */
void LegatoOnOff(fluid_channel_t* chan, int value)
{
	/* Special handling of the monophonic list  */
	if (IsChanPoly(chan) && chan->nNotes) /* The monophonic list have notes */
	{
		if (value < 64 ) /* legato is released */
		{	/* return from monophonic to polyphonic with note in monophonic list */
			fluid_channel_keep_lastnote_monolist(chan);
		}
		else /* legato is depressed */
		{	/* Inters in monophonic from polyphonic with note in monophonic list */
			/* Stop the running note to remain coherent with Breath Sync mode */
			if (IsChanBreathSync(chan) && !fluid_channel_breath_msb(chan))
				fluid_synth_noteoff_monopoly(chan->synth,chan->channum,
									ChanLastNote(chan),1);
		}
	}
}

/**
 * The function handle CC Breath On/Off detection.
 * @param chan  fluid_channel_t.
 */
void BreathOnOff(fluid_channel_t* chan, int value)
{	
	if (IsChanBreathSync(chan) && IsChanPlayingMono(chan) && (chan->nNotes))
	{	
		/* The monophonic list isn't empty */
		if((value > 0) && (chan->previous_cc_breath == 0))
		{	/* CC Breath On detection */
			fluid_synth_noteon_mono_staccato(chan->synth,chan->channum,
								ChanLastNote(chan),ChanLastVel(chan));
		}
		else if(  (value == 0) && (chan->previous_cc_breath > 0))
		{	/* CC Breath Off detection */
			fluid_synth_noteoff_monopoly(chan->synth, chan->channum,
										ChanLastNote(chan), 1);
		}
	}
	chan->previous_cc_breath = value;
}
