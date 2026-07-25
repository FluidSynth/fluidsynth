#include "test.h"
#include "utils/fluid_sys.h"

/*
 * Unit tests for fluid_shell_parse_argv_internal().
 *
 * Test cases were taken from test/command-lines.txt
 *
 * Parsing rules summary:
 *   - Outside quotes: backslash escapes the next character (any char except \n);
 *     '#' at the start of a token begins a comment that swallows the rest of the line.
 *   - Single quotes: every character is literal; no escape processing at all.
 *     A single apostrophe always ends the single-quote region.
 *   - Double quotes: only \", \\, \`, \$, and \<newline> are recognised escapes;
 *     any other \X keeps both the backslash and X.
 *   - An unterminated quote or a trailing backslash causes a parse error (FALSE).
 *   - An empty input or a line that is entirely a comment returns FALSE (no tokens).
 */

static void check(const char *line, int expect_success, int expected_argc, const char **expected_argv)
{
    int argc = 0;
    char **argv = NULL;
    int result;
    int i;

    FLUID_LOG(FLUID_INFO, "parsing: %s", line);
    result = fluid_shell_parse_argv_internal(line, &argc, &argv);

    FLUID_LOG(FLUID_INFO, "result: expected=%d actual=%d", !!expect_success, !!result);
    TEST_ASSERT(!!result == !!expect_success);
    if (!expect_success)
        return;

    FLUID_LOG(FLUID_INFO, "argc: expected=%d actual=%d", expected_argc, argc);
    TEST_ASSERT(argc == expected_argc);

    for (i = 0; i < expected_argc; i++)
    {
        FLUID_LOG(FLUID_INFO, "arg[%d]: expected=\"%s\" actual=\"%s\"", i, expected_argv[i], argv[i]);
        TEST_ASSERT(strcmp(argv[i], expected_argv[i]) == 0);
    }

    fluid_strfreev_internal(argv);
}

