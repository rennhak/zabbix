# Microsoft Developer Studio Project File - Name="zabbix_agentd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=zabbix_agentd - Win32 Test
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zabbix_agentd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zabbix_agentd.mak" CFG="zabbix_agentd - Win32 Test"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zabbix_agentd - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "zabbix_agentd - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "zabbix_agentd - Win32 TODO" (based on "Win32 (x86) Console Application")
!MESSAGE "zabbix_agentd - Win32 Test" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../include/" /I "../../../include/" /I "../../../src/zabbix_agent" /D "NDEBUG" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZABBIX_SERVICE" /FR /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release/zabbixw32.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include/" /I "../../../include/" /I "../../../src/zabbix_agent" /D "_DEBUG" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZABBIX_SERVICE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Debug/zabbixw32.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zabbix_agentd___Win32_TODO"
# PROP BASE Intermediate_Dir "zabbix_agentd___Win32_TODO"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "TODO"
# PROP Intermediate_Dir "TODO"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include/" /I "../../../include/" /D "_DEBUG" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include/" /I "../../../include/" /I "../../../src/zabbix_agent" /D "_DEBUG" /D "TODO" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZABBIX_SERVICE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /fo"Debug/zabbixw32.res" /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Debug/zabbixw32.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zabbix_agentd___Win32_Test"
# PROP BASE Intermediate_Dir "zabbix_agentd___Win32_Test"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Test"
# PROP Intermediate_Dir "Test"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include/" /I "../../../include/" /I "../../../src/zabbix_agent" /D "_DEBUG" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../include/" /I "../../../include/" /I "../../../src/zabbix_agent" /D "_DEBUG" /D "ZABBIX_TEST" /D "HAVE_ASSERT_H" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "ZABBIX_SERVICE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /fo"Debug/zabbixw32.res" /d "_DEBUG"
# ADD RSC /l 0x409 /fo"Debug/zabbixw32.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib pdh.lib psapi.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "zabbix_agentd - Win32 Release"
# Name "zabbix_agentd - Win32 Debug"
# Name "zabbix_agentd - Win32 TODO"
# Name "zabbix_agentd - Win32 Test"
# Begin Group "src"

# PROP Default_Filter ""
# Begin Group "libs"

# PROP Default_Filter ""
# Begin Group "zbxcommon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\alias.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\comms.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\gnuregex.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\misc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\regexp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\str.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\xml.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcommon\zbxgetopt.c
# End Source File
# End Group
# Begin Group "zbxlog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxlog\log.c
# End Source File
# End Group
# Begin Group "zbxcrypto"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcrypto\base64.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxcrypto\md5.c
# End Source File
# End Group
# Begin Group "zbxnet"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxnet\security.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxnet\zbxsock.c
# End Source File
# End Group
# Begin Group "zbxconf"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxconf\cfg.c
# End Source File
# End Group
# Begin Group "zbxsysinfo"

# PROP Default_Filter ""
# Begin Group "aix"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\aix.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\AIX_new.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\aix\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "freebsd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\freebsd.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\freebsd\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "hpux"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\hpux.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\hpux\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "linux"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\linux.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\linux\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "netbsd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\netbsd.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\netbsd\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "openbsd"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\openbsd.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\OpenBSD3.7.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\openbsd\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "osf"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\osf.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osf\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "osx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\osx.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\osx\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "solaris"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\solaris.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\SunOS5.9.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\solaris\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "unknown"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\cpu.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\diskio.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\diskspace.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\inodes.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\kernel.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\memory.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\net.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\proc.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\sensors.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\swap.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\unknown.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\unknown\uptime.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\cpu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\diskio.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\diskspace.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\inodes.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\kernel.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\memory.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\net.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\proc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\sensors.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\swap.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\system_w32.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\uptime.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\win32\win32.c
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\common\common.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\common\file.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\common\http.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\common\ntp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsysinfo\common\system.c
# End Source File
# End Group
# End Group
# Begin Group "zbxwin32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxwin32\perfmon.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# ADD CPP /W4

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxwin32\service.c
# End Source File
# End Group
# Begin Group "zbxnix"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxnix\daemon.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxnix\pid.c

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "zbxsys"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsys\mutexs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\libs\zbxsys\threads.c
# End Source File
# End Group
# Begin Group "zbxplugin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\src\libs\zbxplugin\zbxplugin.c
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\active.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\active.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\cpustat.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\cpustat.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\diskdevices.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\diskdevices.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\interfaces.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\interfaces.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\listener.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\listener.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\logfiles.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\logfiles.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\stats.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\stats.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\zabbix_agentd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\zbxconf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\zbxconf.h
# End Source File
# End Group
# Begin Group "inlcude"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\include\alias.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\base64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\cfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\common.h
# End Source File
# Begin Source File

