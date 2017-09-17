#include "fluidsynth_Sample.h"
#include "fluidsynth_jni.h"

JNIEXPORT jint JNICALL 
Java_fluidsynth_Sample_newSample(JNIEnv *env, jobject obj, jstring filename, jint rootkey)
{
  const char *cfilename = env->GetStringUTFChars(filename, 0);
  int err = fluid_jni_new_sample(cfilename, rootkey);
  env->ReleaseStringUTFChars(filename, cfilename);
  return err;
}

JNIEXPORT void JNICALL 
Java_fluidsynth_Sample_deleteSample(JNIEnv *env, jobject obj, jint samplenum)
{
  fluid_jni_delete_sample(samplenum);
}
