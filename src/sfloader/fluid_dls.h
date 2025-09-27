#ifndef _FLUID_DLS_H
#define _FLUID_DLS_H

#include "fluidsynth/types.h"

#ifdef __cplusplus
extern "C" {
#endif

fluid_sfloader_t *new_fluid_dls_loader(fluid_synth_t *synth, fluid_settings_t *settings);

#ifdef __cplusplus
}
#endif

#endif // _FLUID_DLS_H