SOURCE=..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\daemon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\db.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\email.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\gnuregex.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\log.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\mutexs.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\perfmon.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\pid.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\service.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\sms.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\sysinc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\sysinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\threads.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zbxgetopt.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zbxplugin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zbxsecurity.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zbxsock.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zbxtypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\zlog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\messages.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\messages.mc

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

# Begin Custom Build - Message Compiling...
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\messages.mc
InputName=messages

BuildCmds= \
	mc -s -U -h  $(ProjDir)\..\..\..\src\zabbix_agent\ -r $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName) \
	del $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# Begin Custom Build - Message Compiling...
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\messages.mc
InputName=messages

BuildCmds= \
	mc -s -U -h  $(ProjDir)\..\..\..\src\zabbix_agent\ -r $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName) \
	del $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

# Begin Custom Build - Message Compiling...
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\messages.mc
InputName=messages

BuildCmds= \
	mc -s -U -h  $(ProjDir)\..\..\..\src\zabbix_agent\ -r $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName) \
	del $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

# Begin Custom Build - Message Compiling...
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\messages.mc
InputName=messages

BuildCmds= \
	mc -s -U -h  $(ProjDir)\..\..\..\src\zabbix_agent\ -r $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName) \
	del $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc \
	

"$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"Msg00001.bin" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\..\src\zabbix_agent\resources.rc

!IF  "$(CFG)" == "zabbix_agentd - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Resource Compiling...
IntDir=.\Release
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\resources.rc
InputName=resources

"$(IntDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rc /l 0x409 /fo"$(INTDIR)\$(InputName).res" /i $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc

# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__RESOU="Msg00001.bin"	
# Begin Custom Build - Resource Compiling...
IntDir=.\Debug
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\resources.rc
InputName=resources

"$(IntDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rc /l 0x409 /fo"$(INTDIR)\$(InputName).res" /i $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc

# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 TODO"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Resource Compiling...
IntDir=.\TODO
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\resources.rc
InputName=resources

"$(IntDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rc /l 0x409 /fo"$(INTDIR)\$(InputName).res" /i $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc

# End Custom Build

!ELSEIF  "$(CFG)" == "zabbix_agentd - Win32 Test"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Resource Compiling...
IntDir=.\Test
ProjDir=.
InputPath=..\..\..\src\zabbix_agent\resources.rc
InputName=resources

"$(IntDir)\$(InputName).res" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	rc /l 0x409 /fo"$(INTDIR)\$(InputName).res" /i $(ProjDir)\..\..\..\src\zabbix_agent\ $(ProjDir)\..\..\..\src\zabbix_agent\$(InputName).rc

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Package Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\configure.in
# End Source File
# Begin Source File

SOURCE=..\..\..\do
# End Source File
# Begin Source File

SOURCE=..\..\..\go
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\AUTHORS
# End Source File
# Begin Source File

SOURCE=..\..\..\ChangeLog
# End Source File
# Begin Source File

SOURCE=..\..\..\COPYING
# End Source File
# Begin Source File

SOURCE=..\..\..\FAQ
# End Source File
# Begin Source File

SOURCE=..\..\..\INSTALL
# End Source File
# Begin Source File

SOURCE=..\..\..\NEWS
# End Source File
# Begin Source File

SOURCE=..\..\..\README
# End Source File
# Begin Source File

SOURCE=..\..\..\TODO
# End Source File
# End Group
# End Target
# End Project
