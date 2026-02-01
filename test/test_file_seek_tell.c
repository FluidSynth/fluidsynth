#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "utils/fluid_sys.h"

void noop(void)
{
    // do nothing
}

// Some ftell or fseek implementations of some MinGW versions are broken.
// Mimick the simple behavior of the sfont loader to test seek and tell are working as expected.
int main(void)
{
    fluid_long_long_t pos, size;
    fluid_sfloader_t* loader = new_fluid_sfloader((fluid_sfloader_load_t)noop, (fluid_sfloader_free_t)noop);
    
    fluid_file_callbacks_t* fcbs = &loader->file_callbacks;

    void * file = fcbs->fopen(TEST_SOUNDFONT);
    TEST_ASSERT(file != NULL);

    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 0);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_END) == FLUID_OK);
    size = fcbs->ftell(file);
    TEST_ASSERT(size == 314640);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_SET) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 0);

    {
        unsigned char buf[1024];

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 4);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 8);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 12);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 16);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 20);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 24);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 28);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 32);

        TEST_ASSERT(fcbs->fread(buf, 2, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 34);

        TEST_ASSERT(fcbs->fread(buf, 2, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 36);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 40);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 44);

        TEST_ASSERT(fcbs->fread(buf, 28, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 72);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 76);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 80);

        TEST_ASSERT(fcbs->fread(buf, 8, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 88);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 92);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 96);

        TEST_ASSERT(fcbs->fread(buf, 6, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 102);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 106);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 110);

        TEST_ASSERT(fcbs->fread(buf, 2, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 112);

        TEST_ASSERT(fcbs->fread(buf, 2, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 114);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 118);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 122);

        TEST_ASSERT(fcbs->fread(buf, 8, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 130);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 134);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 138);

        TEST_ASSERT(fcbs->fread(buf, 12, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 150);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 154);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 158);

        TEST_ASSERT(fcbs->fread(buf, 24, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 182);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 186);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 190);

        TEST_ASSERT(fcbs->fread(buf, 14, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 204);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 208);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 212);

        TEST_ASSERT(fcbs->fread(buf, 184, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 396);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 400);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 404);

        TEST_ASSERT(fcbs->fread(buf, 40, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 444);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 448);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 452);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 456);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 460);

        TEST_ASSERT(fcbs->fread(buf, 4, file) == FLUID_OK);
        pos = fcbs->ftell(file);
        TEST_ASSERT(pos == 464);
    }

    TEST_ASSERT(fcbs->fseek(file, 252534, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 252534 + 464);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 252534 + 464);

    TEST_ASSERT(fcbs->fseek(file, -252534, SEEK_CUR) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 464);

    TEST_ASSERT(fcbs->fseek(file, 0, SEEK_SET) == FLUID_OK);
    pos = fcbs->ftell(file);
    TEST_ASSERT(pos == 0);

    TEST_ASSERT(fcbs->fclose(file) == FLUID_OK);
    delete_fluid_sfloader(loader);

    return EXIT_SUCCESS;
}