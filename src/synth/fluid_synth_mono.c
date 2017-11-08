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

/* Macros interface to monophonic list variables */
/* Returns the most recent velocity from i_last entry of the monophonic list */
#define fluid_channel_last_vel(chan)	(chan->monolist[chan->i_last].vel)

/* 
  prev_note is used to determine fromkey_portamento as well as 
  fromkey_legato (see get_fromkey_portamento_legato()).

  prev_note is updated on noteOn/noteOff mono by the legato detector as this:
  - On noteOn mono, before adding a new note into the monolist,the most
    recent  note in the list (i.e at i_last position) is kept in prev_note.
  - Similarly, on  noteOff mono , before removing a note out of the monolist, 
    the most recent note (i.e those at i_last position) is kept in prev_note.
*/
#define fluid_channel_prev_note(chan)	(chan->prev_note)

/* 
  LEGATO_PLAYING bit of channel mode keeps trace of the legato /staccato 
  state playing.
  LEGATO_PLAYING bit is updated on noteOn/noteOff mono by the legato detector:
  - On noteOn, before inserting a new note into the monolist.
  - On noteOff, after removing a note out of the monolist.

  - On noteOn, this state is used by fluid_synth_noteon_mono_LOCAL()
  to play the current  note legato or staccato.
  - On noteOff, this state is used by fluid_synth_noteoff_mono_LOCAL()
  to play the current noteOff legato with the most recent note.
*/
/* b7, 1: means legato playing , 0: means staccato playing */
#define LEGATO_PLAYING  0x80 
/* End of interface to monophonic list variables */

/****************************************************************************** 
  The legato detector is composed as this: 
  - monophonic list variable.
  and functions
  - fluid_channel_add_monolist(), for inserting a new note
  - fluid_channel_search_monolist(), for seraching the position of a note
    into the list.
  - fluid_channel_remove_monolist(), for removing a note out of the list.

            The monophonic list
   +------------------------------------------------+
   | +--------------------------------------------+ |
   | |  +----+   +----+          +----+   +----+  | |
   | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
   +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
        +----+   +----+          +----+   +----+
         /|\                      /|\
          |                        |
        i_first                   i_last
 
  The list allows an easy automatic detection of a legato passage when it is
  played on a MIDI keyboard input device.
  It is useful also when the input device is an ewi (electronic wind instrument)
  or evi (electronic valve instrument) and these instruments are unable to send
  MIDI CC legato on/off.
  
  The list remembers the notes in playing order. 
  - (a) On noteOn n2, if a previous note n1 exists, there is a legato
     detection with n1 (with or without portamento from n1 to n2 See note below).
  - (b) On noteOff of the running note n2, if a previous note n1 exists,
     there is a legato detection from n2 to n1, allowing fast trills playing
     (with or without portamento from n2 to n1. See note below).

  Notes in the list are inserted to the end of the list that works like a 
  circular buffer.The features are:  
  
  1) It is always possible to play an infinite legato passage in
     direct order (n1_On,n2_On,n3_On,....).
  
  2) Playing legato in the reverse order (n10_Off, n9_Off,,...) results in
     fast trills playing as the list memorizes 10 most recent notes.
  
  3) Playing an infinite lagato passage in ascendant or descendant order, 
     without playing trills is always possible using the usual way like this:
      First we begin with an ascendant passage,
      n1On, (n2On,n1Off), (n3On,n2Off) , (n4On,n3Off), then
	  we continue with a descendant passage
      (n3On,n4off), (n2On,n3off), (n1On,n2off), n1Off...and so on
   
 Each MIDI channel have a legato detector.

 Note:
  Portamento is a feature independant of the legato detector. So
  portamento isn't part of the lagato detector. However portamento
  (when enabled) is triggered at noteOn (like legato). Like in legato
  situation it is usual to have a portamento from a note 'fromkey' to another
  note 'tokey'. Portamento fromkey note choice is determined at noteOn by
  get_fromkey_portamento_legato() (see below).
  
  More informations in FluidPolyMono-0003.pdf chapter 4 (Appendices).
******************************************************************************/


