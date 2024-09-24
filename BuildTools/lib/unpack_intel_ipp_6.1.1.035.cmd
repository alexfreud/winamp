@if not exist "c:\Program Files\7-Zip\7z.exe" goto missing7-Zip

SET PATH=c:\Program Files\7-Zip\;%PATH%

SET LIBIPPFILENAME=intel_ipp_6.1.1.035.7z
SET LIBIPPFOLDER=intel_ipp_6.1.1.035

@echo ************************************
@echo * Unpack Intel IPP 6.1.1.035 for VS 2019 (%LIBIPPFILENAME%)
@echo ************************************
@if exist ..\%LIBIPPFOLDER% goto existlibipp

call 7z.exe x -y %LIBIPPFILENAME% -o../../../external_dependencies/

@pause
goto :exit

:missing7-Zip
@echo 7-Zip archive tool not detected.
@pause
@exit
goto :eof


:existlibipp
@echo intel_ipp_6.1.1.035 for VS 2019 folder already exists. Unpack operation is aborted.
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%