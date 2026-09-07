#////////////////////////////////////////////////////////////////////
#// Alexander A. Telyatnikov (Alter) 2002-2016. Kiev, Ukraine
#// All rights reserved
#////////////////////////////////////////////////////////////////////

!IF "$(CFG)" == ""
CFG=UniATA - Win32 Debug
!MESSAGE No configuration specified. Defaulting to UniATA - Win32 Debug.
!ENDIF

!IF "$(ARCH)" == "x64"
ARCH_CPP_DEFINES=/D_AMD64_ /D_WIN64
ARCH_LIB_DIR=amd64
LINK_MACHINE=AMD64
ARCH_KERNEL_LIBS=
!ELSE
ARCH_CPP_DEFINES=/D_X86_
ARCH_LIB_DIR=i386
LINK_MACHINE=I386
ARCH_KERNEL_LIBS=int64.lib
!ENDIF

!IF "$(DDK_TARGET)" == ""
DDK_TARGET=WXP
!ENDIF

!IF "$(CFG)" != "UniATA - Win32 Release" && "$(CFG)" != "UniATA - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UniATA.mak" CFG="UniATA - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UniATA - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "UniATA - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF "$(DST_WINDIR)" == ""
DST_WINDIR=C:\WINNT\System32\Drivers
!ENDIF

!IF "$(BaseDir)" == ""
BaseDir=$(BASEDIR)
!ENDIF 

!IF "$(BaseDir)" == ""
!ERROR BASEDIR environment variable not set
!ELSEIF "$(BaseDir)" == "$(BASEDIRNT4)" || "$(BaseDir)" == "$(BASEDIR2K)"
#makefile for nt4/w2k
!IF "$(BASEDIR351)" == ""
BaseDirLib=$(BaseDir)
!ELSE 
BaseDirLib=$(BASEDIR351)
!ENDIF 
DDKINC=/I $(BaseDir)\inc
!ELSEIF "$(BaseDir)" == "$(BASEDIRXP)"
#makefile for XP
BaseDirLib=$(BaseDir)
DDKINC=/I $(BaseDir)\inc\ddk /I $(BaseDir)\inc\api /I $(BaseDir)\inc\crt /I $(BaseDir)\inc
USE_XPDDK=1
!ELSE 
!ERROR BASEDIR is not equal to BASEDIRNT4, BASEDIR2K or BASEDIRXP
!ENDIF 

!MESSAGE *** Using DDK at $(BaseDir)
!MESSAGE *** Using Lib at $(BaseDirLib)

CPP=cl.exe
MCC=mc.exe
MTL=midl.exe
RSC=rc.exe
LINK32=link.exe

!IF  "$(CFG)" == "UniATA - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
!IF "$(ARCH)" == "x64"
OUTDIR=.\Release_x64
INTDIR=.\Release_x64
!ENDIF
ALTDIR=.\Debug
!IF "$(USE_XPDDK)" == ""
LibPath=$(BaseDirLib)\Lib\i386\Free\
!ELSE 
LibPath=$(BaseDirLib)\Lib\$(DDK_TARGET)\$(ARCH_LIB_DIR)\
!ENDIF 

!ELSEIF  "$(CFG)" == "UniATA - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
ALTDIR=.\Release
!IF "$(USE_XPDDK)" == ""
LibPath=$(BaseDirLib)\Lib\i386\Checked\
!ELSE 
LibPath=$(BaseDirLib)\Lib\i386\
!ENDIF 

!ENDIF 

# Begin Custom Macros
OutDir=$(OUTDIR)
DistDir=$(OUTDIR)_Dist
SymDir=$(OUTDIR)_sym
# End Custom Macros

!MESSAGE ALTDIR: $(ALTDIR)

TargetPath=$(OUTDIR)\IdeDma.sys
InputPath=$(OUTDIR)\IdeDma.sys
SOURCE="$(InputPath)"

DEF_FILE= \
	".\IdeDma.def"
LINK32_OBJS= \
	"$(INTDIR)\bm_devs.obj" \
	"$(INTDIR)\atacmd_map.obj" \
	"$(INTDIR)\id_probe.obj" \
	"$(INTDIR)\id_ata.obj" \
	"$(INTDIR)\id_dma.obj" \
	"$(INTDIR)\id_init.obj" \
	"$(INTDIR)\id_queue.obj" \
	"$(INTDIR)\id_sata.obj" \
	"$(INTDIR)\id_badblock.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\idedma.res"

