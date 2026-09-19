# 📥 Get FluidSynth

## Supported Platform

<a href="https://repology.org/metapackage/fluidsynth/versions">
    <img src="https://repology.org/badge/vertical-allrepos/fluidsynth.svg" alt="Packaging status" align="right">
</a>

  * FluidSynth is developed on **Linux** and thus Linux usually has the most up-to-date support.
  * It is also regularly tested on **Mac OS X**. For instructions on how to install fluidsynth on this platform, [see below](#distributions).
  * FluidSynth can also run on **Windows**, and building is supported with either MinGW or MSVC.
  * FluidSynth may be run on **FreeBSD** and its derivatives.
  * We have had successful reports on FluidSynth running on **Solaris** and **OS/2**, but this is not officially supported. It's likely that it will also work on other platforms (most notably Unix like operating systems). 

## Distributions

Many operating systems already provide a package for FluidSynth. See the graphic to right for the currently available packages for various Linux distributions, macOS Ports and BSD derivatives.

  * Gentoo
    ```bash
    emerge fluidsynth
    ```

  * Ubuntu or Debian
    ```bash
    sudo apt-get install fluidsynth
    ```

  * Arch Linux
    ```bash
    pacman -S fluidsynth
    ```
  * Fedora
    ```bash
    sudo dnf install fluidsynth
    ```
    

  * OpenSUSE
    ```bash
    sudo zypper install fluidsynth
    ```

  * Mac OS X
     * With [Fink](http://www.finkproject.org/):
        ```bash
        fink install fluidsynth
        ```
     * With [Homebrew](https://brew.sh/):
        ```bash
        brew install fluidsynth
        ```
     * With [MacPorts](http://www.macports.org/):
        ```bash
        sudo port install fluidsynth
        ```

  * Windows
     * With [Chocolatey](https://chocolatey.org/):
        ```posh
        choco install fluidsynth
        ```
        

**Please contribute to this section if you know how to install FluidSynth packages for a distribution not yet listed.**

## Source Archives

FluidSynth is also relatively easy to be built from a source archive. Please see [BuildingWithCMake](BuildingWithCMake.md) for more detailed instructions of how to build FluidSynth from source. 

The [FluidSynth releases](https://github.com/FluidSynth/fluidsynth/releases) page contains released source archives of FluidSynth.

Unpack the source using a command line shell: 
    
    $ tar -xvzf Downloads/fluidsynth-x.y.z.tar.gz
    

## Git

The latest development version of FluidSynth can be found at our git repository **https://github.com/FluidSynth/fluidsynth** and can be [browsed](https://github.com/FluidSynth/fluidsynth). Unless you are a developer, you should always prefer officially released versions of FluidSynth. Development versions are highly unstable and not recommended for production use.

Starting with version 2.5.0, fluidsynth requires git submodules to be checked out.

Example check out of FluidSynth trunk using the command line Git client: 
    
    git clone --recursive https://github.com/FluidSynth/fluidsynth
    