static void test_command_lines(void)
{
    {
        static const char *e[] = {"help", NULL};
        check("help", 1, 1, e);
    }

    {
        static const char *e[] = {"set", "synth.reverb.room-size", "1", NULL};
        check("set synth.reverb.room-size 1", 1, 3, e);
    }

    {
        static const char *e[] = {"load", "soundfont.sf2", "0", "10", NULL};
        check("load soundfont.sf2 0 10", 1, 4, e);
    }

    {
        static const char *e[] = {"load", "test file name with single quotes", "0", NULL};
        check("load 'test file name with single quotes' 0", 1, 3, e);
    }

    {
        static const char *e[] = {"load", "test file name with double quotes", "1", NULL};
        check("load \"test file name with double quotes\" 1", 1, 3, e);
    }

    /* load 'test unterminated single quote  ->  error: unclosed single quote */
    check("load 'test unterminated single quote", 0, 0, NULL);

    /* load "test \"escaped quotes\"   inside quotes"
     * Inside double quotes, \" is a recognised escape that drops the backslash.
     * Token: test "escaped quotes"   inside quotes */
    {
        static const char *e[] = {"load", "test \"escaped quotes\"   inside quotes", NULL};
        check("load \"test \\\"escaped quotes\\\"   inside quotes\"", 1, 2, e);
    }

    /* load 'test \' impossible embedded single quote
     * Inside single quotes, backslash is always literal.  The apostrophe after
     * the backslash ends the single-quote region, giving the token "test \".
     * Tokens: load, test \, impossible, embedded, single, quote */
    {
        static const char *e[] = {"load", "test \\", "impossible", "embedded", "single", "quote", NULL};
        check("load 'test \\' impossible embedded single quote", 1, 6, e);
    }

    /* load 'test \' unterminated non-escaped'  ->  error: trailing ' opens an
     * unclosed single-quote region */
    check("load 'test \\' unterminated non-escaped'", 0, 0, NULL);

    /* comment#inside: '#' mid-token is a literal character */
    {
        static const char *e[] = {"comment#inside", NULL};
        check("comment#inside", 1, 1, e);
    }

    /* comment #outside: '#' at the start of a new token begins a comment;
     * only the first token is produced */
    {
        static const char *e[] = {"comment", NULL};
        check("comment #outside", 1, 1, e);
    }

    /* use<TAB>tabs<TAB>also: tabs are whitespace separators */
    {
        static const char *e[] = {"use", "tabs", "also", NULL};
        check("use\ttabs\talso", 1, 3, e);
    }

    /* escape\ space: backslash-space is an escaped space; the two words merge
     * into a single token */
    {
        static const char *e[] = {"escape space", NULL};
        check("escape\\ space", 1, 1, e);
    }

    /* escape \normal ch\ars: backslash before an ordinary letter is dropped */
    {
        static const char *e[] = {"escape", "normal", "chars", NULL};
        check("escape \\normal ch\\ars", 1, 3, e);
    }

    /* escape \\backslash: \\ produces a single literal backslash */
    {
        static const char *e[] = {"escape", "\\backslash", NULL};
        check("escape \\\\backslash", 1, 2, e);
    }

    /* escape \' single quote: \' outside quotes produces a literal apostrophe */
    {
        static const char *e[] = {"escape", "'", "single", "quote", NULL};
        check("escape \\' single quote", 1, 4, e);
    }

    /* escape \" double quote: \" outside quotes produces a literal double-quote */
    {
        static const char *e[] = {"escape", "\"", "double", "quote", NULL};
        check("escape \\\" double quote", 1, 4, e);
    }

    /* escape \\\ too: \\ gives one backslash, then \<space> gives an escaped
     * space; all three combine with "too" into the single token "\ too" */
    {
        static const char *e[] = {"escape", "\\ too", NULL};
        check("escape \\\\\\ too", 1, 2, e);
    }

    /* escape \<TAB>a tab: backslash-tab produces a literal tab character */
    {
        static const char *e[] = {"escape", "\ta", "tab", NULL};
        check("escape \\\ta tab", 1, 3, e);
    }

    /* escape "normal \chars and \\$ \\\$ special double quotes"
     * Inside double quotes:
     *   \c  (c not in special set) -> \c  (backslash preserved)
     *   \\$ -> \$ (backslash consumed, dollar appended as-is)
     *   \\\$-> \$ (same net result as \\$) */
    {
        static const char *e[] = {"escape", "normal \\chars and \\$ \\$ special double quotes", NULL};
        check("escape \"normal \\chars and \\\\$ \\\\\\$ special double quotes\"", 1, 2, e);
    }

    /* "embed "double quotes" and \'single quotes\' and \" \'nested\'"
     * The alternating bare double-quotes toggle in/out of double-quote mode,
     * merging everything into one token.  Inside double quotes: \' (not special)
     * keeps both chars; \" (special) drops the backslash, leaving just ". */
    {
        static const char *e[] = {"embed double", "quotes and \\'single quotes\\' and \" \\'nested\\'", NULL};
        check("\"embed \"double quotes\" and \\'single quotes\\' and \\\" \\'nested\\'\"", 1, 2, e);
    }

    /* trailing escape \  ->  error: backslash at end of input */
    check("trailing escape \\", 0, 0, NULL);

    /* empty string  ->  no tokens */
    check("", 0, 0, NULL);

    /* # comment only  ->  entire line is a comment, no tokens */
    check("# comment only", 0, 0, NULL);
}

/* Parse one line without asserting specific output; used for fuzz/robustness testing. */
static void fuzz_line(const char *line)
{
    int argc;
    char **argv;

    if (fluid_shell_parse_argv_internal(line, &argc, &argv))
        fluid_strfreev_internal(argv);
}

/* Read a file and pass each line to fuzz_line() to verify that the parser
 * does not crash or leak on arbitrary input. */
static void fuzz_file(const char *name)
{
    FILE *file;
    char *buffer;
    const int max_length = 100000;
    int read_count;
    char *line;
    char *search;
    char *end;

    file = FLUID_FOPEN(name, "rb");
    TEST_ASSERT(file != NULL);

    buffer = (char *)FLUID_MALLOC(max_length + 1);
    TEST_ASSERT(buffer != NULL);

    read_count = (int)FLUID_FREAD(buffer, 1, max_length, file);
    TEST_ASSERT(FLUID_FCLOSE(file) == 0);
    TEST_ASSERT(read_count > 0);

    buffer[read_count] = '\0';
    line = buffer;
    end = buffer + read_count;

    for (search = buffer; search <= end; search++)
    {
        if (*search == '\n' || *search == '\0')
        {
            *search = '\0';
            fuzz_line(line);
            line = search + 1;
        }
    }

    FLUID_FREE(buffer);
}

int main(void)
{
    test_command_lines();
    fuzz_file(TEST_MIDI_UTF8); /* throw some binary data at the parser as well */
    return EXIT_SUCCESS;
}
