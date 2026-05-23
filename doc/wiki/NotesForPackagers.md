# 📦 Notes for Packagers

Since the 1.1.4 release there are some changes in the CMake build system. Please consult the [ChangeLog](ChangeLog.md) for details. One of the changes has been the introduction of several variables with predefined values for install directories, see also the file [cmake_admin/DefaultDirs.cmake](https://github.com/FluidSynth/fluidsynth/blob/master/cmake_admin/DefaultDirs.cmake). The most relevant ones for this release in Linux are the following: 

```cmake
    BIN_INSTALL_DIR="bin" (The install dir for executables)
    LIB_INSTALL_DIR="lib${LIB_SUFFIX}" (The install dir for libraries)
    INCLUDE_INSTALL_DIR="include" (The install dir for headers)
    DATA_INSTALL_DIR="share" (The base install dir for data files)
    DOC_INSTALL_DIR="share/doc" (The install dir for documentation)
    MAN_INSTALL_DIR="share/man/man1" (The man pages install dir)
```    

These directory names were hard-coded in previous releases, but now you can override any or all of them in the command line if you need to do so. For instance, your packaging system may have a `%cmake` macro already overriding the variable `LIB_INSTALL_DIR` with a value of "lib64" in 64-bit systems.

### Systemd integration

Additionally, as of 1.1.10 fluidsynth provides a systemd service file that allows fluidsynth to be set up as user service. The `FLUID_DAEMON_ENV_FILE` variable (if set) causes cmake to generate a service environment file and a properly configured systemd service file (i.e. containing the full path of where the env file is meant to be installed as indicated by the `FLUID_DAEMON_ENV_FILE` var). For instance, you may want to write something like this in your .spec file: 
    
```bash
    %cmake .. -DFLUID_DAEMON_ENV_FILE=/usr/lib/systemd/whatever/sysconfig.fluidsynth    
```

Which will generate fluidsynth.conf and `fluidsynth.service`, 
where the latter already points to the system environment file as given by `FLUID_DAEMON_ENV_FILE`. The packager is responsible to manually install both: the `fluidsynth.service` to the distribution's systemd directory, and the fluidsynth.conf to the same location as given by `FLUID_DAEMON_ENV_FILE`.