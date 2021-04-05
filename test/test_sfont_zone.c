
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

int main(void)
{
    // prepare a soundfont that has one preset and one instrument, with up to 3 zones each

    SFData *sf = FLUID_NEW(SFData);
    SFPreset *preset = FLUID_NEW(SFPreset);
    SFInst *inst = FLUID_NEW(SFInst);
    SFZone *zone1 = FLUID_NEW(SFZone);
    SFZone *zone2 = FLUID_NEW(SFZone);
    SFZone *zone3 = FLUID_NEW(SFZone);

    TEST_ASSERT(sf != NULL);
    FLUID_MEMSET(sf, 0, sizeof(*sf));
    TEST_ASSERT(preset != NULL);
    FLUID_MEMSET(preset, 0, sizeof(*preset));
    TEST_ASSERT(inst != NULL);
    FLUID_MEMSET(inst, 0, sizeof(*inst));
    TEST_ASSERT(zone1 != NULL);
    FLUID_MEMSET(zone1, 0, sizeof(*zone1));
    TEST_ASSERT(zone2 != NULL);
    FLUID_MEMSET(zone2, 0, sizeof(*zone2));
    TEST_ASSERT(zone3 != NULL);
    FLUID_MEMSET(zone3, 0, sizeof(*zone3));

    sf->fcbs = &fcb;
    sf->preset = fluid_list_append(sf->preset, preset);
    sf->inst = fluid_list_append(sf->inst, inst);


    preset->zone = fluid_list_prepend(preset->zone, zone1);

    // test the good case first: one preset, with one zone, with two generators and one terminal generator
    {
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);

        unsigned char buf[] = { Gen_KeyRange, 0, 60, 127, Gen_VelRange, 0, 60, 127, 0,0,0,0 };
        file_buf = buf;
        TEST_ASSERT(load_pgen(sf, sizeof(buf) / sizeof(*buf)));

        SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen->id == Gen_KeyRange);
        TEST_ASSERT(gen->amount.range.lo == 60);
        TEST_ASSERT(gen->amount.range.hi == 127);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
        TEST_ASSERT(gen->id == Gen_VelRange);
        TEST_ASSERT(gen->amount.range.lo == 60);
        TEST_ASSERT(gen->amount.range.hi == 127);

        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
    }
    
    // bad case: too few generators in buffer, trigger an IGEN chunk size mismatch
    {
        inst->zone = fluid_list_prepend(inst->zone, zone1);
        
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);

        unsigned char buf[] = { Gen_VelRange, 0, 0 };
        file_buf = buf;
        TEST_ASSERT(load_pgen(sf, sizeof(buf) / sizeof(*buf)) == FALSE);
        SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen == NULL);
        
        file_buf = buf;
        TEST_ASSERT(load_igen(sf, sizeof(buf) / sizeof(*buf)) == FALSE);
        gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen == NULL);

        unsigned char buf2[] = { Gen_KeyRange, 0, 60, 127, Gen_OverrideRootKey };
        file_buf = buf2;
        TEST_ASSERT(load_pgen(sf, sizeof(buf2) / sizeof(*buf2)) == FALSE);
        gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen != NULL);
        
        file_buf = buf2;
        TEST_ASSERT(load_igen(sf, sizeof(buf2) / sizeof(*buf2)) == FALSE);
        gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen != NULL);

        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
        
        delete_fluid_list(inst->zone);
        inst->zone = NULL;
    }

    // bad case: one preset, with one zone, with two similar generators
    {
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);

        unsigned char buf[] = { Gen_VelRange, 0, 60, 127, Gen_VelRange, 0, 60, 127 };
        file_buf = buf;
        TEST_ASSERT(load_pgen(sf, sizeof(buf) / sizeof(*buf)));

        SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen->id == Gen_VelRange);
        TEST_ASSERT(gen->amount.range.lo == 60);
        TEST_ASSERT(gen->amount.range.hi == 127);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
        TEST_ASSERT(gen == NULL);

        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
    }

    // bad case: one preset, with one zone, generators in wrong order
    {
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);

        unsigned char buf[] = { Gen_VelRange, 0, 60, 127, Gen_KeyRange, 0, 60, 127, Gen_Instrument, 0, 0xDD, 0xDD };
        file_buf = buf;
        TEST_ASSERT(load_pgen(sf, sizeof(buf) / sizeof(*buf)));

        SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen->id == Gen_VelRange);
        TEST_ASSERT(gen->amount.range.lo == 60);
        TEST_ASSERT(gen->amount.range.hi == 127);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
        TEST_ASSERT(gen == NULL);
        gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
        TEST_ASSERT(gen == NULL);

        TEST_ASSERT(FLUID_POINTER_TO_UINT(zone1->instsamp) == 0xDDDD + 1);
        zone1->instsamp = NULL;

        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
    }

    // This test-case is derived from the invalid SoundFont provided in #808
    {
        inst->zone = fluid_list_prepend(inst->zone, zone2);
        inst->zone = fluid_list_prepend(inst->zone, zone1);

        zone1->gen = fluid_list_prepend(zone1->gen, NULL);
        zone1->gen = fluid_list_prepend(zone1->gen, NULL);

        zone2->gen = fluid_list_prepend(zone2->gen, NULL);
        zone2->gen = fluid_list_prepend(zone2->gen, NULL);
        zone2->gen = fluid_list_prepend(zone2->gen, NULL);
        zone2->gen = fluid_list_prepend(zone2->gen, NULL);
        zone2->gen = fluid_list_prepend(zone2->gen, NULL);

        unsigned char buf[] =
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
        TEST_ASSERT(load_igen(sf, sizeof(buf) / sizeof(*buf)));

        SFGen *gen = fluid_list_get(fluid_list_nth(zone1->gen, 0));
        TEST_ASSERT(gen->id == Gen_ReverbSend);
        TEST_ASSERT(gen->amount.range.lo == 50);
        TEST_ASSERT(gen->amount.range.hi == 0);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 1));
        TEST_ASSERT(gen->id == Gen_VolEnvRelease);
        TEST_ASSERT(gen->amount.range.lo == 206);
        TEST_ASSERT(gen->amount.range.hi == 249);

        gen = fluid_list_get(fluid_list_nth(zone1->gen, 2));
        TEST_ASSERT(gen == NULL);

        // zone 2 was dropped
        TEST_ASSERT(inst->zone->next == NULL);

        delete_fluid_list(zone1->gen);
        zone1->gen = NULL;
    }
    
    file_buf = NULL;

    return EXIT_SUCCESS;
}

static SFZone* new_test_zone(fluid_list_t** parent_list, int gen_count)
{
    int i;
    SFZone *zone = FLUID_NEW(SFZone);
    TEST_ASSERT(zone != NULL);
    
    for(i=0; i<gen_count; i++)
    {
        zone->gen = fluid_list_prepend(zone->gen, NULL);
    }
    
    if(parent_list != NULL)
    {
        *parent_list = fluid_list_append(*parent_list, zone);
    }
    
    return zone;
}
