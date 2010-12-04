What is CMake?
==============

CMake is a cross platform build system, that can be used to replace the old 
auto-tools, providing a nice building environment and advanced features.

Some of these features are:
* Out of sources build: CMake allows you to build your software into a directory 
  different to the source tree. You can safely delete the build directory and 
  all its contents once you are done.
* Multiple generators: classic makefiles can be generated for Unix and MinGW, 
  but also Visual Studio, XCode and Eclipse CDT projects among other types.
* Graphic front-ends for configuration and build options.

More information and documentation is available at the CMake project site:
    http://www.cmake.org

CMake is free software. You can get the sources and pre-compiled packages for
Linux and other systems at:
     http://www.cmake.org/cmake/resources/software.html
     
How to use it?
==============

1. You need CMake 2.6.3 or later to build FluidSynth

2. Unpack the FluidSynth sources somewhere, or checkout the repository, 
   and create a build directory. For instance, using a command line shell:
   
$ tar -xvzf Downloads/fluidsynth-x.y.z.tar.gz
$ cd fluidsynth-x.y.z
$ mkdir build

2. Execute CMake from the build directory, providing the source directory 
   location and optionally, the build options. There are several ways.

* From a command line shell:

$ pwd
fluidsynth-x.y.z
$ cd build
$ cmake .. -DCMAKE_INSTALL_PREFIX=/usr -Denable-ladspa=1

Valid values for boolean (enable-xxxx) options: 1, 0, yes, no, on, off.

* There are also several alternative CMake front-ends, if you don't want to use
  the command line interface:
  * ncurses based program, for Linux and Unix: ccmake
  * GUI, Qt4 based program, multiplatform: cmake-gui
  * GUI, Windows native program: CMakeSetup.exe 

3. Execute the build command. If you used the Makefiles generator (the default
   in Linux and other Unix systems) then execute make, gmake or mingw32-make.
   If you generated a project file, use your IDE to build it.

You may find more information in the project Wiki: 
    http://sourceforge.net/apps/trac/fluidsynth/wiki/BuildingWithCMake

Compiling with make
===================

There are many targets available. To see a complete list of them, type:

$ make help

The build process usually hides the compiler command lines, to show them:

$ make VERBOSE=1

There is a "clean" target, but not a "distclean" one. You should use a build
directory different to the source tree. In this case, the "distclean" target 
would be equivalent to simply removing the build directory. 

To compile the developer documentation, install Doxygen (http://www.doxygen.org) 
and use this command from the root build directory:

$ make doxygen

If something fails
==================

If there is an error message while executing CMake, this probably means that a
required package is missing in your system. You should install the missing 
component and run CMake again.

If there is an error executing the build process, after running a flawless CMake
configuration process, this means that there may be an error in the source code, 
or in the build system, or something incompatible in 3rd party libraries.

The CMake build system for FluidSynth is experimental. It will take a while
until it becomes stable and fully tested. You can help providing feedback, 
please send a report containing your problems to the FluidSynth development 
mailing list: http://lists.nongnu.org/mailman/listinfo/fluid-dev


For developers - how to add a new feature to the CMake build system
===================================================================

Let's explain this issue with an example. We are adding D-Bus support to 
FluidSynth as an optional feature, conditionally adding source files that 
require this feature. The first step is to add a macro "option()" to the main 
CMakeLists.txt file, the one that is located at the fluidsynth root directory.

file CMakeLists.txt, line 64:

	option ( enable-dbus "compile DBUS support (if it is available)" on )

Now, let's check if the dbus-1 library and headers are installed, using 
pkg-config:

file CMakeLists.txt, lines 371-377:

	unset ( DBUS_SUPPORT CACHE )
	if ( enable-dbus )
		pkg_check_modules ( DBUS dbus-1>=1.0.0 )
		set ( DBUS_SUPPORT ${DBUS_FOUND} )
    else ( enable-dbus )
        unset_pkg_config ( DBUS )
	endif ( enable-dbus )

The first line clears the value of the CMake variable DBUS_SUPPORT. If the 
value of the option "enable-dbus" is true, then the macro  pkg_check_modules() 
is used to test a package named "dbus-1" with version 1.0.0 or later. This macro 
automatically defines the variables DBUS_LIBRARIES, DBUS_INCLUDEDIR, DBUS_FOUND 
and others. The value of the last one is assigned to our variable DBUS_SUPPORT 
for later use.

There is a report to summarize the performed checks and the enabled features 
after the configuration steps, so let's add a line in this report regarding 
the D-Bus support.

file cmake_admin/report.cmake, lines 14-18:

	if ( DBUS_SUPPORT )
		message ( "D-Bus:                 yes" )
	else ( DBUS_SUPPORT ) 
		message ( "D-Bus:                 no" )
	endif ( DBUS_SUPPORT )

The variable DBUS_SUPPORT is available for the CMake files, but we want to make 
it available to the compilers as well, to conditionally build code using 
"#ifdef DBUS_SUPPORT". This can be done adding a line to the config.cmake file:

file src/config.cmake, lines 22-23:

	/* Define if D-Bus support is enabled */
	#cmakedefine DBUS_SUPPORT @DBUS_SUPPORT@

The file config.cmake will be processed at configure time, producing a header 
file "config.h" in the build directory with this content, if the dbus support 
has been enabled and found:

	/* Define if D-Bus support is enabled */
	#define DBUS_SUPPORT  1

Finally, we can add the new source files to the build system for the compiler 
target with the macro add_library(), and the libraries for the linker target 
with the macros link_directories() and target_link_libraries().

file src/CMakeLists.txt, lines 57-60

	if ( DBUS_SUPPORT )
		set ( fluid_dbus_SOURCES fluid_rtkit.c fluid_rtkit.h )
		include_directories ( ${DBUS_INCLUDEDIR} ${DBUS_INCLUDE_DIRS} )
	endif ( DBUS_SUPPORT )

file src/CMakeLists.txt, lines 163-197

    link_directories (
		...
		${DBUS_LIBDIR} 
		${DBUS_LIBRARY_DIRS} 
	)

	add_library ( libfluidsynth  
		...
		${fluid_dbus_SOURCES}
		...
	)

file src/CMakeLists.txt, lines 163-197

	target_link_libraries ( libfluidsynth
		...
		${DBUS_LIBRARIES}
		...
	)

