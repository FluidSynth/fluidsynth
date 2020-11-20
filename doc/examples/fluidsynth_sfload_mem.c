/*
 * This is a C99 program that demonstrates how to load a soundfont from memory.
 *
 * It only gives a brief overview on how to achieve this with fluidsynth's API.
 * Although it should compile, it's highly incomplete, as the details of it's
 * implementation depend on the users needs.
 */

#include <stdio.h>
#include <string.h>
#include <fluidsynth.h>

void *my_open(const char *filename)
{
    void *p;

    if(filename[0] != '&')
    {
        return NULL;
    }

    sscanf(filename, "&%p", &p);
    return p;
}

int my_read(void *buf, int count, void *handle)
{
    // NYI
    return FLUID_OK;
}

int my_seek(void *handle, long offset, int origin)
{
    // NYI
    return FLUID_OK;
}

int my_close(void *handle)
{
    // NYI
    return FLUID_OK;
}

long my_tell(void *handle)
{
    // NYI
    return 0;
}

int main()
{
    int err = 0;

    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth = new_fluid_synth(settings);

    fluid_sfloader_t *my_sfloader = new_fluid_defsfloader(settings);
    fluid_sfloader_set_callbacks(my_sfloader,
                                 my_open,
                                 my_read,
                                 my_seek,
                                 my_tell,
                                 my_close);
    fluid_synth_add_sfloader(synth, my_sfloader);


    char abused_filename[64];
    const void *pointer_to_sf2_in_mem = 0x1234Beef; // some pointer to where the soundfont shall be loaded from
    sprintf(abused_filename, "&%p", pointer_to_sf2_in_mem);

    int id = fluid_synth_sfload(synth, abused_filename, 0);
    /* now my_open() will be called with abused_filename and should have opened the memory region */

    if(id == FLUID_FAILED)
    {
        puts("oops");
        err = -1;
        goto cleanup;
    }

    /*
     * ~~~ Do your daily business here ~~~
     */

cleanup:
    /* deleting the synth also deletes my_sfloader */
    delete_fluid_synth(synth);

    delete_fluid_settings(settings);

    return err;
}
