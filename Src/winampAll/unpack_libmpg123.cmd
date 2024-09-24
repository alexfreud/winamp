@if not exist "c:\Program Files\7-Zip\7z.exe" goto missing7-Zip

SET PATH=c:\Program Files\7-Zip\;%PATH%

SET LIBMPG123FILENAME=libmpg123.7z
SET LIBMPG123FOLDER=libmpg123
SET MPG123=mpg123
SET VERSION=1.25.13
SET MACHINEX86=x86
SET MACHINEX64=x64
SET DEBUG=debug
SET RELEASE=release

@echo ************************************
@echo * Unpack libmpg123 (%LIBMPG123FILENAME%)
@echo ************************************
@if exist ..\%LIBMPG123FOLDER% goto existlibmpg123

call 7z.exe x -y %LIBMPG123FILENAME% -o../

cd ../%LIBMPG123FOLDER%/%MPG123%-%VERSION%-%MACHINEX86%-%DEBUG%
call create_libmpg123_lib_file_x86_debug.cmd

cd ../%MPG123%-%VERSION%-%MACHINEX86%-%RELEASE%
call create_libmpg123_lib_file_x86_release.cmd

cd ../%MPG123%-%VERSION%-%MACHINEX64%-%DEBUG%
call create_libmpg123_lib_file_x64_debug.cmd

cd ../%MPG123%-%VERSION%-%MACHINEX64%-%RELEASE%
call create_libmpg123_lib_file_x64_release.cmd

@pause
goto :exit

:missing7-Zip
@echo 7-Zip archive tool not detected.
@pause
@exit
goto :eof


:existlibmpg123
@echo libmpg123 folder already exists. Unpack operation is aborted.
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%
