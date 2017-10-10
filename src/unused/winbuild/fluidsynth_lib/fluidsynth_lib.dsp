# Microsoft Developer Studio Project File - Name="fluidsynth_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=fluidsynth_lib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth_lib.mak" CFG="fluidsynth_lib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluidsynth_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fluidsynth_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluidsynth_lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "NDEBUG" /D "WIN32" /D "_LIB" /D "FLUIDSYNTH_NOT_A_DLL" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "NDEBUG" /D "WIN32" /D "_LIB" /D "FLUIDSYNTH_NOT_A_DLL" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD BASE MTL /win32 
# ADD MTL /win32 
# ADD BASE RSC /l 1036 /d "NDEBUG" 
# ADD RSC /l 1036 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\fluidsynth_lib.lib" 
# ADD LIB32 /nologo /out:"..\fluidsynth_lib.lib" 

!ELSEIF  "$(CFG)" == "fluidsynth_lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /I "..\..\include" /Zi /W3 /Od /D "_DEBUG" /D "WIN32" /D "_LIB" /D "FLUIDSYNTH_NOT_A_DLL" /D "_MBCS" /YX /Fp".\Debug\fluidsynth_lib.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD CPP /nologo /MDd /I "..\..\include" /Zi /W3 /Od /D "_DEBUG" /D "WIN32" /D "_LIB" /D "FLUIDSYNTH_NOT_A_DLL" /D "_MBCS" /YX /Fp".\Debug\fluidsynth_lib.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD BASE MTL /win32 
# ADD MTL /win32 
# ADD BASE RSC /l 1036 /d "_DEBUG" 
# ADD RSC /l 1036 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\fluidsynth_lib_debug.lib" 
# ADD LIB32 /nologo /out:"..\fluidsynth_lib_debug.lib" 

!ENDIF

# Begin Target

# Name "fluidsynth_lib - Win32 Release"
# Name "fluidsynth_lib - Win32 Debug"
# End Target
# End Project

