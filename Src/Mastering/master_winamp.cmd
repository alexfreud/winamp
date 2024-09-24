@echo off

if %1()==() goto help
if %1==BETA goto start
if %1==QA goto start
if %1==FINAL goto start
if %1==NIGHT goto start
goto end

:start


echo Configuring Kerberos...
call "d:\kerblogin\kerblogin.cmd"


if %1==QA SET BUILDTYPE=qa
if %1==BETA SET BUILDTYPE=beta
if %1==FINAL SET BUILDTYPE=final
if %1==NIGHT SET BUILDTYPE=night

echo Setting VC 9.0 environments...
call "C:\Program Files\Intel\IPP\6.1.3.047\ia32\tools\env\ippenv.bat"
call %DXSDK_DIR%\Utilities\Bin\dx_setenv.cmd x86
REM call "D:\SDKs\Windows\v7.0\Bin\SetEnv.cmd" /XP /Release
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
if NOT %1==FINAL set LINK=%LINK% /DEBUG 
set CL=%CL% /D_CRT_SECURE_NO_WARNINGS
set PATH=%path%;C:\program files\nsis\unicode

SET TARGET_ARCH=x86
if %3()==() goto skip64
if %3==x86 goto skip64
SET TARGET_ARCH=x64
call "D:\SDKs\Windows\v7.0\Bin\setenv.cmd" /x64 /release
call "d:\dxsdk\dx_setenv.cmd" AMD64
set CL=%CL% /GS- /I "d:\dxsdk\Include"
set LINK=%LINK% /machine:AMD64
goto skip32
:skip64
REM call "D:\SDKs\Windows\v7.0\Bin\SetEnv.cmd" /XP /Release

:skip32

title Nullsoft Winamp build script -= %1 =-

if NOT %2()==() SET BRANDING=%2
if NOT DEFINED BRANDING SET BRANDING=NULLSOFT

echo BRANDING=%BRANDING%


echo Preparing folders...
if exist ts.txt del ts.txt
nstimestamp\nstimestamp>ts.txt
set /p TIMESTAMP=<ts.txt
del ts.txt

f:
cd \
set SANDBOX=f:\sandbox
set CURSANDBOX=%SANDBOX%\%TIMESTAMP%
echo Using %CURSANDBOX% for Sandbox
set PROGRAMFILES=%CURSANDBOX%\Output
set ROOTDIR=%CURSANDBOX%

if NOT exist "%SANDBOX%" mkdir "%SANDBOX%"
if errorlevel 1 goto SANDBOX_ERROR 

if exist "%CURSANDBOX%" rd /s /q "%CURSANDBOX%"
mkdir "%CURSANDBOX%"
if errorlevel 1 goto SANDBOX_ERROR 

if exist "%PROGRAMFILES%" rd /s /q "%PROGRAMFILES%"
mkdir "%PROGRAMFILES%"
if errorlevel 1 goto SANDBOX_ERROR 

SET INSTALLER_LANG=Mastering\Winamp\installer_%BUILDTYPE%.lang
SET INSTALLER_CONFIG=Mastering\Winamp\installer_%BUILDTYPE%.config


echo Retrieving mastering scripts from cvs...

if "%TARGET_ARCH%"=="x64" goto master64
echo using x86 mastering script
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\master_winamp_%BUILDTYPE%.xml" >nul
if errorlevel 1 goto CVS_ERROR_1
goto master86
:master64
echo using x64 mastering script
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\master_winamp_%BUILDTYPE%_x64.xml" >nul
if errorlevel 1 goto CVS_ERROR_1
:master86
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_winamp_%BUILDTYPE%.xml" >nul
if errorlevel 1 goto CVS_ERROR_2
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\configmap.xml"  >nul
if errorlevel 1 goto CVS_ERROR_3
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\version_%BUILDTYPE%.info"  >nul
if errorlevel 1 goto CVS_ERROR_4
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\VerCtrl\verctrl.exe" >nul
if errorlevel 1 goto CVS_ERROR_5
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\constants.h" >nul
if errorlevel 1 goto CVS_ERROR_6
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\buildnumber.h" >nul
if errorlevel 1 goto CVS_ERROR_7
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\talkback.ini" >nul
if errorlevel 1 goto CVS_ERROR_7
if NOT %1==NIGHT cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\set_tags_%BUILDTYPE%.cmd" >nul
if errorlevel 1 goto CVS_ERROR_8
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\fileNames.cmd" >nul
if errorlevel 1 goto CVS_ERROR_9
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_installer.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_wbm.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\rebase.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_webdev.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_orb.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\build_wadetect.cmd" >nul
if errorlevel 1 goto CVS_ERROR_10
cvs checkout  -N -d "%CURSANDBOX%"   "%INSTALLER_LANG%" >nul
if errorlevel 1 goto CVS_ERROR_11
cvs checkout  -N -d "%CURSANDBOX%"   "%INSTALLER_CONFIG%" >nul
if errorlevel 1 goto CVS_ERROR_12

