#include <jni.h>
#include <string>

extern "C" int run_all_fluidsynth_tests();

extern "C" JNIEXPORT jstring JNICALL
Java_org_fluidsynth_fluidsynthtests_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    run_all_fluidsynth_tests();

    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}