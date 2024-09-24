@if not exist "c:\Program Files\7-Zip\7z.exe" goto missing7-Zip

SET PATH=c:\Program Files\7-Zip\;%PATH%

SET DIRECTXFILENAME=microsoft_directx_sdk_2010.7z
SET DIRECTXFOLDER=microsoft_directx_sdk_2010

@echo ************************************
@echo * Unpack Microsoft DirectX SDK (June 2010) (%DIRECTXFILENAME%)
@echo ************************************
@if exist ..\%DIRECTXFOLDER% goto existdirectx

call 7z.exe x -y %DIRECTXFILENAME% -o../../../external_dependencies/

@pause
goto :exit

:missing7-Zip
@echo 7-Zip archive tool not detected.
@pause
@exit
goto :eof


:existdirectx
@echo Microsoft DirectX SDK (June 2010) folder already exists. Unpack operation is aborted.
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%
