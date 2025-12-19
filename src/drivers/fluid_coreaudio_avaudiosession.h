#ifndef FLUID_COREAUDIO_AVAUDIOSESSION_H
#define FLUID_COREAUDIO_AVAUDIOSESSION_H

#ifdef __cplusplus
extern "C" {
#endif

/* PowerSaving mode is not yet supported, but in oboe driver,
   PowerSaving mode is 1, so we leave LowLatency as 2,
   for consistency with fluid_oboe.c */
#define AVAUDIOSESSION_MODE_NONE 0
#define AVAUDIOSESSION_MODE_LOW_LATENCY 2

int setupAVAudioSession(int mode, int period_size, double sample_rate, char* msgbuf, int msgbufsize);

#ifdef __cplusplus
}
#endif

#endif /* FLUID_COREAUDIO_AVAUDIOSESSION_H */
