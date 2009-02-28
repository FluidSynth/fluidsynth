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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluid_midi.h"
#include "fluid_sys.h"
#include "fluid_synth.h"
#include "fluid_settings.h"

/* all outgoing user messages are stored in a global text buffer */
#define MIDI_MESSAGE_LENGTH 1024
char midi_message_buffer[MIDI_MESSAGE_LENGTH];


/* Taken from Nagano Daisuke's USB-MIDI driver */

static int remains_f0f6[] = {
	0,	/** 0xF0 **/
	2,	/** 0XF1 **/
	3,	/** 0XF2 **/
	2,	/** 0XF3 **/
	2,	/** 0XF4 (Undefined by MIDI Spec, and subject to change) **/
	2,	/** 0XF5 (Undefined by MIDI Spec, and subject to change) **/
	1	/** 0XF6 **/
};

static int remains_80e0[] = {
	3,	/** 0x8X Note Off **/
	3,	/** 0x9X Note On **/
	3,	/** 0xAX Poly-key pressure **/
	3,	/** 0xBX Control Change **/
	2,	/** 0xCX Program Change **/
	2,	/** 0xDX Channel pressure **/
	3 	/** 0xEX PitchBend Change **/
};


/***************************************************************
 *
 *                      MIDIFILE
 */

/**
 * Open a MIDI file and return a new MIDI file handle.
 * @internal
 * @param filename Path of file to open.
 * @return New MIDI file handle or NULL on error.
 */
