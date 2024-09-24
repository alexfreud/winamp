@if not exist "c:\Program Files\7-Zip\7z.exe" goto missing7-Zip

SET PATH=c:\Program Files\7-Zip\;%PATH%

SET LIBVPXFILENAME=libvpx_v1.8.2_msvc16.7z
SET LIBVPXFOLDER=libvpx_v1.8.2_msvc16

@echo ************************************
@echo * Unpack libvpx_v1.8.2 for VS 2019 (%LIBVPXFILENAME%)
@echo ************************************
@if exist ..\%LIBVPXFOLDER% goto existlibvpx

call 7z.exe x -y %LIBVPXFILENAME% -o../

@pause
goto :exit

:missing7-Zip
@echo 7-Zip archive tool not detected.
@pause
@exit
goto :eof


:existlibvpx
@echo libvpx_v1.8.2 for VS 2019 folder already exists. Unpack operation is aborted.
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%