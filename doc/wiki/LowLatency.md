# ⏱️ Low latency tips

There are several FluidSynth options which affect audio latency. Many of these depend on what audio driver is being used. Additionally, the operating system being used, how it is set up and what audio card you have will limit the lowest artifact-free latency that can be achieved. 

## Specifying the audio driver

To specify the audio driver which FluidSynth will use, supply the `-a DRIVER` option, for example: 
    
    
    fluidsynth -a alsa
    
    

## Audio buffer size and count

These options are used for the majority of audio drivers that FluidSynth supports. An exception is that the Jack driver does not use the audio buffer size or count options. 

Command line switches and equivalent FluidSynth settings (in parentheses): 

  * `-r RATE` (`synth.sample-rate`): Sample rate 
  * `-c NUM` (`audio.periods`): Set the number of audio buffers 
  * `-z SIZE` (`audio.period-size`): Set audio buffer size 

The sample rate sets the native sample rate that FluidSynth synthesizes to. Typical values include 44100 and 48000. 

!!! Note
 
    The actual sample rate being used by the audio driver may differ, 
    in which case the synth will be out of tune. Ensure that the internal FluidSynth sample rate matches that of your driver for proper operation.

The audio buffer count and size sets the values of these parameters used by the audio driver. Total latency in samples is `NUM * SIZE`. To calculate latency in seconds use `NUM * SIZE / RATE`. For example: `2 * 256 / 48000 = ~10ms` latency. Suggested values for `NUM` is 2 or 3. Suggested values for `SIZE` include: 64, 128, 256, 512, 1024. Non-power of 2 values may also be used depending on the selected audio driver. Note that also sound cards have different limits on this value. 

## ALSA specific tips

ALSA is a pretty flexible audio system and is the de-facto standard on Linux systems. 

Specifying which ALSA device to use for playback can have a huge effect on achievable low latency. The hardware device layer is the best, followed by the plug hardware layer. Using dmix is not recommended for live playback since it typically has a rather high latency response. 
(Unfortunately specifying the hardware layer may bypass all the desktop volume controls.) 

To use the hardware layer, specify the ALSA audio device in the form "hw:N" where N is the card number. For example: 
    
    
    fluidsynth -a alsa -o audio.alsa.device=hw:0
    
    

If FluidSynth fails to initialize your sound card, you may need to specify a different sample rate (some sound cards only operate at fixed rates). Use the -o synth.sample-rate=RATE or -r=RATE command-line options. Playing with the audio buffer size and count may also be required. 
If your sound card does not support 16-bit audio (for example it is fixed to 24 bit), then you will need to use the plughw layer as FluidSynth currently does not support 24 bit directly. 

To use the plug hardware layer, specify the ALSA audio device in the form "plughw:N" where N is the card number. For example: 
    
    
    fluidsynth -a alsa -o audio.alsa.device=plughw:0
    
    

The advantage to the plug hardware layer is that it will do sample conversion as necessary, though this will require additional CPU, so the hw layer is preferred if possible. 

## Linux kernel

The version of the Linux kernel being used and its configured build-time configuration options can have a pretty dramatic effect on achievable artifact-free low latency. This can be an art in and of itself. Some people use RT kernels which may or may not have a low-latency patch applied and usually have full preemption enabled. 

FIXME: Add more info and/or links to Linux kernel tuning. 

## Windows

Actually getting low latency on Windows is only possible through the use of PortAudio driver and using WDM/KS host API.
Making use of WDM/KS is done through the use of `audio.portaudio.device` option.
To determine which device on your system is actually accessible via WDM/KS you can use the fluidsynth info command:

```
info audio.portaudio.device 
   audio.portaudio.device: 
Type:          string 
Value:         38:Windows WDM-KS:Speakers 1 (SB PCI) 
Default value: PortAudio Default 
Options:       17:Windows DirectSound:PÚriphÚrique audio principal, 
               18:Windows DirectSound:SB Live! Audio [CE00], 19:Windows DirectSound:Realtek HD Audio output, 
               20:Windows DirectSound:SB PCI, 24:Windows WDM-KS:Speakers 1 (Realtek HD Audio output), 
               25:Windows WDM-KS:Speakers 2 (Realtek HD Audio output),  26:Windows WDM-KS:Speakers 3 (Realtek HD Audiooutput),
               32:Windows WDM-KS:Speakers 1 (SB Live! Audio [CE00]), 
               33:Windows WDM-KS:Speakers 2 (SB Live! Audio [CE00]), 38:Windows WDM-KS:Speakers 1 (SB PCI), 
               39:Windows WDM-KS:Speakers 2 (SB PCI), 6:MME:Mappeur de sons Microsoft - Output, 
                7:MME:SB Live! Audio [CE00], 8:MME:Realtek HD Audio output, 9:MME:SB PCI, PortAudio Default 
Real-time:     no 
```


Possible device making use of WDM/KS are named 

    xx:Windows WDM-KS:device-name.

* To specify the audio driver FluidSynth will use, supply the `-a DRIVER` option.
* To specify the audio device driver FluidSynth will use, supply the `-o audio.portaudio.device` option.
* To specify the buffer size this device should accept, supply the `-o audio.period-size`.
* Note: Because PortAudio driver ignores `audio.periods` settings you don't need to supply it. Also, you must 
        ignore it when computing latency: `latency = audio.period-size/sample-rate.`

For example: 
    
    fluidsynth -a portaudio -o audio.portaudio.device="38:Windows WDM-KS:Speakers 1 (SB PCI)" 
               -o audio.period-size=1024

In this example with a sample rate of 44100Hz, latency will be: `audio.period-size/sample-rate = 23 ms`.