/**
 * Adds a note into monophonic list. The function is part of the legato 
 * detector. fluid_channel_add_monolist() is intended to be called by
 * fluid_synth_noteon_mono_LOCAL().
 *
 *            The monophonic list
 *  +------------------------------------------------+
 *  | +--------------------------------------------+ |
 *  | |  +----+   +----+          +----+   +----+  | |
 *  | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
 *  +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
 *       +----+   +----+          +----+   +----+
 *        /|\                      /|\
 *         |                        |
 *      i_first                   i_last
 *
 * The monophonic list is a circular buffer of  SIZE_MONOLIST elements
 * Each element is linked forward and backward at initialisation time.
 * when a note is added at noteOn each element is use in the forward direction
 * and indexed by i_last variable. 
 *
 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127).
 * @param vel MIDI velocity (0-127, 0=noteoff).
 * @param onenote. When 1 the function adds the note but the monophonic list
 *                 that keeps only one note (used on noteOn poly).
 * Note: i_last index keeps a trace of the most recent note added.
 *       prev_note keeps a trace of the note prior i_last note.
 *       LEGATO_PLAYING bit keeps trace of legato/staccato playing state.
 * 
 * More informations in FluidPolyMono-0003.pdf chapter 4 (Appendices).
*/


static void
fluid_channel_add_monolist(fluid_channel_t* chan, unsigned char key,
						   unsigned char vel, unsigned char onenote)
{
	unsigned char i_last = chan->i_last;
	/* Update legato/ sataccato playing state */
	if (chan->n_notes) chan->mode |= LEGATO_PLAYING; /* Legato state */
	else chan->mode &= ~ LEGATO_PLAYING; /* Staccato state */
	/* keeps trace of the note prior last note */
	if(chan->n_notes) chan->prev_note = chan->monolist[i_last].note;
	/* moves i_last forward before writing new note */
	i_last = chan->monolist[i_last].next; 
	chan->i_last = i_last; 			/* now ilast indexes the last note */
	chan->monolist[i_last].note = key; /* we save note and velocity */
	chan->monolist[i_last].vel = vel; 	
	if (onenote) 
	{ /* clear monolist to one note addition */
		chan->i_first = i_last;
		chan->n_notes = 0;
	}
	if(chan->n_notes < SIZE_MONOLIST) 
	{
		chan->n_notes++; /* update n_notes */
	}
	else { /* The end of buffer is reach. So circular motion for i_first */
		/* i_first index is moved forward */
		chan->i_first = chan->monolist[i_last].next;
	}
}

/**
 * Searching a note in the monophonic list.The function is part of the legato 
 * detector. fluid_channel_search_monolist() is intended to be called by 
 * fluid_synth_noteoff_mono_LOCAL().
 *
 * The search start from the first note in the list indexed by i_first
 *
 *                The monophonic list
 *  +------------------------------------------------+
 *  | +--------------------------------------------+ |
 *  | |  +----+   +----+          +----+   +----+  | |
 *  | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
 *  +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
 *       +----+   +----+          +----+   +----+
 *        /|\                      /|\
 *         |                        |
 *      i_first                   i_last
 * 
 * @param chan  fluid_channel_t.
 * @param key MIDI note number (0-127) to search.
 * @return index of the note if find, INVALID_NOTE otherwise.
 * 
 */
static unsigned short
fluid_channel_search_monolist(fluid_channel_t* chan, unsigned char key)
{
	short n = chan->n_notes; /* number of notes in monophonic list */
	short i= chan->i_first; /* searching starts from i_first included */
	while(n) 
	{
		if(chan->monolist[i].note == key) break; /* found */
		i = chan->monolist[i].next; /* next element */
		n--;
	}
	if (n)
	{
		return i;/* found i */
	}
	else
	{
		return INVALID_NOTE; /* not found */
	}
}

/**
 * removes a note out of the monophonic list.The function is part of 
 * the legato detector. 
 * fluid_channel_remove_monolist() is intended to be called by 
 * fluid_synth_noteoff_mono_LOCAL().
 *
 *            The monophonic list
 *  +------------------------------------------------+
 *  | +--------------------------------------------+ |
 *  | |  +----+   +----+          +----+   +----+  | |
 *  | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
 *  +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
 *       +----+   +----+          +----+   +----+
 *        /|\                      /|\
 *         |                        |
 *      i_first                   i_last
 *
 * The monophonic list is a circular buffer of  SIZE_MONOLIST elements
 * Each element is linked forward and backward at initialisation time.
 * when a note is removed at noteOff the element concerned is fast unlinked
 * and relinked after the i_last element.
 *
 * @param chan  fluid_channel_t.
 * @param 
 *   i, index of the note to remove. If i is invalid or the list is 
 *      empty, the function do nothing and returns INVALID_NOTE.
 * @return prev index prior the last note if i is the last note in the list,
 *        INVALID_NOTE otherwise. When the returned index is valid it means
 *        a legato dectection.
 *
 * Note: the following variables in Channel keeps trace of the situation.
 *       - i_last index keeps a trace of the most recent note played even if
 *       the list is empty.
 *       - prev_note keeps a trace of the note removed if it is i_last.
 *       - LEGATO_PLAYING bit keeps a trace of legato/staccato playing state.
 * 
 * More informations in FluidPolyMono-0003.pdf chapter 4 (Appendices).
 */
