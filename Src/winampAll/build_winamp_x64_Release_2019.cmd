@if not exist "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" goto missingVisualStudio2019x64

@echo **************************************
@echo * Build winampAll_2019.sln Release x64
@echo ************************************** 
call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
@@msbuild winampAll_2019.sln /p:configuration=Release /p:Platform="x64"

@pause
goto :exit

:missingVisualStudio2019x64
@echo Microsoft Visual Studio 2019 for x64 configuration not detected
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%