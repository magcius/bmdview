# Microsoft Developer Studio Project File - Name="gl_template" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gl_template - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "gl_template.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "gl_template.mak" CFG="gl_template - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "gl_template - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE "gl_template - Win32 Debug" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gl_template - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".." /I "fileformats\3ds\lib3ds-1.2.0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 lib3ds-120.lib comdlg32.lib glu32.lib shell32.lib glew32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /machine:I386 /out:"Release/bmdview2.exe" /libpath:"fileformats\3ds\lib3ds-1.2.0\msvc\build\release"
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\bmdview2.exe ..\bmdview2.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "gl_template - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".." /I "fileformats\3ds\lib3ds-1.2.0" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 lib3ds-120s.lib comdlg32.lib glu32.lib shell32.lib glew32.lib opengl32.lib kernel32.lib user32.lib gdi32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"Debug/bmdview2.exe" /pdbtype:sept /libpath:"fileformats\3ds\lib3ds-1.2.0\msvc\build\release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\bmdview2.exe ..\bmdview2.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "gl_template - Win32 Release"
# Name "gl_template - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "addons cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\addons\bck.cpp
# End Source File
# Begin Source File

SOURCE=.\addons\btp.cpp
# End Source File
# Begin Source File

SOURCE=.\addons\export3ds.cpp
# End Source File
# Begin Source File

SOURCE=.\addons\exportTexture.cpp
# End Source File
# Begin Source File

SOURCE=.\addons\exportx.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\bmdread.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\common.cpp
# End Source File
# Begin Source File

SOURCE=.\drawBmd.cpp
# End Source File
# Begin Source File

SOURCE=.\drawtext.cpp
# End Source File
# Begin Source File

SOURCE=.\drw1.cpp
# End Source File
# Begin Source File

SOURCE=.\evp1.cpp
# End Source File
# Begin Source File

SOURCE=.\inf1.cpp
# End Source File
# Begin Source File

SOURCE=.\jnt1.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mat3.cpp
# End Source File
# Begin Source File

SOURCE=.\mdl3.cpp
# End Source File
# Begin Source File

SOURCE=.\oglblock.cpp
# End Source File
# Begin Source File

SOURCE=.\openfile.cpp
# End Source File
# Begin Source File

SOURCE=.\parameters.cpp
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\shp1.cpp
# End Source File
# Begin Source File

SOURCE=.\simple_gl.cpp
# End Source File
# Begin Source File

SOURCE=.\simple_gl_common.cpp
# End Source File
# Begin Source File

SOURCE=.\tex1.cpp
# End Source File
# Begin Source File

SOURCE=.\textbox.cpp
# End Source File
# Begin Source File

SOURCE=.\transformtools.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\vtx1.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "addons h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\addons\bck.h
# End Source File
# Begin Source File

SOURCE=.\addons\btp.h
# End Source File
# Begin Source File

SOURCE=.\addons\export3ds.h
# End Source File
# Begin Source File

SOURCE=.\addons\exportTexture.h
# End Source File
# Begin Source File

SOURCE=.\addons\exportx.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\bmdread.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\clock.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\drawbmd.h
# End Source File
# Begin Source File

SOURCE=.\drawtext.h
# End Source File
# Begin Source File

SOURCE=.\drw1.h
# End Source File
# Begin Source File

SOURCE=.\evp1.h
# End Source File
# Begin Source File

SOURCE=.\inf1.h
# End Source File
# Begin Source File

SOURCE=.\jnt1.h
# End Source File
# Begin Source File

SOURCE=.\mat3.h
# End Source File
# Begin Source File

SOURCE=.\Matrix44.h
# End Source File
# Begin Source File

SOURCE=.\mdl3.h
# End Source File
# Begin Source File

SOURCE=.\oglblock.h
# End Source File
# Begin Source File

SOURCE=.\openfile.h
# End Source File
# Begin Source File

SOURCE=.\parameters.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\shp1.h
# End Source File
# Begin Source File

SOURCE=.\simple_gl.h
# End Source File
# Begin Source File

SOURCE=.\simple_gl_common.h
# End Source File
# Begin Source File

SOURCE=.\tex1.h
# End Source File
# Begin Source File

SOURCE=.\textbox.h
# End Source File
# Begin Source File

SOURCE=.\transformtools.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# Begin Source File

SOURCE=.\Vector3.h
# End Source File
# Begin Source File

SOURCE=.\vtx1.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon.ico
# End Source File
# End Group
# End Target
# End Project
