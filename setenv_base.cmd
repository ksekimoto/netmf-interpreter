@echo off

set PORT_BUILD=
set NO_ADS_WRAPPER=
SET COMPILER_TOOL=

if NOT "%1"=="" GOTO :ARGSOK
if NOT "%2"=="" GOTO :ARGSOK

:ERROR
@echo.
@echo Error: Invalid Arguments!
@echo.
@echo Usage: setenv_base COMPILER_TOOL COMPILER_TOOL_VERSION
@echo     where  COMPILER_TOOL         is BLACKFIN, GCC, GCCOP, ADS, RVDS, MDK, SHC, VS
@echo            COMPILER_TOOL_VERSION is the version of the compiler
@echo
@echo     e.g.  setenv.cmd GCC 4.2.1
@echo.
GOTO :EOF


:ARGSOK

SET COMPILER_TOOL=%1
SET COMPILER_TOOL_VERSION_NUM=%2
SET COMPILER_TOOL_VERSION=%1%2
SET "ARG3=%~3"
@rem SET ARG3=%~3

SET TFSCONFIG=MFConfig.xml

IF /I NOT "%COMPILER_TOOL%"=="RVDS" IF /I NOT "%COMPILER_TOOL%"=="ADS" set NO_ADS_WRAPPER=1

@ECHO Compiler: %COMPILER_TOOL% %COMPILER_TOOL_VERSION_NUM% %ARG3%

SET CURRENTCD=%CD%
SET VSCMD_START_DIR=%CD%

CALL "%~dp0\tools\scripts\init.cmd"

rem @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

SET SPOCLIENT=%CLRROOT%
pushd %SPOCLIENT%\..
SET SPOROOT=%CD%
popd

@REM - Load the user aliases if alias.txt exists
if EXIST alias.txt DOSKEY /macrofile=alias.txt

set NetMfTargetsBaseDir=%SPOCLIENT%\Framework\IDE\Targets\

set _SDROOT_=%SPOROOT:current=%
if "%_SDROOT_:~-1%" == "\" set _SDROOT_=%_SDROOT_:~0,-1%

rem @ make sure we start with a clean path
if "%DOTNETMF_OLD_PATH%"=="" goto :save_current_path
goto :restore_path_from_old

:save_current_path
set DOTNETMF_OLD_PATH=%PATH%
goto :after_path_saved_or_restored

:restore_path_from_old
set PATH=%DOTNETMF_OLD_PATH%

:after_path_saved_or_restored

set PATH=%SPOROOT%\bin;%PATH%
set PATH=%SPOROOT%\tools\x86\perl\bin;%SPOROOT%\tools\x86\bin;%CLRROOT%\tools\bin;%PATH%;%CLRROOT%\tools\scripts
set PATH=%CLRROOT%\BuildOutput\Public\%FLAVOR_WIN%\Test\Server\dll;%PATH%

cd %CURRENTCD%

set CURRENTCD=

@rem IF /I NOT "%COMPILER_TOOL%" == "VS" (
@rem     IF NOT "%VS150COMNTOOLS%" == "" (
@rem         ECHO "VsDevCmd.bat is called"
@rem         CALL %VS150COMNTOOLS%VsDevCmd.bat
@rem         ECHO "VsDevCmd.bat end"
@rem     ) ELSE (    
@rem         @ECHO WARNING: Could not find VsDevCmd.bat.
@rem         @ECHO WARNING: VISUAL STUDIO 2017 DOES NOT APPEAR TO BE INSTALLED ON THIS MACHINE
@rem         @ECHO WARNING: MAKE SURE YOU ARE RUNNING FROM THE DEVELOPER COMMAND PROMPT
@rem         GOTO :EOF
@rem     )
@rem )

IF /I "%COMPILER_TOOL%"=="VS"       GOTO :SET_TOOCHAIN

IF "%VS150COMNTOOLS%" == ""         GOTO :VS_ERR

@ECHO "VsDevCmd.bat is called"
CALL "%VS150COMNTOOLS%VsDevCmd.bat"
@ECHO "VsDevCmd.bat end"
GOTO :SET_TOOLCHAIN

:VS_ERR
@ECHO WARNING: Could not find VsDevCmd.bat.
@ECHO WARNING: VISUAL STUDIO 2017 DOES NOT APPEAR TO BE INSTALLED ON THIS MACHINE
@ECHO WARNING: MAKE SURE YOU ARE RUNNING FROM THE DEVELOPER COMMAND PROMPT
GOTO :EOF

:SET_TOOLCHAIN
set TINYCLR_USE_MSBUILD=1   

Title MF %FLAVOR_WIN% (%COMPILER_TOOL% %COMPILER_TOOL_VERSION_NUM%)
rem @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
rem set tool-chain specific environment variables 

