
#ifndef _FLUID_JNI_H
#define _FLUID_JNI_H

#include <fluidsynth.h>

int fluid_jni_new_synth(void);
int fluid_jni_delete_synth(int num);
int fluid_jni_add(int samplenum, int bank, int preset, int lokey, int hikey);
int fluid_jni_remove(int samplenum, int bank, int preset);
int fluid_jni_sfload(const char*  filename);

int fluid_jni_new_sample(const char* filename, int rootkey);
int fluid_jni_delete_sample(int num);
int fluid_jni_get_sample_num(void);
fluid_sample_t* fluid_jni_get_sample(int num);

#endif /* _FLUID_JNI_H */
