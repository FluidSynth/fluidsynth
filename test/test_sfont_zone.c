
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "sfloader/fluid_sffile.h"
#include "utils/fluid_sys.h"



static unsigned char *file_buf = NULL;
static int test_reader(void *buf, fluid_long_long_t count, void *h)
{
    FLUID_MEMCPY(buf, file_buf, count);
    file_buf += count;
    return FLUID_OK;
}

static int test_seek(void *handle, fluid_long_long_t offset, int origin)
{
    // shouldn't happen?
    TEST_ASSERT(0);
}

static const fluid_file_callbacks_t fcb =
{
    NULL, &test_reader, &test_seek, NULL, NULL
};

int main(void)
{
    // prepare a soundfont that has one preset and one instrument, with up to 3 zones each

    SFData* sf = FLUID_NEW(SFData);
    SFPreset *preset = FLUID_NEW(SFPreset);
    SFZone *zone1 = FLUID_NEW(SFZone);
    SFZone *zone2 = FLUID_NEW(SFZone);
    SFZone *zone3 = FLUID_NEW(SFZone);

    TEST_ASSERT(sf != NULL);
    FLUID_MEMSET(sf, 0, sizeof(*sf));
    TEST_ASSERT(preset != NULL);
    FLUID_MEMSET(preset, 0, sizeof(*preset));
    TEST_ASSERT(zone1 != NULL);
    FLUID_MEMSET(zone1, 0, sizeof(*zone1));
    TEST_ASSERT(zone2 != NULL);
    FLUID_MEMSET(zone2, 0, sizeof(*zone2));
    TEST_ASSERT(zone3 != NULL);
    FLUID_MEMSET(zone3, 0, sizeof(*zone3));

    sf->fcbs = &fcb;
    sf->preset = fluid_list_append(sf->preset, preset);

    //preset->zone = fluid_list_prepend(preset->zone, zone3);
    //preset->zone = fluid_list_prepend(preset->zone, zone2);
    preset->zone = fluid_list_prepend(preset->zone, zone1);

    zone1->gen = fluid_list_prepend(zone1->gen, NULL);
    zone1->gen = fluid_list_prepend(zone1->gen, NULL);

    // test the good case first: one preset, with one zone, with two generators
    unsigned char buf[] = { Gen_KeyRange, 0, 60, 127, Gen_VelRange, 0, 60, 127 };
    file_buf = (unsigned char *)buf;
    TEST_ASSERT(load_pgen(sf, sizeof(buf) / sizeof(*buf)));

    SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
    TEST_ASSERT(gen->id == Gen_KeyRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
    TEST_ASSERT(gen->id == Gen_VelRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    return EXIT_SUCCESS;
}
