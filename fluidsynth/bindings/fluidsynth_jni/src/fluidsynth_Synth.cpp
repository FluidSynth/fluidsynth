#include "fluidsynth_Synth.h"
#include "fluidsynth_jni.h"

JNIEXPORT jint JNICALL 
Java_fluidsynth_Synth_newSynth(JNIEnv *env, jobject obj)
{
  return fluid_jni_new_synth();
}

JNIEXPORT void JNICALL 
Java_fluidsynth_Synth_deleteSynth(JNIEnv *env, jobject obj, jint synth)
{
  fluid_jni_delete_synth(synth);
}

JNIEXPORT jint JNICALL 
Java_fluidsynth_Synth_add(JNIEnv *env, jobject obj, jint synth, jint samplenum,
			   jint bank, jint preset, jint lokey, jint hikey)
{
  return fluid_jni_add(samplenum, bank, preset, lokey, hikey);  
}

JNIEXPORT jint JNICALL 
Java_fluidsynth_Synth_remove(JNIEnv *env, jobject obj, jint synth, 
			     jint samplenum, jint bank, jint preset)
{
  return fluid_jni_remove(samplenum, bank, preset);  
}

JNIEXPORT jint JNICALL 
Java_fluidsynth_Synth_loadSoundFont(JNIEnv *env, jobject obj, jint synth, jstring filename)
{
  const char *cfilename = env->GetStringUTFChars(filename, 0);
  int err = fluid_jni_sfload(cfilename);
  env->ReleaseStringUTFChars(filename, cfilename);
  return err;
}

