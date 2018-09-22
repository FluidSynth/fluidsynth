#include <stdio.h>
#include <stdlib.h>

/* callback for general access to matrices */
typedef fluid_real_t (*emit_matrix_cb)(int y, int x);

/* Generators */
extern int gen_rvoice_table_dsp(FILE *fp);
extern int gen_conv_table(FILE *fp);

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const fluid_real_t *tbl, int size);

/* Emit a matrix of real numbers */
void emit_matrix(FILE *fp, const char *tblname, emit_matrix_cb tbl_cb, int sizeh, int sizel);

