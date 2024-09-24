@echo off

REM        set SIGNNAME=Winamp %WINAMP_VERSION_MAJOR%.%WINAMP_VERSION_MINOR%%WINAMP_VERSION_MINOR_SECOND% %%~j

signtool.exe sign /f Winamp_SA.pfx /p WaWa!_59_!WaWa /t http://timestamp.digicert.com /du "http://www.winamp.com" /d "Winamp 5.9" /v Winamp59_9999_final_full_en-us.exe

REM        ___________________________________________________________________

REM if not defined WAPROJECTS set WAPROJECTS=c:\projects
REM if not defined CURSANDBOX set CURSANDBOX=%WAPROJECTS%

REM set KEYFILE=%CURSANDBOX%\codesign\WinampSA.pfx
REM %CURSANDBOX%\codesign\signtool.exe sign /f %KEYFILE% /p !Wa!Wa!5_6_7_8!Wa!Wa! /d %1 /du "http://www.winamp.com" /t http://timestamp.comodoca.com/authenticode /v %2

SET errCode=%ERRORLEVEL%

IF %errCode% NEQ 0 @echo SimpleSign Failed

pause /B %errCode%

exit