cvs checkout  -N -d "%CURSANDBOX%"   "codesign\signcode-pwd.exe" >nul
cvs checkout  -N -d "%CURSANDBOX%"   "Mastering\Winamp\simple_sign.cmd" >nul

f:
cd \
echo Patching version number...
cd "%CURSANDBOX%\Mastering\Winamp"
copy "version_%BUILDTYPE%.info"  "version.info"
for /f "tokens=1-3" %%a in (version.info) do set %%a%%b%%c
cd "%CURSANDBOX%\Mastering\VerCtrl" 
verctrl.exe %1 %BRANDING% INC > nul
SET ERRORLEVEL=0

cd "%CURSANDBOX%\Mastering\Winamp" 
cvs commit -m"version patch" "constants.h" > nul
if errorlevel 1 goto CVS_ERROR_COMMIT

if NOT %1==NIGHT echo Setting Tags...
if NOT %1==NIGHT call "%CURSANDBOX%\Mastering\Winamp\set_tags_%BUILDTYPE%.cmd" 
if errorlevel 1 goto SET_TAGS_ERROR

SET GIT_ASK_YESNO=false
git clone -b "%REPLICANT_GIT_BRANCH%" ssh://winamp@git.cm.aol.com/nullsoft/replicant.git "%CURSANDBOX%\replicant"
if errorlevel 1 goto GIT_ERROR

echo Setting Installer names...
call "%CURSANDBOX%\Mastering\Winamp\fileNames.cmd"
if errorlevel 1 goto SET_FILENAMES

if %1==QA echo Starting mastering script (build type: QA)
if %1==BETA echo Starting mastering script (build type: BETA)...
if %1==NIGHT echo Starting mastering script (build type: NIGHT)...
if %1==FINAL echo Starting mastering script (build type: FINAL)...

if "%TARGET_ARCH%"=="x64" goto build64
perl -S dgbuild.pl master_winamp_%BUILDTYPE%.xml > nul
goto build86
:build64
perl -S dgbuild.pl master_winamp_%BUILDTYPE%_x64.xml > nul
:build86


if errorlevel 1 goto BUILD_ERROR

echo Build done. Check http://nulldev.stream.aol.com/builds/default.asp for result.
goto END

:SANDBOX_ERROR
echo Error!!! Unable to prepare folder '%CURSANDBOX%'
goto END

:OUTPUT_ERROR
echo Error!!! Unable to prepare folder '%PROGRAMFILES%'
goto END

:CVS_ERROR_LOGIN
echo Error!!! Unable to login into the cvs
goto END

:CVS_ERROR_1
echo Error!!! Unable to checkout 'Mastering/Winamp/master_winamp.xml'
goto END

:CVS_ERROR_2
echo Error!!! Unable to checkout 'Mastering/Winamp/build_winamp.xml'
goto END

:CVS_ERROR_3
echo Error!!! Unable to checkout 'Mastering/Winamp/configmap.xml'
goto END

:CVS_ERROR_4
echo Error!!! Unable to checkout 'Mastering/Winamp/version.info'
goto END

:CVS_ERROR_5
echo Error!!! Unable to checkout 'Mastering/Winamp/verctrl.exe'
goto END

:CVS_ERROR_6
echo Error!!! Unable to checkout 'Mastering/Winamp/constants.h'
goto END

:CVS_ERROR_7
echo Error!!! Unable to checkout 'Mastering/Winamp/buildnumber.h'
goto END

:CVS_ERROR_8
echo Error!!! Unable to checkout 'Mastering/Winamp/set_tags_%BUILDTYPE%.cmd'
goto END

:CVS_ERROR_9
echo Error!!! Unable to checkout 'Mastering/Winamp/fileNames.cmd'
goto END
:CVS_ERROR_10
echo Error!!! Unable to checkout 'Mastering/Winamp/build_installer.cmd'
goto END
:CVS_ERROR_11
echo Error!!! Unable to checkout 'Mastering/Winamp/%INSTALLER_LANG%'
goto END
:CVS_ERROR_12
echo Error!!! Unable to checkout 'Mastering/Winamp/%INSTALLER_CONFIG%'
goto END

:CVS_ERROR_COMMIT
echo Error!!! Unable to commit 'Mastering/Winamp/constants.h'
goto END

:SET_TAGS_ERROR
echo Error!!! Unable to set tags.
goto END

:SET_FILENAMES
echo Error!!! Unable to set installer file names.
goto END

:BUILD_ERROR
echo Error!!! Build error.
goto END

:help 
echo Specify build type (BETA, NIGHT, FINAL, QA) 
goto end

:GIT_ERROR
echo Error!!! Unable to clone git repository
goto END

:END
exit