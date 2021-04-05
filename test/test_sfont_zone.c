
#include "test.h"
#include "fluidsynth.h"
#include "sfloader/fluid_sfont.h"
#include "sfloader/fluid_defsfont.h"
#include "sfloader/fluid_sffile.h"
#include "utils/fluid_sys.h"


static const unsigned char *file_buf = NULL;
static int test_reader(void *buf, fluid_long_long_t count, void *h)
{
    FLUID_MEMCPY(buf, file_buf, count);
    file_buf += count;
    return FLUID_OK;
}

static int test_seek(void *handle, fluid_long_long_t offset, int origin)
{
    if (origin == SEEK_CUR)
    {
        file_buf += offset;
        return FLUID_OK;
    }

    // shouldn't happen?
    TEST_ASSERT(0);
}

static const fluid_file_callbacks_t fcb =
{
    NULL, &test_reader, &test_seek, NULL, NULL
};

static SFZone* new_test_zone(fluid_list_t** parent_list, int gen_count)
{
    int i;
    SFZone *zone = FLUID_NEW(SFZone);
    TEST_ASSERT(zone != NULL);
    FLUID_MEMSET(zone, 0, sizeof(*zone));
    
    for (i = 0; i < gen_count; i++)
    {
        zone->gen = fluid_list_prepend(zone->gen, NULL);
    }
    
    if(parent_list != NULL)
    {
        *parent_list = fluid_list_append(*parent_list, zone);
    }
    
    return zone;
}

// test the good case first: one zone, with two generators and one terminal generator
static void good_test_1zone_2gen_1termgen(int (*load_func)(SFData *sf, int size), SFData* sf, SFZone *zone)
{
    static const unsigned char buf[] =
    {
        Gen_KeyRange, 0, 60, 127, Gen_VelRange, 0, 60, 127, 0, 0, 0, 0
    };
    file_buf = buf;
    TEST_ASSERT(load_func(sf, sizeof(buf) / sizeof(*buf)));

    SFGen *gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_KeyRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_VelRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    file_buf = NULL;
}

// bad case: too few generators in buffer, triggering a chunk size mismatch
static void bad_test_too_short_gen_buffer(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    static const unsigned char buf[] = { Gen_VelRange, 0, 0 };
    file_buf = buf;
    TEST_ASSERT(load_func(sf, sizeof(buf) / sizeof(*buf)) == FALSE);
    SFGen *gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen == NULL);
    TEST_ASSERT(file_buf == buf);

    static const unsigned char buf2[] = { Gen_KeyRange, 0, 60, 127, Gen_OverrideRootKey };
    file_buf = buf2;
    TEST_ASSERT(load_func(sf, sizeof(buf2) / sizeof(*buf2)) == FALSE);
    gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_KeyRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    TEST_ASSERT(file_buf == buf2 + sizeof(buf2) - 1);
    file_buf = NULL;
}

// bad case: one zone, with two similar generators
static void bad_test_duplicate_gen(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    static const unsigned char buf[] = { Gen_VelRange, 0, 60, 127, Gen_VelRange, 0, 60, 127 };
    file_buf = buf;
    TEST_ASSERT(load_func(sf, sizeof(buf) / sizeof(*buf)));

    SFGen *gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_VelRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    file_buf = NULL;
}