IF /I "%COMPILER_TOOL%"=="VS"       GOTO :SET_VS_VARS 
IF /I "%COMPILER_TOOL%"=="GCC"      GOTO :SET_GCC_VARS 
IF /I "%COMPILER_TOOL%"=="MDK"      GOTO :SET_MDK_VARS

IF "%COMPILER_TOOL%"=="" GOTO :ERROR

GOTO :EOF

rem @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
:SET_GCC_VARS
@echo setting vars for GCC compiler %COMPILER_TOOL_VERSION%

rem use a default for GCC
IF "%ARG3%"=="" (
    SET "ARG3=%SystemDrive%\gnu\gcc"
    IF NOT EXIST "%ARG3%" set "ARG3=%ProgramFiles(x86)%\GNU Tools ARM Embedded\4.9 2015q1"
)
IF NOT EXIST "%ARG3%" GOTO :BAD_GCC_ARG

set ARMROOT=
set ADS_TOOLS_BIN=
set NO_ADS_WRAPPER=1
set GNU_VERSION=%COMPILER_TOOL_VERSION_NUM%
SET COMPILER_TOOL_VERSION_NUM=%COMPILER_TOOL_VERSION_NUM:~0,3%
SET COMPILER_TOOL_VERSION=%COMPILER_TOOL%%COMPILER_TOOL_VERSION_NUM:~0,3%
set DOTNETMF_COMPILER=%COMPILER_TOOL_VERSION%

IF "%4" == "SH" GOTO :SHGCC
IF "%4" == "RX" GOTO :RXGCC
IF "%4" == "RZ" GOTO :RZGCC
IF "%4" == "KPIT_RZ" GOTO :KPIT_RZGCC
IF "%4" == "H8" GOTO :H8GCC

IF /I "%COMPILER_TOOL%"=="GCC" (
IF EXIST "%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%" (
set "ARMINC=%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%\include"
set "ARMLIB=%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%"
set "GNU_TOOLS=%ARG3%"
set "GNU_TOOLS_BIN=%ARG3%\bin"
set GNU_TARGET=arm-none-eabi
) ELSE (
@ECHO Could not find "%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%"
GOTO :BAD_GCC_ARG
))
GOTO :GCC_END

:SHGCC
set COMPILER_TOOL=GCC
IF EXIST "%ARG3%\lib\gcc\sh-elf\%GNU_VERSION%" (
set SHINC=%ARG3%\lib\gcc\sh-elf\%GNU_VERSION%\include
set SHLIB=%ARG3%\lib\gcc\sh-elf\%GNU_VERSION%
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=sh-elf
) ELSE (
@ECHO Could not find %ARG3%\lib\gcc\sh-elf\%GNU_VERSION%
GOTO :BAD_GCC_ARG
)
GOTO :GCC_END

:RXGCC
set COMPILER_TOOL=GCC
IF EXIST "%ARG3%\lib\gcc\rx-elf\%GNU_VERSION%" (
set RXINC=%ARG3%\lib\gcc\rx-elf\%GNU_VERSION%\include
set RXLIB=%ARG3%\lib\gcc\rx-elf\%GNU_VERSION%
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=rx-elf
) ELSE (
@ECHO Could not find %ARG3%\lib\gcc\rx-elf\%GNU_VERSION%
GOTO :BAD_GCC_ARG
)
GOTO :GCC_END

:RZGCC
set COMPILER_TOOL=GCC
set PATH=%ARG3%\bin;%PATH%
IF EXIST "%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%" (
@rem set ARMINC=%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%\include
set ARMINC=%ARG3%\include
set ARMLIB=%ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=arm-none-eabi
) ELSE (
@ECHO Could not find %ARG3%\lib\gcc\arm-none-eabi\%GNU_VERSION%
GOTO :BAD_GCC_ARG
)
GOTO :GCC_END

:KPIT_RZGCC
set COMPILER_TOOL=GCC
set PATH=%ARG3%\bin;%PATH%
IF EXIST "%ARG3%\lib\gcc\arm-rz-eabi\%GNU_VERSION%" (
@rem set RZINC=%ARG3%\lib\gcc\arm-rz-eabi\%GNU_VERSION%\include
set RZINC=%ARG3%\include
set RZLIB=%ARG3%\lib\gcc\arm-rz-eabi\%GNU_VERSION%
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=arm-rz-eabi
) ELSE (
@ECHO Could not find %ARG3%\lib\gcc\arm-rz-eabi\%GNU_VERSION%
GOTO :BAD_GCC_ARG
)
GOTO :GCC_END