static unsigned char
fluid_channel_remove_monolist(fluid_channel_t* chan, short i)
{
	unsigned char i_prev = INVALID_NOTE;
	unsigned char i_last = chan->i_last;
	/* check if index is valid */
	if(!is_valid_note(i) || i >= SIZE_MONOLIST || !chan->n_notes)
	{
		return INVALID_NOTE;
	}
	/* The element is about to be removed and inserted between i_last and next */
	/* Note: when i is egal to i_last or egal to i_first, Removing/Inserting
	   isn't necessary */
	if (i == i_last) 
	{ /* Removing/Inserting isn't necessary */
		/* keep trace of the note prior last note */
		chan->prev_note= chan->monolist[i_last].note;
		/* moves i_last backward to the previous  */
		i_prev = chan->monolist[i].prev; /* return the note prior i_last */
		chan->i_last = i_prev; /* i_last index is moved backward */
	}
	else 
	{ /* i is before i_last */
		if(i == chan->i_first)
		{
			/* Removing/Inserting isn't necessary */
			/* i_first index is moved forward to the next element*/
			chan->i_first = chan->monolist[i].next;
		}
		else 
		{ /* i is between i_first ans i_last */
			/* Unlink element i and inserting between i_last and next */
			unsigned char next,prev,nextend;
			/* removing by chaining prev and next */
			next = chan->monolist[i].next;
			prev = chan->monolist[i].prev;
			chan->monolist[next].prev = prev;
			chan->monolist[prev].next = next;
			/* inserting element i after i_last */
			nextend = chan->monolist[i_last].next;
			chan->monolist[i].next = nextend; 
			chan->monolist[nextend].prev = i;
			chan->monolist[i].prev = i_last;
			chan->monolist[i_last].next = i;
		}
	}
	chan->n_notes--; /* update the number of note in the list */
	/* Update legato/ staccato playing state */
	if (chan->n_notes)
	{
		chan->mode |= LEGATO_PLAYING; /* Legato state */
	}
	else
	{
		chan->mode &= ~ LEGATO_PLAYING; /* Staccato state */
	}
	return i_prev;
}

/**
 * On noteOff on a polyphonic channel,the monophonic list is fully flushed.
 *
 *            The monophonic list
 *  +------------------------------------------------+
 *  | +--------------------------------------------+ |
 *  | |  +----+   +----+          +----+   +----+  | |
 *  | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
 *  +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
 *       +----+   +----+          +----+   +----+
 *        /|\                      /|\
 *         |                        |
 *      i_first                   i_last
 *
 * @param chan  fluid_channel_t.
 * Note: i_last index keeps a trace of the most recent note played even if
 *       the list is empty.
 *       prev_note keeps a trace of the note .
 *       LEGATO_PLAYING bit keeps a trace of legato/staccato playing.
 */
void fluid_channel_clear_monolist(fluid_channel_t* chan)
{
	/* keeps trace off the most recent note played */
	chan->prev_note= chan->monolist[chan->i_last].note;

	/* flush the monolist */
	chan->i_first = chan->monolist[chan->i_last].next;
	chan->n_notes = 0;
	/* Update legato/ sataccato playing state */
	chan->mode &= ~ LEGATO_PLAYING; /* Staccato state */
}

/**
 * The monophonic list is flushed keeping last note only.
 * The function is entended to be called on legato Off when the
 * monolist have notes.
 *
 *            The monophonic list
 *  +------------------------------------------------+
 *  | +--------------------------------------------+ |
 *  | |  +----+   +----+          +----+   +----+  | |
 *  | +--|note|<--|note|<--....<--|note|<--|note|<-+ |
 *  +--->|vel |-->|vel |-->....-->|vel |-->|vel |----+
 *       +----+   +----+          +----+   +----+
 *        /|\                      /|\
 *         |                        |
 *      i_first                   i_last
 *
 * @param chan  fluid_channel_t.
 * Note: i_last index keeps a trace of the most recent note played.
 *       prev_note keeps a trace of the note .
 *       LEGATO_PLAYING bit keeps trace of legato/staccato playing.
 */
static void fluid_channel_keep_lastnote_monolist(fluid_channel_t* chan)
{
	chan->i_first = chan->i_last;
	chan->n_notes = 1;
}

