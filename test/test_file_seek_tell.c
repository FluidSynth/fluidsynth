
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "utils/fluid_sys.h"
#include <string.h>

void noop(void)
{
    // do nothing
}

// Some ftell or fseek implementations of some MinGW versions are broken.
// Mimick the simple behavior of the sfont loader to test seek and tell are working as expected.
int main(void)
{
    fluid_long_long_t pos, size;
    unsigned char header[12];
    fluid_sfloader_t* loader = new_fluid_sfloader((fluid_sfloader_load_t)noop, (fluid_sfloader_free_t)noop);
    void *file;

    fluid_file_callbacks_t* fcbs = &loader->file_callbacks;

    file = fcbs->fopen(TEST_SOUNDFONT);
    TEST_ASSERT(file != NULL);

    TEST_ASSERT(fcbs->fread(header, 12, file) == FLUID_OK);
    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_SET) == FLUID_OK);
    TEST_ASSERT(memcmp(header, "RIFF", 4) == 0);
    TEST_ASSERT(memcmp(header + 8, "sfbk", 4) == 0);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_END) == FLUID_OK);
    size = fcbs->ftell(file);
    TEST_ASSERT(size == 314640);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_SET) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 0);

    TEST_ASSERT(fcbs->fseek(file, 464, SEEK_SET) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 464);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 464);

    TEST_ASSERT(fcbs->fseek(file, 252534, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 252534 + 464);

    TEST_ASSERT(fcbs->fseek(file, -252534, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 464);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_END) == FLUID_OK);
    size = fcbs->ftell(file);
    TEST_ASSERT(size == 314640);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_SET) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 0);

    fcbs->fclose(file);
    delete_fluid_sfloader(loader);

    return EXIT_SUCCESS;
}
