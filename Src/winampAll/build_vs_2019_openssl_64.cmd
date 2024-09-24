@if not exist "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" goto missingVisualStudio2019x64
@if not exist "c:\Program Files\7-Zip\7z.exe" goto missing7-Zip
@if not exist "c:\Strawberry\perl\bin\perl.exe" goto missingPerl
@if not exist "c:\Program Files\NASM\nasm.exe" goto missingNASM

SET PATH=c:\Program Files\7-Zip\;c:\Strawberry\perl\bin\;c:\Program Files\NASM\;%PATH%
@if not exist "c:\OpenSSL\" mkdir "c:\OpenSSL" 
SET OPENSSLFILENAME=openssl-1.0.1u.7z
SET PLATFORM=x64
SET OPENSSLFOLDER=openssl-1.0.1u

call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

@echo ************************************
@echo * Build OpenSSL (%OPENSSLFILENAME%) Debug x64
@echo ************************************
@if exist %OPENSSLFOLDER%%PLATFORM% (
	@echo Delete temporary '%OPENSSLFOLDER%%PLATFORM%' folder
	@call rd /S /Q %OPENSSLFOLDER%%PLATFORM% 
	)
call 7z.exe x -y %OPENSSLFILENAME% -o%OPENSSLFOLDER%%PLATFORM%
cd %OPENSSLFOLDER%%PLATFORM%
cd %OPENSSLFOLDER%
@ perl Configure debug-VC-WIN64A no-shared --prefix=c:\OpenSSL\Debug_x64_static
call ms\do_nasm
call ms\do_win64a
nmake -f ms\nt.mak 
nmake -f ms\nt.mak install
call xcopy /Y /D tmp32.dbg\lib.pdb c:\OpenSSL\Debug_x64_static\lib\

@echo **************************************
@echo * Build OpenSSL (%OPENSSLFILENAME%) Release x64
@echo ************************************** 
@if exist %OPENSSLFOLDER%%PLATFORM% (
	@echo Delete temporary '%OPENSSLFOLDER%%PLATFORM%' folder
	@call rd /S /Q %OPENSSLFOLDER%%PLATFORM% 
	)
call 7z.exe x -y %OPENSSLFILENAME% -o%OPENSSLFOLDER%%PLATFORM%
cd %OPENSSLFOLDER%%PLATFORM%
cd %OPENSSLFOLDER%
@ perl Configure VC-WIN64A no-shared --prefix=c:\OpenSSL\Release_x64_static
call ms\do_nasm
call ms\do_win64a
nmake -f ms\nt.mak 
nmake -f ms\nt.mak install
call xcopy /Y /D tmp32\lib.pdb c:\OpenSSL\Release_x64_static\lib\

@pause
goto :exit

:missing7-Zip
@echo 7-Zip archive tool not detected
@pause
@exit
goto :eof

:missingPerl
@echo Perl interpretator not detected
@pause
@exit
goto :eof

:missingVisualStudio2019x64
@echo Microsoft Visual Studio 2019 for x64 configuration not detected
@pause
@exit
goto :eof

:missingNASM
@echo Microsoft NASM assembler not detected
@pause
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%