/**
 * On noteOn on a polyphonic channel,adds the note into the monophonic list
 * keeping only this note.
 * @param 
 *   chan  fluid_channel_t.
 *   key, vel, note and velocity added in the monolist
 * Note: i_last index keeps a trace of the most recent note inserted.
 *       prev_note keeps a trace of the note prior i_last note.
 *       LEGATO_PLAYING bit keeps trace of legato/staccato playing.
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
 * get_fromkey_portamento_legato returns two informations:
 *    - fromkey note for portamento.
 *    - fromkey note for legato.
 *                                                 +-----> fromkey_portamento
 *                                           ______|________    
 *                portamento modes >------->|               |   
 *                                          | get_fromkey   |   
 *  Porta.on/off >------------------------->|_______________|    
 *  (PTC)                                          |          
 *                                                 +-----> fromkey_legato
 *
 * The functions is intended to be call on noteOn mono
 * see fluid_synth_noteon_mono_staccato(), fluid_synth_noteon_mono_legato()
 * -------
 * 1)The function determines if a portamento must occur on next noteOn. 
 * The value returned is 'fromkey portamento' which is the pitchstart key 
 * of a portamento, as function of PTC or (default_fromkey, prev_note) both
 * if Portamento On. By order of precedence the result is:
 *  1.1) PTC have precedence over Portamento On.
 *       If CC PTC has been received, its value supersedes and any
 *       portamento pedal On, default_fromkey,prev_note or portamento mode.
 *  1.2) Otherwise ,when Portamento On the function takes the following value:
 *       - default_fromkey if valid 
 *       - otherwise prev_note(prev_note is the note prior the most recent
 *         note played).
 *       Then portamento mode is applied to validate the value choosen.
 *       Where portamento mode is: 
 *       - each note, a portamento occurs on each note.
 *       - legato only, portamento only on notes played legato. 
 *       - staccato only, portamento only on notes played staccato. 
 *  1.3) Otherwise, portamento is off,INVALID_NOTE is returned (portamento is disabled).
 * ------
 * 2)The function determines if a legato playing must occur on next noteOn.
 *  'fromkey legato note' is returned as a function of default_fromkey, PTC,
 *   current mono/poly mode,actual 'staccato/legato' playing state and prev_note.
 *   By order of precedence the result is:
 *   2.1) If valid, default_fromkey have precedence over any other value.
 *   2.2) Otherwise if CC PTC has been received its value is returned.
 *   2.3) Otherwise fromkey legato is determined from the mono/poly mode,
 *        the actual 'staccato/legato' playing state (LEGATO_PLAYING) and prev_note
 *        as this:
 *        - in (poly/Mono) staccato , INVALID_NOTE is returned.
 *        - in poly  legato , actually we don't want playing legato. So
 *          INVALID_NOTE is returned.
 *        - in mono legato , prev_note is returned.
 *
 * On input
 * @param chan  fluid_channel_t.
 * @param defaultFromkey, the defaut 'fromkey portamento' note or 'fromkey legato'
 *       note (see description above).
 * 
 * @return
 *  1)'fromkey portamento' is returned in fluid_synth_t.fromkey_portamento.
 *  If valid,it means that portamento is enabled .
 *
 *  2) The 'fromkey legato' note is returned.
 *
 * Notes about usage:
 * The function is intended to be called when the following event occurs:
 * - On noteOn (Poly or Mono) after insertion in the monophonic list.
 * - On noteOff(mono legato playing). In this case, default_fromkey must be valid.
 *
 * Typical calling usage:
 * - In poly, default_fromkey must be INVALID_NOTE. 
 * - In mono staccato playing,default_fromkey must be INVALID_NOTE.
 * - In mono legato playing,default_fromkey must be valid.
 */
