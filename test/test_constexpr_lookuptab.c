#include "test.h"
#include "fluidsynth.h"
#include "fluidsynth_priv.h"
#include "fluid_rvoice_dsp.h"
#include "auto_gen_array.h"
#include "auto_gen_math.h"
#include <math.h>

static fluid_real_t interp_coeff_linear_runtime[FLUID_INTERP_MAX][2];
static fluid_real_t interp_coeff_runtime[FLUID_INTERP_MAX][4];
static fluid_real_t sinc_table7_runtime[FLUID_INTERP_MAX][SINC_INTERP_ORDER];

static const fluid_real_t interp_coeff_linear_const[FLUID_INTERP_MAX][2] = { AUTO_GEN_ARRAY_256(INTERP_COEFF_LINEAR) };
static const fluid_real_t interp_coeff_const[FLUID_INTERP_MAX][4] = { AUTO_GEN_ARRAY_256(INTERP_COEFF) };
static const fluid_real_t sinc_table7_const[FLUID_INTERP_MAX][SINC_INTERP_ORDER] = { AUTO_GEN_ARRAY_256(INTERP_COEFF_SINC) };

#define EPS (1.e-6)

// load our sf2 and sf3 test soundfonts, with and without dynamic sample loading
int main(void)
{
    int i, j;
    fluid_real_t x,y;

    fluid_rvoice_dsp_config_LOCAL(interp_coeff_linear_runtime, interp_coeff_runtime, sinc_table7_runtime);
        
    for(i = 0; i < FLUID_INTERP_MAX; i++)
    {
        for(j = 0; j < 2; j++)
        {
            x = interp_coeff_linear_runtime[i][j];
            y = interp_coeff_linear_const[i][j];    
            
            TEST_ASSERT(fabs(x - y) <= EPS);
        }
    }
    
    for(i = 0; i < FLUID_INTERP_MAX; i++)
    {
        for(j = 0; j < 4; j++)
        {
            x = interp_coeff_runtime[i][j];
            y = interp_coeff_const[i][j];    
            
            TEST_ASSERT(fabs(x - y) <= EPS);
        }
    }
    
    for(i = 0; i < FLUID_INTERP_MAX; i++)
    {
        for(j = 0; j < SINC_INTERP_ORDER; j++)
        {
            x = sinc_table7_runtime[i][j];
            y = sinc_table7_const[i][j];    
            
            TEST_ASSERT(fabs(x - y) <= EPS);
        }
    }
    
    return EXIT_SUCCESS;
}
