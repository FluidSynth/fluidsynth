#include "test.h"
#include "utils/fluid_sys.h"

#include <glib.h>


// test for fluid_shell_parse_argv_internal() vs. g_shell_parse_argv()

static void parse_line(const char *line)
{
    int i;

    int glib_argc;
    char **glib_argv;
    int glib_result;

    int internal_argc;
    char **internal_argv;
    int internal_result;

    FLUID_LOG(FLUID_INFO, "parsing: %s", line);

    glib_result = g_shell_parse_argv(line, &glib_argc, &glib_argv, NULL);
    internal_result = fluid_shell_parse_argv_internal(line, &internal_argc, &internal_argv);

    FLUID_LOG(FLUID_INFO, "result %d vs %d", glib_result, internal_result);
    TEST_ASSERT(glib_result == internal_result);
    if (!glib_result)
        return;

    FLUID_LOG(FLUID_INFO, "parsed %d vs %d", glib_argc, internal_argc);
    TEST_ASSERT(glib_argc == internal_argc);

    for (i = 0; i < glib_argc; i++)
    {
        FLUID_LOG(FLUID_INFO, "glib: \"%s\"", glib_argv[i]);
        FLUID_LOG(FLUID_INFO, "intr: \"%s\"", internal_argv[i]);

        TEST_ASSERT(strcmp(glib_argv[i], internal_argv[i]) == 0);
    }

    g_strfreev(glib_argv);
    fluid_strfreev_internal(internal_argv);
}

static void parse_file(const char *name)
{
    FILE *file;
    char *buffer;
    int max_length = 100000;
    int read;
    const char *line;
    char *search;

    file = FLUID_FOPEN(name, "rb");
    TEST_ASSERT(file != NULL);

    buffer = (char *)FLUID_MALLOC(max_length);
    TEST_ASSERT(buffer != NULL);

    read = FLUID_FREAD(buffer, 1, max_length, file);
    TEST_ASSERT(read > 0 && read <= max_length);

    buffer[read] = 0;
    line = buffer;
    search = buffer;
    do
    {
        if (*search == '\n' || *search == 0)
        {
            *search = 0;
            parse_line(line);
            line = search + 1;
        }

        search++;
        read--;
    }
    while (read > 0);

    FLUID_FREE(buffer);
}

int main(void)
{
    parse_file(TEST_COMMAND_LINES);
    parse_file(TEST_MIDI_UTF8); /* throw some binary data at it as well */
    return EXIT_SUCCESS;
}