static unsigned char get_fromkey_portamento_legato(fluid_channel_t* chan, 
								   unsigned char default_fromkey)
{
	unsigned char ptc =  portamentoCtrl(chan);
	if(is_valid_note(ptc))
	{	/* CC PTC has been received */
		clearPortamentoCtrl(chan);	/* clear the CC PTC receive */
		chan->synth->fromkey_portamento =  ptc;/* return fromkey portamento */
		/* returns fromkey legato */
		if(!is_valid_note(default_fromkey)) default_fromkey= ptc;
	}
	else 
	{	/* determines and returns fromkey portamento */
		unsigned char fromkey_portamento = INVALID_NOTE;
		if(fluid_channel_portamento(chan))
		{	/* Portamento when Portamento pedal is On */
			/* 'fromkey portamento'is determined from the portamento mode
			 and the most recent note played (prev_note)*/
			unsigned char portamentomode = chan->portamentomode;
			if(is_valid_note(default_fromkey))
			{	
				fromkey_portamento = default_fromkey; /* on each note */
			}
			else fromkey_portamento = fluid_channel_prev_note(chan); /* on each note */
			if(portamentomode == LEGATO_ONLY)
			{   /* Mode portamento:legato only */
				if(!(chan->mode  & LEGATO_PLAYING))
				{	
					fromkey_portamento = INVALID_NOTE;
				}
			}
			else if(portamentomode == STACCATO_ONLY)
			{	/* Mode portamento:staccato only */
				if(chan->mode  & LEGATO_PLAYING) 
				{
					fromkey_portamento = INVALID_NOTE;
				}
			}
			/* else Mode portamento: on each note (staccato/legato) */
		}
		/* Returns fromkey portamento */
		chan->synth->fromkey_portamento = fromkey_portamento;
		/* Determines and returns fromkey legato */
		if(!is_valid_note(default_fromkey))
		{
			/* in staccato (poly/Mono) returns INVALID_NOTE */
			/* In mono mode legato playing returns the note prior most 
			   recent note played */
			if (is_fluid_channel_playing_mono(chan) && (chan->mode  & LEGATO_PLAYING))
			{
				default_fromkey = fluid_channel_prev_note(chan); /* note prior last note */
			}
			/* In poly mode legato playing, actually we don't want playing legato.
			So return INVALID_NOTE */
		}
	}
	return default_fromkey; /* Return legato fromkey */
}

/**
 * The function changes the state (Valid/Invalid) of the previous note played in
 * a staccato manner (fluid_channel_prev_note()).
 * When potamento mode: 'each note' or 'staccato only' is selected, on next 
 * noteOn a portamento will be started from the most recent note played 
 * staccato fluid_channel_last_note.
 * It will be possible that it isn't appropriate. To give the musician the 
 * possibility to choose this note , prev_note will be marked invalid on noteOff
 * if portamento pedal is Off.
 *
 * The function is intended to be called when the following event occurs:
 * - On noteOff (in poly or mono mode), to mark prev_note invalid.
 * - On Portamento Off(in poly or mono mode), to mark prev_note invalid.
 * @param chan  fluid_channel_t.
 */
void invalid_prev_note_staccato(fluid_channel_t* chan)
{
	if(!(chan->mode  & LEGATO_PLAYING)) /* the monophonic list is empty */ 
	if(! fluid_channel_portamento(chan))
	{	/* mark prev_note invalid */
		fluid_channel_clear_prev_note(chan);
	}
	/* else prev_note still remains valid for next fromkey portamento */
}
/*****************************************************************************
 noteon - noteoff functions in Mono mode
******************************************************************************/
/*
 *  noteon - noteoff on a channel in "monophonic playing".
 *  
 *  A channel needs to be played monophonic if this channel has been set in
 *  monophonic mode by basic channel API.(see fluid_synth_polymono.c).
 *  A channel needs also to be played monophonic if it has been set in
 *  polyphonic mode and legato pedal is On during the playing.
 *  When a channel is in "monophonic playing" state, only one note at a time can be
 *  played in a staccato or legato manner (with or without portamento).
 *  More informations in FluidPolyMono-0003.pdf chapter 4 (Appendices).
 *                                           _______________                                     
 *                 ________________         |    noteon     |
 *                | legato detector|    O-->| mono_staccato |--*-> preset_noteon
 *  noteon_mono ->| (add_monolist) |--O--   |_______________|  |   (with or without)
 *  LOCAL         |________________|    O         /|\          |   (portamento)
 *                  /|\ set_onenote     |          | fromkey   |
 *                   |                  |          | portamento|
 *  noteOn poly  >---*------------------*          |           |
 *                                      |          |           | 
 *                                      |    _____ |________   |
 *                portamento modes >--- | ->|               |  |
 *                                      |   |  get-fromkey  |  |
 *  Porta.on/off >--------------------- | ->|_______________|  | 
 *  (PTC)                               |          |           |
 *                                      |  fromkey | fromkey   |
 *                                      |  legato  | portamento|
 *                                      |    _____\|/_______   |
 *                                      *-->| noteon        |--/
 *                                      |   | mono_legato   |----> voices
 *                legato modes >------- | ->|_______________|      triggering
 *                                      |                          (with or without)
 *                                      |                          (portamento)
 *                                      |                    
 *                                      |
 *  noteOff poly >---*----------------- | ---------+
 *                   |  clear           |          |
 *                 _\|/_____________    |          |
 *                | legato detector |   O          |
 *  noteoff_mono->|(search_monolist)|-O--    _____\|/_______
 *  LOCAL         |(remove_monolist)|   O-->|   noteoff     | 
 *                |_________________|       |   monopoly    |----> noteoff
 *  Sust.on/off  >------------------------->|_______________|
 *  Sost.on/off										        
------------------------------------------------------------------------------*/
static int fluid_synth_noteon_mono_staccato(fluid_synth_t* synth, int chan, 
									 int key, int vel);
