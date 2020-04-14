/* FluidSynth Metronome - Sequencer API example
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -o fluidsynth_metronome -lfluidsynth fluidsynth_metronome.c
 *
 * To run:
 *   fluidsynth_metronome soundfont [beats [tempo]]
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
/* default tempo, beats per minute */
#define TEMPO 120
unsigned int note_duration = 60000 / TEMPO;
/* metronome click/bell */
unsigned int weak_note = 33;
unsigned int strong_note = 34;
/* number of notes in one pattern */
unsigned int pattern_size = 4;
/* prototype */
void
sequencer_callback(unsigned int time, fluid_event_t *event,
                   fluid_sequencer_t *seq, void *data);

/* schedule a note on message */
void
schedule_noteon(int chan, short key, unsigned int ticks)
{
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, synth_destination);
    fluid_event_noteon(ev, chan, key, 127);
    fluid_sequencer_send_at(sequencer, ev, ticks, 1);
    delete_fluid_event(ev);
}

/* schedule a timer event (shall trigger the callback) */
void
schedule_timer_event(void)
{
    fluid_event_t *ev = new_fluid_event();
    fluid_event_set_source(ev, -1);
    fluid_event_set_dest(ev, client_destination);
    fluid_event_timer(ev, NULL);
    fluid_sequencer_send_at(sequencer, ev, time_marker, 1);
    delete_fluid_event(ev);
}

/* schedule the metronome pattern */
void
schedule_pattern(void)
{
    int i, note_time;
    note_time = time_marker;

    for(i = 0; i < pattern_size; ++i)
    {
        schedule_noteon(9, i ? weak_note : strong_note, note_time);
        note_time += note_duration;
    }

    time_marker = note_time;
}

void
sequencer_callback(unsigned int time, fluid_event_t *event,
                   fluid_sequencer_t *seq, void *data)
{
    schedule_timer_event();
    schedule_pattern();
}

void
usage(char *prog_name)
{
    printf("Usage: %s soundfont.sf2 [beats [tempo]]\n", prog_name);
    printf("\t(optional) beats: number of pattern beats, default %d\n",
           pattern_size);
    printf("\t(optional) tempo: BPM (Beats Per Minute), default %d\n", TEMPO);
}

int
main(int argc, char *argv[])
{
    int n;
    fluid_settings_t *settings;
    settings = new_fluid_settings();

    if(argc < 2)
    {
        usage(argv[0]);
    }
    else
    {
        /* create the synth, driver and sequencer instances */
        synth = new_fluid_synth(settings);

        /* load a SoundFont */
        n = fluid_synth_sfload(synth, argv[1], 1);

        if(n != -1)
        {
            sequencer = new_fluid_sequencer2(0);
            /* register the synth with the sequencer */
            synth_destination = fluid_sequencer_register_fluidsynth(sequencer,
                                synth);
            /* register the client name and callback */
            client_destination = fluid_sequencer_register_client(sequencer,
                                "fluidsynth_metronome", sequencer_callback, NULL);

            audiodriver = new_fluid_audio_driver(settings, synth);

            if(argc > 2)
            {
                n = atoi(argv[2]);

                if(n > 0)
                {
                    pattern_size = n;
                }
            }

            if(argc > 3)
            {
                n = atoi(argv[3]);

                if(n > 0)
                {
                    note_duration = 60000 / n;
                }
            }

            /* get the current time in ticks */
            time_marker = fluid_sequencer_get_tick(sequencer);
            /* schedule patterns */
            schedule_pattern();
            schedule_timer_event();
            schedule_pattern();
            /* wait for user input */
            printf("press <Enter> to stop\n");
            n = getchar();
        }

        /* clean and exit */
        delete_fluid_audio_driver(audiodriver);
        delete_fluid_sequencer(sequencer);
        delete_fluid_synth(synth);
    }

    delete_fluid_settings(settings);
    return 0;
}
