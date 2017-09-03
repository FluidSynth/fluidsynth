/* FluidSynth Arpeggio - Sequencer API example
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -o fluidsynth_arpeggio -lfluidsynth fluidsynth_arpeggio.c
 *
 * To run:
 *   fluidsynth_arpeggio soundfont [steps [duration]]
 *
 * [Pedro Lopez-Cabanillas <plcl@users.sf.net>]
 */

#include <stdlib.h>
#include <stdio.h>
#include <fluidsynth.h>

fluid_synth_t *synth;
fluid_audio_driver_t *audiodriver;
fluid_sequencer_t *sequencer;
short synth_destination, client_destination;
unsigned int time_marker;
/* duration of the pattern in ticks. */
unsigned int duration = 1440;
/* notes of the arpeggio */
unsigned int notes[] = { 60, 64, 67, 72, 76, 79, 84, 79, 76, 72, 67, 64 };
/* number of notes in one pattern */
unsigned int pattern_size;
/* prototype */
void
sequencer_callback (unsigned int time, fluid_event_t *event,
                    fluid_sequencer_t *seq, void *data);

/* schedule a note on message */
void
schedule_noteon (int chan, short key, unsigned int ticks)
{
    fluid_event_t *ev = new_fluid_event ();
    fluid_event_set_source (ev, -1);
    fluid_event_set_dest (ev, synth_destination);
    fluid_event_noteon (ev, chan, key, 127);
    fluid_sequencer_send_at (sequencer, ev, ticks, 1);
    delete_fluid_event (ev);
}

/* schedule a note off message */
void
schedule_noteoff (int chan, short key, unsigned int ticks)
{
    fluid_event_t *ev = new_fluid_event ();
    fluid_event_set_source (ev, -1);
    fluid_event_set_dest (ev, synth_destination);
    fluid_event_noteoff (ev, chan, key);
    fluid_sequencer_send_at (sequencer, ev, ticks, 1);
    delete_fluid_event (ev);
}

/* schedule a timer event (shall trigger the callback) */
void
schedule_timer_event ()
{
    fluid_event_t *ev = new_fluid_event ();
    fluid_event_set_source (ev, -1);
    fluid_event_set_dest (ev, client_destination);
    fluid_event_timer (ev, NULL);
    fluid_sequencer_send_at (sequencer, ev, time_marker, 1);
    delete_fluid_event (ev);
}

/* schedule the arpeggio's notes */
void
schedule_pattern ()
{
    int i, note_time, note_duration;
    note_time = time_marker;
    note_duration = duration / pattern_size;
    for (i = 0; i < pattern_size; ++i) {
        schedule_noteon (0, notes[i], note_time);
        note_time += note_duration;
        schedule_noteoff (0, notes[i], note_time);
    }
    time_marker += duration;
}

void
sequencer_callback (unsigned int time, fluid_event_t *event,
                    fluid_sequencer_t *seq, void *data)
{
    schedule_timer_event ();
    schedule_pattern ();
}

void
usage (char* prog_name)
{
    printf ("Usage: %s soundfont.sf2 [steps [duration]]\n", prog_name);
    printf ("\t(optional) steps: number of pattern notes, from 2 to %d\n",
            pattern_size);
    printf ("\t(optional) duration: of the pattern in ticks, default %d\n",
            duration);
}

int
main (int argc, char* argv[])
{
    int n;
    fluid_settings_t *settings;
    settings = new_fluid_settings ();
    pattern_size = sizeof(notes) / sizeof(int);
    if (argc < 2) {
        usage (argv[0]);
    } else {
        /* create the synth, driver and sequencer instances */
        synth = new_fluid_synth (settings);
        audiodriver = new_fluid_audio_driver (settings, synth);
        sequencer = new_fluid_sequencer ();
        /* register the synth with the sequencer */
        synth_destination = fluid_sequencer_register_fluidsynth (sequencer,
                synth);
        /* register the client name and callback */
        client_destination = fluid_sequencer_register_client (sequencer,
                "arpeggio", sequencer_callback, NULL);
        /* load a SoundFont */
        n = fluid_synth_sfload (synth, argv[1], 1);
        if (n != -1) {
            if (argc > 2) {
                n = atoi (argv[2]);
                if ((n > 1) && (n <= pattern_size)) pattern_size = n;
            }
            if (argc > 3) {
                n = atoi (argv[3]);
                if (n > 0) duration = n;
            }
            /* get the current time in ticks */
            time_marker = fluid_sequencer_get_tick (sequencer);
            /* schedule patterns */
            schedule_pattern ();
            schedule_timer_event ();
            schedule_pattern ();
            /* wait for user input */
            printf ("press <Enter> to stop\n");
            n = getchar ();
        }
        /* clean and exit */
        delete_fluid_sequencer (sequencer);
        delete_fluid_audio_driver (audiodriver);
        delete_fluid_synth (synth);
    }
    delete_fluid_settings (settings);
    return 0;
}