int fluid_synth_noteoff_monopoly(fluid_synth_t* synth, int chan, int key,
							 char Mono);

int fluid_synth_noteon_mono_legato(fluid_synth_t* synth, int chan,
							   int fromkey, int tokey, int vel);

/**
 * Plays a noteon event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing".
 *                                          _______________                                     
 *                ________________         |    noteon     |
 *               | legato detector|    O-->| mono_staccato |--->preset_noteon
 * noteon_mono ->| (add_monolist) |--O--   |_______________|     
 * LOCAL         |________________|    O                       
 *                                     |                      
 *                                     |                      
 *                                     |                    
 *                                     |                       
 *                                     |                      
 *                                     |                      
 *                                     |                      
 *                                     |                       
 *                                     |                      
 *                                     |    _______________   
 *                                     |   |   noteon      |
 *                                     +-->| mono_legato   |---> voices
 *                                         |_______________|     triggering
 * 
 * The function uses the legato detector (see above) to determine if the note must
 * be played staccato or legato.
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
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

	if (!(channel->mode &  BREATH_SYNC) || fluid_channel_breath_msb(channel) )
	{
		/* legato/staccato playing detection */
		if(channel->mode  & LEGATO_PLAYING)
		{ /* legato playing */
			/* legato from iPrev to key */
			/* the voices from iPrev key number are to be used to play key number */
			/* fromkey must be valid */
			return 	fluid_synth_noteon_mono_legato(synth, chan,
				             fluid_channel_prev_note(channel), key, vel);
		}
		else 
		{	/* staccato playing */
			return fluid_synth_noteon_mono_staccato(synth, chan, key, vel);
		}
	}
	else return FLUID_OK;
}

/**
 * Plays a noteoff event for a Synth instance in "monophonic playing" state.
 * Please see the description above about "monophonic playing"
 *
 *                                           _______________    
 *                                          |   noteon      |
 *                                      +-->|mono_legato    |----> voices
 *                                      |   |_______________|      triggering
 *                                      |                          (with or without)
 *                                      |                          (portamento)
 *                                      |                    
 *                                      |           
 *                                      |
 *                                      |
 *                                      |             
 *                                      |          
 *                 _________________    |          
 *                | legato detector |   O          
 *  noteoff_mono->|(search_monolist)|-O--    _______________
 *  LOCAL         |(remove_monolist)|   O-->|   noteoff     | 
 *                |_________________|       |   monopoly    |----> noteoff
 *                                          |_______________|
 *
 * The function uses the legato detector (see above) to determine if the noteoff must
 * be played staccato or legato.
 *
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

	if (is_valid_note(i))
	{ /* the note is in monophonic list */
		/* Remove note out of the monophonic list */
		iPrev = fluid_channel_remove_monolist(channel,i);

		if (!(channel->mode &  BREATH_SYNC) || fluid_channel_breath_msb(channel) )
		{
			/* legato playing detection */
			if(channel->mode  & LEGATO_PLAYING) 
			{ /* the list contains others notes */
				if(is_valid_note(iPrev)) 
				{ /* legato playing detection */
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
			else 
			{ /* the monophonic list is empty */
				/* plays the monophonic note noteoff and eventually held
				by sustain/sostenuto */
				status = fluid_synth_noteoff_monopoly(synth, chan, key, 1);
			}
		}
		else	status = FLUID_OK;
	}
	else 
	{ /* the note is not found in the list so the note was 
		   played On when the channel was in polyphonic playing */
			/* play the noteoff as for polyphonic  */
			status = fluid_synth_noteoff_monopoly(synth, chan, key, 0);
	}
	return status;
}

