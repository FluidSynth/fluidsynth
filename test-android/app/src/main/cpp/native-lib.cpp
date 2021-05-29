#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_org_androidaudioplugin_fluidsynth_1tests_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}