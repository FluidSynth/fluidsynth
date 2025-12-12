#import <unistd.h>
#import <AVFoundation/AVFoundation.h>
#import "fluid_coreaudio_avaudiosession.h"

void renderError(NSError* error, const char* desc, char* msgbuf, int msgbufsize) {
    if (error != nil) {
        snprintf(msgbuf, msgbufsize, "Error %s for low-latency audio session: %s",
                 desc, error.description.UTF8String);
    }
    else {
        snprintf(msgbuf, msgbufsize, "%s", desc);
    }
}

int setupAVAudioSession(int mode, int period_size, double sample_rate, char* msgbuf, int msgbufsize) {
    NSError *error = nil;
    BOOL success = false;
    NSTimeInterval bufDuration;
    NSTimeInterval periodDuration;
    NSString *category;
    NSUInteger options;

    AVAudioSession *session = [AVAudioSession sharedInstance];

    if (mode == AVAUDIOSESSION_MODE_LOW_LATENCY) {
        category = AVAudioSessionCategoryPlayAndRecord;
    }
    else {
        category = AVAudioSessionCategoryPlayback;
    }
    options = AVAudioSessionCategoryOptionMixWithOthers | AVAudioSessionCategoryOptionDefaultToSpeaker;

    success = [session setCategory:category withOptions:options error:&error];
    if (!success) {
        renderError(error, "setting audio session category", msgbuf, msgbufsize);
        return 0;
    }

    success = [session setMode:AVAudioSessionModeDefault error:&error];
    if (!success) {
        renderError(error, "setting audio session mode", msgbuf, msgbufsize);
        return 0;
    }

    success = [session setActive:YES error:&error];
    if (!success) {
        renderError(error, "activating audio session", msgbuf, msgbufsize);
        return 0;
    }

    if (mode != AVAUDIOSESSION_MODE_LOW_LATENCY) {
        snprintf(msgbuf, msgbufsize, "setupAVAudioSession: success");
    }
    else {
        periodDuration = (double)period_size / sample_rate;
        if (periodDuration == 0.0) {
            periodDuration = 0.001;
        }
        bufDuration = periodDuration;

        while (session.IOBufferDuration > bufDuration) {
            success = [session setPreferredIOBufferDuration:bufDuration error:&error];

            if (!success) {
                renderError(error, "setting preferred I/O buffer duration", msgbuf, msgbufsize);
                return 0;
            }

            bufDuration += periodDuration;
        }

        snprintf(msgbuf, msgbufsize, "setupAVAudioSession for low latency: success, io buffer duration = %f seconds",
                 session.IOBufferDuration);
    }

    return 1;
}
