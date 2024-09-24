@echo off

if not defined MAKENSIS set MAKENSIS=C:\Program Files\NSIS\UNICODE\makensis.exe
if not defined WAPROJECTS set WAPROJECTS=c:\projects
if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

set MAKENSIS_COMMON_PARAM=/V1 /P4
set SCRIPT=%CURSANDBOX%\installer\orb\orb.nsi
    
@echo "%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"
"%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"

@%CURSANDBOX%\Mastering\Winamp\simple_sign.cmd "Winamp Remote" "%CURSANDBOX%\installer\orb\orbembed.exe"