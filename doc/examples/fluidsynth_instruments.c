/* FluidSynth Instruments - An example of using fluidsynth >= 2.x
 *
 * This code is in the public domain.
 *
 * To compile:
 *   gcc -o fluidsynth_instruments fluidsynth_instruments.c -lfluidsynth
 *
 * To run:
 *   fluidsynth_instruments soundfont
 *
 * [Pedro López-Cabanillas <plcl@users.sf.net>]
 */

#include <stdio.h>
#include <fluidsynth.h>

int main(int argc, char** argv)
{
	fluid_settings_t* settings = NULL;
	fluid_synth_t* synth = NULL;
	fluid_sfont_t* sfont = NULL;
	int err = 0, sfid = -1;

	if (argc != 2) {
		fprintf(stderr, "Usage: fluidsynth_instr [soundfont]\n");
		return 1;
	}

	/* Create the settings object. This example uses the default
	 * values for the settings. */
	settings = new_fluid_settings();
	if (settings == NULL) {
		fprintf(stderr, "Failed to create the settings\n");
		err = 2;
		goto cleanup;
	}

	/* Create the synthesizer */
	synth = new_fluid_synth(settings);
	if (synth == NULL) {
		fprintf(stderr, "Failed to create the synthesizer\n");
		err = 3;
		goto cleanup;
	}

	/* Load the soundfont */
	sfid = fluid_synth_sfload(synth, argv[1], 1);
	if (sfid == -1) {
		fprintf(stderr, "Failed to load the SoundFont\n");
		err = 4;
		goto cleanup;
	}

    /* Enumeration of banks and programs */
    sfont = fluid_synth_get_sfont_by_id(synth, sfid);
    if (sfont != NULL) {
        fluid_preset_t *preset;
        fluid_sfont_iteration_start(sfont);
        while ((preset = fluid_sfont_iteration_next(sfont)) != NULL) {
            int bank = fluid_preset_get_banknum(preset);
            int prog = fluid_preset_get_num(preset);
            const char* name = fluid_preset_get_name(preset);
			printf("bank: %d prog: %d name: %s\n", bank, prog, name);
		}
	}

	printf("done\n");

 cleanup:
	if (synth) {
		delete_fluid_synth(synth);
	}
	if (settings) {
		delete_fluid_settings(settings);
	}
	return err;
}
