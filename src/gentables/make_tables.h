#include <stdio.h>
#include <stdlib.h>


#define EMIT_ARRAY(__fp__, __arr__) emit_array(__fp__, #__arr__, __arr__, FLUID_N_ELEMENTS(__arr__))

/* callback for general access to matrices */
typedef fluid_real_t (*emit_matrix_cb)(int y, int x);

/* Generators */
void gen_rvoice_table_dsp(FILE *fp);
void gen_conv_table(FILE *fp);

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const double *tbl, int size);

/* Emit a matrix of real numbers */
void emit_matrix(FILE *fp, const char *tblname, emit_matrix_cb tbl_cb, int sizeh, int sizel);

