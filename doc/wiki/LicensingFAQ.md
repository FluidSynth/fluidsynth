# 📜 Licensing FAQ

**Q: Can I consider the below to be legal advice?**

A: NO. It is written by a layman and is not guaranteed in any way to be free from errors. 

**Q: Can I include FluidSynth in my closed-source commercial project?**

A: That depends. FluidSynth is licensed under LGPL, which (unlike GPL) makes it possible to dynamically link with code under other licenses. Two important things to remember: 

  * The end user of your project must be able to change/upgrade the FluidSynth part of the project independently of you. This is usually solved by shipping FluidSynth as a separate library (so/dll) file instead of linking the source code directly. 
  * All changes to FluidSynth itself (i e the so/dll) must be made available under the terms of the LGPL. 

More information about LGPL is available on <http://www.gnu.org>. 

**Q: Can I include FluidSynth in an iPhone app which I sell on Apple App Store?**

A: Please see [here](#ios-and-the-app-store) for information. 

**Q: I'm willing to pay for a commercial license of FluidSynth. Is there someone I can contact?**

A: The FluidSynth project has so many contributors (and thus copyright holders) that contacting them all, asking for their consent etc., would be very difficult. 

**Q: What about VST and ASIO?**

A: VST2 SDK was licensed under a proprietary license that is incompatible with the LGPL. In 2017, Steinberg [published](https://sdk.steinberg.net/viewtopic.php?f=4&t=282) the VST3 SDK with a dual license: proprietary or GPLv3, and now it is [hosted at GitHub](https://github.com/steinbergmedia/vst3sdk). If you use VST3 or other libraries under the GPLv3 license to build FluidSynth, the resulting binaries would be under this license, instead of the LGPL.

The ASIO headers are still incompatible with LGPL as they forbid redistribution (even now supports GNU GPL v3, it is still incompatible with LGPL). However, it should be possible anyway via dynamic linking (just keep FluidSynth in one library and the headers in another). For ASIO, you can compile FluidSynth with [PortAudio](http://www.portaudio.com) support, and compile PortAudio with ASIO support. 

**Q: Can I use FluidSynth to make commercial music?**

A: Yes, definitely! FluidSynth can not claim anything when it comes to the sound output. Just make sure to check the license conditions on the **input** to FluidSynth - i.e. the midi input and the soundfonts. 


## iOS and the App Store

It is questionable whether iOS and the App Store can fulfil the requirements of the LGPL. From a [long thread](http://lists.nongnu.org/archive/html/fluid-dev/2011-09/msg00033.html) on the fluid-dev mailinglist, it was concluded that the developer distributing an application using FluidSynth must fulfil the following conditions: 

  * He/she must release all changes to the FluidSynth source code under the LGPL. 
  * He/she must release all other code of the application, either as source or as linkable object files, so that an independent user can relink the application with a different version of FluidSynth. 

In addition, the App Store distribution mechanism might be incompatible with the LGPL, so the developer risks that Apple chooses to remove the application. (And as of 2011, Apple has been likely to remove an app if there are legal doubts about it.) To avoid that risk, the developer can choose to distribute his application through e g [Cydia](http://en.wikipedia.org/wiki/Cydia). 

The following FluidSynth copyright holders have agreed not to actively raise complaints against FluidSynth App Store applications, provided the above conditions are met: 

  * Peter Hanappe 
  * Josh "Element" Green 
  * Pedro Lopez-Cabanillas 
  * David Henningsson 
  * Matt Giuca 
  * Antoine Schmitt 
  * Jason Vasquez 
  * Tom Moebert
  * Marcus Weseloh

However, FluidSynth has a lot of copyright holders, so the above is NOT a guarantee that no complaints will be raised against Apple for distributing FluidSynth. 