:H8GCC
set COMPILER_TOOL=GCC
IF EXIST "%ARG3%\lib\gcc\h8300-elf\%GNU_VERSION%" (
set H8INC=%ARG3%\lib\gcc\h8300-elf\%GNU_VERSION%\include
set H8LIB=%ARG3%\lib\gcc\h8300-elf\%GNU_VERSION%\h8300h
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=h8300-elf
) ELSE (
@ECHO Could not find %ARG3%\lib\gcc\h8300-elf\%GNU_VERSION%
GOTO :BAD_GCC_ARG
)
GOTO :GCC_END

IF /I "%COMPILER_TOOL%"=="GCCOP" (
IF EXIST "%ARG3%\include\elips_bs" (

set ARMINC=%ARG3%\include\elips_bs
set ARMLIB=%ARG3%\lib
set GNU_TOOLS=%ARG3%
set GNU_TOOLS_BIN=%ARG3%\bin
set GNU_TARGET=arm-elf
set COMPILER_PATH=%ARG3%

) ELSE (
@ECHO Could not find %ARG3%\include\elips_bs
GOTO :BAD_GCC_ARG
))

:GCC_END
GOTO :EOF

:BAD_GCC_ARG
@ECHO.
@ECHO Error - Invalid argument (%ARG3%).  Usage: setenv_GCC.cmd GCC_VERSION GCC_TOOL_PATH
@ECHO         Example:  setenv_gcc.cmd 4.2.1 c:\tools\gcc
@ECHO.

GOTO :EOF

rem @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
:SET_MDK_VARS
@ECHO Setting MDK env var and path for version %COMPILER_TOOL_VERSION%

rem use a default for MDK
set NO_ADS_WRAPPER=1
set DOTNETMF_COMPILER=%COMPILER_TOOL_VERSION%

IF "%ARG3%"=="" set ARG3=%SystemDrive%\Keil_v5\ARM
IF NOT EXIST "%ARG3%" set ARG3=%SystemDrive%\Keil\ARM
IF NOT EXIST "%ARG3%" GOTO :BAD_MDK_ARG

SET MDK_TOOL_PATH=%ARG3%
SET PATH=%MDK_TOOL_PATH%;%PATH%

SET MDK_EXT=%COMPILER_TOOL_VERSION_NUM:~0,1%
SET MDK_SUB_EXT=%COMPILER_TOOL_VERSION_NUM:~2,2%

IF "%MDK_EXT%"=="3" SET MDK_EXT=31
IF "%MDK_EXT%"=="4" (
  IF "%MDK_SUB_EXT%"=="54" (
    SET MDK_EXT=40 
  ) ELSE (
    SET MDK_EXT=31 
  )
)

IF "%MDK_EXT%"=="" (
  @echo Unsupported MDK version %COMPILER_TOOL_VERSION_NUM%
  GOTO :BAD_MDK_ARG
)

SET RVCT%MDK_EXT%BIN=%MDK_TOOL_PATH%\ARM\BIN%MDK_EXT%
SET RVCT31LIB=%MDK_TOOL_PATH%\RV31\LIB
SET RVCT31INC=%MDK_TOOL_PATH%\RV31\INC

GOTO :EOF

:BAD_MDK_ARG
@ECHO.
@ECHO Error - Invalid argument.  Could not find MDK path %ARG3%.
@ECHO.

GOTO :EOF

rem @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
:SET_VS_VARS
set VSSDK12INSTALLDIR=%SPOROOT%\tools\x86\MicrosoftSDKs\VSSDK\vs12\
if NOT EXIST "%VSSDK12INSTALLDIR%" set VSSDK12INSTALLDIR=%VSSDK120Install%

set VSSDK14INSTALLDIR=%SPOROOT%\tools\x86\MicrosoftSDKs\VSSDK\vs14\
if NOT EXIST "%VSSDK14INSTALLDIR%" set VSSDK14INSTALLDIR=%VSSDK140Install%

set VSSDK15INSTALLDIR=%SPOROOT%\tools\x86\MicrosoftSDKs\VSSDK\vs15\
if NOT EXIST "%VSSDK15INSTALLDIR%" set VSSDK15INSTALLDIR=%VSSDK150Install%

set NO_ADS_WRAPPER=1
set DOTNETMF_COMPILER=%COMPILER_TOOL_VERSION%

IF "%COMPILER_TOOL_VERSION_NUM%"=="15" (
  IF "" == "%VS150COMNTOOLS%" GOTO BAD_VS_ARG
  CALL "%VS150COMNTOOLS%VsDevCmd.bat"
  GOTO :EOF
)

:BAD_VS_ARG
@ECHO.
@ECHO Error - Invalid argument.
@ECHO Could not find VsDevCmd.bat for VS%COMPILER_TOOL_VERSION_NUM%
@ECHO.

GOTO :EOF
