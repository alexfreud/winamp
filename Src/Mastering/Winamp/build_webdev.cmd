@echo off

if not defined MAKENSIS set MAKENSIS=C:\Program Files\NSIS\UNICODE\makensis.exe
if not defined WAPROJECTS set WAPROJECTS=c:\projects
if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

set MAKENSIS_COMMON_PARAM=/V1 /P4 /DUSE_MUI /DLZMA
set SCRIPT=%CURSANDBOX%\installer\webdev\webdev.nsi
    
@echo "%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"
"%MAKENSIS%" %MAKENSIS_COMMON_PARAM% "%SCRIPT%"

