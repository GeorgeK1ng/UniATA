@echo off
setlocal EnableExtensions

if "%~3"=="" goto :usage
if not defined DDKROOT (
  echo DDKROOT is not set. 1>&2
  exit /b 2
)

set "BUILD_ARCH=%~1"
set "BUILD_TARGET=%~2"
set "ARTIFACT_NAME=%~3"

if /I "%BUILD_ARCH%"=="x86" goto :architecture_ok
if /I "%BUILD_ARCH%"=="x64" goto :architecture_ok
echo Unsupported architecture "%BUILD_ARCH%". Use x86 or x64. 1>&2
exit /b 2

:architecture_ok
echo Configuring WDK at "%DDKROOT%" for %BUILD_ARCH% %BUILD_TARGET%...
call "%DDKROOT%\bin\setenv.bat" "%DDKROOT%" fre %BUILD_ARCH% %BUILD_TARGET%
if not errorlevel 1 goto :configured
set "BUILD_ERROR=%ERRORLEVEL%"
echo WDK setenv.bat failed with exit code %BUILD_ERROR%. 1>&2
exit /b %BUILD_ERROR%

:configured
rem The checked-in makefile predates WDK 7.1. The environment aliases below
rem let it consume the compiler, headers and libraries selected by SetEnv.
set "BASEDIR=%BASEDIR%"
set "BASEDIRXP=%BASEDIR%"
set "NO_BUILD_INF=1"
set "NO_BUILD_CMD=1"

pushd driver
nmake /nologo /f idedma.mak CFG="UniATA - Win32 Release" ARCH=%BUILD_ARCH% /A
if not errorlevel 1 goto :built
set "BUILD_ERROR=%ERRORLEVEL%"
popd
exit /b %BUILD_ERROR%

:built
popd

set "OUTPUT_DIR=driver\Release"
if not exist "%OUTPUT_DIR%\IdeDma.sys" (
  echo Build completed without producing %OUTPUT_DIR%\IdeDma.sys. 1>&2
  exit /b 1
)

set "ARTIFACT_DIR=artifact\%ARTIFACT_NAME%"
if not exist "%ARTIFACT_DIR%" mkdir "%ARTIFACT_DIR%"
copy /y "%OUTPUT_DIR%\IdeDma.sys" "%ARTIFACT_DIR%\IdeDma.sys" >nul
if exist "%OUTPUT_DIR%\IdeDma.pdb" copy /y "%OUTPUT_DIR%\IdeDma.pdb" "%ARTIFACT_DIR%\IdeDma.pdb" >nul
exit /b 0

:usage
echo Usage: %~nx0 ^<x86^|x64^> ^<WXP^|WNET^|WLH^|WIN7^> ^<artifact-name^> 1>&2
exit /b 2