!IF  "$(CFG)" == "UniATA - Win32 Release"
ALL : "$(OUTDIR)\IdeDma.sys" ".\copy.msg"
!ELSEIF  "$(CFG)" == "UniATA - Win32 Debug"
ALL : "$(OUTDIR)\IdeDma.sys" ".\copy.msg"
!ENDIF 

"uniata_ver.h" : "uniata_ver.h_"
!IF "$(VER)" != ""
	copy "uniata_ver.h_" "uniata_ver.h"
	.\Dist\tools\srchrep.exe -src "**VER**" -dest "$(VER)" "uniata_ver.h"
	.\Dist\tools\srchrep.exe -src "**VER_DOT**" -dest "$(VER)" "uniata_ver.h"
	.\build_inf.exe --ver_h > uniata_ver.h
        .\Dist\tools\srchrep.exe -e -src "\r\n" -dest "\n" "uniata_ver.h"
!ENDIF 

"..\build_inf\pcidump.h" :
!IF EXISTS("..\..\..\pcidump\head\driver\pcidump.h")
 	copy ..\..\..\pcidump\head\driver\pcidump.h ..\build_inf
!ENDIF

"build_inf.exe" : "bm_devs.h" "uniata_ver.h_" "..\build_inf\pcidump.h"
!IF "$(NO_BUILD_INF)" == ""
!IF "$(VER)" != ""
	copy "uniata_ver.h_" "uniata_ver.h"
	.\Dist\tools\srchrep.exe -src "**VER**" -dest "$(VER)" "uniata_ver.h"
!ENDIF 
!IF EXISTS("..\..\..\pcidump\head\driver\pcidump.h")
	copy ..\..\..\pcidump\head\driver\pcidump.h ..\build_inf
!ENDIF
	cd ..\build_inf
	nmake /A CFG="build_inf - Win32 Release"
	cd ..\driver
	copy ..\build_inf\Release\build_inf.exe .\build_inf.exe
	copy ..\build_inf\Release\build_inf.exe .\Dist\tools\build_inf.exe
!IF "$(VER)" != ""
	.\build_inf.exe --ver_h > uniata_ver.h
        .\Dist\tools\srchrep.exe -e -src "\r\n" -dest "\n" "uniata_ver.h"
!ENDIF 
!ENDIF


"build_atacmd.exe" : "atapi.h"
!IF "$(NO_BUILD_CMD)" == ""
	cd ..\build_atacmd
	nmake /A CFG="build_atacmd - Win32 Release"
	cd ..\driver
	copy ..\build_atacmd\Release\build_atacmd.exe .\build_atacmd.exe
!ENDIF


"atactl.exe" : "uata_ctl.h"
!IF "$(NO_BUILD_INF)" == ""
	cd ..\atactl
	nmake /A CFG="atactl - Win32 Release"
	cd ..\driver
	copy ..\atactl\Release\atactl.exe .\atactl.exe
	copy ..\atactl\Release\atactl.exe .\Dist\atactl.exe
!ENDIF 

PKG : "build_inf.exe" "build_atacmd.exe" "BusMaster_v$(VER).rar"
!IF "$(VER)" == ""
	@echo ERROR: Version not specified !!! & exit /b 1
!ENDIF

#ALL :
#	nmake CFG="UniATA - Win32 Release"
#	nmake CFG="UniATA - Win32 Debug"

"Release\idedma.sys" : "uniata_ver.h"
	nmake CFG="UniATA - Win32 Release"

"Debug\idedma.sys" : "uniata_ver.h"
	nmake CFG="UniATA - Win32 Debug"

"BusMaster_v$(VER).rar" : ".\Debug\idedma.res" "uniata_ver.h" "..\pkg_files.dist" "..\pkg_files.src" "Release\idedma.sys" "Debug\idedma.sys"
	cd ..
	driver\Dist\tools\fix_dep.bat
	cd driver