/*----------------------------------------------------------------------------
 staccato playing
-----------------------------------------------------------------------------*/
/**
 * Plays noteon for a monophonic note in staccato manner.
 * Please see the description above about "monophonic playing".
 *                                         _______________                                     
 *                                        |    noteon     |
 *  noteon_mono >------------------------>| mono_staccato |----> preset_noteon
 *                                        |_______________|      (with or without)   
 *  LOCAL                                       /|\              (portamento)
 *                                               | fromkey    
 *                                               | portamento 
 *                                               |            
 *                                               |             
 *                                         ______|________    
 *                portamento modes >----->|               |   
 *                                        |  get_fromkey  |   
 *  Porta.on/off >----------------------->|_______________|    
 *  Portamento                                                  
 *  (PTC)                                                        
 *
 * We are in staccato situation (where no previous note have been depressed).
 * Before the note been passed to fluid_preset_noteon(), the function must determine
 * the from_key_portamento parameter used by fluid_preset_noteon().
 * 
 * from_key_portamento is returned by get_fromkey_portamento_legato() function.
 * fromkey_portamento is set to valid/invalid  key value depending of the portamento
 * modes (see portamento mode API) , CC portamento On/Off , and CC portamento control
 * (PTC).
 *
 * @param synth instance.
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
	get_fromkey_portamento_legato( channel, INVALID_NOTE);
	/* The note needs to be played by voices allocation  */
	return fluid_preset_noteon(channel->preset, synth, chan, key, vel);
}

/**
 * Plays noteoff for a polyphonic or monophonic note
 * Please see the description above about "monophonic playing".
 *
 *                                       
 *  noteOff poly >---------------------------------+
 *                                                 |
 *                                                 |
 *                                                 |
 *  noteoff_mono                             _____\|/_______
 *  LOCAL        >------------------------->|   noteoff     | 
 *                                          |   monopoly    |----> noteoff
 *  Sust.on/off  >------------------------->|_______________|
 *  Sost.on/off										        
 *
 * The function has the same behavior when the noteoff is poly of mono, except
 * that for mono noteoff, if any pedal (sustain or sostenuto ) is depressed, the
 * key is memorized. This is neccessary when the next mono note will be played
 * staccato, as any current mono note currently sustained will need to be released 
 * (see fluid_synth_noteon_mono_staccato()).
 * Note also that for a monophonic legato passage, the function is called only when
 * the last noteoff of the passage occurs. That means that if sustain or sostenuto
 * is depressed, only the last note of a legato passage will be sustained.
 *
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param key MIDI note number (0-127).
 * @param Mono, 1 noteoff on monophonic note.
 *              0 noteoff on polyphonic note.
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 *
 * Note: On return, on monophonic, possible sustained note is memorized in
 * key_sustained. Memorization is done here on noteOff.
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
 *
 *                                                              
 *                                         _______________    
 *                portamento modes >----->|               |   
 *                                        | get_fromkey   |
 *  Porta.on/off >----------------------->|_______________|   
 *  Portamento                                   |           
 *  (PTC)                                        |           +-->preset_noteon
 *                                       fromkey | fromkey   |  (with or without)
 *                                       legato  | portamento|  (portamento) 
 *                                         _____\|/_______   |
 *                                        |   noteon      |--+  
 *  noteon_mono >------------------------>| mono_legato   |----->voices
 *  LOCAL                                 |_______________|      triggering
 *                                              /|\              (with or without)
 *                                               |               (portamento)
 *                legato modes >-----------------+         
 *
 * We are in legato situation (where a previous note have been depressed).
 * The function must determine the from_key_portamento and from_key_legato parameters
 * used by fluid_preset_noteon() function or the voices triggering functions.
 *
 * from_key_portamento and from_key_legato are returned by 
 * get_fromkey_portamento_legato() function.
 * fromkey_portamento is set to valid/invalid  key value depending of the portamento
 * modes (see portamento mode API) , CC portamento On/Off , and CC portamento control
 * (PTC).
 * Then, depending of the legato modes (see legato mode API), the function will call
 * the appropriate triggering functions: 
 * @param synth instance.
 * @param chan MIDI channel number (0 to MIDI channel count - 1).
 * @param fromkey MIDI note number (0-127).
 * @param tokey MIDI note number (0-127).
 * @param vel MIDI velocity (0-127).
 * @return FLUID_OK on success, FLUID_FAILED otherwise.
 *
 * Note: The voices with key 'fromkey' are to be used to play key 'tokey'.
 * The fonction is able to play legato through Preset Zone(s) (PZ) and
 * Instrument Zone(s) (IZ) as far as possible.
 * When key tokey is outside the current Instrument Zone,
 * Preset Zone, current fromkey' voices are released.If necessary new voices
 * are restarted when tokey enters inside new Instruent(s) Zones,Preset Zone(s).
 */
