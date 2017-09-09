# Microsoft Developer Studio Project File - Name="fluidsynth_dll" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=fluidsynth_dll - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth_dll.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fluidsynth_dll.mak" CFG="fluidsynth_dll - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fluidsynth_dll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "fluidsynth_dll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "fluidsynth_dll - Win32 Release"

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
# ADD BASE CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "FLUIDSYNTH_DLL_EXPORTS" /D "FLUIDSYNTH_SEQ_DLL_EXPORTS" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD CPP /nologo /MD /I "..\..\include" /W3 /O2 /Ob1 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "FLUIDSYNTH_DLL_EXPORTS" /D "FLUIDSYNTH_SEQ_DLL_EXPORTS" /D "_MBCS" /GF /Gy /YX /Fo".\Release\" /Fd".\Release\" /c 
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /tlb ".\Release/fluidsynth_dll.tlb" /win32 
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /tlb ".\Release/fluidsynth_dll.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth.dll" /incremental:no /pdb:".\Release/fluidsynth.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/fluidsynth.lib" /machine:ix86 
# ADD LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth.dll" /incremental:no /pdb:".\Release/fluidsynth.pdb" /pdbtype:sept /subsystem:windows /implib:".\Release/fluidsynth.lib" /machine:ix86 

!ELSEIF  "$(CFG)" == "fluidsynth_dll - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /I "..\..\include" /Zi /W3 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "FLUIDSYNTH_DLL_EXPORTS" /D "FLUIDSYNTH_SEQ_DLL_EXPORTS" /D "_MBCS" /YX /Fp".\Debug\fluidsynth_dll.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD CPP /nologo /MTd /I "..\..\include" /Zi /W3 /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "FLUIDSYNTH_DLL_EXPORTS" /D "FLUIDSYNTH_SEQ_DLL_EXPORTS" /D "_MBCS" /YX /Fp".\Debug\fluidsynth_dll.pch" /Fo".\Debug\" /Fd".\Debug\" /GZ /c 
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /tlb ".\Debug/fluidsynth_dll.tlb" /win32 
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /tlb ".\Debug/fluidsynth_dll.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 
# ADD BSC32 
LINK32=link.exe
# ADD BASE LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth_debug.dll" /incremental:no /debug /pdb:".\Debug/fluidsynth_debug.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/fluidsynth_debug.lib" /machine:ix86 
# ADD LINK32 odbc32.lib odbccp32.lib dsound.lib winmm.lib /nologo /out:"../fluidsynth_debug.dll" /incremental:no /debug /pdb:".\Debug/fluidsynth_debug.pdb" /pdbtype:sept /subsystem:windows /implib:".\Debug/fluidsynth_debug.lib" /machine:ix86 

!ENDIF

# Begin Target

# Name "fluidsynth_dll - Win32 Release"
# Name "fluidsynth_dll - Win32 Debug"
# End Target
# End Project