#!IF  "$(CFG)" == "UniATA - Win32 Release"
#	nmake /A CFG="UniATA - Win32 Debug"
#	nmake /A CFG="UniATA - Win32 Release"
#!ELSEIF  "$(CFG)" == "UniATA - Win32 Debug"
#	nmake /A CFG="UniATA - Win32 Release"
#	nmake /A CFG="UniATA - Win32 Debug"
#!ENDIF 
	if exist DST_WINDIR copy $(ALTDIR)\IdeDma.sys $(DST_WINDIR)\IdeDma.sys 
	if exist DST_WINDIR copy $(ALTDIR)\IdeDma.sys $(DST_WINDIR)\IdeDmb.sys 

	rar a    -s -mdD -m5 BusMaster_v$(VER).rar     @..\pkg_files.dist
	rar a -r -s -mdD -m5 BusMaster_v$(VER)_Dbg.rar Debug_Dist
	rar a    -s -mdD -m5 BusMaster_v$(VER)_src.rar @..\pkg_files.src
	rar a    -s -mdD -m5 BusMaster_v$(VER)_sym.rar Release_sym


CLEAN :
	-@erase $(LINK32_OBJS)
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\IdeDma.sys"
	-@erase ".\copy.msg"
	-@erase "$(INTDIR)\idedma.pch"
!IF  "$(CFG)" == "UniATA - Win32 Debug"
	-@erase "$(OUTDIR)\IdeDma.map"
	-@erase "$(OUTDIR)\IdeDma.pdb"
!ENDIF 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)" mkdir "$(OUTDIR)"

"$(DistDir)" :
    if not exist "$(DistDir)" mkdir "$(DistDir)"

"$(SymDir)" :
    if not exist "$(SymDir)" mkdir "$(SymDir)"

!IF  "$(CFG)" == "UniATA - Win32 Release"

CPP_PROJ_BASE=/nologo /Gz /MT /W3 /GX /O2 /Oy- $(DDKINC) /DNDEBUG /DWIN32 /D_WINDOWS /D_MBCS /D_USRDLL /DIdeDma_EXPORTS $(ARCH_CPP_DEFINES) /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010000 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
CPP_PROJ=$(CPP_PROJ_BASE) /Yu"stdafx.h"

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
	
LINK32_FLAGS=/LIBPATH:$(LibPath) ntoskrnl.lib $(ARCH_KERNEL_LIBS) Hal.lib ScsiPort.lib .\Lib\Release\CrossNtK.lib /nologo /entry:"DriverEntry" /incremental:no /debug /pdb:"$(OUTDIR)\IdeDma.pdb" /machine:$(LINK_MACHINE) /nodefaultlib /def:".\IdeDma.def" /out:"$(OUTDIR)\IdeDma.sys" /driver /subsystem:native /opt:ref /opt:icf
#LINK32_FLAGS=$(BaseDir)\Lib\i386\Free\ntoskrnl.lib $(BaseDir)\Lib\i386\Free\int64.lib $(BaseDir)\Lib\i386\Checked\Hal.lib /nologo /entry:"DriverEntry" /incremental:no /pdb:"$(OUTDIR)\IdeDma.pdb" /machine:I386 /nodefaultlib /def:".\IdeDma.def" /out:"$(OUTDIR)\IdeDma.sys" /driver /subsystem:native 

!ELSEIF  "$(CFG)" == "UniATA - Win32 Debug"

CPP_PROJ_BASE=/nologo /Gz /MTd /W3 /GX /Z7 /Od /Oy- $(DDKINC) /D_DEBUG /DDBG /DWIN32 /D_WINDOWS /D_MBCS /D_USRDLL /DIdeDma_EXPORTS $(ARCH_CPP_DEFINES) /D_WIN32_WINNT=0x0501 /DNTDDI_VERSION=0x05010000 /Fp"$(INTDIR)\idedma.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
CPP_PROJ=$(CPP_PROJ_BASE) /Yu"stdafx.h"

MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 

LINK32_FLAGS=/LIBPATH:$(LibPath) ntoskrnl.lib int64.lib Hal.lib ScsiPort.lib .\Lib\Debug\PostDbgMesgK.lib .\Lib\Debug\CrossNtK.lib /nologo /entry:"DriverEntry" /incremental:no /pdb:"$(OUTDIR)\IdeDma.pdb" /map:"$(INTDIR)\IdeDma.map" /debug /machine:I386 /nodefaultlib /def:".\IdeDma.def" /out:"$(OUTDIR)\IdeDma.sys" /pdbtype:sept /driver /subsystem:native,4.00 

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("idedma.dep")
!INCLUDE "idedma.dep"
!ELSE 
!MESSAGE Warning: cannot find "idedma.dep"
!ENDIF 
!ENDIF 