int fluid_synth_noteon_mono_legato(fluid_synth_t* synth, int chan,
							   int fromkey, int tokey, int vel)
{
	fluid_channel_t* channel = synth->channel[chan];
	unsigned char legatomode = channel->legatomode;
	fluid_voice_t* voice;
	int i ;
	/* Gets possible 'fromkey portamento' and possible 'fromkey legato' note  */
	fromkey = get_fromkey_portamento_legato( channel, (unsigned char)fromkey);

	if (is_valid_note(fromkey)) for (i = 0; i < synth->polyphony; i++) 
	{
		/* search fromkey voices: only those who don't have 'note off' */
		voice = synth->voice[i];
		if (_ON(voice) && (voice->chan == chan) && (voice->key == fromkey))
		{
			/* Checks if tokey is inside the range of the running voice */
			if (fluid_inst_zone_inside_range(voice->inst_zone, tokey, vel))
			{
				switch (legatomode) 
				{
					case RETRIGGER_0: /* mode 0 */
						fluid_voice_update_release(voice,0); /* fast release */
					break;
					case RETRIGGER_1: /* mode 1 */
						fluid_voice_update_release(voice,1); /* normal release */
					break;
					case MULTI_RETRIGGER: /* mode 2 */
						/* Skip in attack section */
						fluid_voice_update_multi_retrigger_attack(voice,tokey,vel);
					break;
					case SINGLE_TRIGGER_0: /* mode 3 */
						fluid_voice_update_single_trigger0(voice,fromkey,tokey,vel);
					break;
					case SINGLE_TRIGGER_1: /* mode 4 */
						fluid_voice_update_single_trigger1(voice,fromkey,tokey,vel);
					break;
					default: /* Invalid mode */
					FLUID_LOG(FLUID_WARN, 
						"Failed to execute legato mode: %d",legatomode);
					return FLUID_FAILED;
				}
				if (legatomode >= MULTI_RETRIGGER)	
				{
					/* Starts portamento if enabled */
					if(	is_valid_note(synth->fromkey_portamento))
					{
						/* Sends portamento parameters to the voice dsp */
						fluid_voice_update_portamento(voice,
											synth->fromkey_portamento,tokey);
					}
					/* The voice is now used to play tokey in legato manner */
					/* Marks this Instrument Zone to be ignored during next
					   fluid_preset_noteon() */
					SetIgnoreInstZone (voice->inst_zone);
				}
			}
			else  
			{ /* tokey note is outside the voice range, so the voice is released */
				fluid_voice_update_release(voice,legatomode);
			}
		}
	}
	/* May be,tokey will enter in other Insrument Zone(s),Preset Zone(s), in
	   this case it needs to be played by voices allocation  */
	return fluid_preset_noteon(channel->preset,synth,chan,tokey,vel);
}

/**
 * The function handles Poly/mono commutation on Legato pedal On/Off.
 * @param chan  fluid_channel_t.
 * @param value, value of the CC legato.
 */
void legato_on_off(fluid_channel_t* chan, int value)
{
	/* Special handling of the monophonic list  */
	if (!(chan->mode & MONO) && chan->nNotes) /* The monophonic list have notes */
	{
		if (value < 64 ) /* legato is released */
		{	/* returns from monophonic to polyphonic with note in monophonic list */
			fluid_channel_keep_lastnote_monolist(chan);
		}
		else /* legato is depressed */
		{	/* Inters in monophonic from polyphonic with note in monophonic list */
			/* Stops the running note to remain coherent with Breath Sync mode */
			if ((chan->mode &  BREATH_SYNC) && !fluid_channel_breath_msb(chan))
				fluid_synth_noteoff_monopoly(chan->synth,chan->channum,
				                        fluid_channel_last_note(chan),1);
		}
	}
}

/**
 * The function handles CC Breath On/Off detection. When a channel is in 
 * Breath Sync mode and in monophonic playing, the breath controller allows
 * to tigger noteon/noteoff note when the musician starts to breath (noteon) and
 * stops to breath (noteoff).
 * @param chan  fluid_channel_t.
 * @param value, value of the CC Breath..
 */
void breath_note_on_off(fluid_channel_t* chan, int value)
{	
	if ((chan->mode &  BREATH_SYNC)  && is_fluid_channel_playing_mono(chan) &&
		(chan->n_notes))
	{	
		/* The monophonic list isn't empty */
		if((value > 0) && (chan->previous_cc_breath == 0))
		{	/* CC Breath On detection */
			fluid_synth_noteon_mono_staccato(chan->synth,chan->channum,
								fluid_channel_last_note(chan),
								fluid_channel_last_vel(chan));
		}
		else if(  (value == 0) && (chan->previous_cc_breath > 0))
		{	/* CC Breath Off detection */
			fluid_synth_noteoff_monopoly(chan->synth, chan->channum,
										fluid_channel_last_note(chan), 1);
		}
	}
	chan->previous_cc_breath = value;
}
