# Microsoft Developer Studio Project File - Name="fluidsynth" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=fluidsynth - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth.mak" CFG="fluidsynth - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluidsynth - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "fluidsynth - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluidsynth - Win32 Debug"

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
# ADD BASE CPP /nologo /MDd /I "..\..\include" /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /Fp".\Debug\fluidsynth.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD CPP /nologo /MDd /I "..\..\include" /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /Fp".\Debug\fluidsynth.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD BASE MTL /tlb ".\Debug/fluidsynth.tlb" /win32 
# ADD MTL /tlb ".\Debug/fluidsynth.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth_debug.exe" /incremental:no /debug /pdb:".\Debug/fluidsynth_debug.pdb" /pdbtype:sept /subsystem:console /machine:ix86 
# ADD LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth_debug.exe" /incremental:no /debug /pdb:".\Debug/fluidsynth_debug.pdb" /pdbtype:sept /subsystem:console /machine:ix86 

!ELSEIF  "$(CFG)" == "fluidsynth - Win32 Release"

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
# ADD BASE CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD BASE MTL /tlb ".\Release/fluidsynth.tlb" /win32 
# ADD MTL /tlb ".\Release/fluidsynth.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib dsound.lib /nologo /out:"../fluidsynth.exe" /incremental:no /pdb:".\Release/fluidsynth.pdb" /pdbtype:sept /subsystem:console /machine:ix86 
# ADD LINK32 odbc32.lib odbccp32.lib dsound.lib /nologo /out:"../fluidsynth.exe" /incremental:no /pdb:".\Release/fluidsynth.pdb" /pdbtype:sept /subsystem:console /machine:ix86 

!ENDIF

# Begin Target

# Name "fluidsynth - Win32 Debug"
# Name "fluidsynth - Win32 Release"
# End Target
# End Project

