#include <stdint.h>

#include "test.h"
#include "fluidsynth.h"
#include "midi/fluid_midi.h"

/* Minimal valid MIDI file: one track with a valid tempo and end-of-track. */
static const unsigned char test_midi[] =
{
    'M', 'T', 'h', 'd', 0, 0, 0, 6, 0, 0, 0, 1, 0, 96,
    'M', 'T', 'r', 'k', 0, 0, 0, 11,
    0, 0xFF, 0x51, 3, 0x07, 0xA1, 0x20,
    0, 0xFF, 0x2F, 0
};

static void write_byte(FILE *file, unsigned char value)
{
    TEST_ASSERT(fwrite(&value, 1, 1, file) == 1);
}

static void write_u16(FILE *file, uint16_t value)
{
    write_byte(file, (unsigned char)(value & 0xFF));
    write_byte(file, (unsigned char)(value >> 8));
}

static void write_u32(FILE *file, uint32_t value)
{
    write_byte(file, (unsigned char)(value & 0xFF));
    write_byte(file, (unsigned char)((value >> 8) & 0xFF));
    write_byte(file, (unsigned char)((value >> 16) & 0xFF));
    write_byte(file, (unsigned char)((value >> 24) & 0xFF));
}

static void write_id(FILE *file, const char *id)
{
    TEST_ASSERT(fwrite(id, 1, 4, file) == 4);
}

static void write_list(FILE *file, int bank_offset, uint32_t dbnk_size, uint32_t info_size)
{
    uint32_t i;

    write_id(file, "LIST");
    write_u32(file, 4 + info_size);
    write_id(file, "INFO");
    write_id(file, "DBNK");
    write_u32(file, dbnk_size);
    write_u16(file, (uint16_t)bank_offset);

    for(i = 2; i < dbnk_size; i++)
    {
        write_byte(file, 0);
    }
}

/*
 * Write an RMIDI file containing the given MIDI and, optionally, the given
 * sound bank. The bank must be a complete RIFF file (RIFF header included).
 * dbnk_size is the payload size of the DBNK chunk, 0 omits the chunk. Anything
 * other than 2 makes it invalid. If list_after_bank is set, the LIST chunk is
 * written after the bank.
 */
static void write_rmidi(const char *path,
                        const unsigned char *midi, size_t midi_size,
                        const unsigned char *bank, size_t bank_size,
                        int bank_offset, uint32_t dbnk_size, int list_after_bank)
{
    FILE *file;
    uint32_t data_size, info_size, list_size, riff_size;

    data_size = (uint32_t)midi_size + (uint32_t)(midi_size % 2);
    info_size = dbnk_size > 0 ? 8 + dbnk_size : 0; /* DBNK chunk: id + size + payload */
    list_size = dbnk_size > 0 ? 8 + 4 + info_size : 0;
    riff_size = 4 + 8 + data_size + list_size + (uint32_t)bank_size;

    file = fopen(path, "wb");
    TEST_ASSERT(file != NULL);

    write_id(file, "RIFF");
    write_u32(file, riff_size);
    write_id(file, "RMID");

    write_id(file, "data");
    write_u32(file, (uint32_t)midi_size);
    TEST_ASSERT(fwrite(midi, 1, midi_size, file) == midi_size);

    if(midi_size % 2)
    {
        write_byte(file, 0);
    }

    if(dbnk_size > 0 && !list_after_bank)
    {
        write_list(file, bank_offset, dbnk_size, info_size);
    }

    if(bank_size > 0)
    {
        TEST_ASSERT(fwrite(bank, 1, bank_size, file) == bank_size);
    }

    if(dbnk_size > 0 && list_after_bank)
    {
        write_list(file, bank_offset, dbnk_size, info_size);
    }

    TEST_ASSERT(fclose(file) == 0);
}

static unsigned char *read_file(const char *path, size_t *size)
{
    FILE *file;
    long length;
    unsigned char *data;

    file = fopen(path, "rb");
    TEST_ASSERT(file != NULL);
    TEST_ASSERT(fseek(file, 0, SEEK_END) == 0);
    length = ftell(file);
    TEST_ASSERT(length > 0);
    TEST_ASSERT(fseek(file, 0, SEEK_SET) == 0);

    data = (unsigned char *)malloc((size_t)length);
    TEST_ASSERT(data != NULL);
    TEST_ASSERT(fread(data, 1, (size_t)length, file) == (size_t)length);
    TEST_ASSERT(fclose(file) == 0);

    *size = (size_t)length;
    return data;
}