// bad case: with one zone, generators in wrong order
static void bad_test_gen_wrong_order(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone)
{
    static const unsigned char buf[] =
    {
        Gen_VelRange, 0, 60, 127,
        Gen_KeyRange, 0, 60, 127,
        Gen_Instrument, 0, 0xDD, 0xDD
    };
    file_buf = buf;
    TEST_ASSERT(load_func(sf, sizeof(buf) / sizeof(*buf)));

    SFGen *gen = fluid_list_get(fluid_list_nth(zone->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_VelRange);
    TEST_ASSERT(gen->amount.range.lo == 60);
    TEST_ASSERT(gen->amount.range.hi == 127);

    // The INSTRUMENT generator is mistakenly accepted by load_igen. This will be fixed by Marcus' PR.
    // Once merge, this if clause should be removed.
    if (load_func != load_igen)
    {
        gen = fluid_list_get(fluid_list_nth(zone->gen, 1));
        TEST_ASSERT(gen == NULL);
    }
   
    gen = fluid_list_get(fluid_list_nth(zone->gen, 2));
    TEST_ASSERT(gen == NULL);

    if (load_func == load_pgen)
    {
        TEST_ASSERT(FLUID_POINTER_TO_UINT(zone->instsamp) == 0xDDDD + 1);
        zone->instsamp = NULL;
    }

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    file_buf = NULL;
}

// This test-case is derived from the invalid SoundFont provided in #808
static void bad_test_issue_808(int (*load_func)(SFData *sf, int size), SFData *sf, SFZone *zone1, SFZone *zone2)
{
    static const unsigned char buf[] =
    {
        // zone 1
        Gen_ReverbSend, 0, 50, 0,
        Gen_VolEnvRelease, 0, 206, 249,
        // zone 2
        Gen_KeyRange, 0, 0, 35,
        Gen_OverrideRootKey, 0, 43, 0,
        Gen_StartAddrCoarseOfs, 0, 0, 0,
        Gen_SampleModes, 0, 1, 0,
        Gen_StartAddrOfs, 0, 0, 0
    };

    file_buf = buf;
    TEST_ASSERT(load_func(sf, sizeof(buf) / sizeof(*buf)));

    SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_ReverbSend);
    TEST_ASSERT(gen->amount.range.lo == 50);
    TEST_ASSERT(gen->amount.range.hi == 0);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
    TEST_ASSERT(gen != NULL);
    TEST_ASSERT(gen->id == Gen_VolEnvRelease);
    TEST_ASSERT(gen->amount.range.lo == 206);
    TEST_ASSERT(gen->amount.range.hi == 249);

    gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
    TEST_ASSERT(gen == NULL);

    TEST_ASSERT(file_buf == buf + sizeof(buf));
    file_buf = NULL;
}

int main(void)
{
    // prepare a soundfont that has one preset and one instrument, with up to 2 zones

    SFZone *zone1, *zone2;
    SFData *sf = FLUID_NEW(SFData);
    SFPreset *preset = FLUID_NEW(SFPreset);
    SFInst *inst = FLUID_NEW(SFInst);

    TEST_ASSERT(sf != NULL);
    FLUID_MEMSET(sf, 0, sizeof(*sf));
    TEST_ASSERT(preset != NULL);
    FLUID_MEMSET(preset, 0, sizeof(*preset));
    TEST_ASSERT(inst != NULL);
    FLUID_MEMSET(inst, 0, sizeof(*inst));

    sf->fcbs = &fcb;
    sf->preset = fluid_list_append(sf->preset, preset);
    sf->inst = fluid_list_append(sf->inst, inst);

    // Calls the given test function for 1 zone once for preset and once for inst case.
    #define TEST_CASE_1(TEST_FUNC, GEN_COUNT)            \
    do                                                   \
    {                                                    \
        zone1 = new_test_zone(&preset->zone, GEN_COUNT); \
        TEST_FUNC(&load_pgen, sf, zone1);                \
        delete_zone(zone1);                              \
        delete_fluid_list(preset->zone);                 \
        preset->zone = NULL;                             \
                                                         \
        zone1 = new_test_zone(&inst->zone, GEN_COUNT);   \
        TEST_FUNC(&load_igen, sf, zone1);                \
        delete_zone(zone1);                              \
        delete_fluid_list(inst->zone);                   \
        inst->zone = NULL;                               \
    } while (0)

    TEST_CASE_1(good_test_1zone_2gen_1termgen, 2);
    TEST_CASE_1(good_test_1zone_2gen_1termgen, 3);

    TEST_CASE_1(bad_test_too_short_gen_buffer, 1);

    TEST_CASE_1(bad_test_duplicate_gen, 1);
    TEST_CASE_1(bad_test_duplicate_gen, 2);

    TEST_CASE_1(bad_test_gen_wrong_order, 3);

    zone1 = new_test_zone(&preset->zone, 2);
    zone2 = new_test_zone(&preset->zone, 5);
    bad_test_issue_808(&load_pgen, sf, zone1, zone2);
    // zone 2 was dropped
    TEST_ASSERT(preset->zone->next == NULL);
    delete_zone(zone1);
    // zone2 already deleted
    zone2 = NULL;
    delete_fluid_list(preset->zone);
    preset->zone = NULL;

    zone1 = new_test_zone(&inst->zone, 2);
    zone2 = new_test_zone(&inst->zone, 5);
    bad_test_issue_808(&load_igen, sf, zone1, zone2);
    // zone 2 was dropped
    TEST_ASSERT(inst->zone->next == NULL);
    delete_zone(zone1);
    // zone2 already deleted
    zone2 = NULL;
    delete_fluid_list(inst->zone);
    inst->zone = NULL;

    delete_inst(inst);
    delete_preset(preset);
    FLUID_FREE(sf);
    return EXIT_SUCCESS;
}