fluid_midi_file* new_fluid_midi_file(char* filename)
{
	fluid_midi_file* mf;

	mf = FLUID_NEW(fluid_midi_file);
	if (mf == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	FLUID_MEMSET(mf, 0, sizeof(fluid_midi_file));

	mf->c = -1;
	mf->running_status = -1;
	mf->fp = FLUID_FOPEN(filename, "rb");

	if (mf->fp == NULL) {
		FLUID_LOG(FLUID_ERR, "Couldn't open the MIDI file");
		FLUID_FREE(mf);
		return NULL;
	}

	if (fluid_midi_file_read_mthd(mf) != FLUID_OK) {
		FLUID_FREE(mf);
		return NULL;
	}
	return mf;
}

/**
 * Delete a MIDI file handle.
 * @internal
 * @param mf MIDI file handle to close and free.
 */
void delete_fluid_midi_file(fluid_midi_file* mf)
{
	if (mf == NULL) {
		return;
	}
	if (mf->fp != NULL) {
		FLUID_FCLOSE(mf->fp);
	}
	FLUID_FREE(mf);
	return;
}

/*
 * Get the next byte in a MIDI file.
 */
int fluid_midi_file_getc(fluid_midi_file* mf)
{
	unsigned char c;
	int n;
	if (mf->c >= 0) {
		c = mf->c;
		mf->c = -1;
	} else {
		n = FLUID_FREAD(&c, 1, 1, mf->fp);
		mf->trackpos++;
	}
	return (int) c;
}

/*
 * fluid_midi_file_push
 */
int fluid_midi_file_push(fluid_midi_file* mf, int c)
{
	mf->c = c;
	return FLUID_OK;
}

/*
 * fluid_midi_file_read
 */
int fluid_midi_file_read(fluid_midi_file* mf, void* buf, int len)
{
	int num = FLUID_FREAD(buf, 1, len, mf->fp);
	mf->trackpos += num;
#if DEBUG
	if (num != len) {
		FLUID_LOG(FLUID_DBG, "Coulnd't read the requested number of bytes");
	}
#endif
	return (num != len)? FLUID_FAILED : FLUID_OK;
}

/*
 * fluid_midi_file_skip
 */
int fluid_midi_file_skip(fluid_midi_file* mf, int skip)
{
	int err = FLUID_FSEEK(mf->fp, skip, SEEK_CUR);
	if (err) {
		FLUID_LOG(FLUID_ERR, "Failed to seek position in file");
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

/*
 * fluid_midi_file_read_mthd
 */
int fluid_midi_file_read_mthd(fluid_midi_file* mf)
{
	char mthd[15];
	if (fluid_midi_file_read(mf, mthd, 14) != FLUID_OK) {
		return FLUID_FAILED;
	}
	if ((FLUID_STRNCMP(mthd, "MThd", 4) != 0) || (mthd[7] != 6) || (mthd[9] > 2)) {
		FLUID_LOG(FLUID_ERR, "Doesn't look like a MIDI file: invalid MThd header");
		return FLUID_FAILED;
	}
	mf->type = mthd[9];
	mf->ntracks = (unsigned) mthd[11];
	mf->ntracks += (unsigned int) (mthd[10]) << 16;
	if((mthd[12]) < 0){
		mf->uses_smpte = 1;
		mf->smpte_fps = -mthd[12];
		mf->smpte_res = (unsigned) mthd[13];
		FLUID_LOG(FLUID_ERR, "File uses SMPTE timing -- Not implemented yet");
		return FLUID_FAILED;
	} else {
		mf->uses_smpte = 0;
		mf->division = (mthd[12] << 8) | (mthd[13] & 0xff);
		FLUID_LOG(FLUID_DBG, "Division=%d", mf->division);
	}
	return FLUID_OK;
}

/*
 * fluid_midi_file_load_tracks
 */
int fluid_midi_file_load_tracks(fluid_midi_file* mf, fluid_player_t* player)
{
	int i;
	for (i = 0; i < mf->ntracks; i++) {
		if (fluid_midi_file_read_track(mf, player, i) != FLUID_OK) {
			return FLUID_FAILED;
		}
	}
	return FLUID_OK;
}

/*
 * fluid_isasciistring
 */
int fluid_isasciistring(char* s)
{
	int i;
	int len = (int) FLUID_STRLEN(s);
	for (i = 0; i < len; i++) {
		if (!fluid_isascii(s[i])) {
			return 0;
		}
	}
	return 1;
}

/*
 * fluid_getlength
 */
long fluid_getlength(unsigned char *s)
{
	long i = 0;
	i = s[3] | (s[2]<<8) | (s[1]<<16) | (s[0]<<24);
	return i;
}

/*
 * fluid_midi_file_read_tracklen
 */
int fluid_midi_file_read_tracklen(fluid_midi_file* mf)
{
	unsigned char length[5];
	if (fluid_midi_file_read(mf, length, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}
	mf->tracklen = fluid_getlength(length);
	mf->trackpos = 0;
	mf->eot = 0;
	return FLUID_OK;
}

/*
 * fluid_midi_file_eot
 */
int fluid_midi_file_eot(fluid_midi_file* mf)
{
#if DEBUG
	if (mf->trackpos > mf->tracklen) {
		printf("track overrun: %d > %d\n", mf->trackpos, mf->tracklen);
	}
#endif
	return mf->eot || (mf->trackpos >= mf->tracklen);
}

/*
 * fluid_midi_file_read_track
 */
int fluid_midi_file_read_track(fluid_midi_file* mf, fluid_player_t* player, int num)
{
	fluid_track_t* track;
	unsigned char id[5], length[5];
	int found_track = 0;
	int skip;

	if (fluid_midi_file_read(mf, id, 4) != FLUID_OK) {
		return FLUID_FAILED;
	}
	id[4]='\0';
	mf->dtime = 0;

	while (!found_track){

		if (fluid_isasciistring((char*) id) == 0) {
			FLUID_LOG(FLUID_ERR, "An non-ascii track header found, currupt file");
			return FLUID_FAILED;

		} else if (strcmp((char*) id, "MTrk") == 0) {

			found_track = 1;

			if (fluid_midi_file_read_tracklen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			track = new_fluid_track(num);
			if (track == NULL) {
				FLUID_LOG(FLUID_ERR, "Out of memory");
				return FLUID_FAILED;
			}

			while (!fluid_midi_file_eot(mf)) {
				if (fluid_midi_file_read_event(mf, track) != FLUID_OK) {
					return FLUID_FAILED;
				}
			}

			fluid_player_add_track(player, track);

		} else {
			found_track = 0;
			if (fluid_midi_file_read(mf, length, 4) != FLUID_OK) {
				return FLUID_FAILED;
			}
			skip = fluid_getlength(length);
/*        fseek(mf->fp, skip, SEEK_CUR); */
			if (fluid_midi_file_skip(mf, skip) != FLUID_OK) {
				return FLUID_FAILED;
			}
		}
	}
	if (feof(mf->fp)) {
		FLUID_LOG(FLUID_ERR, "Unexpected end of file");
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

/*
 * fluid_midi_file_read_varlen
 */
int fluid_midi_file_read_varlen(fluid_midi_file* mf)
{
	int i;
	int c;
	mf->varlen = 0;
	for (i = 0;;i++) {
		if (i == 4) {
			FLUID_LOG(FLUID_ERR, "Invalid variable length number");
			return FLUID_FAILED;
		}
		c = fluid_midi_file_getc(mf);
		if (c < 0) {
			FLUID_LOG(FLUID_ERR, "Unexpected end of file");
			return FLUID_FAILED;
		}
		if (c & 0x80){
			mf->varlen |= (int) (c & 0x7F);
			mf->varlen <<= 7;
		} else {
			mf->varlen += c;
			break;
		}
	}
	return FLUID_OK;
}

/*
 * fluid_midi_file_read_event
 */
int fluid_midi_file_read_event(fluid_midi_file* mf, fluid_track_t* track)
{
	int status;
	int type;
	int tempo;
	unsigned char* metadata = NULL;
	unsigned char* dyn_buf = NULL;
	unsigned char static_buf[256];
	int nominator, denominator, clocks, notes, sf, mi;
	fluid_midi_event_t* evt;
	int channel = 0;
	int param1 = 0;
	int param2 = 0;

	/* read the delta-time of the event */
	if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
		return FLUID_FAILED;
	}
	mf->dtime += mf->varlen;

	/* read the status byte */
	status = fluid_midi_file_getc(mf);
	if (status < 0) {
		FLUID_LOG(FLUID_ERR, "Unexpected end of file");
		return FLUID_FAILED;
	}

	/* not a valid status byte: use the running status instead */
	if ((status & 0x80) == 0) {
		if ((mf->running_status & 0x80) == 0) {
			FLUID_LOG(FLUID_ERR, "Undefined status and invalid running status");
			return FLUID_FAILED;
		}
		fluid_midi_file_push(mf, status);
		status = mf->running_status;
	}

	/* check what message we have */
	if (status & 0x80) {
		mf->running_status = status;

		if ((status == MIDI_SYSEX) || (status == MIDI_EOX)) {     /* system exclusif */
			/*
			 * Sysex messages are not handled yet
			 */
			/* read the length of the message */
			if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			if (mf->varlen) {

				if (mf->varlen < 255) {
					metadata = &static_buf[0];
				} else {
					FLUID_LOG(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__, __LINE__, mf->varlen);
					dyn_buf = FLUID_MALLOC(mf->varlen + 1);
					if (dyn_buf == NULL) {
						FLUID_LOG(FLUID_PANIC, "Out of memory");
						return FLUID_FAILED;
					}
					metadata = dyn_buf;
				}

				/* read the data of the message */
				if (fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK) {
					if (dyn_buf) {
						FLUID_FREE(dyn_buf);
					}
					return FLUID_FAILED;
				}

				if (dyn_buf) {
					FLUID_LOG(FLUID_DBG, "%s: %d: free metadata", __FILE__, __LINE__);
					FLUID_FREE(dyn_buf);
				}
			}

			return FLUID_OK;

		} else if (status == MIDI_META_EVENT) {             /* meta events */

			int result = FLUID_OK;

			/* get the type of the meta message */
			type = fluid_midi_file_getc(mf);
			if (type < 0) {
				FLUID_LOG(FLUID_ERR, "Unexpected end of file");
				return FLUID_FAILED;
			}

			/* get the length of the data part */
			if (fluid_midi_file_read_varlen(mf) != FLUID_OK) {
				return FLUID_FAILED;
			}

			if (mf->varlen < 255) {
				metadata = &static_buf[0];
			} else {
				FLUID_LOG(FLUID_DBG, "%s: %d: alloc metadata, len = %d", __FILE__, __LINE__, mf->varlen);
				dyn_buf = FLUID_MALLOC(mf->varlen + 1);
				if (dyn_buf == NULL) {
					FLUID_LOG(FLUID_PANIC, "Out of memory");
					return FLUID_FAILED;
				}
				metadata = dyn_buf;
			}

			/* read the data */
			if (mf->varlen)
			{
				if (fluid_midi_file_read(mf, metadata, mf->varlen) != FLUID_OK) {
					if (dyn_buf) {
						FLUID_FREE(dyn_buf);
					}
					return FLUID_FAILED;
				}
			}

			/* handle meta data */
			switch (type) {

			case MIDI_COPYRIGHT:
				metadata[mf->varlen] = 0;
				break;

			case MIDI_TRACK_NAME:
				metadata[mf->varlen] = 0;
				fluid_track_set_name(track, (char*) metadata);
				break;

			case MIDI_INST_NAME:
				metadata[mf->varlen] = 0;
				break;

			case MIDI_LYRIC:
				break;

			case MIDI_MARKER:
				break;

			case MIDI_CUE_POINT:
				break; /* don't care much for text events */

			case MIDI_EOT:
				if (mf->varlen != 0) {
					FLUID_LOG(FLUID_ERR, "Invalid length for EndOfTrack event");
					result = FLUID_FAILED;
					break;
				}
				mf->eot = 1;
				break;

			case MIDI_SET_TEMPO:
				if (mf->varlen != 3) {
					FLUID_LOG(FLUID_ERR, "Invalid length for SetTempo meta event");
					result = FLUID_FAILED;
					break;
				}
				tempo = (metadata[0] << 16) + (metadata[1] << 8) + metadata[2];
				evt = new_fluid_midi_event();
				if (evt == NULL) {
					FLUID_LOG(FLUID_ERR, "Out of memory");
					result = FLUID_FAILED;
					break;
				}
				evt->dtime = mf->dtime;
				evt->type = MIDI_SET_TEMPO;
				evt->channel = 0;
				evt->param1 = tempo;
				evt->param2 = 0;
				fluid_track_add_event(track, evt);
				mf->dtime = 0;
				break;

			case MIDI_SMPTE_OFFSET:
				if (mf->varlen != 5) {
					FLUID_LOG(FLUID_ERR, "Invalid length for SMPTE Offset meta event");
					result = FLUID_FAILED;
					break;
				}
				break; /* we don't use smtp */

			case MIDI_TIME_SIGNATURE:
				if (mf->varlen != 4) {
					FLUID_LOG(FLUID_ERR, "Invalid length for TimeSignature meta event");
					result = FLUID_FAILED;
					break;
				}
				nominator = metadata[0];
				denominator = pow(2.0, (double) metadata[1]);
				clocks = metadata[2];
				notes = metadata[3];

				FLUID_LOG(FLUID_DBG, "signature=%d/%d, metronome=%d, 32nd-notes=%d",
					  nominator, denominator, clocks, notes);

				break;

			case MIDI_KEY_SIGNATURE:
				if (mf->varlen != 2) {
					FLUID_LOG(FLUID_ERR, "Invalid length for KeySignature meta event");
					result = FLUID_FAILED;
					break;
				}
				sf = metadata[0];
				mi = metadata[1];
				break;

			case MIDI_SEQUENCER_EVENT:
				break;

			default:
				break;
			}

			if (dyn_buf) {
				FLUID_LOG(FLUID_DBG, "%s: %d: free metadata", __FILE__, __LINE__);
				FLUID_FREE(dyn_buf);
			}

			return result;

		} else {                /* channel messages */

			type = status & 0xf0;
			channel = status & 0x0f;

			/* all channel message have at least 1 byte of associated data */
			if ((param1 = fluid_midi_file_getc(mf)) < 0) {
				FLUID_LOG(FLUID_ERR, "Unexpected end of file");
				return FLUID_FAILED;
			}

			switch (type) {

			case NOTE_ON:
				if ((param2 = fluid_midi_file_getc(mf)) < 0) {
					FLUID_LOG(FLUID_ERR, "Unexpected end of file");
					return FLUID_FAILED;
				}
				break;

			case NOTE_OFF:
				if ((param2 = fluid_midi_file_getc(mf)) < 0) {
					FLUID_LOG(FLUID_ERR, "Unexpected end of file");
					return FLUID_FAILED;
				}
				break;

			case KEY_PRESSURE:
				if ((param2 = fluid_midi_file_getc(mf)) < 0) {
					FLUID_LOG(FLUID_ERR, "Unexpected end of file");
					return FLUID_FAILED;
				}
				break;

			case CONTROL_CHANGE:
				if ((param2 = fluid_midi_file_getc(mf)) < 0) {
					FLUID_LOG(FLUID_ERR, "Unexpected end of file");
					return FLUID_FAILED;
				}
				break;

			case PROGRAM_CHANGE:
				break;

			case CHANNEL_PRESSURE:
				break;

			case PITCH_BEND:
				if ((param2 = fluid_midi_file_getc(mf)) < 0) {
					FLUID_LOG(FLUID_ERR, "Unexpected end of file");
					return FLUID_FAILED;
				}

				param1 = ((param2 & 0x7f) << 7) | (param1 & 0x7f);
				param2 = 0;
				break;

			default:
				/* Can't possibly happen !? */
				FLUID_LOG(FLUID_ERR, "Unrecognized MIDI event");
				return FLUID_FAILED;
			}
			evt = new_fluid_midi_event();
			if (evt == NULL) {
				FLUID_LOG(FLUID_ERR, "Out of memory");
				return FLUID_FAILED;
			}
			evt->dtime = mf->dtime;
			evt->type = type;
			evt->channel = channel;
			evt->param1 = param1;
			evt->param2 = param2;
			fluid_track_add_event(track, evt);
			mf->dtime = 0;
		}
	}
	return FLUID_OK;
}

/*
 * fluid_midi_file_get_division
 */
int fluid_midi_file_get_division(fluid_midi_file* midifile)
{
	return midifile->division;
}

/******************************************************
 *
 *     fluid_track_t
 */

/**
 * Create a MIDI event structure.
 * @return New MIDI event structure or NULL when out of memory.
 */
fluid_midi_event_t* new_fluid_midi_event()
{
	fluid_midi_event_t* evt;
	evt = FLUID_NEW(fluid_midi_event_t);
	if (evt == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	evt->dtime = 0;
	evt->type = 0;
	evt->channel = 0;
	evt->param1 = 0;
	evt->param2 = 0;
	evt->next = NULL;
	return evt;
}

/**
 * Delete MIDI event structure.
 * @param evt MIDI event structure
 * @return Always returns 0
 */
int delete_fluid_midi_event(fluid_midi_event_t* evt)
{
	fluid_midi_event_t *temp;

	while (evt)
	{
		temp = evt->next;
		FLUID_FREE(evt);
		evt = temp;
	}
	return FLUID_OK;
}

/**
 * Get the event type field of a MIDI event structure.
 * DOCME - Event type enum appears to be internal (fluid_midi.h)
 * @param evt MIDI event structure
 * @return Event type field
 */
int fluid_midi_event_get_type(fluid_midi_event_t* evt)
{
	return evt->type;
}

/**
 * Set the event type field of a MIDI event structure.
 * DOCME - Event type enum appears to be internal (fluid_midi.h)
 * @param evt MIDI event structure
 * @param type Event type field
 * @return Always returns 0
 */
int fluid_midi_event_set_type(fluid_midi_event_t* evt, int type)
{
	evt->type = type;
	return FLUID_OK;
}

/**
 * Get the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Channel field
 */
int fluid_midi_event_get_channel(fluid_midi_event_t* evt)
{
	return evt->channel;
}

/**
 * Set the channel field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param chan MIDI channel field
 * @return Always returns 0
 */
int fluid_midi_event_set_channel(fluid_midi_event_t* evt, int chan)
{
	evt->channel = chan;
	return FLUID_OK;
}

/**
 * Get the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI note number (0-127)
 */
int fluid_midi_event_get_key(fluid_midi_event_t* evt)
{
	return evt->param1;
}

/**
 * Set the key field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI note number (0-127)
 * @return Always returns 0
 */
int fluid_midi_event_set_key(fluid_midi_event_t* evt, int v)
{
	evt->param1 = v;
	return FLUID_OK;
}

/**
 * Get the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI velocity number (0-127)
 */
int fluid_midi_event_get_velocity(fluid_midi_event_t* evt)
{
	return evt->param2;
}

/**
 * Set the velocity field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI velocity value
 * @return Always returns 0
 */
int fluid_midi_event_set_velocity(fluid_midi_event_t* evt, int v)
{
	evt->param2 = v;
	return FLUID_OK;
}

/**
 * Get the control number of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI control number
 */
int fluid_midi_event_get_control(fluid_midi_event_t* evt)
{
	return evt->param1;
}

/**
 * Set the control field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v MIDI control number
 * @return Always returns 0
 */
int fluid_midi_event_set_control(fluid_midi_event_t* evt, int v)
{
	evt->param1 = v;
	return FLUID_OK;
}

/**
 * Get the value field from a MIDI event structure.
 * @param evt MIDI event structure
 * @return Value field
 */
int fluid_midi_event_get_value(fluid_midi_event_t* evt)
{
	return evt->param2;
}

/**
 * Set the value field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param v Value to assign
 * @return Always returns 0
 */
int fluid_midi_event_set_value(fluid_midi_event_t* evt, int v)
{
	evt->param2 = v;
	return FLUID_OK;
}

/**
 * Get the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return MIDI program number (0-127)
 */
int fluid_midi_event_get_program(fluid_midi_event_t* evt)
{
	return evt->param1;
}

/**
 * Set the program field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val MIDI program number (0-127)
 * @return Always returns 0
 */
int fluid_midi_event_set_program(fluid_midi_event_t* evt, int val)
{
	evt->param1 = val;
	return FLUID_OK;
}

/**
 * Get the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @return Pitch value (DOCME units?)
 */
int fluid_midi_event_get_pitch(fluid_midi_event_t* evt)
{
	return evt->param1;
}

/**
 * Set the pitch field of a MIDI event structure.
 * @param evt MIDI event structure
 * @param val Pitch value (DOCME units?)
 * @return Always returns 0
 */
int fluid_midi_event_set_pitch(fluid_midi_event_t* evt, int val)
{
	evt->param1 = val;
	return FLUID_OK;
}

/*
 * fluid_midi_event_get_param1
 */
/* int fluid_midi_event_get_param1(fluid_midi_event_t* evt) */
/* { */
/*   return evt->param1; */
/* } */

/*
 * fluid_midi_event_set_param1
 */
/* int fluid_midi_event_set_param1(fluid_midi_event_t* evt, int v) */
/* { */
/*   evt->param1 = v; */
/*   return FLUID_OK; */
/* } */

/*
 * fluid_midi_event_get_param2
 */
/* int fluid_midi_event_get_param2(fluid_midi_event_t* evt) */
/* { */
/*   return evt->param2; */
/* } */

/*
 * fluid_midi_event_set_param2
 */
/* int fluid_midi_event_set_param2(fluid_midi_event_t* evt, int v) */
/* { */
/*   evt->param2 = v; */
/*   return FLUID_OK; */
/* } */

/******************************************************
 *
 *     fluid_track_t
 */

/*
 * new_fluid_track
 */
fluid_track_t* new_fluid_track(int num)
{
	fluid_track_t* track;
	track = FLUID_NEW(fluid_track_t);
	if (track == NULL) {
		return NULL;
	}
	track->name = NULL;
	track->num = num;
	track->first = NULL;
	track->cur = NULL;
	track->last = NULL;
	track->ticks = 0;
	return track;
}

/*
 * delete_fluid_track
 */
int delete_fluid_track(fluid_track_t* track)
{
	if (track->name != NULL) {
		FLUID_FREE(track->name);
	}
	if (track->first != NULL) {
		delete_fluid_midi_event(track->first);
	}
	FLUID_FREE(track);
	return FLUID_OK;
}

/*
 * fluid_track_set_name
 */
int fluid_track_set_name(fluid_track_t* track, char* name)
{
	int len;
	if (track->name != NULL) {
		FLUID_FREE(track->name);
	}
	if (name == NULL) {
		track->name = NULL;
		return FLUID_OK;
	}
	len = FLUID_STRLEN(name);
	track->name = FLUID_MALLOC(len + 1);
	if (track->name == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return FLUID_FAILED;
	}
	FLUID_STRCPY(track->name, name);
	return FLUID_OK;
}

/*
 * fluid_track_get_name
 */
char* fluid_track_get_name(fluid_track_t* track)
{
	return track->name;
}

/*
 * fluid_track_get_duration
 */
int fluid_track_get_duration(fluid_track_t* track)
{
	int time = 0;
	fluid_midi_event_t* evt = track->first;
	while (evt != NULL) {
		time += evt->dtime;
		evt = evt->next;
	}
	return time;
}

/*
 * fluid_track_count_events
 */
int fluid_track_count_events(fluid_track_t* track, int* on, int* off)
{
	fluid_midi_event_t* evt = track->first;
	while (evt != NULL) {
		if (evt->type == NOTE_ON) {
			(*on)++;
		} else if (evt->type == NOTE_OFF) {
			(*off)++;
		}
		evt = evt->next;
	}
	return FLUID_OK;
}

/*
 * fluid_track_add_event
 */
int fluid_track_add_event(fluid_track_t* track, fluid_midi_event_t* evt)
{
	evt->next = NULL;
	if (track->first == NULL) {
		track->first = evt;
		track->cur = evt;
		track->last = evt;
	} else {
		track->last->next = evt;
		track->last = evt;
	}
	return FLUID_OK;
}

/*
 * fluid_track_first_event
 */
fluid_midi_event_t* fluid_track_first_event(fluid_track_t* track)
{
	track->cur = track->first;
	return track->cur;
}

/*
 * fluid_track_next_event
 */
fluid_midi_event_t* fluid_track_next_event(fluid_track_t* track)
{
	if (track->cur != NULL) {
		track->cur = track->cur->next;
	}
	return track->cur;
}

/*
 * fluid_track_reset
 */
int
fluid_track_reset(fluid_track_t* track)
{
	track->ticks = 0;
	track->cur = track->first;
	return FLUID_OK;
}

/*
 * fluid_track_send_events
 */
int
fluid_track_send_events(fluid_track_t* track,
			fluid_synth_t* synth,
			fluid_player_t* player,
			unsigned int ticks)
{
	int status = FLUID_OK;
	fluid_midi_event_t* event;

	while (1) {

		event = track->cur;
		if (event == NULL) {
			return status;
		}

/* 		printf("track=%02d\tticks=%05u\ttrack=%05u\tdtime=%05u\tnext=%05u\n", */
/* 		       track->num, */
/* 		       ticks, */
/* 		       track->ticks, */
/* 		       event->dtime, */
/* 		       track->ticks + event->dtime); */

		if (track->ticks + event->dtime > ticks) {
			return status;
		}


		track->ticks += event->dtime;
		status = fluid_midi_send_event(synth, player, event);
		fluid_track_next_event(track);

	}
	return status;
}

/******************************************************
 *
 *     fluid_player
 */

/**
 * Create a new MIDI player.
 * @param synth Fluid synthesizer instance to create player for
 * @return New MIDI player instance or NULL on error (out of memory)
 */
fluid_player_t* new_fluid_player(fluid_synth_t* synth)
{
	int i;
	fluid_player_t* player;
	player = FLUID_NEW(fluid_player_t);
	if (player == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	player->status = FLUID_PLAYER_READY;
	player->loop = 0;
	player->ntracks = 0;
	for (i = 0; i < MAX_NUMBER_OF_TRACKS; i++) {
		player->track[i] = NULL;
	}
	player->synth = synth;
	player->timer = NULL;
	player->playlist = NULL;
	player->current_file = NULL;
	player->division = 0;
	player->send_program_change = 1;
	player->miditempo = 480000;
	player->deltatime = 4.0;
	return player;
}

/**
 * Delete a MIDI player instance.
 * @param player MIDI player instance
 * @return Always returns 0
 */
int delete_fluid_player(fluid_player_t* player)
{
	if (player == NULL) {
		return FLUID_OK;
	}
	fluid_player_stop(player);
	fluid_player_reset(player);
	FLUID_FREE(player);
	return FLUID_OK;
}

int fluid_player_reset(fluid_player_t* player)
{
	int i;

	for (i = 0; i < MAX_NUMBER_OF_TRACKS; i++) {
		if (player->track[i] != NULL) {
			delete_fluid_track(player->track[i]);
			player->track[i] = NULL;
		}
	}
	player->current_file = NULL;
	player->status = FLUID_PLAYER_READY;
	player->loop = 0;
	player->ntracks = 0;
	player->division = 0;
	player->send_program_change = 1;
	player->miditempo = 480000;
	player->deltatime = 4.0;
	return 0;
}

/*
 * fluid_player_add_track
 */
int fluid_player_add_track(fluid_player_t* player, fluid_track_t* track)
{
	if (player->ntracks < MAX_NUMBER_OF_TRACKS) {
		player->track[player->ntracks++] = track;
		return FLUID_OK;
	} else {
		return FLUID_FAILED;
	}
}

/*
 * fluid_player_count_tracks
 */
int fluid_player_count_tracks(fluid_player_t* player)
{
	return player->ntracks;
}

/*
 * fluid_player_get_track
 */
fluid_track_t* fluid_player_get_track(fluid_player_t* player, int i)
{
	if ((i >= 0) && (i < MAX_NUMBER_OF_TRACKS)) {
		return player->track[i];
	} else {
		return NULL;
	}
}

int fluid_player_add(fluid_player_t* player, char* midifile)
{
	char *s = FLUID_STRDUP(midifile);
	player->playlist = fluid_list_append(player->playlist, s);
	return 0;
}

/*
 * fluid_player_load
 */
int fluid_player_load(fluid_player_t* player, char *filename)
{
	fluid_midi_file* midifile;

	midifile = new_fluid_midi_file(filename);
	if (midifile == NULL) {
		return FLUID_FAILED;
	}
	player->division = fluid_midi_file_get_division(midifile);

	/*FLUID_LOG(FLUID_DBG, "quarter note division=%d\n", player->division); */

	if (fluid_midi_file_load_tracks(midifile, player) != FLUID_OK){
		return FLUID_FAILED;
	}
	delete_fluid_midi_file(midifile);
	return FLUID_OK;
}

/*
 * fluid_player_callback
 */
int fluid_player_callback(void* data, unsigned int msec)
{
	int i;
	int status = FLUID_PLAYER_DONE;
	fluid_player_t* player;
	fluid_synth_t* synth;
	player = (fluid_player_t*) data;
	synth = player->synth;

	/* Load the next file if necessary */
	while (player->current_file == NULL) {

		if (player->playlist == NULL) {
			return 0;
		}

		fluid_player_reset(player);

		player->current_file = fluid_list_get(player->playlist);
		player->playlist = fluid_list_next(player->playlist);

		FLUID_LOG(FLUID_DBG, "%s: %d: Loading midifile %s", __FILE__, __LINE__, player->current_file);

		if (fluid_player_load(player, player->current_file) == FLUID_OK) {

			player->begin_msec = msec;
			player->start_msec = msec;
			player->start_ticks = 0;
			player->cur_ticks = 0;

			for (i = 0; i < player->ntracks; i++) {
				if (player->track[i] != NULL) {
					fluid_track_reset(player->track[i]);
				}
			}

		} else {
			player->current_file = NULL;
		}
	}

	player->cur_msec = msec;
	player->cur_ticks = (player->start_ticks +
			     (int) ((double) (player->cur_msec - player->start_msec) / player->deltatime));

	for (i = 0; i < player->ntracks; i++) {
		if (!fluid_track_eot(player->track[i])) {
			status = FLUID_PLAYER_PLAYING;
			if (fluid_track_send_events(player->track[i], synth, player, player->cur_ticks) != FLUID_OK) {
				/* */
			}
		}
	}

	player->status = status;

	if (player->status == FLUID_PLAYER_DONE) {
		FLUID_LOG(FLUID_DBG, "%s: %d: Duration=%.3f sec",
			  __FILE__, __LINE__, (msec - player->begin_msec) / 1000.0);
		player->current_file = NULL;
	}

	return 1;
}

/**
 * Activates play mode for a MIDI player if not already playing.
 * @param player MIDI player instance
 * @return 0 on success, -1 on failure
 */
int fluid_player_play(fluid_player_t* player)
{
	if (player->status == FLUID_PLAYER_PLAYING) {
		return FLUID_OK;
	}

	if (player->playlist == NULL) {
		return FLUID_OK;
	}

	player->status = FLUID_PLAYER_PLAYING;

	player->timer = new_fluid_timer((int) player->deltatime, fluid_player_callback,
					(void*) player, 1, 0);
	if (player->timer == NULL) {
		return FLUID_FAILED;
	}
	return FLUID_OK;
}

/**
 * Stops a MIDI player.
 * @param player MIDI player instance
 * @return Always returns 0
 */
int fluid_player_stop(fluid_player_t* player)
{
	if (player->timer != NULL) {
		delete_fluid_timer(player->timer);
	}
	player->status = FLUID_PLAYER_DONE;
	player->timer = NULL;
	return FLUID_OK;
}

/* FIXME - Looping seems to not actually be implemented? */

/**
 * Enable looping of a MIDI player (DOCME - Does this actually work?)
 * @param player MIDI player instance
 * @param loop Value for looping (DOCME - What would this value be, boolean/time index?)
 * @return Always returns 0
 */
int fluid_player_set_loop(fluid_player_t* player, int loop)
{
	player->loop = loop;
	return FLUID_OK;
}

/**
 * Set the tempo of a MIDI player.
 * @param player MIDI player instance
 * @param tempo Tempo to set playback speed to (DOCME - Units?)
 * @return Always returns 0
 *
 */
int fluid_player_set_midi_tempo(fluid_player_t* player, int tempo)
{
	player->miditempo = tempo;
	player->deltatime = (double) tempo / player->division / 1000.0; /* in milliseconds */
	player->start_msec = player->cur_msec;
	player->start_ticks = player->cur_ticks;

	FLUID_LOG(FLUID_DBG,"tempo=%d, tick time=%f msec, cur time=%d msec, cur tick=%d",
		  tempo, player->deltatime, player->cur_msec, player->cur_ticks);

	return FLUID_OK;
}

/**
 * Set the tempo of a MIDI player in beats per minute.
 * @param player MIDI player instance
 * @param bpm Tempo in beats per minute
 * @return Always returns 0
 */
int fluid_player_set_bpm(fluid_player_t* player, int bpm)
{
	return fluid_player_set_midi_tempo(player, (int)((double) 60 * 1e6 / bpm));
}

/**
 * Wait for a MIDI player to terminate (when done playing).
 * @param player MIDI player instance
 * @return 0 on success, -1 otherwise
 *
 */
int fluid_player_join(fluid_player_t* player)
{
	return player->timer? fluid_timer_join(player->timer) : FLUID_OK;
}

/************************************************************************
 *       MIDI PARSER
 *
 */

/*
 * new_fluid_midi_parser
 */
fluid_midi_parser_t* new_fluid_midi_parser()
{
	fluid_midi_parser_t* parser;
	parser = FLUID_NEW(fluid_midi_parser_t);
	if (parser == NULL) {
		FLUID_LOG(FLUID_ERR, "Out of memory");
		return NULL;
	}
	parser->status = 0; /* As long as the status is 0, the parser won't do anything -> no need to initialize all the fields. */
	return parser;
}

/*
 * delete_fluid_midi_parser
 */
int delete_fluid_midi_parser(fluid_midi_parser_t* parser)
{
	FLUID_FREE(parser);
	return FLUID_OK;
}

/*
 * fluid_midi_parser_parse
 *
 * Purpose:
 * The MIDI byte stream is fed into the parser, one byte at a time.
 * As soon as the parser has recognized an event, it will return it.
 * Otherwise it returns NULL.
 */
fluid_midi_event_t* fluid_midi_parser_parse(fluid_midi_parser_t* parser, unsigned char c)
{
	/*********************************************************************/
	/* 'Process' system real-time messages                               */
	/*********************************************************************/
	/* There are not too many real-time messages that are of interest here.
	 * They can occur anywhere, even in the middle of a noteon message!
	 * Real-time range: 0xF8 .. 0xFF
	 * Note: Real-time does not affect (running) status.
	 */
	if (c >= 0xF8){
		if (c == MIDI_SYSTEM_RESET){
			parser->event.type = c;
			parser->status = 0; /* clear the status */
			return &parser->event;
		};
		return NULL;
	};

	/*********************************************************************/
	/* 'Process' system common messages (again, just skip them)          */
	/*********************************************************************/
	/* There are no system common messages that are of interest here.
	 * System common range: 0xF0 .. 0xF7
	 */

	if (c > 0xF0){
		/* MIDI specs say: To ignore a non-real-time message, just discard all data up to
		 * the next status byte.
		 * And our parser will ignore data that is received without a valid status.
		 * Note: system common cancels running status. */
		parser->status = 0;
		return NULL;
	};

	/*********************************************************************/
	/* Process voice category messages:                                  */
	/*********************************************************************/
	/* Now that we have handled realtime and system common messages, only
	 * voice messages are left.
	 * Only a status byte has bit # 7 set.
	 * So no matter the status of the parser (in case we have lost sync),
	 * as soon as a byte >= 0x80 comes in, we are dealing with a status byte
	 * and start a new event.
	 */

	if (c & 0x80){
		parser->channel = c & 0x0F;
		parser->status = c & 0xF0;
		/* The event consumes x bytes of data... (subtract 1 for the status byte) */
		parser->nr_bytes_total=fluid_midi_event_length(parser->status)-1;
		/* of which we have read 0 at this time. */
		parser->nr_bytes = 0;
		return NULL;
	};

	/*********************************************************************/
	/* Process data                                                      */
	/*********************************************************************/
	/* If we made it this far, then the received char belongs to the data
	 * of the last event. */
	if (parser->status == 0){
		/* We are not interested in the event currently received.
		 * Discard the data. */
		return NULL;
	};

	/* Store the first couple of bytes */
	if (parser->nr_bytes < FLUID_MIDI_PARSER_MAX_PAR){
		parser->p[parser->nr_bytes]=c;
	};
	parser->nr_bytes++;

	/* Do we still need more data to get this event complete? */
	if (parser->nr_bytes < parser->nr_bytes_total){
		return NULL;
	};

	/*********************************************************************/
	/* Send the event                                                    */
	/*********************************************************************/
	/* The event is ready-to-go.
	 * About 'running status':
	 * The MIDI protocol has a built-in compression mechanism. If several similar events are sent
	 * in-a-row, for example note-ons, then the event type is only sent once. For this case,
	 * the last event type (status) is remembered.
	 * We simply keep the status as it is, just reset
	 * the parameter counter. If another status byte comes in, it will overwrite the status. */
	parser->event.type = parser->status;
	parser->event.channel = parser->channel;
	parser->nr_bytes = 0; /* Related to running status! */
	switch (parser->status){
	case NOTE_OFF:
	case NOTE_ON:
	case KEY_PRESSURE:
	case CONTROL_CHANGE:
	case PROGRAM_CHANGE:
	case CHANNEL_PRESSURE:
		parser->event.param1 = parser->p[0]; /* For example key number */
		parser->event.param2 = parser->p[1]; /* For example velocity */
		break;
	case PITCH_BEND:
		/* Pitch-bend is transmitted with 14-bit precision. */
		parser->event.param1 = ((parser->p[1] << 7) | parser->p[0]); /* Note: '|' does here the same as '+' (no common bits), but might be faster */
		break;
	default:
		/* Unlikely */
		return NULL;
	};
	return &parser->event;
};

/* Purpose:
 * Returns the length of the MIDI message starting with c.
 * Taken from Nagano Daisuke's USB-MIDI driver */
int fluid_midi_event_length(unsigned char event){
	if ( event < 0xf0 ) {
		return remains_80e0[((event-0x80)>>4)&0x0f];
	} else if ( event < 0xf7 ) {
		return remains_f0f6[event-0xf0];
	} else {
		return 1;
	}
}

/************************************************************************
 *       fluid_midi_send_event
 *
 * This is a utility function that doesn't really belong to any class
 * or structure. It is called by fluid_midi_track and fluid_midi_device.
 */
int fluid_midi_send_event(fluid_synth_t* synth, fluid_player_t* player, fluid_midi_event_t* event)
{
	switch (event->type) {
	case NOTE_ON:
		if (fluid_synth_noteon(synth, event->channel, event->param1, event->param2) != FLUID_OK) {
			return FLUID_FAILED;
		}
		break;
	case NOTE_OFF:
		if (fluid_synth_noteoff(synth, event->channel, event->param1) != FLUID_OK) {
			return FLUID_FAILED;
		}
		break;
	case CONTROL_CHANGE:
		if (fluid_synth_cc(synth, event->channel, event->param1, event->param2) != FLUID_OK) {
			return FLUID_FAILED;
		}
		break;
	case MIDI_SET_TEMPO:
		if (player != NULL) {
			if (fluid_player_set_midi_tempo(player, event->param1) != FLUID_OK) {
				return FLUID_FAILED;
			}
		}
		break;
	case PROGRAM_CHANGE:
		if (fluid_synth_program_change(synth, event->channel, event->param1) != FLUID_OK) {
			return FLUID_FAILED;
		}
		break;
	case PITCH_BEND:
		if (fluid_synth_pitch_bend(synth, event->channel, event->param1) != FLUID_OK) {
			return FLUID_FAILED;
		}
		break;
	default:
		break;
	}
	return FLUID_OK;
}
