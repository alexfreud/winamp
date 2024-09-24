@echo off

if not defined MAKENSIS set MAKENSIS=C:\Program Files\NSIS\UNICODE\makensis.exe
if not defined WAPROJECTS set WAPROJECTS=c:\projects
if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

set MAKENSIS_COMMON_PARAM=/V1 /P4 /DUSE_MUI /DLZMA
if %TARGET_ARCH%==x64 set MAKENSIS_COMMON_PARAM=%MAKENSIS_COMMON_PARAM% /DWINAMP64
set SCRIPT=%CURSANDBOX%\installer\winamp\main.nsi

if not defined INSTALLER_LANG set INSTALLER_LANG=Mastering\Winamp\installer_beta.lang
if not defined INSTALLER_CONFIG set INSTALLER_CONFIG=Mastering\Winamp\installer_beta.config

SET WINAMP_VERSION_MAJOR=5
SET WINAMP_VERSION_MINOR=9
SET WINAMP_VERSION_MINOR_SECOND=0


for /F %%i in (%CURSANDBOX%\%INSTALLER_LANG%) do (

  set MAKENSIS_RUN_PARAM=  
  for /F "eol=; tokens=1,2,3,4,5,6,7,8,9 delims=," %%j in (%CURSANDBOX%\%INSTALLER_CONFIG%) do (m00stercow
  

    if /I "%%i" NEQ "all" (
      set MAKENSIS_RUN_PARAM=/DLANG_USE_%%i /DLANG=%%i 
    ) else (
      set MAKENSIS_RUN_PARAM=/DLANG_USE_%%i
    )
    if /I "%%k" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%k
    if /I "%%l" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%l
    if /I "%%m" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%m
    if /I "%%n" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%n
    if /I "%%o" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%o
    if /I "%%p" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%p
    if /I "%%q" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%q
    if /I "%%r" NEQ "" set MAKENSIS_RUN_PARAM=!MAKENSIS_RUN_PARAM! /D%%r

    @echo.
    @echo.
    @echo Executing Makensis [Lang = '%%i', Configuration = '%%~j'] 
    @echo.
    @echo.

    @echo "%MAKENSIS%" %MAKENSIS_COMMON_PARAM% !MAKENSIS_RUN_PARAM! "%SCRIPT%"
    "%MAKENSIS%" %MAKENSIS_COMMON_PARAM% !MAKENSIS_RUN_PARAM! "%SCRIPT%"

    if "%errorlevel%" EQU "0" (
        @echo.
        @echo.

        set SIGNNAME=Winamp %WINAMP_VERSION_MAJOR%.%WINAMP_VERSION_MINOR%%WINAMP_VERSION_MINOR_SECOND% %%~j

        if /I "%%k" EQU "lite" ( set INSTALLERNAME=%INSTALL_LITE%
        ) else ( if /I "%%k" == "std" ( set INSTALLERNAME=%INSTALL_STD%
        ) else ( if /I "%%k" == "full" (
            if /I "%%l" == "pro" ( set INSTALLERNAME=%INSTALL_PRO%
            ) else ( if /I "%%l" == "bundle" ( set INSTALLERNAME=%INSTALL_BUNDLE%
            ) else ( if /I "%%l" == "eMusic-7plus" ( set INSTALLERNAME=%INSTALL_EMUSIC%
            ) else ( set INSTALLERNAME=%INSTALL_FULL%)))
          )
        ))
        
        @echo Signing [Configuration='%~2', File='!INSTALLERNAME!_%%i.exe']
        @echo.
        @echo.

        call "%CURSANDBOX%\Mastering\Winamp\simple_sign.cmd" "!SIGNNAME!" "%CURSANDBOX%\installer\winamp\!INSTALLERNAME!_%%i.exe"
    ) else ( 
      @echo Makensis Failed 
    )
  )
)
