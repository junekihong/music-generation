# Microsoft Developer Studio Project File - Name="allegro" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=allegro - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "allegro.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "allegro.mak" CFG="allegro - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "allegro - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "allegro - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "allegro - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "allegro___Win32_Release"
# PROP BASE Intermediate_Dir "allegro___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\portmidi\porttime" /I "..\portmidi\pm_common" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "allegro - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "allegro___Win32_Debug"
# PROP BASE Intermediate_Dir "allegro___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\portmidi\pm_common" /I "..\portmidi\porttime" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "allegro - Win32 Release"
# Name "allegro - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\allegro.cpp
# End Source File
# Begin Source File

SOURCE=.\allegrord.cpp
# End Source File
# Begin Source File

SOURCE=.\allegrosmfrd.cpp
# End Source File
# Begin Source File

SOURCE=.\allegrosmfwr.cpp
# End Source File
# Begin Source File

SOURCE=.\allegrowr.cpp
# End Source File
# Begin Source File

SOURCE=.\mfmidi.cpp
# End Source File
# Begin Source File

SOURCE=.\seq2midi.cpp
# End Source File
# Begin Source File

SOURCE=.\strparse.cpp
# End Source File
# Begin Source File

SOURCE=.\trace.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\allegro.h
# End Source File
# Begin Source File

SOURCE=.\allegrord.h
# End Source File
# Begin Source File

SOURCE=.\allegrosmfrd.h
# End Source File
# Begin Source File

SOURCE=.\allegrosmfwr.h
# End Source File
# Begin Source File

SOURCE=.\allegrowr.h
# End Source File
# Begin Source File

SOURCE=.\mfmidi.h
# End Source File
# Begin Source File

SOURCE=.\seq2midi.h
# End Source File
# Begin Source File

SOURCE=.\strparse.h
# End Source File
# Begin Source File

SOURCE=.\trace.h
# End Source File
# End Group
# End Target
# End Project