static int has_preset(fluid_sfont_t *sfont, int bank, int num)
{
    fluid_preset_t *preset = fluid_sfont_get_preset(sfont, bank, num);

    if(preset == NULL)
    {
        return FALSE;
    }

    return fluid_preset_get_banknum(preset) == bank &&
           fluid_preset_get_num(preset) == num;
}

static int sfont_has_bank(fluid_sfont_t *sfont, int bank)
{
    fluid_preset_t *preset;

    fluid_sfont_iteration_start(sfont);

    while((preset = fluid_sfont_iteration_next(sfont)) != NULL)
    {
        if(fluid_preset_get_banknum(preset) == bank)
        {
            return TRUE;
        }
    }

    return FALSE;
}

static void test_player_load(const char *path)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_player_t *player;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);
    player = new_fluid_player(synth);
    TEST_ASSERT(player != NULL);

    TEST_SUCCESS(fluid_player_add(player, path));
    TEST_SUCCESS(fluid_player_play(player));
    TEST_SUCCESS(fluid_synth_process(synth, 64, 0, NULL, 0, NULL));
    TEST_ASSERT(player->ntracks == 1);

    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

static void test_sf2_bank_offset(const unsigned char *bank, size_t bank_size, const char *path)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    int id;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    write_rmidi(path, test_midi, sizeof(test_midi), bank, bank_size, 3, 2, FALSE);

    TEST_ASSERT(fluid_is_soundfont(path) == TRUE);
    TEST_ASSERT(fluid_is_midifile(path) == TRUE);

    TEST_SUCCESS(id = fluid_synth_sfload(synth, path, 1));
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);

    /* Bank 0 presets are shifted by the RMIDI bank offset. */
    TEST_ASSERT(!has_preset(sfont, 0, 0));
    TEST_ASSERT(has_preset(sfont, 3, 0));
    /* With the default GS bank select, the offset is added to the drum bank select MSB. */
    TEST_ASSERT(!has_preset(sfont, 128, 0));
    TEST_ASSERT(has_preset(sfont, 131, 0));

    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    test_player_load(path);
}

static void test_sf2_default_offset(const unsigned char *bank, size_t bank_size, const char *path)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    int id;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* Without a DBNK chunk, the RMIDI default offset of 1 applies. */
    write_rmidi(path, test_midi, sizeof(test_midi), bank, bank_size, 0, 0, FALSE);

    TEST_ASSERT(fluid_is_soundfont(path) == TRUE);
    TEST_SUCCESS(id = fluid_synth_sfload(synth, path, 1));
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);

    TEST_ASSERT(!has_preset(sfont, 0, 0));
    TEST_ASSERT(has_preset(sfont, 1, 0));
    TEST_ASSERT(has_preset(sfont, 129, 0));

    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

/*
 * Load an RMIDI file containing the SF2 bank and check that its bank 0 preset
 * ends up in the expected bank.
 */
static void check_sf2_offset(const unsigned char *bank, size_t bank_size, const char *path,
                             int bank_offset, uint32_t dbnk_size, int list_after_bank,
                             int expected_bank, int expected_drum_bank)
{
    fluid_settings_t *settings = new_fluid_settings();
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    int id;

    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    write_rmidi(path, test_midi, sizeof(test_midi), bank, bank_size,
                bank_offset, dbnk_size, list_after_bank);

    TEST_SUCCESS(id = fluid_synth_sfload(synth, path, 1));
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);

    TEST_ASSERT(has_preset(sfont, expected_bank, 0));
    TEST_ASSERT(has_preset(sfont, expected_drum_bank, 0));

    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
}

static void test_dbnk_handling(const unsigned char *bank, size_t bank_size, const char *path)
{
    /* The LIST chunk may come after the bank. */
    check_sf2_offset(bank, bank_size, path, 3, 2, TRUE, 3, 131);
    /* An invalid DBNK chunk is ignored, so the default offset of 1 applies. */
    check_sf2_offset(bank, bank_size, path, 3, 4, FALSE, 1, 129);
    check_sf2_offset(bank, bank_size, path, 128, 2, FALSE, 1, 129);
    check_sf2_offset(bank, bank_size, path, 128, 2, TRUE, 1, 129);
}

