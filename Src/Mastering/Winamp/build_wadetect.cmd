@echo off

if not defined MAKENSIS set MAKENSIS=C:\Program Files\NSIS\UNICODE\makensis.exe
if not defined WAPROJECTS set WAPROJECTS=c:\projects
if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

set MAKENSIS_COMMON_PARAM=/V1 /P4 /DLZMA /DLANG_USE_ALL
set SCRIPT=%CURSANDBOX%\installer\browserplugin\main.nsi

@echo.
@echo.
@echo Executing Makensis for Winamp Detect [Lang = '%%i'] 
@echo.
@echo.
  
@echo "%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"
"%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"

if "%errorlevel%" EQU "0" (
  if /I "%BUILDTYPE%" == "final" (
	@echo.
    @echo.

    set SIGNNAME=Winamp Detect
	set INSTALLERNAME=installWaDetect

    @echo Signing [File='!INSTALLERNAME!_%%i.exe']
    @echo.
    @echo.
     
	call "%CURSANDBOX%\Mastering\Winamp\simple_sign.cmd" "!SIGNNAME!" "%CURSANDBOX%\installer\browserplugin\!INSTALLERNAME!_%%i.exe"
  )
) else ( 
  @echo Makensis Failed 
)


