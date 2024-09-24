@echo off

if not defined WAPROJECTS set WAPROJECTS=c:\projects
if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

set KEYFILE=%CURSANDBOX%\codesign\nullsoft_key_15_mar_2011_private.pfx
%CURSANDBOX%\codesign\signtool.exe sign /p b05allisonZer0G /f "%KEYFILE%" /d %1 /du "http://www.winamp.com" /t http://timestamp.verisign.com/scripts/timstamp.dll /v %2
SET errCode=%ERRORLEVEL%

IF %errCode% NEQ 0 @echo SimpleSign Failed

exit /B %errCode%