/* The drum kit bank depends on how the bank select style maps the MSB. */
static void test_sf2_drum_banks(const unsigned char *bank, size_t bank_size, const char *path)
{
    static const struct
    {
        const char *style;
        int drum_bank;
    } styles[] =
    {
        { "gs", 131 },   /* 128 + MSB */
        { "mma", 384 },  /* MSB * 128 */
        { "gm", 128 },   /* the MSB can't select a drum kit */
        { "gm2", 128 },
        { "xg", 128 }
    };
    size_t i;

    for(i = 0; i < sizeof(styles) / sizeof(styles[0]); i++)
    {
        fluid_settings_t *settings = new_fluid_settings();
        fluid_synth_t *synth;
        fluid_sfont_t *sfont;
        int id;

        TEST_ASSERT(settings != NULL);
        TEST_SUCCESS(fluid_settings_setstr(settings, "synth.midi-bank-select", styles[i].style));
        synth = new_fluid_synth(settings);
        TEST_ASSERT(synth != NULL);

        write_rmidi(path, test_midi, sizeof(test_midi), bank, bank_size, 3, 2, FALSE);

        TEST_SUCCESS(id = fluid_synth_sfload(synth, path, 1));
        sfont = fluid_synth_get_sfont_by_id(synth, id);
        TEST_ASSERT(sfont != NULL);
        TEST_ASSERT(has_preset(sfont, styles[i].drum_bank, 0));

        TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
        delete_fluid_synth(synth);
        delete_fluid_settings(settings);
    }
}

static void test_no_bank(const char *path)
{
    /* A DBNK chunk must be ignored when no embedded bank is present. */
    write_rmidi(path, test_midi, sizeof(test_midi), NULL, 0, 7, 2, FALSE);

    TEST_ASSERT(fluid_is_soundfont(path) == FALSE);
    TEST_ASSERT(fluid_is_midifile(path) == TRUE);

    test_player_load(path);
}

#ifdef ENABLE_NATIVE_DLS
static void test_dls_bank_offset(const char *path)
{
    unsigned char *bank;
    size_t bank_size;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_sfont_t *sfont;
    int id;

    bank = read_file(TEST_DLS, &bank_size);

    settings = new_fluid_settings();
    TEST_ASSERT(settings != NULL);
    synth = new_fluid_synth(settings);
    TEST_ASSERT(synth != NULL);

    /* A plain DLS bank is not shifted. */
    TEST_SUCCESS(id = fluid_synth_sfload(synth, TEST_DLS, 1));
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);
    TEST_ASSERT(has_preset(sfont, 0, 0));
    TEST_ASSERT(!has_preset(sfont, 1, 0));
    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));

    /*
     * DLS RMIDI files have no DBNK chunk; a bank offset of 1 is assumed.
     * The embedded DLS bank is shifted just like an SF2 bank.
     */
    write_rmidi(path, test_midi, sizeof(test_midi), bank, bank_size, 0, 0, FALSE);

    TEST_ASSERT(fluid_is_soundfont(path) == TRUE);
    TEST_ASSERT(fluid_is_midifile(path) == TRUE);

    TEST_SUCCESS(id = fluid_synth_sfload(synth, path, 1));
    sfont = fluid_synth_get_sfont_by_id(synth, id);
    TEST_ASSERT(sfont != NULL);
    TEST_ASSERT(sfont_has_bank(sfont, 1));
    TEST_ASSERT(has_preset(sfont, 1, 0));
    TEST_ASSERT(!has_preset(sfont, 0, 0));

    TEST_SUCCESS(fluid_synth_sfunload(synth, id, 0));
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    free(bank);
    test_player_load(path);
}
#endif

int main(void)
{
    unsigned char *bank;
    size_t bank_size;
    const char *path = "test_rmidi.rmi";

    TEST_DISABLE_STDOUT_BUFFERING();

    bank = read_file(TEST_SOUNDFONT, &bank_size);

    test_sf2_bank_offset(bank, bank_size, path);
    test_sf2_default_offset(bank, bank_size, path);
    test_dbnk_handling(bank, bank_size, path);
    test_sf2_drum_banks(bank, bank_size, path);
    test_no_bank(path);
#ifdef ENABLE_NATIVE_DLS
    test_dls_bank_offset(path);
#endif

    free(bank);
    remove(path);

    return EXIT_SUCCESS;
}
