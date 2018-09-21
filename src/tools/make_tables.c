#include "fluid_sys.h"
#include "make_tables.h"

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const fluid_real_t *tbl, int size)
{
    int i;

    fprintf(fp, "static const fluid_real_t %s[] = {\n", tblname);

    for (i = 0; i < size; i++)
    {
        fprintf(fp, "    %a%c\n",
            tbl[i],
            (i < (size-1)) ? ',' : ' ');
    }
    fprintf(fp, "};\n\n");
}

/* Emit a matrix of real numbers */
void emit_matrix(FILE *fp, const char *tblname, emit_matrix_cb tbl_cb, int sizeh, int sizel)
{
    int i, j;

    fprintf(fp, "static const fluid_real_t %s[%d][%d] = {\n    {\n", tblname, sizeh, sizel);

    for (i = 0; i < sizeh; i++)
    {
        for (j = 0; j < sizel; j++)
        {
            fprintf(fp, "        %a%c\n",
                tbl_cb(i, j), 
                (j < (sizel-1)) ? ',' : ' ');
        }


        if (i < (sizeh-1))
            fprintf(fp, "    }, {\n");
        else
            fprintf(fp, "    }\n};\n\n");
    }
}

int main (int argc, char *argv[])
{
    FILE *fp;
    int   res, function;
 
    // make sure we have enough arguments
    if (argc < 3)
        return 1;

    /* open the output file */
    fp = fopen(argv[2], "w");
    if (!fp)
        return 2;

    /* Emit warning header */
    fprintf(fp, "/* THIS FILE HAS BEEN AUTOMATICALLY GENERATED. DO NOT EDIT. */\n\n");

    // open the output file
    function = atoi(argv[1]);
    switch (function) {
    case  0: res = gen_rvoice_table_dsp(fp); break;
    case  1: res = gen_conv_table(fp); break;
    default: res = -1;
    }

    fclose(fp);

    return res;
}
