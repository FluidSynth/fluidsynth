
#ifndef _FLUID_MAKE_TABLES_H
#define _FLUID_MAKE_TABLES_H

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // Enable M_LN10 and M_PI constants under VisualStudio
#endif
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EMIT_ARRAY(__fp__, __arr__) emit_array(__fp__, #__arr__, __arr__, sizeof(__arr__)/sizeof(*__arr__))

/* callback for general access to matrices */
typedef double (*emit_matrix_cb)(int y, int x);

/* Generators */
void gen_rvoice_table_dsp(FILE *fp);
void gen_conv_table(FILE *fp);

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const double *tbl, int size);

/* Emit a matrix of real numbers */
void emit_matrix(FILE *fp, const char *tblname, emit_matrix_cb tbl_cb, int sizeh, int sizel);

#endif