"$(OUTDIR)\IdeDma.sys" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS) "uniata_ver.h"
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

".\copy.msg" : "$(INTDIR)\idedma.res" $(SOURCE) "$(INTDIR)" "$(OUTDIR)" "$(DistDir)" "$(SymDir)" "Dist\tools\rebuild_inf.bat"
	<<tempfile.bat 
	@echo off 
	if exist DST_WINDIR copy $(TargetPath) $(DST_WINDIR)\IdeDma.sys 
	if exist DST_WINDIR copy $(TargetPath) $(DST_WINDIR)\IdeDmb.sys 
!IF "$(NO_BUILD_INF)" == ""
	del /Q /S $(DistDir)\*
	copy $(TargetPath) $(DistDir)\uniata.sys
!IF  "$(CFG)" == "UniATA - Win32 Release"
	copy $(OutDir)\idedma.pdb  $(SymDir)\uniata.pdb
!ELSEIF  "$(CFG)" == "UniATA - Win32 Debug"
	copy $(OutDir)\idedma.pdb  $(DistDir)\uniata.pdb
!ENDIF 
	cd Dist\tools
	call rebuild_inf.bat $(VER)
	cd ..\..
	xcopy /S Dist\* $(DistDir)
	echo $(DistDir)
!ENDIF 
	echo Driver copied > copy.msg 
<< 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("IdeDma.dep")
!INCLUDE "IdeDma.dep"
!ELSE 
!MESSAGE Warning: cannot find "IdeDma.dep"
!ENDIF 
!ENDIF 


REL_PATH=.
SRC_EXT=cpp

SRC=id_ata
!INCLUDE build.mak

SRC=id_dma
!INCLUDE build.mak

SRC=id_init
!INCLUDE build.mak

SRC=id_probe
!INCLUDE build.mak

SRC=id_queue
!INCLUDE build.mak

SRC=id_sata
!INCLUDE build.mak

SRC=id_badblock
!INCLUDE build.mak

SRC=atacmd_map
!INCLUDE build.mak

SRC=bm_devs
!INCLUDE build.mak

SRC=stdafx
CPP_PROJ=$(CPP_PROJ_BASE) /Yc"stdafx.h"
"$(INTDIR)\StdAfx.obj" "$(INTDIR)\idedma.pch" : $(SOURCE) "$(INTDIR)" "uataerr.h"
	$(CPP) @<<
  $(CPP_PROJ) $(SOURCE)
<<


"uataerr.h" : uataerr.mc
    $(MCC) uataerr.mc

SRC=idedma

"uniata_ver.h_.lk" :
    echo "." > "uniata_ver.h_.lk"

"uniata_ver.h_" : "uniata_ver.h_.lk"
    del "uniata_ver.h_.lk"

!IF "$(NO_BUILD_CMD)" == ""
".\atacmd_map.h" : "build_atacmd.exe"
	    echo "Build ATA command map"
	    .\build_atacmd.exe > atacmd_map.h
	    .\Dist\tools\srchrep.exe -e -src "\r\n" -dest "\n" "atacmd_map.h"
!ENDIF

".\uniata_ver.h" : "uniata_ver.h_" "build_inf.exe"
!IF "$(VER)" != ""
    copy "uniata_ver.h_" "uniata_ver.h"
    .\Dist\tools\srchrep.exe -src "**VER**" -dest "$(VER)" "uniata_ver.h"
#    cd ..\build_inf
#    nmake /A CFG="build_inf - Win32 Release"
#    cd ..\driver
#    copy ..\build_inf\Release\build_inf.exe .\build_inf.exe
#    copy ..\build_inf\Release\build_inf.exe .\Dist\tools\build_inf.exe
    .\build_inf.exe --ver_h > uniata_ver.h
    .\Dist\tools\srchrep.exe -e -src "\r\n" -dest "\n" "uniata_ver.h"
#!MESSAGE  "$(INTDIR)\IdeDma.sys"  "$(ALTDIR)\IdeDma.sys"
!ENDIF 

"$(INTDIR)\$(SRC).res" : "$(SRC).rc" "$(INTDIR)" "uniata_ver.h" "uataerr.h" "build_inf.exe"
    -@erase ".\$(SRC).res"
    $(RSC) $(RSC_PROJ) "$(SRC).rc"
    -@erase "$(INTDIR)\$(SRC).res"
    -@move "$(SRC).res" "$(INTDIR)\$(SRC).res"

