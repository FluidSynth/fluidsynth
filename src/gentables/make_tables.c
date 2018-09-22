#include "fluid_sys.h"
#include "make_tables.h"

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const double *tbl, int size)
{
    int i;

    fprintf(fp, "static const fluid_real_t %s[%d] = {\n", tblname, size);

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

static void open_table(FILE**fp, char* file)
{        
    /* open the output file */
    *fp = fopen(file, "w");
    if (*fp == NULL)
    {
        exit(-2);
    }
    
    /* Emit warning header */
    fprintf(*fp, "/* THIS FILE HAS BEEN AUTOMATICALLY GENERATED. DO NOT EDIT. */\n\n");
}

int main (int argc, char *argv[])
{
    char buf[2048] = {0};
    FILE *fp;
    int   res, function;
 
    // make sure we have enough arguments
    if (argc < 2)
        return -1;
    
    strcat(buf, argv[1]);
    strcat(buf, "fluid_conv_tables.c");

    open_header(&fp, buf);
    gen_conv_table(fp);

    fclose(fp);

    return res;
}
