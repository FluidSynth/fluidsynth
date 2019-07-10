
#include "test.h"
#include "fluidsynth.h" // use local fluidsynth header

// simple test to ensure that manually unregistering and deleting the internal fluid_seqbind_t works without crashing
int main(void)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);
    fluid_sequencer_t *seq = new_fluid_sequencer2(0 /*i.e. use sample timer*/);

    // silently creates a fluid_seqbind_t
    int seqid = fluid_sequencer_register_fluidsynth(seq, synth);

    fluid_event_t *evt = new_fluid_event();
    fluid_event_set_source(evt, -1);
    fluid_event_set_dest(evt, seqid);

    // manually free the fluid_seqbind_t
    fluid_event_unregistering(evt);
    fluid_sequencer_send_now(seq, evt);

    // client should be removed, deleting the seq should not free the struct again
    delete_fluid_event(evt);
    delete_fluid_sequencer(seq);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return EXIT_SUCCESS;
}
