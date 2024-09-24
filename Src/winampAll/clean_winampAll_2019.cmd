@echo *******************************
@echo * Delete Release, Debug folders and files for winampAll_2019.sln
@echo *******************************
@echo -------------------------------
@echo *******************************
@echo * winampAll project
@echo *******************************
@if exist .vs (
	@echo Delete .vs
	@call rd /S /Q .vs
	)
@echo -------------------------------
@echo *******************************
@echo * aacdec-mft project
@echo *******************************
@if exist ..\aacdec-mft\x86_Debug (
	@echo Delete ..\aacdec-mft\x86_Debug
	@call rd /S /Q ..\aacdec-mft\x86_Debug
	)
@if exist ..\aacdec-mft\x86_Release (
	@echo Delete ..\aacdec-mft\x86_Release
	@call rd /S /Q ..\aacdec-mft\x86_Release
	)	
@if exist ..\aacdec-mft\x64_Debug (
	@echo Delete ..\aacdec-mft\x64_Debug
	@call rd /S /Q ..\aacdec-mft\x64_Debug
	)
@if exist ..\aacdec-mft\x64_Release (
	@echo Delete ..\aacdec-mft\x64_Release
	@call rd /S /Q ..\aacdec-mft\x64_Release
	)
@if exist ..\aacdec-mft\.vs (
	@echo Delete ..\aacdec-mft\.vs
	@call rd /S /Q ..\aacdec-mft\.vs
	)
@if exist ..\aacdec-mft\*.user (
	@echo Delete ..\aacdec-mft\*.user
	@call del ..\aacdec-mft\*.user
	)
@if exist ..\aacdec-mft\*.filters (
	@echo Delete ..\aacdec-mft\*.filters
	@call del ..\aacdec-mft\*.filters
	)
@if exist ..\aacdec-mft\*.htm (
	@echo Delete ..\aacdec-mft\*.htm
	@call del ..\aacdec-mft\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * adpcm project
@echo *******************************
@if exist ..\adpcm\x86_Debug (
	@echo Delete ..\adpcm\x86_Debug
	@call rd /S /Q ..\adpcm\x86_Debug
	)
@if exist ..\adpcm\x86_Release (
	@echo Delete ..\adpcm\x86_Release
	@call rd /S /Q ..\adpcm\x86_Release
	)	
@if exist ..\adpcm\x64_Debug (
	@echo Delete ..\adpcm\x64_Debug
	@call rd /S /Q ..\adpcm\x64_Debug
	)
@if exist ..\adpcm\x64_Release (
	@echo Delete ..\adpcm\x64_Release
	@call rd /S /Q ..\adpcm\x64_Release
	)
@if exist ..\adpcm\.vs (
	@echo Delete ..\adpcm\.vs
	@call rd /S /Q ..\adpcm\.vs
	)
@if exist ..\adpcm\*.user (
	@echo Delete ..\adpcm\*.user
	@call del ..\adpcm\*.user
	)
@if exist ..\adpcm\*.filters (
	@echo Delete ..\adpcm\*.filters
	@call del ..\adpcm\*.filters
	)
@if exist ..\adpcm\*.htm (
	@echo Delete ..\adpcm\*.htm
	@call del ..\adpcm\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * alac project
@echo *******************************
@if exist ..\alac\x86_Debug (
	@echo Delete ..\alac\x86_Debug
	@call rd /S /Q ..\alac\x86_Debug
	)
@if exist ..\alac\x86_Release (
	@echo Delete ..\alac\x86_Release
	@call rd /S /Q ..\alac\x86_Release
	)
@if exist ..\alac\x64_Debug (
	@echo Delete ..\alac\x64_Debug
	@call rd /S /Q ..\alac\x64_Debug
	)
@if exist ..\alac\x64_Release (
	@echo Delete ..\alac\x64_Release
	@call rd /S /Q ..\alac\x64_Release
	)
@if exist ..\alac\.vs (
	@echo Delete ..\alac\.vs
	@call rd /S /Q ..\alac\.vs
	)
@if exist ..\alac\*.user (
	@echo Delete ..\alac\*.user
	@call del ..\alac\*.user
	)
@if exist ..\alac\*.filters (
	@echo Delete ..\alac\*.filters
	@call del ..\alac\*.filters
	)
@if exist ..\alac\*.htm (
	@echo Delete ..\alac\*.htm
	@call del ..\alac\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * albumart project
@echo *******************************
@if exist ..\albumart\x86_Debug (
	@echo Delete ..\albumart\x86_Debug
	@call rd /S /Q ..\albumart\x86_Debug
	)
@if exist ..\albumart\x86_Release (
	@echo Delete ..\albumart\x86_Release
	@call rd /S /Q ..\albumart\x86_Release
	)
@if exist ..\albumart\x64_Debug (
	@echo Delete ..\aalbumart\x64_Debug
	@call rd /S /Q ..\albumart\x64_Debug
	)
@if exist ..\albumart\x64_Release (
	@echo Delete ..\albumart\x64_Release
	@call rd /S /Q ..\albumart\x64_Release
	)
@if exist ..\albumart\.vs (
	@echo Delete ..\albumart\.vs
	@call rd /S /Q ..\albumart\.vs
	)
@if exist ..\albumart\*.user (
	@echo Delete ..\albumart\*.user
	@call del ..\albumart\*.user
	)
@if exist ..\albumart\*.filters (
	@echo Delete ..\albumart\*.filters
	@call del ..\albumart\*.filters
	)
@if exist ..\albumart\*.htm (
	@echo Delete ..\albumart\*.htm
	@call del ..\albumart\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * apev2 project
@echo *******************************
@if exist ..\apev2\x86_Debug (
	@echo Delete ..\apev2\x86_Debug
	@call rd /S /Q ..\apev2\x86_Debug
	)
@if exist ..\apev2\x86_Release (
	@echo Delete ..\apev2\x86_Release
	@call rd /S /Q ..\apev2\x86_Release
	)
@if exist ..\apev2\x64_Debug (
	@echo Delete ..\apev2\x64_Debug
	@call rd /S /Q ..\apev2\x64_Debug
	)
@if exist ..\apev2\x64_Release (
	@echo Delete ..\apev2\x64_Release
	@call rd /S /Q ..\apev2\x64_Release
	)
@if exist ..\apev2\.vs (
	@echo Delete ..\apev2\.vs
	@call rd /S /Q ..\apev2\.vs
	)
@if exist ..\apev2\*.user (
	@echo Delete ..\apev2\*.user
	@call del ..\apev2\*.user
	)
@if exist ..\apev2\*.filters (
	@echo Delete ..\apev2\*.filters
	@call del ..\apev2\*.filters
	)
@if exist ..\apev2\*.htm (
	@echo Delete ..\apev2\*.htm
	@call del ..\apev2\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * auth project
@echo *******************************
@if exist ..\auth\x86_Debug (
	@echo Delete ..\auth\x86_Debug
	@call rd /S /Q ..\auth\x86_Debug
	)
@if exist ..\auth\x86_Release (
	@echo Delete ..\auth\x86_Release
	@call rd /S /Q ..\auth\x86_Release
	)
@if exist ..\auth\x64_Debug (
	@echo Delete ..\auth\x64_Debug
	@call rd /S /Q ..\auth\x64_Debug
	)
@if exist ..\auth\x64_Release (
	@echo Delete ..\auth\x64_Release
	@call rd /S /Q ..\auth\x64_Release
	)
@if exist ..\auth\.vs (
	@echo Delete ..\auth\.vs
	@call rd /S /Q ..\auth\.vs
	)
@if exist ..\auth\*.user (
	@echo Delete ..\auth\*.user
	@call del ..\auth\*.user
	)
@if exist ..\auth\*.filters (
	@echo Delete ..\auth\*.filters
	@call del ..\auth\*.filters
	)
@if exist ..\auth\*.htm (
	@echo Delete ..\auth\*.htm
	@call del ..\auth\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * bfc project
@echo *******************************
@if exist ..\Wasabi\bfc\x86_Debug (
	@echo Delete ..\Wasabi\bfc\x86_Debug
	@call rd /S /Q ..\Wasabi\bfc\x86_Debug
	)
@if exist ..\Wasabi\bfc\x86_Release (
	@echo Delete ..\Wasabi\bfc\x86_Release
	@call rd /S /Q ..\Wasabi\bfc\x86_Release
	)
@if exist ..\Wasabi\bfc\x64_Debug (
	@echo Delete ..\Wasabi\bfc\x64_Debug
	@call rd /S /Q ..\Wasabi\bfc\x64_Debug
	)
@if exist ..\Wasabi\bfc\x64_Release (
	@echo Delete ..\Wasabi\bfc\x64_Release
	@call rd /S /Q ..\Wasabi\bfc\x64_Release
	)
@if exist ..\Wasabi\bfc\.vs (
	@echo Delete ..\Wasabi\bfc\.vs
	@call rd /S /Q ..\Wasabi\bfc\.vs
	)
@if exist ..\Wasabi\bfc\*.user (
	@echo Delete ..\Wasabi\bfc\*.user
	@call del ..\Wasabi\bfc\*.user
	)
@if exist ..\Wasabi\bfc\*.filters (
	@echo Delete ..\Wasabi\bfc\*.filters
	@call del ..\Wasabi\bfc\*.filters
	)
@if exist ..\Wasabi\bfc\*.htm (
	@echo Delete ..\Wasabi\bfc\*.htm
	@call del ..\Wasabi\bfc\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * bmp project
@echo *******************************
@if exist ..\bmp\x86_Debug (
	@echo Delete ..\bmp\x86_Debug
	@call rd /S /Q ..\bmp\x86_Debug
	)
@if exist ..\bmp\x86_Release (
	@echo Delete ..\bmp\x86_Release
	@call rd /S /Q ..\bmp\x86_Release
	)
@if exist ..\bmp\x64_Debug (
	@echo Delete ..\abmp\x64_Debug
	@call rd /S /Q ..\bmp\x64_Debug
	)
@if exist ..\bmp\x64_Release (
	@echo Delete ..\abmp\x64_Release
	@call rd /S /Q ..\bmp\x64_Release
	)
@if exist ..\bmp\.vs (
	@echo Delete ..\bmp\.vs
	@call rd /S /Q ..\bmp\.vs
	)
@if exist ..\bmp\*.user (
	@echo Delete ..\bmp\*.user
	@call del ..\bmp\*.user
	)
@if exist ..\bmp\*.filters (
	@echo Delete ..\bmp\*.filters
	@call del ..\bmp\*.filters
	)
@if exist ..\bmp\*.htm (
	@echo Delete ..\bmp\*.htm
	@call del ..\bmp\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * devices project
@echo *******************************
@if exist ..\devices\x86_Debug (
	@echo Delete ..\devices\x86_Debug
	@call rd /S /Q ..\devices\x86_Debug
	)
@if exist ..\devices\x86_Release (
	@echo Delete ..\devices\x86_Release
	@call rd /S /Q ..\devices\x86_Release
	)
@if exist ..\devices\x64_Debug (
	@echo Delete ..\devices\x64_Debug
	@call rd /S /Q ..\devices\x64_Debug
	)
@if exist ..\devices\x64_Release (
	@echo Delete ..\devices\x64_Release
	@call rd /S /Q ..\devices\x64_Release
	)
@if exist ..\devices\.vs (
	@echo Delete ..\devices\.vs
	@call rd /S /Q ..\devices\.vs
	)
@if exist ..\devices\*.user (
	@echo Delete ..\devices\*.user
	@call del ..\devices\*.user
	)
@if exist ..\devices\*.filters (
	@echo Delete ..\devices\*.filters
	@call del ..\devices\*.filters
	)
@if exist ..\devices\*.htm (
	@echo Delete ..\devices\*.htm
	@call del ..\devices\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * dlmgr project
@echo *******************************
@if exist ..\dlmgr\x86_Debug (
	@echo Delete ..\dlmgr\x86_Debug
	@call rd /S /Q ..\dlmgr\x86_Debug
	)
@if exist ..\dlmgr\x86_Release (
	@echo Delete ..\dlmgr\x86_Release
	@call rd /S /Q ..\dlmgr\x86_Release
	)
@if exist ..\dlmgr\x64_Debug (
	@echo Delete ..\dlmgr\x64_Debug
	@call rd /S /Q ..\dlmgr\x64_Debug
	)
@if exist ..\dlmgr\x64_Release (
	@echo Delete ..\dlmgr\x64_Release
	@call rd /S /Q ..\dlmgr\x64_Release
	)
@if exist ..\dlmgr\.vs (
	@echo Delete ..\dlmgr\.vs
	@call rd /S /Q ..\dlmgr\.vs
	)
@if exist ..\dlmgr\*.user (
	@echo Delete ..\dlmgr\*.user
	@call del ..\dlmgr\*.user
	)
@if exist ..\dlmgr\*.filters (
	@echo Delete ..\dlmgr\*.filters
	@call del ..\dlmgr\*.filters
	)
@if exist ..\dlmgr\*.htm (
	@echo Delete ..\dlmgr\*.htm
	@call del ..\dlmgr\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * enc_flac project
@echo *******************************
@if exist ..\enc_flac\x86_Debug (
	@echo Delete ..\enc_flac\x86_Debug
	@call rd /S /Q ..\enc_flac\x86_Debug
	)
@if exist ..\enc_flac\x86_Release (
	@echo Delete ..\enc_flac\x86_Release
	@call rd /S /Q ..\enc_flac\x86_Release
	)
@if exist ..\enc_flac\x64_Debug (
	@echo Delete ..\enc_flac\x64_Debug
	@call rd /S /Q ..\enc_flac\x64_Debug
	)
@if exist ..\enc_flac\x64_Release (
	@echo Delete ..\enc_flac\x64_Release
	@call rd /S /Q ..\enc_flac\x64_Release
	)
@if exist ..\enc_flac\.vs (
	@echo Delete ..\enc_flac\.vs
	@call rd /S /Q ..\enc_flac\.vs
	)
@if exist ..\enc_flac\*.user (
	@echo Delete ..\enc_flac\*.user
	@call del ..\enc_flac\*.user
	)
@if exist ..\enc_flac\*.filters (
	@echo Delete ..\enc_flac\*.filters
	@call del ..\enc_flac\*.filters
	)
@if exist ..\enc_flac\*.htm (
	@echo Delete ..\enc_flac\*.htm
	@call del ..\enc_flac\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * enc_lame project
@echo *******************************
@if exist ..\enc_lame\x86_Debug (
	@echo Delete ..\enc_lame\x86_Debug
	@call rd /S /Q ..\enc_lame\x86_Debug
	)
@if exist ..\enc_lame\x86_Release (
	@echo Delete ..\enc_lame\x86_Release
	@call rd /S /Q ..\enc_lame\x86_Release
	)
@if exist ..\enc_lame\x64_Debug (
	@echo Delete ..\enc_lame\x64_Debug
	@call rd /S /Q ..\enc_lame\x64_Debug
	)
@if exist ..\enc_lame\x64_Release (
	@echo Delete ..\enc_lame\x64_Release
	@call rd /S /Q ..\enc_lame\x64_Release
	)
@if exist ..\enc_lame\.vs (
	@echo Delete ..\enc_lame\.vs
	@call rd /S /Q ..\enc_lame\.vs
	)
@if exist ..\enc_lame\*.user (
	@echo Delete ..\enc_lame\*.user
	@call del ..\enc_lame\*.user
	)
@if exist ..\enc_lame\*.filters (
	@echo Delete ..\enc_lame\*.filters
	@call del ..\enc_lame\*.filters
	)
@if exist ..\enc_lame\*.htm (
	@echo Delete ..\enc_lame\*.htm
	@call del ..\enc_lame\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * enc_vorbis project
@echo *******************************
@if exist ..\enc_vorbis\x86_Debug (
	@echo Delete ..\enc_vorbis\x86_Debug
	@call rd /S /Q ..\enc_vorbis\x86_Debug
	)
@if exist ..\enc_vorbis\x86_Release (
	@echo Delete ..\enc_vorbis\x86_Release
	@call rd /S /Q ..\enc_vorbis\x86_Release
	)
@if exist ..\enc_vorbis\x64_Debug (
	@echo Delete ..\enc_vorbis\x64_Debug
	@call rd /S /Q ..\enc_vorbis\x64_Debug
	)
@if exist ..\enc_vorbis\x64_Release (
	@echo Delete ..\enc_vorbis\x64_Release
	@call rd /S /Q ..\enc_vorbis\x64_Release
	)
@if exist ..\enc_vorbis\.vs (
	@echo Delete ..\enc_vorbis\.vs
	@call rd /S /Q ..\enc_vorbis\.vs
	)
@if exist ..\enc_vorbis\*.user (
	@echo Delete ..\enc_vorbis\*.user
	@call del ..\enc_vorbis\*.user
	)
@if exist ..\enc_vorbis\*.filters (
	@echo Delete ..\enc_vorbis\*.filters
	@call del ..\enc_vorbis\*.filters
	)
@if exist ..\enc_vorbis\*.htm (
	@echo Delete ..\enc_vorbis\*.htm
	@call del ..\enc_vorbis\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * enc_wav project
@echo *******************************
@if exist ..\enc_wav\x86_Debug (
	@echo Delete ..\enc_wav\x86_Debug
	@call rd /S /Q ..\enc_wav\x86_Debug
	)
@if exist ..\enc_wav\x86_Release (
	@echo Delete ..\enc_wav\x86_Release
	@call rd /S /Q ..\enc_wav\x86_Release
	)
@if exist ..\enc_wav\x64_Debug (
	@echo Delete ..\enc_wav\x64_Debug
	@call rd /S /Q ..\enc_wav\x64_Debug
	)
@if exist ..\enc_wav\x64_Release (
	@echo Delete ..\enc_wav\x64_Release
	@call rd /S /Q ..\enc_wav\x64_Release
	)
@if exist ..\enc_wav\.vs (
	@echo Delete ..\enc_wav\.vs
	@call rd /S /Q ..\enc_wav\.vs
	)
@if exist ..\enc_wav\*.user (
	@echo Delete ..\enc_wav\*.user
	@call del ..\enc_wav\*.user
	)
@if exist ..\enc_wav\*.filters (
	@echo Delete ..\enc_wav\*.filters
	@call del ..\enc_wav\*.filters
	)
@if exist ..\enc_wav\*.htm (
	@echo Delete ..\enc_wav\*.htm
	@call del ..\enc_wav\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * enc_wma project
@echo *******************************
@if exist ..\enc_wma\x86_Debug (
	@echo Delete ..\enc_wma\x86_Debug
	@call rd /S /Q ..\enc_wma\x86_Debug
	)
@if exist ..\enc_wma\x86_Release (
	@echo Delete ..\enc_wma\x86_Release
	@call rd /S /Q ..\enc_wma\x86_Release
	)
@if exist ..\enc_wma\x64_Debug (
	@echo Delete ..\enc_wma\x64_Debug
	@call rd /S /Q ..\enc_wma\x64_Debug
	)
@if exist ..\enc_wma\x64_Release (
	@echo Delete ..\enc_wma\x64_Release
	@call rd /S /Q ..\enc_wma\x64_Release
	)
@if exist ..\enc_wma\.vs (
	@echo Delete ..\enc_wma\.vs
	@call rd /S /Q ..\enc_wma\.vs
	)
@if exist ..\enc_wma\*.user (
	@echo Delete ..\enc_wma\*.user
	@call del ..\enc_wma\*.user
	)
@if exist ..\enc_wma\*.filters (
	@echo Delete ..\enc_wma\*.filters
	@call del ..\enc_wma\*.filters
	)
@if exist ..\enc_wma\*.htm (
	@echo Delete ..\enc_wma\*.htm
	@call del ..\enc_wma\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * expat project
@echo *******************************
@if exist ..\expat\x86_Debug (
	@echo Delete ..\expat\x86_Debug
	@call rd /S /Q ..\expat\x86_Debug
	)
@if exist ..\expat\x86_Release (
	@echo Delete ..\expat\x86_Release
	@call rd /S /Q ..\expat\x86_Release
	)
@if exist ..\expat\x64_Debug (
	@echo Delete ..\expat\x64_Debug
	@call rd /S /Q ..\expat\x64_Debug
	)
@if exist ..\expat\x64_Release (
	@echo Delete ..\expat\x64_Release
	@call rd /S /Q ..\expat\x64_Release
	)
@if exist ..\expat\.vs (
	@echo Delete ..\expat\.vs
	@call rd /S /Q ..\expat\.vs
	)
@if exist ..\expat\*.user (
	@echo Delete ..\expat\*.user
	@call del ..\expat\*.user
	)
@if exist ..\expat\*.filters (
	@echo Delete ..\expat\*.filters
	@call del ..\expat\*.filters
	)
@if exist ..\expat\*.htm (
	@echo Delete ..\expat\*.htm
	@call del ..\expat\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * Elevator project
@echo *******************************
@if exist ..\Elevator\x86_Debug (
	@echo Delete ..\Elevator\x86_Debug
	@call rd /S /Q ..\Elevator\x86_Debug
	)
@if exist ..\Elevator\x86_Release (
	@echo Delete ..\Elevator\x86_Release
	@call rd /S /Q ..\Elevator\x86_Release
	)
@if exist ..\Elevator\x64_Debug (
	@echo Delete ..\Elevator\x64_Debug
	@call rd /S /Q ..\Elevator\x64_Debug
	)
@if exist ..\Elevator\x64_Release (
	@echo  Delete ..\Elevator\x64_Release
	@call rd /S /Q ..\Elevator\x64_Release
	)
@if exist ..\Elevator\x86_DebugPS (
	@echo Delete ..\Elevator\x86_DebugPS
	@call rd /S /Q ..\Elevator\x86_DebugPS
	)
@if exist ..\Elevator\x86_ReleasePS (
	@echo Delete ..\Elevator\x86_ReleasePS
	@call rd /S /Q ..\Elevator\x86_ReleasePS
	)
@if exist ..\Elevator\x64_DebugPS (
	@echo Delete ..\Elevator\x64_DebugPS
	@call rd /S /Q ..\Elevator\x64_DebugPS
	)
@if exist ..\Elevator\x64_ReleasePS (
	@echo Delete ..\Elevator\x64_ReleasePS
	@call rd /S /Q ..\Elevator\x64_ReleasePS
	)
@if exist ..\Elevator\.vs (
	@echo Delete ..\Elevator\.vs
	@call rd /S /Q ..\Elevator\.vs
	)
@if exist ..\Elevator\*.user (
	@echo Delete ..\Elevator\*.user
	@call del ..\Elevator\*.user
	)
@if exist ..\Elevator\*.filters (
	@echo Delete ..\Elevator\*.filters
	@call del ..\Elevator\*.filters
	)
@if exist ..\Elevator\*.htm (
	@echo Delete ..\Elevator\*.htm
	@call del ..\Elevator\*.htm
	)
@if exist ..\Elevator\IFileTypeRegistrar_32.h (
	@echo Delete ..\Elevator\IFileTypeRegistrar_32.h
	@call del ..\Elevator\IFileTypeRegistrar_32.h
	)
@if exist ..\Elevator\IFileTypeRegistrar_64.h (
	@echo Delete ..\Elevator\IFileTypeRegistrar_64.h
	@call del ..\Elevator\IFileTypeRegistrar_64.h
	)	
@echo -------------------------------
@echo *******************************
@echo * f263 project
@echo *******************************
@if exist ..\f263\x86_Debug (
	@echo Delete ..\f263\x86_Debug
	@call rd /S /Q ..\f263\x86_Debug
	)
@if exist ..\f263\x86_Release (
	@echo Delete ..\f263\x86_Release
	@call rd /S /Q ..\f263\x86_Release
	)
@if exist ..\f263\x64_Debug (
	@echo Delete ..\f263\x64_Debug
	@call rd /S /Q ..\f263\x64_Debug
	)
@if exist ..\f263\x64_Release (
	@echo Delete ..\f263\x64_Release
	@call rd /S /Q ..\f263\x64_Release
	)
@if exist ..\f263\x86_Debug_static (
	@echo Delete ..\f263\x86_Debug_static
	@call rd /S /Q ..\f263\x86_Debug_static
	)
@if exist ..\f263\x86_Release_static (
	@echo Delete ..\f263\x86_Release_static
	@call rd /S /Q ..\f263\x86_Release_static
	)
@if exist ..\f263\x64_Debug_static (
	@echo Delete ..\f263\x64_Debug_static
	@call rd /S /Q ..\f263\x64_Debug_static
	)
@if exist ..\f263\x64_Release_static (
	@echo Delete ..\f263\x64_Release_static
	@call rd /S /Q ..\f263\x64_Release_static
	)
@if exist ..\f263\.vs (
	@echo Delete ..\f263\.vs
	@call rd /S /Q ..\f263\.vs
	)
@if exist ..\f263\*.user (
	@echo Delete ..\f263\*.user
	@call del ..\f263\*.user
	)
@if exist ..\f263\*.filters (
	@echo Delete ..\f263\*.filters
	@call del ..\f263\*.filters
	)
@if exist ..\f263\*.htm (
	@echo Delete ..\f263\*.htm
	@call del ..\f263\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * filereader project
@echo *******************************
@if exist ..\filereader\x86_Debug (
	@echo Delete ..\filereader\x86_Debug
	@call rd /S /Q ..\filereader\x86_Debug
	)
@if exist ..\filereader\x86_Release (
	@echo Delete ..\filereader\x86_Release
	@call rd /S /Q ..\filereader\x86_Release
	)
@if exist ..\filereader\x64_Debug (
	@echo Delete ..\filereader\x64_Debug
	@call rd /S /Q ..\filereader\x64_Debug
	)
@if exist ..\filereader\x64_Release (
	@echo Delete ..\filereader\x64_Release
	@call rd /S /Q ..\filereader\x64_Release
	)
@if exist ..\filereader\.vs (
	@echo Delete ..\filereader\.vs
	@call rd /S /Q ..\filereader\.vs
	)
@if exist ..\filereader\*.user (
	@echo Delete ..\filereader\*.user
	@call del ..\filereader\*.user
	)
@if exist ..\filereader\*.filters (
	@echo Delete ..\filereader\*.filters
	@call del ..\filereader\*.filters
	)
@if exist ..\filereader\*.htm (
	@echo Delete ..\filereader\*.htm
	@call del ..\filereader\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * freetype project
@echo *******************************
@if exist ..\freetype\x86_Debug (
	@echo Delete ..\freetype\x86_Debug
	@call rd /S /Q ..\freetype\x86_Debug
	)
@if exist ..\freetype\x86_Release (
	@echo Delete ..\freetype\x86_Release
	@call rd /S /Q ..\freetype\x86_Release
	)
@if exist ..\freetype\x64_Debug (
	@echo Delete ..\freetype\x64_Debug
	@call rd /S /Q ..\freetype\x64_Debug
	)
@if exist ..\freetype\x64_Release (
	@echo Delete ..\freetype\x64_Release
	@call rd /S /Q ..\freetype\x64_Release
	)
@if exist ..\freetype\.vs (
	@echo Delete ..\freetype\.vs
	@call rd /S /Q ..\freetype\.vs
	)
@if exist ..\freetype\*.user (
	@echo Delete ..\freetype\*.user
	@call del ..\freetype\*.user
	)
@if exist ..\freetype\*.filters (
	@echo Delete ..\freetype\*.filters
	@call del ..\freetype\*.filters
	)
@if exist ..\freetype\*.htm (
	@echo Delete ..\freetype\*.htm
	@call del ..\freetype\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * freetypewac project
@echo *******************************
@if exist ..\freetypewac\x86_Debug (
	@echo Delete ..\freetypewac\x86_Debug
	@call rd /S /Q ..\freetypewac\x86_Debug
	)
@if exist ..\freetypewac\x86_Release (
	@echo Delete ..\freetypewac\x86_Release
	@call rd /S /Q ..\freetypewac\x86_Release
	)
@if exist ..\freetypewac\x64_Debug (
	@echo Delete ..\freetypewac\x64_Debug
	@call rd /S /Q ..\freetypewac\x64_Debug
	)
@if exist ..\freetypewac\x64_Release (
	@echo Delete ..\freetypewac\x64_Release
	@call rd /S /Q ..\freetypewac\x64_Release
	)
@if exist ..\freetypewac\.vs (
	@echo Delete ..\freetypewac\.vs
	@call rd /S /Q ..\freetypewac\.vs
	)
@if exist ..\freetypewac\*.user (
	@echo Delete ..\freetypewac\*.user
	@call del ..\freetypewac\*.user
	)
@if exist ..\freetypewac\*.filters (
	@echo Delete ..\freetypewac\*.filters
	@call del ..\freetypewac\*.filters
	)
@if exist ..\freetypewac\*.htm (
	@echo Delete ..\freetypewac\*.htm
	@call del ..\freetypewac\*.htm
	)		
@echo -------------------------------
@echo *******************************
@echo * h264 project
@echo *******************************
@if exist ..\h264\x86_Debug (
	@echo Delete ..\h264\x86_Debug
	@call rd /S /Q ..\h264\x86_Debug
	)
@if exist ..\h264\x86_Release (
	@echo Delete ..\h264\x86_Release
	@call rd /S /Q ..\h264\x86_Release
	)
@if exist ..\h264\x64_Debug (
	@echo Delete ..\h264\x64_Debug
	@call rd /S /Q ..\h264\x64_Debug
	)
@if exist ..\h264\x64_Release (
	@echo Delete ..\h264\x64_Release
	@call rd /S /Q ..\h264\x64_Release
	)
@if exist ..\h264\.vs (
	@echo Delete ..\h264\.vs
	@call rd /S /Q ..\h264\.vs
	)
@if exist ..\h264\*.user (
	@echo Delete ..\h264\*.user
	@call del ..\h264\*.user
	)
@if exist ..\h264\*.filters (
	@echo Delete ..\h264\*.filters
	@call del ..\h264\*.filters
	)
@if exist ..\h264\*.htm (
	@echo Delete ..\h264\*.htm
	@call del ..\h264\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * gen_crasher project
@echo *******************************
@if exist ..\gen_crasher\x86_Debug (
	@echo Delete ..\gen_crasher\x86_Debug
	@call rd /S /Q ..\gen_crasher\x86_Debug
	)
@if exist ..\gen_crasher\x86_Release (
	@echo Delete ..\gen_crasher\x86_Release
	@call rd /S /Q ..\gen_crasher\x86_Release
	)
@if exist ..\gen_crasher\x64_Debug (
	@echo Delete ..\gen_crasher\x64_Debug
	@call rd /S /Q ..\gen_crasher\x64_Debug
	)
@if exist ..\gen_crasher\x64_Release (
	@echo Delete ..\gen_crasher\x64_Release
	@call rd /S /Q ..\gen_crasher\x64_Release
	)
@if exist ..\gen_crasher\feedback\x86_Debug (
	@echo Delete ..\gen_crasher\feedback\x86_Debug
	@call rd /S /Q ..\gen_crasher\feedback\x86_Debug
	)
@if exist ..\gen_crasher\feedback\x86_Release (
	@echo Delete ..\gen_crasher\feedback\x86_Release
	@call rd /S /Q ..\gen_crasher\feedback\x86_Release
	)		
@if exist ..\gen_crasher\feedback\x64_Release (
	@echo Delete ..\gen_crasher\feedback\x64_Release
	@call rd /S /Q ..\gen_crasher\feedback\x64_Release
	)
@if exist ..\gen_crasher\feedback\x64_Debug (
	@echo Delete ..\gen_crasher\feedback\x64_Debug
	@call rd /S /Q ..\gen_crasher\feedback\x64_Debug
	)
@if exist ..\gen_crasher\.vs (
	@echo Delete ..\gen_crasher\.vs
	@call rd /S /Q ..\gen_crasher\.vs
	)
@if exist ..\gen_crasher\feedback\.vs (
	@echo Delete ..\gen_crasher\feedback\.vs
	@call rd /S /Q ..\gen_crasher\feedback\.vs
	)	
@if exist ..\gen_crasher\*.user (
	@echo Delete ..\gen_crasher\*.user
	@call del ..\gen_crasher\*.user
	)
@if exist ..\gen_crasher\feedback\*.user (
	@echo Delete ..\gen_crasher\feedback\*.user
	@call del ..\gen_crasher\feedback\*.user
	)
@if exist ..\gen_crasher\*.filters (
	@echo Delete ..\gen_crasher\*.filters
	@call del ..\gen_crasher\*.filters
	)
@if exist ..\gen_crasher\feedback\*.filters (
	@echo Delete ..\gen_crasher\feedback\*.filters
	@call del ..\gen_crasher\feedback\*.filters
	)
@if exist ..\gen_crasher\*.htm (
	@echo Delete ..\gen_crasher\*.htm
	@call del ..\gen_crasher\*.htm
	)	
@if exist ..\gen_crasher\feedback\*.htm (
	@echo Delete ..\gen_crasher\feedback\*.htm
	@call del ..\gen_crasher\feedback\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * gen_deviceprovider project
@echo *******************************
@if exist ..\ml_devices\gen_deviceprovider\x86_Debug (
	@echo Delete ..\ml_devices\gen_deviceprovider\x86_Debug
	@call rd /S /Q ..\ml_devices\gen_deviceprovider\x86_Debug
	)
@if exist ..\ml_devices\gen_deviceprovider\x86_Release (
	@echo Delete ..\ml_devices\gen_deviceprovider\x86_Release
	@call rd /S /Q ..\ml_devices\gen_deviceprovider\x86_Release
	)
@if exist ..\ml_devices\gen_deviceprovider\x64_Debug (
	@echo Delete ..\ml_devices\gen_deviceprovider\x64_Debug
	@call rd /S /Q ..\ml_devices\gen_deviceprovider\x64_Debug
	)
@if exist ..\ml_devices\gen_deviceprovider\x64_Release (
	@echo Delete ..\ml_devices\gen_deviceprovider\x64_Release
	@call rd /S /Q ..\ml_devices\gen_deviceprovider\x64_Release
	)
@if exist ..\ml_devices\gen_deviceprovider\.vs (
	@echo Delete ..\ml_devices\gen_deviceprovider\.vs
	@call rd /S /Q ..\ml_devices\gen_deviceprovider\.vs
	)
@if exist ..\ml_devices\gen_deviceprovider\*.user (
	@echo Delete ..\ml_devices\gen_deviceprovider\*.user
	@call del ..\ml_devices\gen_deviceprovider\*.user
	)
@if exist ..\ml_devices\gen_deviceprovider\*.filters (
	@echo Delete ..\ml_devices\gen_deviceprovider\*.filters
	@call del ..\ml_devices\gen_deviceprovider\*.filters
	)
@if exist ..\ml_devices\gen_deviceprovider\*.htm (
	@echo Delete ..\ml_devices\gen_deviceprovider\*.htm
	@call del ..\ml_devices\gen_deviceprovider\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * gen_hotkeys project
@echo *******************************
@if exist ..\gen_hotkeys\x86_Debug (
	@echo Delete ..\gen_hotkeys\x86_Debug
	@call rd /S /Q ..\gen_hotkeys\x86_Debug
	)
@if exist ..\gen_hotkeys\x86_Release (
	@echo Delete ..\gen_hotkeys\x86_Release
	@call rd /S /Q ..\gen_hotkeys\x86_Release
	)
@if exist ..\gen_hotkeys\x64_Debug (
	@echo Delete ..\gen_hotkeys\x64_Debug
	@call rd /S /Q ..\gen_hotkeys\x64_Debug
	)
@if exist ..\gen_hotkeys\x64_Release (
	@echo Delete ..\gen_hotkeys\x64_Release
	@call rd /S /Q ..\gen_hotkeys\x64_Release
	)
@if exist ..\gen_hotkeys\.vs (
	@echo Delete ..\gen_hotkeys\.vs
	@call rd /S /Q ..\gen_hotkeys\.vs
	)
@if exist ..\gen_hotkeys\*.user (
	@echo Delete ..\gen_hotkeys\*.user
	@call del ..\gen_hotkeys\*.user
	)
@if exist ..\gen_hotkeys\*.filters (
	@echo Delete ..\gen_hotkeys\*.filters
	@call del ..\gen_hotkeys\*.filters
	)
@if exist ..\gen_hotkeys\*.htm (
	@echo Delete ..\gen_hotkeys\*.htm
	@call del ..\gen_hotkeys\*.htm
	)
@echo -------------------------------	
@echo *******************************
@echo * gen_ff project
@echo *******************************
@if exist ..\gen_ff\x86_Debug (
	@echo Delete ..\gen_ff\x86_Debug
	@call rd /S /Q ..\gen_ff\x86_Debug
	)
@if exist ..\gen_ff\x86_Release (
	@echo Delete ..\gen_ff\x86_Release
	@call rd /S /Q ..\gen_ff\x86_Release
	)
@if exist ..\gen_ff\x64_Debug (
	@echo Delete ..\gen_ff\x64_Debug
	@call rd /S /Q ..\gen_ff\x64_Debug
	)
@if exist ..\gen_ff\x64_Release (
	@echo Delete ..\gen_ff\x64_Release
	@call rd /S /Q ..\gen_ff\x64_Release
	)
@if exist ..\gen_ff\.vs (
	@echo Delete ..\gen_ff\.vs
	@call rd /S /Q ..\gen_ff\.vs
	)
@if exist ..\gen_ff\*.user (
	@echo Delete ..\gen_ff\*.user
	@call del ..\gen_ff\*.user
	)
@if exist ..\gen_ff\*.filters (
	@echo Delete ..\gen_ff\*.filters
	@call del ..\gen_ff\*.filters
	)
@if exist ..\gen_ff\*.htm (
	@echo Delete ..\gen_ff\*.htm
	@call del ..\gen_ff\*.htm
	)	
@echo -------------------------------	
@echo *******************************
@echo * gen_ml project
@echo *******************************
@if exist ..\gen_ml\x86_Debug (
	@echo Delete ..\gen_ml\x86_Debug
	@call rd /S /Q ..\gen_ml\x86_Debug
	)
@if exist ..\gen_ml\x86_Release (
	@echo Delete ..\gen_ml\x86_Release
	@call rd /S /Q ..\gen_ml\x86_Release
	)
@if exist ..\gen_ml\x64_Debug (
	@echo Delete ..\gen_ml\x64_Debug
	@call rd /S /Q ..\gen_ml\x64_Debug
	)
@if exist ..\gen_ml\x64_Release (
	@echo Delete ..\gen_ml\x64_Release
	@call rd /S /Q ..\gen_ml\x64_Release
	)
@if exist ..\gen_ml\.vs (
	@echo Delete ..\gen_ml\.vs
	@call rd /S /Q ..\gen_ml\.vs
	)
@if exist ..\gen_ml\*.user (
	@echo Delete ..\gen_ml\*.user
	@call del ..\gen_ml\*.user
	)
@if exist ..\gen_ml\*.filters (
	@echo Delete ..\gen_ml\*.filters
	@call del ..\gen_ml\*.filters
	)
@if exist ..\gen_ml\*.htm (
	@echo Delete ..\gen_ml\*.htm
	@call del ..\gen_ml\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * gen_tray project
@echo *******************************
@if exist ..\gen_tray\x86_Debug (
	@echo Delete ..\gen_tray\x86_Debug
	@call rd /S /Q ..\gen_tray\x86_Debug
	)
@if exist ..\gen_tray\x86_Release (
	@echo Delete ..\gen_tray\x86_Release
	@call rd /S /Q ..\gen_tray\x86_Release
	)
@if exist ..\gen_tray\x64_Debug (
	@echo Delete ..\gen_tray\x64_Debug
	@call rd /S /Q ..\gen_tray\x64_Debug
	)
@if exist ..\gen_tray\x64_Release (
	@echo Delete ..\gen_tray\x64_Release
	@call rd /S /Q ..\gen_tray\x64_Release
	)
@if exist ..\gen_tray\.vs (
	@echo Delete ..\gen_tray\.vs
	@call rd /S /Q ..\gen_tray\.vs
	)
@if exist ..\gen_tray\*.user (
	@echo Delete ..\gen_tray\*.user
	@call del ..\gen_tray\*.user
	)
@if exist ..\gen_tray\*.filters (
	@echo Delete ..\gen_tray\*.filters
	@call del ..\gen_tray\*.filters
	)
@if exist ..\gen_tray\*.htm (
	@echo Delete ..\gen_tray\*.htm
	@call del ..\gen_tray\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * gif project
@echo *******************************
@if exist ..\gif\x86_Debug (
	@echo Delete ..\gif\x86_Debug
	@call rd /S /Q ..\gif\x86_Debug
	)
@if exist ..\gif\x86_Release (
	@echo Delete ..\gif\x86_Release
	@call rd /S /Q ..\gif\x86_Release
	)
@if exist ..\gif\x64_Debug (
	@echo Delete ..\gif\x64_Debug
	@call rd /S /Q ..\gif\x64_Debug
	)
@if exist ..\gif\x64_Release (
	@echo Delete ..\gif\x64_Release
	@call rd /S /Q ..\gif\x64_Release
	)
@if exist ..\gif\.vs (
	@echo Delete ..\gif\.vs
	@call rd /S /Q ..\gif\.vs
	)
@if exist ..\gif\*.user (
	@echo Delete ..\gif\*.user
	@call del ..\gif\*.user
	)
@if exist ..\gif\*.filters (
	@echo Delete ..\gif\*.filters
	@call del ..\gif\*.filters
	)
@if exist ..\gif\*.htm (
	@echo Delete ..\gif\*.htm
	@call del ..\gif\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * giflib project
@echo *******************************
@if exist ..\giflib\x86_Debug (
	@echo Delete ..\giflib\x86_Debug
	@call rd /S /Q ..\giflib\x86_Debug
	)
@if exist ..\giflib\x86_Release (
	@echo Delete ..\giflib\x86_Release
	@call rd /S /Q ..\giflib\x86_Release
	)
@if exist ..\giflib\x64_Debug (
	@echo Delete ..\giflib\x64_Debug
	@call rd /S /Q ..\giflib\x64_Debug
	)
@if exist ..\giflib\x64_Release (
	@echo Delete ..\giflib\x64_Release
	@call rd /S /Q ..\giflib\x64_Release
	)
@if exist ..\giflib\.vs (
	@echo Delete ..\giflib\.vs
	@call rd /S /Q ..\giflib\.vs
	)
@if exist ..\giflib\*.user (
	@echo Delete ..\giflib\*.user
	@call del ..\giflib\*.user
	)
@if exist ..\giflib\*.filters (
	@echo Delete ..\giflib\*.filters
	@call del ..\giflib\*.filters
	)
@if exist ..\giflib\*.htm (
	@echo Delete ..\giflib\*.htm
	@call del ..\giflib\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * gracenote project
@echo *******************************
@if exist ..\gracenote\x86_Debug (
	@echo Delete ..\gracenote\x86_Debug
	@call rd /S /Q ..\gracenote\x86_Debug
	)
@if exist ..\gracenote\x86_Release (
	@echo Delete ..\gracenote\x86_Release
	@call rd /S /Q ..\gracenote\x86_Release
	)
@if exist ..\gracenote\x64_Debug (
	@echo Delete ..\gracenote\x64_Debug
	@call rd /S /Q ..\gracenote\x64_Debug
	)
@if exist ..\gracenote\x64_Release (
	@echo Delete ..\gracenote\x64_Release
	@call rd /S /Q ..\gracenote\x64_Release
	)
@if exist ..\gracenote\.vs (
	@echo Delete ..\gracenote\.vs
	@call rd /S /Q ..\gracenote\.vs
	)
@if exist ..\gracenote\*.user (
	@echo Delete ..\gracenote\*.user
	@call del ..\gracenote\*.user
	)
@if exist ..\gracenote\*.filters (
	@echo Delete ..\gracenote\*.filters
	@call del ..\gracenote\*.filters
	)
@if exist ..\gracenote\*.htm (
	@echo Delete ..\gracenote\*.htm
	@call del ..\gracenote\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * id3v2 project
@echo *******************************
@if exist ..\id3v2\x86_Debug (
	@echo Delete ..\id3v2\x86_Debug
	@call rd /S /Q ..\id3v2\x86_Debug
	)
@if exist ..\id3v2\x86_Release (
	@echo Delete ..\id3v2\x86_Release
	@call rd /S /Q ..\id3v2\x86_Release
	)
@if exist ..\id3v2\x64_Debug (
	@echo Delete ..\id3v2\x64_Debug
	@call rd /S /Q ..\id3v2\x64_Debug
	)
@if exist ..\id3v2\x64_Release (
	@echo Delete ..\id3v2\x64_Release
	@call rd /S /Q ..\id3v2\x64_Release
	)
@if exist ..\id3v2\.vs (
	@echo Delete ..\id3v2\.vs
	@call rd /S /Q ..\id3v2\.vs
	)
@if exist ..\id3v2\*.user (
	@echo Delete ..\id3v2\*.user
	@call del ..\id3v2\*.user
	)
@if exist ..\id3v2\*.filters (
	@echo Delete ..\id3v2\*.filters
	@call del ..\id3v2\*.filters
	)
@if exist ..\id3v2\*.htm (
	@echo Delete ..\id3v2\*.htm
	@call del ..\id3v2\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ijg project
@echo *******************************
@if exist ..\ijg\x86_Debug (
	@echo Delete ..\ijg\x86_Debug
	@call rd /S /Q ..\ijg\x86_Debug
	)
@if exist ..\ijg\x86_Release (
	@echo Delete ..\ijg\x86_Release
	@call rd /S /Q ..\ijg\x86_Release
	)
@if exist ..\ijg\x64_Debug (
	@echo Delete ..\ijg\x64_Debug
	@call rd /S /Q ..\ijg\x64_Debug
	)
@if exist ..\ijg\x64_Release (
	@echo Delete ..\ijg\x64_Release
	@call rd /S /Q ..\ijg\x64_Release
	)
@if exist ..\ijg\.vs (
	@echo Delete ..\ijg\.vs
	@call rd /S /Q ..\ijg\.vs
	)
@if exist ..\ijg\*.user (
	@echo Delete ..\ijg\*.user
	@call del ..\ijg\*.user
	)
@if exist ..\ijg\*.filters (
	@echo Delete ..\ijg\*.filters
	@call del ..\ijg\*.filters
	)
@if exist ..\ijg\*.htm (
	@echo Delete ..\ijg\*.htm
	@call del ..\ijg\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_avi project
@echo *******************************
@if exist ..\in_avi\x86_Debug (
	@echo Delete ..\in_avi\x86_Debug
	@call rd /S /Q ..\in_avi\x86_Debug
	)
@if exist ..\in_avi\x86_Release (
	@echo Delete ..\in_avi\x86_Release
	@call rd /S /Q ..\in_avi\x86_Release
	)
@if exist ..\in_avi\x64_Debug (
	@echo Delete ..\in_avi\x64_Debug
	@call rd /S /Q ..\in_avi\x64_Debug
	)
@if exist ..\in_avi\x64_Release (
	@echo Delete ..\in_avi\x64_Release
	@call rd /S /Q ..\in_avi\x64_Release
	)
@if exist ..\in_avi\.vs (
	@echo Delete ..\in_avi\.vs
	@call rd /S /Q ..\in_avi\.vs
	)
@if exist ..\in_avi\*.user (
	@echo Delete ..\in_avi\*.user
	@call del ..\in_avi\*.user
	)
@if exist ..\in_avi\*.filters (
	@echo Delete ..\in_avi\*.filters
	@call del ..\in_avi\*.filters
	)
@if exist ..\in_avi\*.htm (
	@echo Delete ..\in_avi\*.htm
	@call del ..\in_avi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_cdda project
@echo *******************************
@if exist ..\in_cdda\x86_Debug (
	@echo Delete ..\in_cdda\x86_Debug
	@call rd /S /Q ..\in_cdda\x86_Debug
	)
@if exist ..\in_cdda\x86_Release (
	@echo Delete ..\in_cdda\x86_Release
	@call rd /S /Q ..\in_cdda\x86_Release
	)
@if exist ..\in_cdda\x64_Debug (
	@echo Delete ..\in_cdda\x64_Debug
	@call rd /S /Q ..\in_cdda\x64_Debug
	)
@if exist ..\in_cdda\x64_Release (
	@echo Delete ..\in_cdda\x64_Release
	@call rd /S /Q ..\in_cdda\x64_Release
	)
@if exist ..\in_cdda\.vs (
	@echo Delete ..\in_cdda\.vs
	@call rd /S /Q ..\in_cdda\.vs
	)
@if exist ..\in_cdda\*.user (
	@echo Delete ..\in_cdda\*.user
	@call del ..\in_cdda\*.user
	)
@if exist ..\in_cdda\*.filters (
	@echo Delete ..\in_cdda\*.filters
	@call del ..\in_cdda\*.filters
	)
@if exist ..\in_cdda\*.htm (
	@echo Delete ..\in_cdda\*.htm
	@call del ..\in_cdda\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_dshow project
@echo *******************************
@if exist ..\in_dshow\x86_Debug (
	@echo Delete ..\in_dshow\x86_Debug
	@call rd /S /Q ..\in_dshow\x86_Debug
	)
@if exist ..\in_dshow\x86_Release (
	@echo Delete ..\in_dshow\x86_Release
	@call rd /S /Q ..\in_dshow\x86_Release
	)
@if exist ..\in_dshow\x64_Debug (
	@echo Delete ..\in_dshow\x64_Debug
	@call rd /S /Q ..\in_dshow\x64_Debug
	)
@if exist ..\in_dshow\x64_Release (
	@echo Delete ..\in_dshow\x64_Release
	@call rd /S /Q ..\in_dshow\x64_Release
	)
@if exist ..\in_dshow\.vs (
	@echo Delete ..\in_dshow\.vs
	@call rd /S /Q ..\in_dshow\.vs
	)
@if exist ..\in_dshow\*.user (
	@echo Delete ..\in_dshow\*.user
	@call del ..\in_dshow\*.user
	)
@if exist ..\in_dshow\*.filters (
	@echo Delete ..\in_dshow\*.filters
	@call del ..\in_dshow\*.filters
	)
@if exist ..\in_dshow\*.htm (
	@echo Delete ..\in_dshow\*.htm
	@call del ..\in_dshow\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_flac project
@echo *******************************
@if exist ..\in_flac\x86_Debug (
	@echo Delete ..\in_flac\x86_Debug
	@call rd /S /Q ..\in_flac\x86_Debug
	)
@if exist ..\in_flac\x86_Release (
	@echo Delete ..\in_flac\x86_Release
	@call rd /S /Q ..\in_flac\x86_Release
	)
@if exist ..\in_flac\x64_Debug (
	@echo Delete ..\in_flac\x64_Debug
	@call rd /S /Q ..\in_flac\x64_Debug
	)
@if exist ..\in_flac\x64_Release (
	@echo Delete ..\in_flac\x64_Release
	@call rd /S /Q ..\in_flac\x64_Release
	)
@if exist ..\in_flac\.vs (
	@echo Delete ..\in_flac\.vs
	@call rd /S /Q ..\in_flac\.vs
	)
@if exist ..\in_flac\*.user (
	@echo Delete ..\in_flac\*.user
	@call del ..\in_flac\*.user
	)
@if exist ..\in_flac\*.filters (
	@echo Delete ..\in_flac\*.filters
	@call del ..\in_flac\*.filters
	)
@if exist ..\in_flac\*.htm (
	@echo Delete ..\in_flac\*.htm
	@call del ..\in_flac\*.htm
	)		
@echo -------------------------------
@echo *******************************
@echo * in_flv project
@echo *******************************
@if exist ..\in_flv\x86_Debug (
	@echo Delete ..\in_flv\x86_Debug
	@call rd /S /Q ..\in_flv\x86_Debug
	)
@if exist ..\in_flv\x86_Release (
	@echo Delete ..\in_flv\x86_Release
	@call rd /S /Q ..\in_flv\x86_Release
	)
@if exist ..\in_flv\x64_Debug (
	@echo Delete ..\in_flv\x64_Debug
	@call rd /S /Q ..\in_flv\x64_Debug
	)
@if exist ..\in_flv\x64_Release (
	@echo Delete ..\in_flv\x64_Release
	@call rd /S /Q ..\in_flv\x64_Release
	)
@if exist ..\in_flv\.vs (
	@echo Delete ..\in_flv\.vs
	@call rd /S /Q ..\in_flv\.vs
	)
@if exist ..\in_flv\*.user (
	@echo Delete ..\in_flv\*.user
	@call del ..\in_flv\*.user
	)
@if exist ..\in_flv\*.filters (
	@echo Delete ..\in_flv\*.filters
	@call del ..\in_flv\*.filters
	)
@if exist ..\in_flv\*.htm (
	@echo Delete ..\in_flv\*.htm
	@call del ..\in_flv\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_linein project
@echo *******************************
@if exist ..\in_linein\x86_Debug (
	@echo Delete ..\in_linein\x86_Debug
	@call rd /S /Q ..\in_linein\x86_Debug
	)
@if exist ..\in_linein\x86_Release (
	@echo Delete ..\in_linein\x86_Release
	@call rd /S /Q ..\in_linein\x86_Release
	)
@if exist ..\in_linein\x64_Debug (
	@echo Delete ..\in_linein\x64_Debug
	@call rd /S /Q ..\in_linein\x64_Debug
	)
@if exist ..\in_linein\x64_Release (
	@echo Delete ..\in_linein\x64_Release
	@call rd /S /Q ..\in_linein\x64_Release
	)
@if exist ..\in_linein\.vs (
	@echo Delete ..\in_linein\.vs
	@call rd /S /Q ..\in_linein\.vs
	)
@if exist ..\in_linein\*.user (
	@echo Delete ..\in_linein\*.user
	@call del ..\in_linein\*.user
	)
@if exist ..\in_linein\*.filters (
	@echo Delete ..\in_linein\*.filters
	@call del ..\in_linein\*.filters
	)
@if exist ..\in_linein\*.htm (
	@echo Delete ..\in_linein\*.htm
	@call del ..\in_linein\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_mkv project
@echo *******************************
@if exist ..\in_mkv\x86_Debug (
	@echo Delete ..\in_mkv\x86_Debug
	@call rd /S /Q ..\in_mkv\x86_Debug
	)
@if exist ..\in_mkv\x86_Release (
	@echo Delete ..\in_mkv\x86_Release
	@call rd /S /Q ..\in_mkv\x86_Release
	)
@if exist ..\in_mkv\x64_Debug (
	@echo Delete ..\in_mkv\x64_Debug
	@call rd /S /Q ..\in_mkv\x64_Debug
	)
@if exist ..\in_mkv\x64_Release (
	@echo Delete ..\in_mkv\x64_Release
	@call rd /S /Q ..\in_mkv\x64_Release
	)
@if exist ..\in_mkv\.vs (
	@echo Delete ..\in_mkv\.vs
	@call rd /S /Q ..\in_mkv\.vs
	)
@if exist ..\in_mkv\*.user (
	@echo Delete ..\in_mkv\*.user
	@call del ..\in_mkv\*.user
	)
@if exist ..\in_mkv\*.filters (
	@echo Delete ..\in_mkv\*.filters
	@call del ..\in_mkv\*.filters
	)
@if exist ..\in_mkv\*.htm (
	@echo Delete ..\in_mkv\*.htm
	@call del ..\in_mkv\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_midi project
@echo *******************************
@if exist ..\in_midi\x86_Debug (
	@echo Delete ..\in_midi\x86_Debug
	@call rd /S /Q ..\in_midi\x86_Debug
	)
@if exist ..\in_midi\x86_Release (
	@echo Delete ..\in_midi\x86_Release
	@call rd /S /Q ..\in_midi\x86_Release
	)
@if exist ..\in_midi\x64_Debug (
	@echo Delete ..\in_midi\x64_Debug
	@call rd /S /Q ..\in_midi\x64_Debug
	)
@if exist ..\in_midi\x64_Release (
	@echo Delete ..\in_midi\x64_Release
	@call rd /S /Q ..\in_midi\x64_Release
	)
@if exist ..\in_midi\.vs (
	@echo Delete ..\in_midi\.vs
	@call rd /S /Q ..\in_midi\.vs
	)
@if exist ..\in_midi\*.user (
	@echo Delete ..\in_midi\*.user
	@call del ..\in_midi\*.user
	)
@if exist ..\in_midi\*.filters (
	@echo Delete ..\in_midi\*.filters
	@call del ..\in_midi\*.filters
	)
@if exist ..\in_midi\*.htm (
	@echo Delete ..\in_midi\*.htm
	@call del ..\in_midi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_mod-openmpt project
@echo *******************************
@if exist ..\in_mod-openmpt\x86_Debug (
	@echo Delete ..\in_mod-openmpt\x86_Debug
	@call rd /S /Q ..\in_mod-openmpt\x86_Debug
	)
@if exist ..\in_mod-openmpt\x86_Release (
	@echo Delete ..\in_mod-openmpt\x86_Release
	@call rd /S /Q ..\in_mod-openmpt\x86_Release
	)
@if exist ..\in_mod-openmpt\x64_Debug (
	@echo Delete ..\in_mod-openmpt\x64_Debug
	@call rd /S /Q ..\in_mod-openmpt\x64_Debug
	)
@if exist ..\in_mod-openmpt\x64_Release (
	@echo Delete ..\in_mod-openmpt\x64_Release
	@call rd /S /Q ..\in_mod-openmpt\x64_Release
	)
@if exist ..\in_mod-openmpt\.vs (
	@echo Delete ..\in_mod-openmpt\.vs
	@call rd /S /Q ..\in_mod-openmpt\.vs
	)
@if exist ..\in_mod-openmpt\*.user (
	@echo Delete ..\in_mod-openmpt\*.user
	@call del ..\in_mod-openmpt\*.user
	)
@if exist ..\in_mod-openmpt\*.filters (
	@echo Delete ..\in_mod-openmpt\*.filters
	@call del ..\in_mod-openmpt\*.filters
	)
@if exist ..\in_mod-openmpt\*.htm (
	@echo Delete ..\in_mod-openmpt\*.htm
	@call del ..\in_mod-openmpt\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_mp3 project
@echo *******************************
@if exist ..\in_mp3\x86_Debug (
	@echo Delete ..\in_mp3\x86_Debug
	@call rd /S /Q ..\in_mp3\x86_Debug
	)
@if exist ..\in_mp3\x86_Release (
	@echo Delete ..\in_mp3\x86_Release
	@call rd /S /Q ..\in_mp3\x86_Release
	)
@if exist ..\in_mp3\x64_Debug (
	@echo Delete ..\in_mp3\x64_Debug
	@call rd /S /Q ..\in_mp3\x64_Debug
	)
@if exist ..\in_mp3\x64_Release (
	@echo Delete ..\in_mp3\x64_Release
	@call rd /S /Q ..\in_mp3\x64_Release
	)
@if exist ..\in_mp3\.vs (
	@echo Delete ..\in_mp3\.vs
	@call rd /S /Q ..\in_mp3\.vs
	)
@if exist ..\in_mp3\*.user (
	@echo Delete ..\in_mp3\*.user
	@call del ..\in_mp3\*.user
	)
@if exist ..\in_mp3\*.filters (
	@echo Delete ..\in_mp3\*.filters
	@call del ..\in_mp3\*.filters
	)
@if exist ..\in_mp3\*.htm (
	@echo Delete ..\in_mp3\*.htm
	@call del ..\in_mp3\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_mp4 project
@echo *******************************
@if exist ..\in_mp4\x86_Debug (
	@echo Delete ..\in_mp4\x86_Debug
	@call rd /S /Q ..\in_mp4\x86_Debug
	)
@if exist ..\in_mp4\x86_Release (
	@echo Delete ..\in_mp4\x86_Release
	@call rd /S /Q ..\in_mp4\x86_Release
	)
@if exist ..\in_mp4\x64_Debug (
	@echo Delete ..\in_mp4\x64_Debug
	@call rd /S /Q ..\in_mp4\x64_Debug
	)
@if exist ..\in_mp4\x64_Release (
	@echo Delete ..\in_mp4\x64_Release
	@call rd /S /Q ..\in_mp4\x64_Release
	)
@if exist ..\in_mp4\.vs (
	@echo Delete ..\in_mp4\.vs
	@call rd /S /Q ..\in_mp4\.vs
	)
@if exist ..\in_mp4\*.user (
	@echo Delete ..\in_mp4\*.user
	@call del ..\in_mp4\*.user
	)
@if exist ..\in_mp4\*.filters (
	@echo Delete ..\in_mp4\*.filters
	@call del ..\in_mp4\*.filters
	)
@if exist ..\in_mp4\*.htm (
	@echo Delete ..\in_mp4\*.htm
	@call del ..\in_mp4\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_nsv project
@echo *******************************
@if exist ..\in_nsv\x86_Debug (
	@echo Delete ..\in_nsv\x86_Debug
	@call rd /S /Q ..\in_nsv\x86_Debug
	)
@if exist ..\in_nsv\x86_Release (
	@echo Delete ..\in_nsv\x86_Release
	@call rd /S /Q ..\in_nsv\x86_Release
	)
@if exist ..\in_nsv\x64_Debug (
	@echo Delete ..\in_nsv\x64_Debug
	@call rd /S /Q ..\in_nsv\x64_Debug
	)
@if exist ..\in_nsv\x64_Release (
	@echo Delete ..\in_nsv\x64_Release
	@call rd /S /Q ..\in_nsv\x64_Release
	)
@if exist ..\in_nsv\.vs (
	@echo Delete ..\in_nsv\.vs
	@call rd /S /Q ..\in_nsv\.vs
	)
@if exist ..\in_nsv\*.user (
	@echo Delete ..\in_nsv\*.user
	@call del ..\in_nsv\*.user
	)
@if exist ..\in_nsv\*.filters (
	@echo Delete ..\in_nsv\*.filters
	@call del ..\in_nsv\*.filters
	)
@if exist ..\in_nsv\*.htm (
	@echo Delete ..\in_nsv\*.htm
	@call del ..\in_nsv\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_swf project
@echo *******************************
@if exist ..\in_swf\x86_Debug (
	@echo Delete ..\in_swf\x86_Debug
	@call rd /S /Q ..\in_swf\x86_Debug
	)
@if exist ..\in_swf\x86_Release (
	@echo Delete ..\in_swf\x86_Release
	@call rd /S /Q ..\in_swf\x86_Release
	)
@if exist ..\in_swf\x64_Debug (
	@echo Delete ..\in_swf\x64_Debug
	@call rd /S /Q ..\in_swf\x64_Debug
	)
@if exist ..\in_swf\x64_Release (
	@echo Delete ..\in_swf\x64_Release
	@call rd /S /Q ..\in_swf\x64_Release
	)
@if exist ..\in_swf\.vs (
	@echo Delete ..\in_swf\.vs
	@call rd /S /Q ..\in_swf\.vs
	)
@if exist ..\in_swf\*.user (
	@echo Delete ..\in_swf\*.user
	@call del ..\in_swf\*.user
	)
@if exist ..\in_swf\*.filters (
	@echo Delete ..\in_swf\*.filters
	@call del ..\in_swf\*.filters
	)
@if exist ..\in_swf\*.htm (
	@echo Delete ..\in_swf\*.htm
	@call del ..\in_swf\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_vorbis project
@echo *******************************
@if exist ..\in_vorbis\x86_Debug (
	@echo Delete ..\in_vorbis\x86_Debug
	@call rd /S /Q ..\in_vorbis\x86_Debug
	)
@if exist ..\in_vorbis\x86_Release (
	@echo Delete ..\in_vorbis\x86_Release
	@call rd /S /Q ..\in_vorbis\x86_Release
	)
@if exist ..\in_vorbis\x64_Debug (
	@echo Delete ..\in_vorbis\x64_Debug
	@call rd /S /Q ..\in_vorbis\x64_Debug
	)
@if exist ..\in_vorbis\x64_Release (
	@echo Delete ..\in_vorbis\x64_Release
	@call rd /S /Q ..\in_vorbis\x64_Release
	)
@if exist ..\in_vorbis\.vs (
	@echo Delete ..\in_vorbis\.vs
	@call rd /S /Q ..\in_vorbis\.vs
	)
@if exist ..\in_vorbis\*.user (
	@echo Delete ..\in_vorbis\*.user
	@call del ..\in_vorbis\*.user
	)
@if exist ..\in_vorbis\*.filters (
	@echo Delete ..\in_vorbis\*.filters
	@call del ..\in_vorbis\*.filters
	)
@if exist ..\in_vorbis\*.htm (
	@echo Delete ..\in_vorbis\*.htm
	@call del ..\in_vorbis\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * in_wave project
@echo *******************************
@if exist ..\in_wave\x86_Debug (
	@echo Delete ..\in_wave\x86_Debug
	@call rd /S /Q ..\in_wave\x86_Debug
	)
@if exist ..\in_wave\x86_Release (
	@echo Delete ..\in_wave\x86_Release
	@call rd /S /Q ..\in_wave\x86_Release
	)
@if exist ..\in_wave\x64_Debug (
	@echo Delete ..\in_wave\x64_Debug
	@call rd /S /Q ..\in_wave\x64_Debug
	)
@if exist ..\in_wave\x64_Release (
	@echo Delete ..\in_wave\x64_Release
	@call rd /S /Q ..\in_wave\x64_Release
	)
@if exist ..\in_wave\.vs (
	@echo Delete ..\in_wave\.vs
	@call rd /S /Q ..\in_wave\.vs
	)
@if exist ..\in_wave\*.user (
	@echo Delete ..\in_wave\*.user
	@call del ..\in_wave\*.user
	)
@if exist ..\in_wave\*.filters (
	@echo Delete ..\in_wave\*.filters
	@call del ..\in_wave\*.filters
	)
@if exist ..\in_wave\*.htm (
	@echo Delete ..\in_wave\*.htm
	@call del ..\in_wave\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * in_wmvdrm project
@echo *******************************
@if exist ..\in_wmvdrm\x86_Debug (
	@echo Delete ..\in_wmvdrm\x86_Debug
	@call rd /S /Q ..\in_wmvdrm\x86_Debug
	)
@if exist ..\in_wmvdrm\x86_Release (
	@echo Delete ..\in_wmvdrm\x86_Release
	@call rd /S /Q ..\in_wmvdrm\x86_Release
	)
@if exist ..\in_wmvdrm\x64_Debug (
	@echo Delete ..\in_wmvdrm\x64_Debug
	@call rd /S /Q ..\in_wmvdrm\x64_Debug
	)
@if exist ..\in_wmvdrm\x64_Release (
	@echo Delete ..\in_wmvdrm\x64_Release
	@call rd /S /Q ..\in_wmvdrm\x64_Release
	)
@if exist ..\in_wmvdrm\.vs (
	@echo Delete ..\in_wmvdrm\.vs
	@call rd /S /Q ..\in_wmvdrm\.vs
	)
@if exist ..\in_wmvdrm\*.user (
	@echo Delete ..\in_wmvdrm\*.user
	@call del ..\in_wmvdrm\*.user
	)
@if exist ..\in_wmvdrm\*.filters (
	@echo Delete ..\in_wmvdrm\*.filters
	@call del ..\in_wmvdrm\*.filters
	)
@if exist ..\in_wmvdrm\*.htm (
	@echo Delete ..\in_wmvdrm\*.htm
	@call del ..\in_wmvdrm\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * jnetlib project
@echo *******************************
@if exist ..\replicant\jnetlib\x86_Debug (
	@echo Delete ..\replicant\jnetlib\x86_Debug
	@call rd /S /Q ..\replicant\jnetlib\x86_Debug
	)
@if exist ..\replicant\jnetlib\x86_Release (
	@echo Delete ..\replicant\jnetlib\x86_Release
	@call rd /S /Q ..\replicant\jnetlib\x86_Release
	)
@if exist ..\replicant\jnetlib\x64_Debug (
	@echo Delete ..\replicant\jnetlib\x64_Debug
	@call rd /S /Q ..\replicant\jnetlib\x64_Debug
	)
@if exist ..\replicant\jnetlib\x64_Release (
	@echo Delete ..\replicant\jnetlib\x64_Release
	@call rd /S /Q ..\replicant\jnetlib\x64_Release
	)
@if exist ..\replicant\jnetlib\.vs (
	@echo Delete ..\replicant\jnetlib\.vs
	@call rd /S /Q ..\replicant\jnetlib\.vs
	)
@if exist ..\replicant\jnetlib\*.user (
	@echo Delete ..\replicant\jnetlib\*.user
	@call del ..\replicant\jnetlib\*.user
	)
@if exist ..\replicant\jnetlib\*.filters (
	@echo Delete ..\replicant\jnetlib\*.filters
	@call del ..\replicant\jnetlib\*.filters
	)
@if exist ..\replicant\jnetlib\*.htm (
	@echo Delete ..\replicant\jnetlib\*.htm
	@call del ..\replicant\jnetlib\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * jpeg project
@echo *******************************
@if exist ..\jpeg\x86_Debug (
	@echo Delete ..\jpeg\x86_Debug
	@call rd /S /Q ..\jpeg\x86_Debug
	)
@if exist ..\jpeg\x86_Release (
	@echo Delete ..\jpeg\x86_Release
	@call rd /S /Q ..\jpeg\x86_Release
	)
@if exist ..\jpeg\x64_Debug (
	@echo Delete ..\jpeg\x64_Debug
	@call rd /S /Q ..\jpeg\x64_Debug
	)
@if exist ..\jpeg\x64_Release (
	@echo Delete ..\jpeg\x64_Release
	@call rd /S /Q ..\jpeg\x64_Release
	)
@if exist ..\jpeg\.vs (
	@echo Delete ..\jpeg\.vs
	@call rd /S /Q ..\jpeg\.vs
	)
@if exist ..\jpeg\*.user (
	@echo Delete ..\jpeg\*.user
	@call del ..\jpeg\*.user
	)
@if exist ..\jpeg\*.filters (
	@echo Delete ..\jpeg\*.filters
	@call del ..\jpeg\*.filters
	)
@if exist ..\jpeg\*.htm (
	@echo Delete ..\jpeg\*.htm
	@call del ..\jpeg\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * libFLAC project
@echo *******************************
@if exist ..\libFLAC\x86_Debug (
	@echo Delete ..\libFLAC\x86_Debug
	@call rd /S /Q ..\libFLAC\x86_Debug
	)
@if exist ..\libFLAC\x86_Release (
	@echo Delete ..\libFLAC\x86_Release
	@call rd /S /Q ..\libFLAC\x86_Release
	)
@if exist ..\libFLAC\x64_Debug (
	@echo Delete ..\libFLAC\x64_Debug
	@call rd /S /Q ..\libFLAC\x64_Debug
	)
@if exist ..\libFLAC\x64_Release (
	@echo Delete ..\libFLAC\x64_Release
	@call rd /S /Q ..\libFLAC\x64_Release
	)
@if exist ..\libFLAC\.vs (
	@echo Delete ..\libFLAC\.vs
	@call rd /S /Q ..\libFLAC\.vs
	)
@if exist ..\libFLAC\*.user (
	@echo Delete ..\libFLAC\*.user
	@call del ..\libFLAC\*.user
	)
@if exist ..\libFLAC\*.filters (
	@echo Delete ..\libFLAC\*.filters
	@call del ..\libFLAC\*.filters
	)
@if exist ..\libFLAC\*.htm (
	@echo Delete ..\libFLAC\*.htm
	@call del ..\libFLAC\*.htm
	)
@if exist ..\libFLAC\src\libFLAC\ia32\*.obj (
	@echo Delete ..\libFLAC\src\libFLAC\ia32\*.obj
	@call del ..\libFLAC\src\libFLAC\ia32\*.obj
	)	
@echo -------------------------------
@echo *******************************
@echo * libmp4v2 project
@echo *******************************
@if exist ..\libmp4v2\x86_Debug (
	@echo Delete ..\libmp4v2\x86_Debug
	@call rd /S /Q ..\libmp4v2\x86_Debug
	)
@if exist ..\libmp4v2\x86_Release (
	@echo Delete ..\libmp4v2\x86_Release
	@call rd /S /Q ..\libmp4v2\x86_Release
	)
@if exist ..\libmp4v2\x64_Debug (
	@echo Delete ..\libmp4v2\x64_Debug
	@call rd /S /Q ..\libmp4v2\x64_Debug
	)
@if exist ..\libmp4v2\x64_Release (
	@echo Delete ..\libmp4v2\x64_Release
	@call rd /S /Q ..\libmp4v2\x64_Release
	)
@if exist ..\libmp4v2\.vs (
	@echo Delete ..\libmp4v2\.vs
	@call rd /S /Q ..\libmp4v2\.vs
	)
@if exist ..\libmp4v2\*.user (
	@echo Delete ..\libmp4v2\*.user
	@call del ..\libmp4v2\*.user
	)
@if exist ..\libmp4v2\*.filters (
	@echo Delete ..\libmp4v2\*.filters
	@call del ..\libmp4v2\*.filters
	)
@if exist ..\libmp4v2\*.htm (
	@echo Delete ..\libmp4v2\*.htm
	@call del ..\libmp4v2\*.htm
	)
	
	
	
@echo -------------------------------
@echo *******************************
@echo * libogg project
@echo *******************************
@if exist ..\libogg\x86_Debug (
	@echo Delete ..\libogg\x86_Debug
	@call rd /S /Q ..\libogg\x86_Debug
	)
@if exist ..\libogg\x86_Release (
	@echo Delete ..\libogg\x86_Release
	@call rd /S /Q ..\libogg\x86_Release
	)
@if exist ..\libogg\x64_Debug (
	@echo Delete ..\libogg\x64_Debug
	@call rd /S /Q ..\libogg\x64_Debug
	)
@if exist ..\libogg\x64_Release (
	@echo Delete ..\libogg\x64_Release
	@call rd /S /Q ..\libogg\x64_Release
	)
@if exist ..\libogg\.vs (
	@echo Delete ..\libogg\.vs
	@call rd /S /Q ..\libogg\.vs
	)
@if exist ..\libogg\*.user (
	@echo Delete ..\libogg\*.user
	@call del ..\libogg\*.user
	)
@if exist ..\libogg\*.filters (
	@echo Delete ..\libogg\*.filters
	@call del ..\libogg\*.filters
	)
@if exist ..\libogg\*.htm (
	@echo Delete ..\libogg\*.htm
	@call del ..\libogg\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * libopenmpt project
@echo *******************************
@if exist ..\libopenmpt\x86_Debug (
	@echo Delete ..\libopenmpt\x86_Debug
	@call rd /S /Q ..\libopenmpt\x86_Debug
	)
@if exist ..\libopenmpt\x86_Debug_libmodplug (
	@echo Delete ..\libopenmpt\x86_Debug_libmodplug
	@call rd /S /Q ..\libopenmpt\x86_Debug_libmodplug
	)	
@if exist ..\libopenmpt\x86_Debug_mpg123 (
	@echo Delete ..\libopenmpt\x86_Debug_mpg123
	@call rd /S /Q ..\libopenmpt\x86_Debug_mpg123
	)	
@if exist ..\libopenmpt\x86_Debug_ogg (
	@echo Delete ..\libopenmpt\x86_Debug_ogg
	@call rd /S /Q ..\libopenmpt\x86_Debug_ogg
	)		
@if exist ..\libopenmpt\x86_Debug_portaudio (
	@echo Delete ..\libopenmpt\x86_Debug_portaudio
	@call rd /S /Q ..\libopenmpt\x86_Debug_portaudio
	)	
@if exist ..\libopenmpt\x86_Debug_portaudiocpp (
	@echo Delete ..\libopenmpt\x86_Debug_portaudiocpp
	@call rd /S /Q ..\libopenmpt\x86_Debug_portaudiocpp
	)	
@if exist ..\libopenmpt\x86_Debug_vorbis (
	@echo Delete ..\libopenmpt\x86_Debug_vorbis
	@call rd /S /Q ..\libopenmpt\x86_Debug_vorbis
	)	
@if exist ..\libopenmpt\x86_Debug_zlib (
	@echo Delete ..\libopenmpt\x86_Debug_zlib
	@call rd /S /Q ..\libopenmpt\x86_Debug_zlib
	)	
@if exist ..\libopenmpt\x86_Release (
	@echo Delete ..\libopenmpt\x86_Release
	@call rd /S /Q ..\libopenmpt\x86_Release
	)
@if exist ..\libopenmpt\x86_Release_libmodplug (
	@echo Delete ..\libopenmpt\x86_Release_libmodplug
	@call rd /S /Q ..\libopenmpt\x86_Release_libmodplug
	)	
@if exist ..\libopenmpt\x86_Release_mpg123 (
	@echo Delete ..\libopenmpt\x86_Release_mpg123
	@call rd /S /Q ..\libopenmpt\x86_Release_mpg123
	)	
@if exist ..\libopenmpt\x86_Release_ogg (
	@echo Delete ..\libopenmpt\x86_Release_ogg
	@call rd /S /Q ..\libopenmpt\x86_Release_ogg
	)	
@if exist ..\libopenmpt\x86_Release_portaudio (
	@echo Delete ..\libopenmpt\x86_Release_portaudio
	@call rd /S /Q ..\libopenmpt\x86_Release_portaudio
	)	
@if exist ..\libopenmpt\x86_Release_portaudiocpp (
	@echo Delete ..\libopenmpt\x86_Release_portaudiocpp
	@call rd /S /Q ..\libopenmpt\x86_Release_portaudiocpp
	)		
@if exist ..\libopenmpt\x86_Release_vorbis (
	@echo Delete ..\libopenmpt\x86_Release_vorbis
	@call rd /S /Q ..\libopenmpt\x86_Release_vorbis
	)	
@if exist ..\libopenmpt\x86_Release_zlib (
	@echo Delete ..\libopenmpt\x86_Release_zlib
	@call rd /S /Q ..\libopenmpt\x86_Release_zlib
	)	
@if exist ..\libopenmpt\x64_Debug (
	@echo Delete ..\libopenmpt\x64_Debug
	@call rd /S /Q ..\libopenmpt\x64_Debug
	)	
@if exist ..\libopenmpt\x64_Debug_libmodplug (
	@echo Delete ..\libopenmpt\x64_Debug_libmodplug
	@call rd /S /Q ..\libopenmpt\x64_Debug_libmodplug
	)	
@if exist ..\libopenmpt\x64_Debug_mpg123 (
	@echo Delete ..\libopenmpt\x64_Debug_mpg123
	@call rd /S /Q ..\libopenmpt\x64_Debug_mpg123
	)
@if exist ..\libopenmpt\x64_Debug_ogg (
	@echo Delete ..\libopenmpt\x64_Debug_ogg
	@call rd /S /Q ..\libopenmpt\x64_Debug_ogg
	)	
@if exist ..\libopenmpt\x64_Debug_portaudio (
	@echo Delete ..\libopenmpt\x64_Debug_portaudio
	@call rd /S /Q ..\libopenmpt\x64_Debug_portaudio
	)	
@if exist ..\libopenmpt\x64_Debug_portaudiocpp (
	@echo Delete ..\libopenmpt\x64_Debug_portaudiocpp
	@call rd /S /Q ..\libopenmpt\x64_Debug_portaudiocpp
	)
@if exist ..\libopenmpt\x64_Debug_vorbis (
	@echo Delete ..\libopenmpt\x64_Debug_vorbis
	@call rd /S /Q ..\libopenmpt\x64_Debug_vorbis
	)
@if exist ..\libopenmpt\x64_Debug_zlib (
	@echo Delete ..\libopenmpt\x64_Debug_zlib
	@call rd /S /Q ..\libopenmpt\x64_Debug_zlib
	)
@if exist ..\libopenmpt\x64_Release (
	@echo Delete ..\libopenmpt\x64_Release
	@call rd /S /Q ..\libopenmpt\x64_Release
	)
@if exist ..\libopenmpt\x64_Release_libmodplug (
	@echo Delete ..\libopenmpt\x64_Release_libmodplug
	@call rd /S /Q ..\libopenmpt\x64_Release_libmodplug
	)
@if exist ..\libopenmpt\x64_Release_mpg123 (
	@echo Delete ..\libopenmpt\x64_Release_mpg123
	@call rd /S /Q ..\libopenmpt\x64_Release_mpg123
	)
@if exist ..\libopenmpt\x64_Release_ogg (
	@echo Delete ..\libopenmpt\x64_Release_ogg
	@call rd /S /Q ..\libopenmpt\x64_Release_ogg
	)
@if exist ..\libopenmpt\x64_Release_portaudio (
	@echo Delete ..\libopenmpt\x64_Release_portaudio
	@call rd /S /Q ..\libopenmpt\x64_Release_portaudio
	)
@if exist ..\libopenmpt\x64_Release_portaudiocpp (
	@echo Delete ..\libopenmpt\x64_Release_portaudiocpp
	@call rd /S /Q ..\libopenmpt\x64_Release_portaudiocpp
	)
@if exist ..\libopenmpt\x64_Release_vorbis (
	@echo Delete ..\libopenmpt\x64_Release_vorbis
	@call rd /S /Q ..\libopenmpt\x64_Release_vorbis
	)
@if exist ..\libopenmpt\x64_Release_zlib (
	@echo Delete ..\libopenmpt\x64_Release_zlib
	@call rd /S /Q ..\libopenmpt\x64_Release_zlib
	)
@if exist ..\libopenmpt\.vs (
	@echo Delete ..\libopenmpt\.vs
	@call rd /S /Q ..\libopenmpt\.vs
	)
@if exist ..\libopenmpt\*.user (
	@echo Delete ..\libopenmpt\*.user
	@call del ..\libopenmpt\*.user
	)
@if exist ..\libopenmpt\*.filters (
	@echo Delete ..\libopenmpt\*.filters
	@call del ..\libopenmpt\*.filters
	)
@if exist ..\libopenmpt\*.htm (
	@echo Delete ..\libopenmpt\*.htm
	@call del ..\libopenmpt\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * libpng project
@echo *******************************
@if exist ..\libpng\x86_Debug (
	@echo Delete ..\libpng\x86_Debug
	@call rd /S /Q ..\libpng\x86_Debug
	)
@if exist ..\libpng\x86_Release (
	@echo Delete ..\libpng\x86_Release
	@call rd /S /Q ..\libpng\x86_Release
	)
@if exist ..\libpng\x64_Debug (
	@echo Delete ..\libpng\x64_Debug
	@call rd /S /Q ..\libpng\x64_Debug
	)
@if exist ..\libpng\x64_Release (
	@echo Delete ..\libpng\x64_Release
	@call rd /S /Q ..\libpng\x64_Release
	)
@if exist ..\libpng\.vs (
	@echo Delete ..\libpng\.vs
	@call rd /S /Q ..\libpng\.vs
	)
@if exist ..\libpng\*.user (
	@echo Delete ..\libpng\*.user
	@call del ..\libpng\*.user
	)
@if exist ..\libpng\*.filters (
	@echo Delete ..\libpng\*.filters
	@call del ..\libpng\*.filters
	)
@if exist ..\libpng\*.htm (
	@echo Delete ..\libpng\*.htm
	@call del ..\libpng\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * libsndfile project
@echo *******************************
@if exist ..\libsndfile\x86_Debug (
	@echo Delete ..\libsndfile\x86_Debug
	@call rd /S /Q ..\libsndfile\x86_Debug
	)
@if exist ..\libsndfile\x86_Release (
	@echo Delete ..\libsndfile\x86_Release
	@call rd /S /Q ..\libsndfile\x86_Release
	)
@if exist ..\libsndfile\x64_Debug (
	@echo Delete ..\libsndfile\x64_Debug
	@call rd /S /Q ..\libsndfile\x64_Debug
	)
@if exist ..\libsndfile\x64_Release (
	@echo Delete ..\libsndfile\x64_Release
	@call rd /S /Q ..\libsndfile\x64_Release
	)
@if exist ..\libsndfile\.vs (
	@echo Delete ..\libsndfile\.vs
	@call rd /S /Q ..\libsndfile\.vs
	)
@if exist ..\libsndfile\*.user (
	@echo Delete ..\libsndfile\*.user
	@call del ..\libsndfile\*.user
	)
@if exist ..\libsndfile\*.filters (
	@echo Delete ..\libsndfile\*.filters
	@call del ..\libsndfile\*.filters
	)
@if exist ..\libsndfile\*.htm (
	@echo Delete ..\libsndfile\*.htm
	@call del ..\libsndfile\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * libtheora project
@echo *******************************
@if exist ..\libtheora\x86_Debug (
	@echo Delete ..\libtheora\x86_Debug
	@call rd /S /Q ..\libtheora\x86_Debug
	)
@if exist ..\libtheora\x86_Release (
	@echo Delete ..\libtheora\x86_Release
	@call rd /S /Q ..\libtheora\x86_Release
	)
@if exist ..\libtheora\x64_Debug (
	@echo Delete ..\libtheora\x64_Debug
	@call rd /S /Q ..\libtheora\x64_Debug
	)
@if exist ..\libtheora\x64_Release (
	@echo Delete ..\libtheora\x64_Release
	@call rd /S /Q ..\libtheora\x64_Release
	)
@if exist ..\libtheora\.vs (
	@echo Delete ..\libtheora\.vs
	@call rd /S /Q ..\libtheora\.vs
	)
@if exist ..\libtheora\*.user (
	@echo Delete ..\libtheora\*.user
	@call del ..\libtheora\*.user
	)
@if exist ..\libtheora\*.filters (
	@echo Delete ..\libtheora\*.filters
	@call del ..\libtheora\*.filters
	)
@if exist ..\libtheora\*.htm (
	@echo Delete ..\libtheora\*.htm
	@call del ..\libtheora\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * libvorbis project
@echo *******************************
@if exist ..\libvorbis\win32\x86_Debug (
	@echo Delete ..\libvorbis\win32\x86_Debug
	@call rd /S /Q ..\libvorbis\win32\x86_Debug
	)
@if exist ..\libvorbis\win32\x86_Release (
	@echo Delete ..\libvorbis\win32\x86_Release
	@call rd /S /Q ..\libvorbis\win32\x86_Release
	)
@if exist ..\libvorbis\win32\x64_Debug (
	@echo Delete ..\libvorbis\win32\x64_Debug
	@call rd /S /Q ..\libvorbis\win32\x64_Debug
	)
@if exist ..\libvorbis\win32\x64_Release (
	@echo Delete ..\libvorbis\win32\x64_Release
	@call rd /S /Q ..\libvorbis\win32\x64_Release
	)
	@if exist ..\libvorbis\win32\x86_Debug_vorbisfile (
	@echo Delete ..\libvorbis\win32\x86_Debug_vorbisfile
	@call rd /S /Q ..\libvorbis\win32\x86_Debug_vorbisfile
	)
@if exist ..\libvorbis\win32\x86_Release_vorbisfile (
	@echo Delete ..\libvorbis\win32\x86_Release_vorbisfile
	@call rd /S /Q ..\libvorbis\win32\x86_Release_vorbisfile
	)
@if exist ..\libvorbis\win32\x64_Debug_vorbisfile (
	@echo Delete ..\libvorbis\win32\x64_Debug_vorbisfile
	@call rd /S /Q ..\libvorbis\win32\x64_Debug_vorbisfile
	)
@if exist ..\libvorbis\win32\x64_Release_vorbisfile (
	@echo Delete ..\libvorbis\win32\x64_Release_vorbisfile
	@call rd /S /Q ..\libvorbis\win32\x64_Release_vorbisfile
	)
@if exist ..\libvorbis\win32\.vs (
	@echo Delete ..\libvorbis\win32\.vs
	@call rd /S /Q ..\libvorbis\win32\.vs
	)
@if exist ..\libvorbis\win32\*.user (
	@echo Delete ..\libvorbis\win32\*.user
	@call del ..\libvorbis\win32\*.user
	)
@if exist ..\libvorbis\win32\*.filters (
	@echo Delete ..\libvorbis\win32\*.filters
	@call del ..\libvorbis\win32\*.filters
	)
@if exist ..\libvorbis\win32\*.htm (
	@echo Delete ..\libvorbis\win32\*.htm
	@call del ..\libvorbis\win32\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * libyajl project
@echo *******************************
@if exist ..\replicant\libyajl\x86_Debug (
	@echo Delete ..\replicant\libyajl\x86_Debug
	@call rd /S /Q ..\replicant\libyajl\x86_Debug
	)
@if exist ..\replicant\libyajl\x86_Release (
	@echo Delete ..\replicant\libyajl\x86_Release
	@call rd /S /Q ..\replicant\libyajl\x86_Release
	)
@if exist ..\replicant\libyajl\x64_Debug (
	@echo Delete ..\replicant\libyajl\x64_Debug
	@call rd /S /Q ..\replicant\libyajl\x64_Debug
	)
@if exist ..\replicant\libyajl\x64_Release (
	@echo Delete ..\replicant\libyajl\x64_Release
	@call rd /S /Q ..\replicant\libyajl\x64_Release
	)
@if exist ..\replicant\libyajl\.vs (
	@echo Delete ..\replicant\libyajl\.vs
	@call rd /S /Q ..\replicant\libyajl\.vs
	)
@if exist ..\replicant\libyajl\*.user (
	@echo Delete ..\replicant\libyajl\*.user
	@call del ..\replicant\libyajl\*.user
	)
@if exist ..\replicant\libyajl\*.filters (
	@echo Delete ..\replicant\libyajl\*.filters
	@call del ..\replicant\libyajl\*.filters
	)
@if exist ..\replicant\libyajl\*.htm (
	@echo Delete ..\replicant\libyajl\*.htm
	@call del ..\replicant\libyajl\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * metadata project
@echo *******************************
@if exist ..\replicant\replicant\metadata\x86_Debug (
	@echo Delete ..\replicant\replicant\metadata\x86_Debug
	@call rd /S /Q ..\replicant\replicant\metadata\x86_Debug
	)
@if exist ..\replicant\replicant\metadata\x86_Release (
	@echo Delete ..\replicant\replicant\metadata\x86_Release
	@call rd /S /Q ..\replicant\replicant\metadata\x86_Release
	)
@if exist ..\replicant\replicant\metadata\x64_Debug (
	@echo Delete ..\replicant\replicant\metadata\x64_Debug
	@call rd /S /Q ..\replicant\replicant\metadata\x64_Debug
	)
@if exist ..\replicant\replicant\metadata\x64_Release (
	@echo Delete ..\replicant\replicant\metadata\x64_Release
	@call rd /S /Q ..\replicant\replicant\metadata\x64_Release
	)
@if exist ..\replicant\replicant\metadata\.vs (
	@echo Delete ..\replicant\replicant\metadata\.vs
	@call rd /S /Q ..\replicant\replicant\metadata\.vs
	)
@if exist ..\replicant\replicant\metadata\*.user (
	@echo Delete ..\replicant\replicant\metadata\*.user
	@call del ..\replicant\replicant\metadata\*.user
	)
@if exist ..\replicant\replicant\metadata\*.filters (
	@echo Delete ..\replicant\replicant\metadata\*.filters
	@call del ..\replicant\replicant\metadata\*.filters
	)
@if exist ..\replicant\replicant\metadata\*.htm (
	@echo Delete ..\replicant\replicant\metadata\*.htm
	@call del ..\replicant\replicant\metadata\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_autotag project
@echo *******************************
@if exist ..\ml_autotag\x86_Debug (
	@echo Delete ..\ml_autotag\x86_Debug
	@call rd /S /Q ..\ml_autotag\x86_Debug
	)
@if exist ..\ml_autotag\x86_Release (
	@echo Delete ..\ml_autotag\x86_Release
	@call rd /S /Q ..\ml_autotag\x86_Release
	)
@if exist ..\ml_autotag\x64_Debug (
	@echo Delete ..\ml_autotag\x64_Debug
	@call rd /S /Q ..\ml_autotag\x64_Debug
	)
@if exist ..\ml_autotag\x64_Release (
	@echo Delete ..\ml_autotag\x64_Release
	@call rd /S /Q ..\ml_autotag\x64_Release
	)
@if exist ..\ml_autotag\.vs (
	@echo Delete ..\ml_autotag\.vs
	@call rd /S /Q ..\ml_autotag\.vs
	)
@if exist ..\ml_autotag\*.user (
	@echo Delete ..\ml_autotag\*.user
	@call del ..\ml_autotag\*.user
	)
@if exist ..\ml_autotag\*.filters (
	@echo Delete ..\ml_autotag\*.filters
	@call del ..\ml_autotag\*.filters
	)
@if exist ..\ml_autotag\*.htm (
	@echo Delete ..\ml_autotag\*.htm
	@call del ..\ml_autotag\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_bookmarks project
@echo *******************************
@if exist ..\ml_bookmarks\x86_Debug (
	@echo Delete ..\ml_bookmarks\x86_Debug
	@call rd /S /Q ..\ml_bookmarks\x86_Debug
	)
@if exist ..\ml_bookmarks\x86_Release (
	@echo Delete ..\ml_bookmarks\x86_Release
	@call rd /S /Q ..\ml_bookmarks\x86_Release
	)
@if exist ..\ml_bookmarks\x64_Debug (
	@echo Delete ..\ml_bookmarks\x64_Debug
	@call rd /S /Q ..\ml_bookmarks\x64_Debug
	)
@if exist ..\ml_bookmarks\x64_Release (
	@echo Delete ..\ml_bookmarks\x64_Release
	@call rd /S /Q ..\ml_bookmarks\x64_Release
	)
@if exist ..\ml_bookmarks\.vs (
	@echo Delete ..\ml_bookmarks\.vs
	@call rd /S /Q ..\ml_bookmarks\.vs
	)
@if exist ..\ml_bookmarks\*.user (
	@echo Delete ..\ml_bookmarks\*.user
	@call del ..\ml_bookmarks\*.user
	)
@if exist ..\ml_bookmarks\*.filters (
	@echo Delete ..\ml_bookmarks\*.filters
	@call del ..\ml_bookmarks\*.filters
	)
@if exist ..\ml_bookmarks\*.htm (
	@echo Delete ..\ml_bookmarks\*.htm
	@call del ..\ml_bookmarks\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_devices project
@echo *******************************
@if exist ..\ml_devices\x86_Debug (
	@echo Delete ..\ml_devices\x86_Debug
	@call rd /S /Q ..\ml_devices\x86_Debug
	)
@if exist ..\ml_devices\x86_Release (
	@echo Delete ..\ml_devices\x86_Release
	@call rd /S /Q ..\ml_devices\x86_Release
	)
@if exist ..\ml_devices\x64_Debug (
	@echo Delete ..\ml_devices\x64_Debug
	@call rd /S /Q ..\ml_devices\x64_Debug
	)
@if exist ..\ml_devices\x64_Release (
	@echo Delete ..\ml_devices\x64_Release
	@call rd /S /Q ..\ml_devices\x64_Release
	)
@if exist ..\ml_devices\.vs (
	@echo Delete ..\ml_devices\.vs
	@call rd /S /Q ..\ml_devices\.vs
	)
@if exist ..\ml_devices\*.user (
	@echo Delete ..\ml_devices\*.user
	@call del ..\ml_devices\*.user
	)
@if exist ..\ml_devices\*.filters (
	@echo Delete ..\ml_devices\*.filters
	@call del ..\ml_devices\*.filters
	)
@if exist ..\ml_devices\*.htm (
	@echo Delete ..\ml_devices\*.htm
	@call del ..\ml_devices\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_disc project
@echo *******************************
@if exist ..\ml_disc\x86_Debug (
	@echo Delete ..\ml_disc\x86_Debug
	@call rd /S /Q ..\ml_disc\x86_Debug
	)
@if exist ..\ml_disc\x86_Release (
	@echo Delete ..\ml_disc\x86_Release
	@call rd /S /Q ..\ml_disc\x86_Release
	)
@if exist ..\ml_disc\x64_Debug (
	@echo Delete ..\ml_disc\x64_Debug
	@call rd /S /Q ..\ml_disc\x64_Debug
	)
@if exist ..\ml_disc\x64_Release (
	@echo Delete ..\ml_disc\x64_Release
	@call rd /S /Q ..\ml_disc\x64_Release
	)
@if exist ..\ml_disc\.vs (
	@echo Delete ..\ml_disc\.vs
	@call rd /S /Q ..\ml_disc\.vs
	)
@if exist ..\ml_disc\*.user (
	@echo Delete ..\ml_disc\*.user
	@call del ..\ml_disc\*.user
	)
@if exist ..\ml_disc\*.filters (
	@echo Delete ..\ml_disc\*.filters
	@call del ..\ml_disc\*.filters
	)
@if exist ..\ml_disc\*.htm (
	@echo Delete ..\ml_disc\*.htm
	@call del ..\ml_disc\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_downloads project
@echo *******************************
@if exist ..\ml_downloads\x86_Debug (
	@echo Delete ..\ml_downloads\x86_Debug
	@call rd /S /Q ..\ml_downloads\x86_Debug
	)
@if exist ..\ml_downloads\x86_Release (
	@echo Delete ..\ml_downloads\x86_Release
	@call rd /S /Q ..\ml_downloads\x86_Release
	)
@if exist ..\ml_downloads\x64_Debug (
	@echo Delete ..\ml_downloads\x64_Debug
	@call rd /S /Q ..\ml_downloads\x64_Debug
	)
@if exist ..\ml_downloads\x64_Release (
	@echo Delete ..\ml_downloads\x64_Release
	@call rd /S /Q ..\ml_downloads\x64_Release
	)
@if exist ..\ml_downloads\.vs (
	@echo Delete ..\ml_downloads\.vs
	@call rd /S /Q ..\ml_downloads\.vs
	)
@if exist ..\ml_downloads\*.user (
	@echo Delete ..\ml_downloads\*.user
	@call del ..\ml_downloads\*.user
	)
@if exist ..\ml_downloads\*.filters (
	@echo Delete ..\ml_downloads\*.filters
	@call del ..\ml_downloads\*.filters
	)
@if exist ..\ml_downloads\*.htm (
	@echo Delete ..\ml_downloads\*.htm
	@call del ..\ml_downloads\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_impex project
@echo *******************************
@if exist ..\ml_impex\x86_Debug (
	@echo Delete ..\ml_impex\x86_Debug
	@call rd /S /Q ..\ml_impex\x86_Debug
	)
@if exist ..\ml_impex\x86_Release (
	@echo Delete ..\ml_impex\x86_Release
	@call rd /S /Q ..\ml_impex\x86_Release
	)
@if exist ..\ml_impex\x64_Debug (
	@echo Delete ..\ml_impex\x64_Debug
	@call rd /S /Q ..\ml_impex\x64_Debug
	)
@if exist ..\ml_impex\x64_Release (
	@echo Delete ..\ml_impex\x64_Release
	@call rd /S /Q ..\ml_impex\x64_Release
	)
@if exist ..\ml_impex\.vs (
	@echo Delete ..\ml_impex\.vs
	@call rd /S /Q ..\ml_impex\.vs
	)
@if exist ..\ml_impex\*.user (
	@echo Delete ..\ml_impex\*.user
	@call del ..\ml_impex\*.user
	)
@if exist ..\ml_impex\*.filters (
	@echo Delete ..\ml_impex\*.filters
	@call del ..\ml_impex\*.filters
	)
@if exist ..\ml_impex\*.htm (
	@echo Delete ..\ml_impex\*.htm
	@call del ..\ml_impex\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_history project
@echo *******************************
@if exist ..\ml_history\x86_Debug (
	@echo Delete ..\ml_history\x86_Debug
	@call rd /S /Q ..\ml_history\x86_Debug
	)
@if exist ..\ml_history\x86_Release (
	@echo Delete ..\ml_history\x86_Release
	@call rd /S /Q ..\ml_history\x86_Release
	)
@if exist ..\ml_history\x64_Debug (
	@echo Delete ..\ml_history\x64_Debug
	@call rd /S /Q ..\ml_history\x64_Debug
	)
@if exist ..\ml_history\x64_Release (
	@echo Delete ..\ml_history\x64_Release
	@call rd /S /Q ..\ml_history\x64_Release
	)
@if exist ..\ml_history\.vs (
	@echo Delete ..\ml_history\.vs
	@call rd /S /Q ..\ml_history\.vs
	)
@if exist ..\ml_history\*.user (
	@echo Delete ..\ml_history\*.user
	@call del ..\ml_history\*.user
	)
@if exist ..\ml_history\*.filters (
	@echo Delete ..\ml_history\*.filters
	@call del ..\ml_history\*.filters
	)
@if exist ..\ml_history\*.htm (
	@echo Delete ..\ml_history\*.htm
	@call del ..\ml_history\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_local project
@echo *******************************
@if exist ..\ml_local\x86_Debug (
	@echo Delete ..\ml_local\x86_Debug
	@call rd /S /Q ..\ml_local\x86_Debug
	)
@if exist ..\ml_local\x86_Release (
	@echo Delete ..\ml_local\x86_Release
	@call rd /S /Q ..\ml_local\x86_Release
	)
@if exist ..\ml_local\x64_Debug (
	@echo Delete ..\ml_local\x64_Debug
	@call rd /S /Q ..\ml_local\x64_Debug
	)
@if exist ..\ml_local\x64_Release (
	@echo Delete ..\ml_local\x64_Release
	@call rd /S /Q ..\ml_local\x64_Release
	)
@if exist ..\ml_local\.vs (
	@echo Delete ..\ml_local\.vs
	@call rd /S /Q ..\ml_local\.vs
	)
@if exist ..\ml_local\*.user (
	@echo Delete ..\ml_local\*.user
	@call del ..\ml_local\*.user
	)
@if exist ..\ml_local\*.filters (
	@echo Delete ..\ml_local\*.filters
	@call del ..\ml_local\*.filters
	)
@if exist ..\ml_local\*.htm (
	@echo Delete ..\ml_local\*.htm
	@call del ..\ml_local\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_nowplaying project
@echo *******************************
@if exist ..\ml_nowplaying\x86_Debug (
	@echo Delete ..\ml_nowplaying\x86_Debug
	@call rd /S /Q ..\ml_nowplaying\x86_Debug
	)
@if exist ..\ml_nowplaying\x86_Release (
	@echo Delete ..\ml_nowplaying\x86_Release
	@call rd /S /Q ..\ml_nowplaying\x86_Release
	)
@if exist ..\ml_nowplaying\x64_Debug (
	@echo Delete ..\ml_nowplaying\x64_Debug
	@call rd /S /Q ..\ml_nowplaying\x64_Debug
	)
@if exist ..\ml_nowplaying\x64_Release (
	@echo Delete ..\ml_nowplaying\x64_Release
	@call rd /S /Q ..\ml_nowplaying\x64_Release
	)
@if exist ..\ml_nowplaying\.vs (
	@echo Delete ..\ml_nowplaying\.vs
	@call rd /S /Q ..\ml_nowplaying\.vs
	)
@if exist ..\ml_nowplaying\*.user (
	@echo Delete ..\ml_nowplaying\*.user
	@call del ..\ml_nowplaying\*.user
	)
@if exist ..\ml_nowplaying\*.filters (
	@echo Delete ..\ml_nowplaying\*.filters
	@call del ..\ml_nowplaying\*.filters
	)
@if exist ..\ml_nowplaying\*.htm (
	@echo Delete ..\ml_nowplaying\*.htm
	@call del ..\ml_nowplaying\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_online project
@echo *******************************
@if exist ..\ml_online\x86_Debug (
	@echo Delete ..\ml_online\x86_Debug
	@call rd /S /Q ..\ml_online\x86_Debug
	)
@if exist ..\ml_online\x86_Release (
	@echo Delete ..\ml_online\x86_Release
	@call rd /S /Q ..\ml_online\x86_Release
	)
@if exist ..\ml_online\x64_Debug (
	@echo Delete ..\ml_online\x64_Debug
	@call rd /S /Q ..\ml_online\x64_Debug
	)
@if exist ..\ml_online\x64_Release (
	@echo Delete ..\ml_online\x64_Release
	@call rd /S /Q ..\ml_online\x64_Release
	)
@if exist ..\ml_online\.vs (
	@echo Delete ..\ml_online\.vs
	@call rd /S /Q ..\ml_online\.vs
	)
@if exist ..\ml_online\*.user (
	@echo Delete ..\ml_online\*.user
	@call del ..\ml_online\*.user
	)
@if exist ..\ml_online\*.filters (
	@echo Delete ..\ml_online\*.filters
	@call del ..\ml_online\*.filters
	)
@if exist ..\ml_online\*.htm (
	@echo Delete ..\ml_online\*.htm
	@call del ..\ml_online\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_playlists project
@echo *******************************
@if exist ..\ml_playlists\x86_Debug (
	@echo Delete ..\ml_playlists\x86_Debug
	@call rd /S /Q ..\ml_playlists\x86_Debug
	)
@if exist ..\ml_playlists\x86_Release (
	@echo Delete ..\ml_playlists\x86_Release
	@call rd /S /Q ..\ml_playlists\x86_Release
	)
@if exist ..\ml_playlists\x64_Debug (
	@echo Delete ..\ml_playlists\x64_Debug
	@call rd /S /Q ..\ml_playlists\x64_Debug
	)
@if exist ..\ml_playlists\x64_Release (
	@echo Delete ..\ml_playlists\x64_Release
	@call rd /S /Q ..\ml_playlists\x64_Release
	)
@if exist ..\ml_playlists\.vs (
	@echo Delete ..\ml_playlists\.vs
	@call rd /S /Q ..\ml_playlists\.vs
	)
@if exist ..\ml_playlists\*.user (
	@echo Delete ..\ml_playlists\*.user
	@call del ..\ml_playlists\*.user
	)
@if exist ..\ml_playlists\*.filters (
	@echo Delete ..\ml_playlists\*.filters
	@call del ..\ml_playlists\*.filters
	)
@if exist ..\ml_playlists\*.htm (
	@echo Delete ..\ml_playlists\*.htm
	@call del ..\ml_playlists\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_plg project
@echo *******************************
@if exist ..\ml_plg\x86_Debug (
	@echo Delete ..\ml_plg\x86_Debug
	@call rd /S /Q ..\ml_plg\x86_Debug
	)
@if exist ..\ml_plg\x86_Release (
	@echo Delete ..\ml_plg\x86_Release
	@call rd /S /Q ..\ml_plg\x86_Release
	)
@if exist ..\ml_plg\x64_Debug (
	@echo Delete ..\ml_plg\x64_Debug
	@call rd /S /Q ..\ml_plg\x64_Debug
	)
@if exist ..\ml_plg\x64_Release (
	@echo Delete ..\ml_plg\x64_Release
	@call rd /S /Q ..\ml_plg\x64_Release
	)
@if exist ..\ml_plg\.vs (
	@echo Delete ..\ml_plg\.vs
	@call rd /S /Q ..\ml_plg\.vs
	)
@if exist ..\ml_plg\*.user (
	@echo Delete ..\ml_plg\*.user
	@call del ..\ml_plg\*.user
	)
@if exist ..\ml_plg\*.filters (
	@echo Delete ..\ml_plg\*.filters
	@call del ..\ml_plg\*.filters
	)
@if exist ..\ml_plg\*.htm (
	@echo Delete ..\ml_plg\*.htm
	@call del ..\ml_plg\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_pmp project
@echo *******************************
@if exist ..\ml_pmp\x86_Debug (
	@echo Delete ..\ml_pmp\x86_Debug
	@call rd /S /Q ..\ml_pmp\x86_Debug
	)
@if exist ..\ml_pmp\x86_Release (
	@echo Delete ..\ml_pmp\x86_Release
	@call rd /S /Q ..\ml_pmp\x86_Release
	)
@if exist ..\ml_pmp\x64_Debug (
	@echo Delete ..\ml_pmp\x64_Debug
	@call rd /S /Q ..\ml_pmp\x64_Debug
	)
@if exist ..\ml_pmp\x64_Release (
	@echo Delete ..\ml_pmp\x64_Release
	@call rd /S /Q ..\ml_pmp\x64_Release
	)
@if exist ..\ml_pmp\.vs (
	@echo Delete ..\ml_pmp\.vs
	@call rd /S /Q ..\ml_pmp\.vs
	)
@if exist ..\ml_pmp\*.user (
	@echo Delete ..\ml_pmp\*.user
	@call del ..\ml_pmp\*.user
	)
@if exist ..\ml_pmp\*.filters (
	@echo Delete ..\ml_pmp\*.filters
	@call del ..\ml_pmp\*.filters
	)
@if exist ..\ml_pmp\*.htm (
	@echo Delete ..\ml_pmp\*.htm
	@call del ..\ml_pmp\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_rg project
@echo *******************************
@if exist ..\ml_rg\x86_Debug (
	@echo Delete ..\ml_rg\x86_Debug
	@call rd /S /Q ..\ml_rg\x86_Debug
	)
@if exist ..\ml_rg\x86_Release (
	@echo Delete ..\ml_rg\x86_Release
	@call rd /S /Q ..\ml_rg\x86_Release
	)
@if exist ..\ml_rg\x64_Debug (
	@echo Delete ..\ml_rg\x64_Debug
	@call rd /S /Q ..\ml_rg\x64_Debug
	)
@if exist ..\ml_rg\x64_Release (
	@echo Delete ..\ml_rg\x64_Release
	@call rd /S /Q ..\ml_rg\x64_Release
	)
@if exist ..\ml_rg\.vs (
	@echo Delete ..\ml_rg\.vs
	@call rd /S /Q ..\ml_rg\.vs
	)
@if exist ..\ml_rg\*.user (
	@echo Delete ..\ml_rg\*.user
	@call del ..\ml_rg\*.user
	)
@if exist ..\ml_rg\*.filters (
	@echo Delete ..\ml_rg\*.filters
	@call del ..\ml_rg\*.filters
	)
@if exist ..\ml_rg\*.htm (
	@echo Delete ..\ml_rg\*.htm
	@call del ..\ml_rg\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_transcode project
@echo *******************************
@if exist ..\ml_transcode\x86_Debug (
	@echo Delete ..\ml_transcode\x86_Debug
	@call rd /S /Q ..\ml_transcode\x86_Debug
	)
@if exist ..\ml_transcode\x86_Release (
	@echo Delete ..\ml_transcode\x86_Release
	@call rd /S /Q ..\ml_transcode\x86_Release
	)
@if exist ..\ml_transcode\x64_Debug (
	@echo Delete ..\ml_transcode\x64_Debug
	@call rd /S /Q ..\ml_transcode\x64_Debug
	)
@if exist ..\ml_transcode\x64_Release (
	@echo Delete ..\ml_transcode\x64_Release
	@call rd /S /Q ..\ml_transcode\x64_Release
	)
@if exist ..\ml_transcode\.vs (
	@echo Delete ..\ml_transcode\.vs
	@call rd /S /Q ..\ml_transcode\.vs
	)
@if exist ..\ml_transcode\*.user (
	@echo Delete ..\ml_transcode\*.user
	@call del ..\ml_transcode\*.user
	)
@if exist ..\ml_transcode\*.filters (
	@echo Delete ..\ml_transcode\*.filters
	@call del ..\ml_transcode\*.filters
	)
@if exist ..\ml_transcode\*.htm (
	@echo Delete ..\ml_transcode\*.htm
	@call del ..\ml_transcode\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ml_webdev project
@echo *******************************
@if exist ..\ml_webdev\x86_Debug (
	@echo Delete ..\ml_webdev\x86_Debug
	@call rd /S /Q ..\ml_webdev\x86_Debug
	)
@if exist ..\ml_webdev\x86_Release (
	@echo Delete ..\ml_webdev\x86_Release
	@call rd /S /Q ..\ml_webdev\x86_Release
	)
@if exist ..\ml_webdev\x64_Debug (
	@echo Delete ..\ml_webdev\x64_Debug
	@call rd /S /Q ..\ml_webdev\x64_Debug
	)
@if exist ..\ml_webdev\x64_Release (
	@echo Delete ..\ml_webdev\x64_Release
	@call rd /S /Q ..\ml_webdev\x64_Release
	)
@if exist ..\ml_webdev\.vs (
	@echo Delete ..\ml_webdev\.vs
	@call rd /S /Q ..\ml_webdev\.vs
	)
@if exist ..\ml_webdev\*.user (
	@echo Delete ..\ml_webdev\*.user
	@call del ..\ml_webdev\*.user
	)
@if exist ..\ml_webdev\*.filters (
	@echo Delete ..\ml_webdev\*.filters
	@call del ..\ml_webdev\*.filters
	)
@if exist ..\ml_webdev\*.htm (
	@echo Delete ..\ml_webdev\*.htm
	@call del ..\ml_webdev\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * ml_wire project
@echo *******************************
@if exist ..\ml_wire\x86_Debug (
	@echo Delete ..\ml_wire\x86_Debug
	@call rd /S /Q ..\ml_wire\x86_Debug
	)
@if exist ..\ml_wire\x86_Release (
	@echo Delete ..\ml_wire\x86_Release
	@call rd /S /Q ..\ml_wire\x86_Release
	)
@if exist ..\ml_wire\x64_Debug (
	@echo Delete ..\ml_wire\x64_Debug
	@call rd /S /Q ..\ml_wire\x64_Debug
	)
@if exist ..\ml_wire\x64_Release (
	@echo Delete ..\ml_wire\x64_Release
	@call rd /S /Q ..\ml_wire\x64_Release
	)
@if exist ..\ml_wire\.vs (
	@echo Delete ..\ml_wire\.vs
	@call rd /S /Q ..\ml_wire\.vs
	)
@if exist ..\ml_wire\*.user (
	@echo Delete ..\ml_wire\*.user
	@call del ..\ml_wire\*.user
	)
@if exist ..\ml_wire\*.filters (
	@echo Delete ..\ml_wire\*.filters
	@call del ..\ml_wire\*.filters
	)
@if exist ..\ml_wire\*.htm (
	@echo Delete ..\ml_wire\*.htm
	@call del ..\ml_wire\*.htm
	)		
@echo -------------------------------
@echo *******************************
@echo * mp3-mpg123 project
@echo *******************************
@if exist ..\mp3-mpg123\x86_Debug (
	@echo Delete ..\mp3-mpg123\x86_Debug
	@call rd /S /Q ..\mp3-mpg123\x86_Debug
	)
@if exist ..\mp3-mpg123\x86_Release (
	@echo Delete ..\mp3-mpg123\x86_Release
	@call rd /S /Q ..\mp3-mpg123\x86_Release
	)
@if exist ..\mp3-mpg123\x64_Debug (
	@echo Delete ..\mp3-mpg123\x64_Debug
	@call rd /S /Q ..\mp3-mpg123\x64_Debug
	)
@if exist ..\mp3-mpg123\x64_Release (
	@echo Delete ..\mp3-mpg123\x64_Release
	@call rd /S /Q ..\mp3-mpg123\x64_Release
	)
@if exist ..\mp3-mpg123\.vs (
	@echo Delete ..\mp3-mpg123\.vs
	@call rd /S /Q ..\mp3-mpg123\.vs
	)
@if exist ..\mp3-mpg123\*.user (
	@echo Delete ..\mp3-mpg123\*.user
	@call del ..\mp3-mpg123\*.user
	)
@if exist ..\mp3-mpg123\*.filters (
	@echo Delete ..\mp3-mpg123\*.filters
	@call del ..\mp3-mpg123\*.filters
	)
@if exist ..\mp3-mpg123\*.htm (
	@echo Delete ..\mp3-mpg123\*.htm
	@call del ..\mp3-mpg123\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * mp4v project
@echo *******************************
@if exist ..\mp4v\x86_Debug (
	@echo Delete ..\mp4v\x86_Debug
	@call rd /S /Q ..\mp4v\x86_Debug
	)
@if exist ..\mp4v\x86_Release (
	@echo Delete ..\mp4v\x86_Release
	@call rd /S /Q ..\mp4v\x86_Release
	)
@if exist ..\mp4v\x64_Debug (
	@echo Delete ..\mp4v\x64_Debug
	@call rd /S /Q ..\mp4v\x64_Debug
	)
@if exist ..\mp4v\x64_Release (
	@echo Delete ..\mp4v\x64_Release
	@call rd /S /Q ..\mp4v\x64_Release
	)
@if exist ..\mp4v\.vs (
	@echo Delete ..\mp4v\.vs
	@call rd /S /Q ..\mp4v\.vs
	)
@if exist ..\mp4v\*.user (
	@echo Delete ..\mp4v\*.user
	@call del ..\mp4v\*.user
	)
@if exist ..\mp4v\*.filters (
	@echo Delete ..\mp4v\*.filters
	@call del ..\mp4v\*.filters
	)
@if exist ..\mp4v\*.htm (
	@echo Delete ..\mp4v\*.htm
	@call del ..\mp4v\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * nde project
@echo *******************************
@if exist ..\nde\x86_Debug (
	@echo Delete ..\nde\x86_Debug
	@call rd /S /Q ..\nde\x86_Debug
	)
@if exist ..\nde\x86_Release (
	@echo Delete ..\nde\x86_Release
	@call rd /S /Q ..\nde\x86_Release
	)
@if exist ..\nde\x64_Debug (
	@echo Delete ..\nde\x64_Debug
	@call rd /S /Q ..\nde\x64_Debug
	)
@if exist ..\nde\x64_Release (
	@echo Delete ..\nde\x64_Release
	@call rd /S /Q ..\nde\x64_Release
	)
@if exist ..\nde\.vs (
	@echo Delete ..\nde\.vs
	@call rd /S /Q ..\nde\.vs
	)
@if exist ..\nde\*.user (
	@echo Delete ..\nde\*.user
	@call del ..\nde\*.user
	)
@if exist ..\nde\*.filters (
	@echo Delete ..\nde\*.filters
	@call del ..\nde\*.filters
	)
@if exist ..\nde\*.htm (
	@echo Delete ..\nde\*.htm
	@call del ..\nde\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * nsapev2 project
@echo *******************************
@if exist ..\replicant\nsapev2\x86_Debug (
	@echo Delete ..\replicant\nsapev2\x86_Debug
	@call rd /S /Q ..\replicant\nsapev2\x86_Debug
	)
@if exist ..\replicant\nsapev2\x86_Release (
	@echo Delete ..\replicant\nsapev2\x86_Release
	@call rd /S /Q ..\replicant\nsapev2\x86_Release
	)
@if exist ..\replicant\nsapev2\x64_Debug (
	@echo Delete ..\replicant\nsapev2\x64_Debug
	@call rd /S /Q ..\replicant\nsapev2\x64_Debug
	)
@if exist ..\replicant\nsapev2\x64_Release (
	@echo Delete ..\replicant\nsapev2\x64_Release
	@call rd /S /Q ..\replicant\nsapev2\x64_Release
	)
@if exist ..\replicant\nsapev2\.vs (
	@echo Delete ..\replicant\nsapev2\.vs
	@call rd /S /Q ..\replicant\nsapev2\.vs
	)
@if exist ..\replicant\nsapev2\*.user (
	@echo Delete ..\replicant\nsapev2\*.user
	@call del ..\replicant\nsapev2\*.user
	)
@if exist ..\replicant\nsapev2\*.filters (
	@echo Delete ..\replicant\nsapev2\*.filters
	@call del ..\replicant\nsapev2\*.filters
	)
@if exist ..\replicant\nsapev2\*.htm (
	@echo Delete ..\replicant\nsapev2\*.htm
	@call del ..\replicant\nsapev2\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * nsid3v1 project
@echo *******************************
@if exist ..\replicant\nsid3v1\x86_Debug (
	@echo Delete ..\replicant\nsid3v1\x86_Debug
	@call rd /S /Q ..\replicant\nsid3v1\x86_Debug
	)
@if exist ..\replicant\nsid3v1\x86_Release (
	@echo Delete ..\replicant\nsid3v1\x86_Release
	@call rd /S /Q ..\replicant\nsid3v1\x86_Release
	)
@if exist ..\replicant\nsid3v1\x64_Debug (
	@echo Delete ..\replicant\nsid3v1\x64_Debug
	@call rd /S /Q ..\replicant\nsid3v1\x64_Debug
	)
@if exist ..\replicant\nsid3v1\x64_Release (
	@echo Delete ..\replicant\nsid3v1\x64_Release
	@call rd /S /Q ..\replicant\nsid3v1\x64_Release
	)
@if exist ..\replicant\nsid3v1\.vs (
	@echo Delete ..\replicant\nsid3v1\.vs
	@call rd /S /Q ..\replicant\nsid3v1\.vs
	)
@if exist ..\replicant\nsid3v1\*.user (
	@echo Delete ..\replicant\nsid3v1\*.user
	@call del ..\replicant\nsid3v1\*.user
	)
@if exist ..\replicant\nsid3v1\*.filters (
	@echo Delete ..\replicant\nsid3v1\*.filters
	@call del ..\replicant\nsid3v1\*.filters
	)
@if exist ..\replicant\nsid3v1\*.htm (
	@echo Delete ..\replicant\nsid3v1\*.htm
	@call del ..\replicant\nsid3v1\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * nsid3v2 project
@echo *******************************
@if exist ..\replicant\nsid3v2\x86_Debug (
	@echo Delete ..\replicant\nsid3v2\x86_Debug
	@call rd /S /Q ..\replicant\nsid3v2\x86_Debug
	)
@if exist ..\replicant\nsid3v2\x86_Release (
	@echo Delete ..\replicant\nsid3v2\x86_Release
	@call rd /S /Q ..\replicant\nsid3v2\x86_Release
	)
@if exist ..\replicant\nsid3v2\x64_Debug (
	@echo Delete ..\replicant\nsid3v2\x64_Debug
	@call rd /S /Q ..\replicant\nsid3v2\x64_Debug
	)
@if exist ..\replicant\nsid3v2\x64_Release (
	@echo Delete ..\replicant\nsid3v2\x64_Release
	@call rd /S /Q ..\replicant\nsid3v2\x64_Release
	)
@if exist ..\replicant\nsid3v2\.vs (
	@echo Delete ..\replicant\nsid3v2\.vs
	@call rd /S /Q ..\replicant\nsid3v2\.vs
	)
@if exist ..\replicant\nsid3v2\*.user (
	@echo Delete ..\replicant\nsid3v2\*.user
	@call del ..\replicant\nsid3v2\*.user
	)
@if exist ..\replicant\nsid3v2\*.filters (
	@echo Delete ..\replicant\nsid3v2\*.filters
	@call del ..\replicant\nsid3v2\*.filters
	)
@if exist ..\replicant\nsid3v2\*.htm (
	@echo Delete ..\replicant\nsid3v2\*.htm
	@call del ..\replicant\nsid3v2\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * nsmp3 project
@echo *******************************
@if exist ..\replicant\nsmp3\x86_Debug (
	@echo Delete ..\replicant\nsmp3\x86_Debug
	@call rd /S /Q ..\replicant\nsmp3\x86_Debug
	)
@if exist ..\replicant\nsmp3\x86_Release (
	@echo Delete ..\replicant\nsmp3\x86_Release
	@call rd /S /Q ..\replicant\nsmp3\x86_Release
	)
@if exist ..\replicant\nsmp3\x64_Debug (
	@echo Delete ..\replicant\nsmp3\x64_Debug
	@call rd /S /Q ..\replicant\nsmp3\x64_Debug
	)
@if exist ..\replicant\nsmp3\x64_Release (
	@echo Delete ..\replicant\nsmp3\x64_Release
	@call rd /S /Q ..\replicant\nsmp3\x64_Release
	)
@if exist ..\replicant\nsmp3\.vs (
	@echo Delete ..\replicant\nsmp3\.vs
	@call rd /S /Q ..\replicant\nsmp3\.vs
	)
@if exist ..\replicant\nsmp3\*.user (
	@echo Delete ..\replicant\nsmp3\*.user
	@call del ..\replicant\nsmp3\*.user
	)
@if exist ..\replicant\nsmp3\*.filters (
	@echo Delete ..\replicant\nsmp3\*.filters
	@call del ..\replicant\nsmp3\*.filters
	)
@if exist ..\replicant\nsmp3\*.htm (
	@echo Delete ..\replicant\nsmp3\*.htm
	@call del ..\replicant\nsmp3\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * nswasabi project
@echo *******************************
@if exist ..\replicant\nswasabi\x86_Debug (
	@echo Delete ..\replicant\nswasabi\x86_Debug
	@call rd /S /Q ..\replicant\nswasabi\x86_Debug
	)
@if exist ..\replicant\nswasabi\x86_Release (
	@echo Delete ..\replicant\nswasabi\x86_Release
	@call rd /S /Q ..\replicant\nswasabi\x86_Release
	)
@if exist ..\replicant\nswasabi\x64_Debug (
	@echo Delete ..\replicant\nswasabi\x64_Debug
	@call rd /S /Q ..\replicant\nswasabi\x64_Debug
	)
@if exist ..\replicant\nswasabi\x64_Release (
	@echo Delete ..\replicant\nswasabi\x64_Release
	@call rd /S /Q ..\replicant\nswasabi\x64_Release
	)
@if exist ..\replicant\nswasabi\.vs (
	@echo Delete ..\replicant\nswasabi\.vs
	@call rd /S /Q ..\replicant\nswasabi\.vs
	)
@if exist ..\replicant\nswasabi\*.user (
	@echo Delete ..\replicant\nswasabi\*.user
	@call del ..\replicant\nswasabi\*.user
	)
@if exist ..\replicant\nswasabi\*.filters (
	@echo Delete ..\replicant\nswasabi\*.filters
	@call del ..\replicant\nswasabi\*.filters
	)
@if exist ..\replicant\nswasabi\*.htm (
	@echo Delete ..\replicant\nswasabi\*.htm
	@call del ..\replicant\nswasabi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * nx project
@echo *******************************
@if exist ..\replicant\nx\x86_Debug (
	@echo Delete ..\replicant\nx\x86_Debug
	@call rd /S /Q ..\replicant\nx\x86_Debug
	)
@if exist ..\replicant\nx\x86_Release (
	@echo Delete ..\replicant\nx\x86_Release
	@call rd /S /Q ..\replicant\nx\x86_Release
	)
@if exist ..\replicant\nx\x64_Debug (
	@echo Delete ..\replicant\nx\x64_Debug
	@call rd /S /Q ..\replicant\nx\x64_Debug
	)
@if exist ..\replicant\nx\x64_Release (
	@echo Delete ..\replicant\nx\x64_Release
	@call rd /S /Q ..\replicant\nx\x64_Release
	)
@if exist ..\replicant\nx\.vs (
	@echo Delete ..\replicant\nx\.vs
	@call rd /S /Q ..\replicant\nx\.vs
	)
@if exist ..\replicant\nx\*.user (
	@echo Delete ..\replicant\nx\*.user
	@call del ..\replicant\nx\*.user
	)
@if exist ..\replicant\nx\*.filters (
	@echo Delete ..\replicant\nx\*.filters
	@call del ..\replicant\nx\*.filters
	)
@if exist ..\replicant\nx\*.htm (
	@echo Delete ..\replicant\nx\*.htm
	@call del ..\replicant\nx\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * nu project
@echo *******************************
@if exist ..\replicant\nu\x86_Debug (
	@echo Delete ..\replicant\nu\x86_Debug
	@call rd /S /Q ..\replicant\nu\x86_Debug
	)
@if exist ..\replicant\nu\x86_Release (
	@echo Delete ..\replicant\nu\x86_Release
	@call rd /S /Q ..\replicant\nu\x86_Release
	)
@if exist ..\replicant\nu\x64_Debug (
	@echo Delete ..\replicant\nu\x64_Debug
	@call rd /S /Q ..\replicant\nu\x64_Debug
	)
@if exist ..\replicant\nu\x64_Release (
	@echo Delete ..\replicant\nu\x64_Release
	@call rd /S /Q ..\replicant\nu\x64_Release
	)
@if exist ..\replicant\nu\.vs (
	@echo Delete ..\replicant\nu\.vs
	@call rd /S /Q ..\replicant\nu\.vs
	)
@if exist ..\replicant\nu\*.user (
	@echo Delete ..\replicant\nu\*.user
	@call del ..\replicant\nu\*.user
	)
@if exist ..\replicant\nu\*.filters (
	@echo Delete ..\replicant\nu\*.filters
	@call del ..\replicant\nu\*.filters
	)
@if exist ..\replicant\nu\*.htm (
	@echo Delete ..\replicant\nu\*.htm
	@call del ..\replicant\nu\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * nsutil project
@echo *******************************
@if exist ..\nsutil\x86_Debug (
	@echo Delete ..\nsutil\x86_Debug
	@call rd /S /Q ..\nsutil\x86_Debug
	)
@if exist ..\nsutil\x86_Release (
	@echo Delete ..\nsutil\x86_Release
	@call rd /S /Q ..\nsutil\x86_Release
	)
@if exist ..\nsutil\x64_Debug (
	@echo Delete ..\nsutil\x64_Debug
	@call rd /S /Q ..\nsutil\x64_Debug
	)
@if exist ..\nsutil\x64_Release (
	@echo Delete ..\nsutil\x64_Release
	@call rd /S /Q ..\nsutil\x64_Release
	)
@if exist ..\nsutil\.vs (
	@echo Delete ..\nsutil\.vs
	@call rd /S /Q ..\nsutil\.vs
	)
@if exist ..\nsutil\*.user (
	@echo Delete ..\nsutil\*.user
	@call del ..\nsutil\*.user
	)
@if exist ..\nsutil\*.filters (
	@echo Delete ..\nsutil\*.filters
	@call del ..\nsutil\*.filters
	)
@if exist ..\nsutil\*.htm (
	@echo Delete ..\nsutil\*.htm
	@call del ..\nsutil\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * omBrowser project
@echo *******************************
@if exist ..\omBrowser\x86_Debug (
	@echo Delete ..\omBrowser\x86_Debug
	@call rd /S /Q ..\omBrowser\x86_Debug
	)
@if exist ..\omBrowser\x86_Release (
	@echo Delete ..\omBrowser\x86_Release
	@call rd /S /Q ..\omBrowser\x86_Release
	)
@if exist ..\omBrowser\x64_Debug (
	@echo Delete ..\omBrowser\x64_Debug
	@call rd /S /Q ..\omBrowser\x64_Debug
	)
@if exist ..\omBrowser\x64_Release (
	@echo Delete ..\omBrowser\x64_Release
	@call rd /S /Q ..\omBrowser\x64_Release
	)
@if exist ..\omBrowser\localization\x86_Debug (
	@echo Delete ..\omBrowser\localization\x86_Debug
	@call rd /S /Q ..\omBrowser\localization\x86_Debug
	)
@if exist ..\omBrowser\localization\x86_Release (
	@echo Delete ..\omBrowser\localization\x86_Release
	@call rd /S /Q ..\omBrowser\localization\x86_Release
	)
	
	@if exist ..\omBrowser\localization\x64_Debug (
	@echo Delete ..\omBrowser\localization\x64_Debug
	@call rd /S /Q ..\omBrowser\localization\x64_Debug
	)
@if exist ..\omBrowser\localization\x64_Release (
	@echo Delete ..\omBrowser\localization\x64_Release
	@call rd /S /Q ..\omBrowser\localization\x64_Release
	)
@if exist ..\omBrowser\.vs (
	@echo Delete ..\omBrowser\.vs
	@call rd /S /Q ..\omBrowser\.vs
	)
@if exist ..\omBrowser\*.user (
	@echo Delete ..\omBrowser\*.user
	@call del ..\omBrowser\*.user
	)
@if exist ..\omBrowser\*.filters (
	@echo Delete ..\omBrowser\*.filters
	@call del ..\omBrowser\*.filters
	)
@if exist ..\omBrowser\*.htm (
	@echo Delete ..\omBrowser\*.htm
	@call del ..\omBrowser\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * out_disk project
@echo *******************************
@if exist ..\out_disk\x86_Debug (
	@echo Delete ..\out_disk\x86_Debug
	@call rd /S /Q ..\out_disk\x86_Debug
	)
@if exist ..\out_disk\x86_Release (
	@echo Delete ..\out_disk\x86_Release
	@call rd /S /Q ..\out_disk\x86_Release
	)
@if exist ..\out_disk\x64_Debug (
	@echo Delete ..\out_disk\x64_Debug
	@call rd /S /Q ..\out_disk\x64_Debug
	)
@if exist ..\out_disk\x64_Release (
	@echo Delete ..\out_disk\x64_Release
	@call rd /S /Q ..\out_disk\x64_Release
	)
@if exist ..\out_disk\.vs (
	@echo Delete ..\out_disk\.vs
	@call rd /S /Q ..\out_disk\.vs
	)
@if exist ..\out_disk\*.user (
	@echo Delete ..\out_disk\*.user
	@call del ..\out_disk\*.user
	)
@if exist ..\out_disk\*.filters (
	@echo Delete ..\out_disk\*.filters
	@call del ..\out_disk\*.filters
	)
@if exist ..\out_disk\*.htm (
	@echo Delete ..\out_disk\*.htm
	@call del ..\out_disk\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * out_ds project
@echo *******************************
@if exist ..\out_ds\x86_Debug (
	@echo Delete ..\out_ds\x86_Debug
	@call rd /S /Q ..\out_ds\x86_Debug
	)
@if exist ..\out_ds\x86_Release (
	@echo Delete ..\out_ds\x86_Release
	@call rd /S /Q ..\out_ds\x86_Release
	)
@if exist ..\out_ds\x64_Debug (
	@echo Delete ..\out_ds\x64_Debug
	@call rd /S /Q ..\out_ds\x64_Debug
	)
@if exist ..\out_ds\x64_Release (
	@echo Delete ..\out_ds\x64_Release
	@call rd /S /Q ..\out_ds\x64_Release
	)
@if exist ..\out_ds\.vs (
	@echo Delete ..\out_ds\.vs
	@call rd /S /Q ..\out_ds\.vs
	)
@if exist ..\out_ds\*.user (
	@echo Delete ..\out_ds\*.user
	@call del ..\out_ds\*.user
	)
@if exist ..\out_ds\*.filters (
	@echo Delete ..\out_ds\*.filters
	@call del ..\out_ds\*.filters
	)
@if exist ..\out_ds\*.htm (
	@echo Delete ..\out_ds\*.htm
	@call del ..\out_ds\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * out_wasapi project
@echo *******************************
@if exist ..\out_wasapi\x86_Debug (
	@echo Delete ..\out_wasapi\x86_Debug
	@call rd /S /Q ..\out_wasapi\x86_Debug
	)
@if exist ..\out_wasapi\x86_Release (
	@echo Delete ..\out_wasapi\x86_Release
	@call rd /S /Q ..\out_wasapi\x86_Release
	)
@if exist ..\out_wasapi\x64_Debug (
	@echo Delete ..\out_wasapi\x64_Debug
	@call rd /S /Q ..\out_wasapi\x64_Debug
	)
@if exist ..\out_wasapi\x64_Release (
	@echo Delete ..\out_wasapi\x64_Release
	@call rd /S /Q ..\out_wasapi\x64_Release
	)
@if exist ..\out_wasapi\.vs (
	@echo Delete ..\out_wasapi\.vs
	@call rd /S /Q ..\out_wasapi\.vs
	)
@if exist ..\out_wasapi\*.user (
	@echo Delete ..\out_wasapi\*.user
	@call del ..\out_wasapi\*.user
	)
@if exist ..\out_wasapi\*.filters (
	@echo Delete ..\out_wasapi\*.filters
	@call del ..\out_wasapi\*.filters
	)
@if exist ..\out_wasapi\*.htm (
	@echo Delete ..\out_wasapi\*.htm
	@call del ..\out_wasapi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * out_wave project
@echo *******************************
@if exist ..\out_wave\x86_Debug (
	@echo Delete ..\out_wave\x86_Debug
	@call rd /S /Q ..\out_wave\x86_Debug
	)
@if exist ..\out_wave\x86_Release (
	@echo Delete ..\out_wave\x86_Release
	@call rd /S /Q ..\out_wave\x86_Release
	)
@if exist ..\out_wave\x64_Debug (
	@echo Delete ..\out_wave\x64_Debug
	@call rd /S /Q ..\out_wave\x64_Debug
	)
@if exist ..\out_wave\x64_Release (
	@echo Delete ..\out_wave\x64_Release
	@call rd /S /Q ..\out_wave\x64_Release
	)
@if exist ..\out_wave\.vs (
	@echo Delete ..\out_wave\.vs
	@call rd /S /Q ..\out_wave\.vs
	)
@if exist ..\out_wave\*.user (
	@echo Delete ..\out_wave\*.user
	@call del ..\out_wave\*.user
	)
@if exist ..\out_wave\*.filters (
	@echo Delete ..\out_wave\*.filters
	@call del ..\out_wave\*.filters
	)
@if exist ..\out_wave\*.htm (
	@echo Delete ..\out_wave\*.htm
	@call del ..\out_wave\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * pcm project
@echo *******************************
@if exist ..\pcm\x86_Debug (
	@echo Delete ..\pcm\x86_Debug
	@call rd /S /Q ..\pcm\x86_Debug
	)
@if exist ..\pcm\x86_Release (
	@echo Delete ..\pcm\x86_Release
	@call rd /S /Q ..\pcm\x86_Release
	)
@if exist ..\pcm\x64_Debug (
	@echo Delete ..\pcm\x64_Debug
	@call rd /S /Q ..\pcm\x64_Debug
	)
@if exist ..\pcm\x64_Release (
	@echo Delete ..\pcm\x64_Release
	@call rd /S /Q ..\pcm\x64_Release
	)
@if exist ..\pcm\.vs (
	@echo Delete ..\pcm\.vs
	@call rd /S /Q ..\pcm\.vs
	)
@if exist ..\pcm\*.user (
	@echo Delete ..\pcm\*.user
	@call del ..\pcm\*.user
	)
@if exist ..\pcm\*.filters (
	@echo Delete ..\pcm\*.filters
	@call del ..\pcm\*.filters
	)
@if exist ..\pcm\*.htm (
	@echo Delete ..\pcm\*.htm
	@call del ..\pcm\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * playlist project
@echo *******************************
@if exist ..\playlist\x86_Debug (
	@echo Delete ..\playlist\x86_Debug
	@call rd /S /Q ..\playlist\x86_Debug
	)
@if exist ..\playlist\x86_Release (
	@echo Delete ..\playlist\x86_Release
	@call rd /S /Q ..\playlist\x86_Release
	)
@if exist ..\playlist\x64_Debug (
	@echo Delete ..\playlist\x64_Debug
	@call rd /S /Q ..\playlist\x64_Debug
	)
@if exist ..\playlist\x64_Release (
	@echo Delete ..\playlist\x64_Release
	@call rd /S /Q ..\playlist\x64_Release
	)
@if exist ..\playlist\.vs (
	@echo Delete ..\playlist\.vs
	@call rd /S /Q ..\playlist\.vs
	)
@if exist ..\playlist\*.user (
	@echo Delete ..\playlist\*.user
	@call del ..\playlist\*.user
	)
@if exist ..\playlist\*.filters (
	@echo Delete ..\playlist\*.filters
	@call del ..\playlist\*.filters
	)
@if exist ..\playlist\*.htm (
	@echo Delete ..\playlist\*.htm
	@call del ..\playlist\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * plist project
@echo *******************************
@if exist ..\plist\x86_Debug (
	@echo Delete ..\plist\x86_Debug
	@call rd /S /Q ..\plist\x86_Debug
	)
@if exist ..\plist\x86_Release (
	@echo Delete ..\plist\x86_Release
	@call rd /S /Q ..\plist\x86_Release
	)
@if exist ..\plist\x64_Debug (
	@echo Delete ..\plist\x64_Debug
	@call rd /S /Q ..\plist\x64_Debug
	)
@if exist ..\plist\x64_Release (
	@echo Delete ..\plist\x64_Release
	@call rd /S /Q ..\plist\x64_Release
	)
@if exist ..\plist\.vs (
	@echo Delete ..\plist\.vs
	@call rd /S /Q ..\plist\.vs
	)
@if exist ..\plist\*.user (
	@echo Delete ..\plist\*.user
	@call del ..\plist\*.user
	)
@if exist ..\plist\*.filters (
	@echo Delete ..\plist\*.filters
	@call del ..\plist\*.filters
	)
@if exist ..\plist\*.htm (
	@echo Delete ..\plist\*.htm
	@call del ..\plist\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * pmp_android project
@echo *******************************
@if exist ..\pmp_android\x86_Debug (
	@echo Delete ..\pmp_android\x86_Debug
	@call rd /S /Q ..\pmp_android\x86_Debug
	)
@if exist ..\pmp_android\x86_Release (
	@echo Delete ..\pmp_android\x86_Release
	@call rd /S /Q ..\pmp_android\x86_Release
	)
@if exist ..\pmp_android\x64_Debug (
	@echo Delete ..\pmp_android\x64_Debug
	@call rd /S /Q ..\pmp_android\x64_Debug
	)
@if exist ..\pmp_android\x64_Release (
	@echo Delete ..\pmp_android\x64_Release
	@call rd /S /Q ..\pmp_android\x64_Release
	)
@if exist ..\pmp_android\.vs (
	@echo Delete ..\pmp_android\.vs
	@call rd /S /Q ..\pmp_android\.vs
	)
@if exist ..\pmp_android\*.user (
	@echo Delete ..\pmp_android\*.user
	@call del ..\pmp_android\*.user
	)
@if exist ..\pmp_android\*.filters (
	@echo Delete ..\pmp_android\*.filters
	@call del ..\pmp_android\*.filters
	)
@if exist ..\pmp_android\*.htm (
	@echo Delete ..\pmp_android\*.htm
	@call del ..\pmp_android\*.htm
	)		
@echo -------------------------------
@echo *******************************
@echo * pmp_ipod project
@echo *******************************
@if exist ..\pmp_ipod\x86_Debug (
	@echo Delete ..\pmp_ipod\x86_Debug
	@call rd /S /Q ..\pmp_ipod\x86_Debug
	)
@if exist ..\pmp_ipod\x86_Release (
	@echo Delete ..\pmp_ipod\x86_Release
	@call rd /S /Q ..\pmp_ipod\x86_Release
	)
@if exist ..\pmp_ipod\x64_Debug (
	@echo Delete ..\pmp_ipod\x64_Debug
	@call rd /S /Q ..\pmp_ipod\x64_Debug
	)
@if exist ..\pmp_ipod\x64_Release (
	@echo Delete ..\pmp_ipod\x64_Release
	@call rd /S /Q ..\pmp_ipod\x64_Release
	)
@if exist ..\pmp_ipod\.vs (
	@echo Delete ..\pmp_ipod\.vs
	@call rd /S /Q ..\pmp_ipod\.vs
	)
@if exist ..\pmp_ipod\*.user (
	@echo Delete ..\pmp_ipod\*.user
	@call del ..\pmp_ipod\*.user
	)
@if exist ..\pmp_ipod\*.filters (
	@echo Delete ..\pmp_ipod\*.filters
	@call del ..\pmp_ipod\*.filters
	)
@if exist ..\pmp_ipod\*.htm (
	@echo Delete ..\pmp_ipod\*.htm
	@call del ..\pmp_ipod\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * pmp_njb project
@echo *******************************
@if exist ..\pmp_njb\x86_Debug (
	@echo Delete ..\pmp_njb\x86_Debug
	@call rd /S /Q ..\pmp_njb\x86_Debug
	)
@if exist ..\pmp_njb\x86_Release (
	@echo Delete ..\pmp_njb\x86_Release
	@call rd /S /Q ..\pmp_njb\x86_Release
	)
@if exist ..\pmp_njb\x64_Debug (
	@echo Delete ..\pmp_njb\x64_Debug
	@call rd /S /Q ..\pmp_njb\x64_Debug
	)
@if exist ..\pmp_njb\x64_Release (
	@echo Delete ..\pmp_njb\x64_Release
	@call rd /S /Q ..\pmp_njb\x64_Release
	)
@if exist ..\pmp_njb\.vs (
	@echo Delete ..\pmp_njb\.vs
	@call rd /S /Q ..\pmp_njb\.vs
	)
@if exist ..\pmp_njb\*.user (
	@echo Delete ..\pmp_njb\*.user
	@call del ..\pmp_njb\*.user
	)
@if exist ..\pmp_njb\*.filters (
	@echo Delete ..\pmp_njb\*.filters
	@call del ..\pmp_njb\*.filters
	)
@if exist ..\pmp_njb\*.htm (
	@echo Delete ..\pmp_njb\*.htm
	@call del ..\pmp_njb\*.htm
	)		
@echo -------------------------------
@echo *******************************
@echo * pmp_usb project
@echo *******************************
@if exist ..\pmp_usb\x86_Debug (
	@echo Delete ..\pmp_usb\x86_Debug
	@call rd /S /Q ..\pmp_usb\x86_Debug
	)
@if exist ..\pmp_usb\x86_Release (
	@echo Delete ..\pmp_usb\x86_Release
	@call rd /S /Q ..\pmp_usb\x86_Release
	)
@if exist ..\pmp_usb\x64_Debug (
	@echo Delete ..\pmp_usb\x64_Debug
	@call rd /S /Q ..\pmp_usb\x64_Debug
	)
@if exist ..\pmp_usb\x64_Release (
	@echo Delete ..\pmp_usb\x64_Release
	@call rd /S /Q ..\pmp_usb\x64_Release
	)
@if exist ..\pmp_usb\.vs (
	@echo Delete ..\pmp_usb\.vs
	@call rd /S /Q ..\pmp_usb\.vs
	)
@if exist ..\pmp_usb\*.user (
	@echo Delete ..\pmp_usb\*.user
	@call del ..\pmp_usb\*.user
	)
@if exist ..\pmp_usb\*.filters (
	@echo Delete ..\pmp_usb\*.filters
	@call del ..\pmp_usb\*.filters
	)
@if exist ..\pmp_usb\*.htm (
	@echo Delete ..\pmp_usb\*.htm
	@call del ..\pmp_usb\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * pmp_wifi project
@echo *******************************
@if exist ..\pmp_wifi\x86_Debug (
	@echo Delete ..\pmp_wifi\x86_Debug
	@call rd /S /Q ..\pmp_wifi\x86_Debug
	)
@if exist ..\pmp_wifi\x86_Release (
	@echo Delete ..\pmp_wifi\x86_Release
	@call rd /S /Q ..\pmp_wifi\x86_Release
	)
@if exist ..\pmp_wifi\x64_Debug (
	@echo Delete ..\pmp_wifi\x64_Debug
	@call rd /S /Q ..\pmp_wifi\x64_Debug
	)
@if exist ..\pmp_wifi\x64_Release (
	@echo Delete ..\pmp_wifi\x64_Release
	@call rd /S /Q ..\pmp_wifi\x64_Release
	)
@if exist ..\pmp_wifi\.vs (
	@echo Delete ..\pmp_wifi\.vs
	@call rd /S /Q ..\pmp_wifi\.vs
	)
@if exist ..\pmp_wifi\*.user (
	@echo Delete ..\pmp_wifi\*.user
	@call del ..\pmp_wifi\*.user
	)
@if exist ..\pmp_wifi\*.filters (
	@echo Delete ..\pmp_wifi\*.filters
	@call del ..\pmp_wifi\*.filters
	)
@if exist ..\pmp_wifi\*.htm (
	@echo Delete ..\pmp_wifi\*.htm
	@call del ..\pmp_wifi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * png project
@echo *******************************
@if exist ..\png\x86_Debug (
	@echo Delete ..\png\x86_Debug
	@call rd /S /Q ..\png\x86_Debug
	)
@if exist ..\png\x86_Release (
	@echo Delete ..\png\x86_Release
	@call rd /S /Q ..\png\x86_Release
	)
@if exist ..\png\x64_Debug (
	@echo Delete ..\png\x64_Debug
	@call rd /S /Q ..\png\x64_Debug
	)
@if exist ..\png\x64_Release (
	@echo Delete ..\png\x64_Release
	@call rd /S /Q ..\png\x64_Release
	)
@if exist ..\png\.vs (
	@echo Delete ..\png\.vs
	@call rd /S /Q ..\png\.vs
	)
@if exist ..\png\*.user (
	@echo Delete ..\png\*.user
	@call del ..\png\*.user
	)
@if exist ..\png\*.filters (
	@echo Delete ..\png\*.filters
	@call del ..\png\*.filters
	)
@if exist ..\png\*.htm (
	@echo Delete ..\png\*.htm
	@call del ..\png\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * ReplayGainAnalysis project
@echo *******************************
@if exist ..\ReplayGainAnalysis\x86_Debug (
	@echo Delete ..\ReplayGainAnalysis\x86_Debug
	@call rd /S /Q ..\ReplayGainAnalysis\x86_Debug
	)
@if exist ..\ReplayGainAnalysis\x86_Release (
	@echo Delete ..\ReplayGainAnalysis\x86_Release
	@call rd /S /Q ..\ReplayGainAnalysis\x86_Release
	)
@if exist ..\ReplayGainAnalysis\x64_Debug (
	@echo Delete ..\ReplayGainAnalysis\x64_Debug
	@call rd /S /Q ..\ReplayGainAnalysis\x64_Debug
	)
@if exist ..\ReplayGainAnalysis\x64_Release (
	@echo Delete ..\ReplayGainAnalysis\x64_Release
	@call rd /S /Q ..\ReplayGainAnalysis\x64_Release
	)
@if exist ..\ReplayGainAnalysis\.vs (
	@echo Delete ..\ReplayGainAnalysis\.vs
	@call rd /S /Q ..\ReplayGainAnalysis\.vs
	)
@if exist ..\ReplayGainAnalysis\*.user (
	@echo Delete ..\ReplayGainAnalysis\*.user
	@call del ..\ReplayGainAnalysis\*.user
	)
@if exist ..\ReplayGainAnalysis\*.filters (
	@echo Delete ..\ReplayGainAnalysis\*.filters
	@call del ..\ReplayGainAnalysis\*.filters
	)
@if exist ..\ReplayGainAnalysis\*.htm (
	@echo Delete ..\ReplayGainAnalysis\*.htm
	@call del ..\ReplayGainAnalysis\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * tagz project
@echo *******************************
@if exist ..\tagz\x86_Debug (
	@echo Delete ..\tagz\x86_Debug
	@call rd /S /Q ..\tagz\x86_Debug
	)
@if exist ..\tagz\x86_Release (
	@echo Delete ..\tagz\x86_Release
	@call rd /S /Q ..\tagz\x86_Release
	)
@if exist ..\tagz\x64_Debug (
	@echo Delete ..\tagz\x64_Debug
	@call rd /S /Q ..\tagz\x64_Debug
	)
@if exist ..\tagz\x64_Release (
	@echo Delete ..\tagz\x64_Release
	@call rd /S /Q ..\tagz\x64_Release
	)
@if exist ..\tagz\.vs (
	@echo Delete ..\tagz\.vs
	@call rd /S /Q ..\tagz\.vs
	)
@if exist ..\tagz\*.user (
	@echo Delete ..\tagz\*.user
	@call del ..\tagz\*.user
	)
@if exist ..\tagz\*.filters (
	@echo Delete ..\tagz\*.filters
	@call del ..\tagz\*.filters
	)
@if exist ..\tagz\*.htm (
	@echo Delete ..\tagz\*.htm
	@call del ..\tagz\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * tataki project
@echo *******************************
@if exist ..\tataki\x86_Debug (
	@echo Delete ..\tataki\x86_Debug
	@call rd /S /Q ..\tataki\x86_Debug
	)
@if exist ..\tataki\x86_Release (
	@echo Delete ..\tataki\x86_Release
	@call rd /S /Q ..\tataki\x86_Release
	)
@if exist ..\tataki\x64_Debug (
	@echo Delete ..\tataki\x64_Debug
	@call rd /S /Q ..\tataki\x64_Debug
	)
@if exist ..\tataki\x64_Release (
	@echo Delete ..\tataki\x64_Release
	@call rd /S /Q ..\tataki\x64_Release
	)
@if exist ..\tataki\.vs (
	@echo Delete ..\tataki\.vs
	@call rd /S /Q ..\tataki\.vs
	)
@if exist ..\tataki\*.user (
	@echo Delete ..\tataki\*.user
	@call del ..\tataki\*.user
	)
@if exist ..\tataki\*.filters (
	@echo Delete ..\tataki\*.filters
	@call del ..\tataki\*.filters
	)
@if exist ..\tataki\*.htm (
	@echo Delete ..\tataki\*.htm
	@call del ..\tataki\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * timer project
@echo *******************************
@if exist ..\timer\x86_Debug (
	@echo Delete ..\timer\x86_Debug
	@call rd /S /Q ..\timer\x86_Debug
	)
@if exist ..\timer\x86_Release (
	@echo Delete ..\timer\x86_Release
	@call rd /S /Q ..\timer\x86_Release
	)
@if exist ..\timer\x64_Debug (
	@echo Delete ..\timer\x64_Debug
	@call rd /S /Q ..\timer\x64_Debug
	)
@if exist ..\timer\x64_Release (
	@echo Delete ..\timer\x64_Release
	@call rd /S /Q ..\timer\x64_Release
	)
@if exist ..\timer\.vs (
	@echo Delete ..\timer\.vs
	@call rd /S /Q ..\timer\.vs
	)
@if exist ..\timer\*.user (
	@echo Delete ..\timer\*.user
	@call del ..\timer\*.user
	)
@if exist ..\timer\*.filters (
	@echo Delete ..\timer\*.filters
	@call del ..\timer\*.filters
	)
@if exist ..\timer\*.htm (
	@echo Delete ..\timer\*.htm
	@call del ..\timer\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * theora project
@echo *******************************
@if exist ..\theora\x86_Debug (
	@echo Delete ..\theora\x86_Debug
	@call rd /S /Q ..\theora\x86_Debug
	)
@if exist ..\theora\x86_Release (
	@echo Delete ..\theora\x86_Release
	@call rd /S /Q ..\theora\x86_Release
	)
@if exist ..\theora\x64_Debug (
	@echo Delete ..\theora\x64_Debug
	@call rd /S /Q ..\theora\x64_Debug
	)
@if exist ..\theora\x64_Release (
	@echo Delete ..\theora\x64_Release
	@call rd /S /Q ..\theora\x64_Release
	)
@if exist ..\theora\.vs (
	@echo Delete ..\theora\.vs
	@call rd /S /Q ..\theora\.vs
	)
@if exist ..\theora\*.user (
	@echo Delete ..\theora\*.user
	@call del ..\theora\*.user
	)
@if exist ..\theora\*.filters (
	@echo Delete ..\theora\*.filters
	@call del ..\theora\*.filters
	)
@if exist ..\theora\*.htm (
	@echo Delete ..\theora\*.htm
	@call del ..\theora\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * vis_avs project
@echo *******************************
@if exist ..\vis_avs\x86_Debug (
	@echo Delete ..\vis_avs\x86_Debug
	@call rd /S /Q ..\vis_avs\x86_Debug
	)
@if exist ..\vis_avs\x86_Release (
	@echo Delete ..\vis_avs\x86_Release
	@call rd /S /Q ..\vis_avs\x86_Release
	)
@if exist ..\vis_avs\x64_Debug (
	@echo Delete ..\vis_avs\x64_Debug
	@call rd /S /Q ..\vis_avs\x64_Debug
	)
@if exist ..\vis_avs\x64_Release (
	@echo Delete ..\vis_avs\x64_Release
	@call rd /S /Q ..\vis_avs\x64_Release
	)
@if exist ..\vis_avs\.vs (
	@echo Delete ..\vis_avs\.vs
	@call rd /S /Q ..\vis_avs\.vs
	)
@if exist ..\vis_avs\*.user (
	@echo Delete ..\vis_avs\*.user
	@call del ..\vis_avs\*.user
	)
@if exist ..\vis_avs\*.filters (
	@echo Delete ..\vis_avs\*.filters
	@call del ..\vis_avs\*.filters
	)
@if exist ..\vis_avs\*.htm (
	@echo Delete ..\vis_avs\*.htm
	@call del ..\vis_avs\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * vis_milk2 project
@echo *******************************
@if exist ..\vis_milk2\x86_Debug (
	@echo Delete ..\vis_milk2\x86_Debug
	@call rd /S /Q ..\vis_milk2\x86_Debug
	)
@if exist ..\vis_milk2\x86_Release (
	@echo Delete ..\vis_milk2\x86_Release
	@call rd /S /Q ..\vis_milk2\x86_Release
	)
@if exist ..\vis_milk2\x64_Debug (
	@echo Delete ..\vis_milk2\x64_Debug
	@call rd /S /Q ..\vis_milk2\x64_Debug
	)
@if exist ..\vis_milk2\x64_Release (
	@echo Delete ..\vis_milk2\x64_Release
	@call rd /S /Q ..\vis_milk2\x64_Release
	)
@if exist ..\vis_milk2\.vs (
	@echo Delete ..\vis_milk2\.vs
	@call rd /S /Q ..\vis_milk2\.vs
	)
@if exist ..\vis_milk2\*.user (
	@echo Delete ..\vis_milk2\*.user
	@call del ..\vis_milk2\*.user
	)
@if exist ..\vis_milk2\*.filters (
	@echo Delete ..\vis_milk2\*.filters
	@call del ..\vis_milk2\*.filters
	)
@if exist ..\vis_milk2\*.htm (
	@echo Delete ..\vis_milk2\*.htm
	@call del ..\vis_milk2\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * vis_nsfs project
@echo *******************************
@if exist ..\vis_nsfs\x86_Debug (
	@echo Delete ..\vis_nsfs\x86_Debug
	@call rd /S /Q ..\vis_nsfs\x86_Debug
	)
@if exist ..\vis_nsfs\x86_Release (
	@echo Delete ..\vis_nsfs\x86_Release
	@call rd /S /Q ..\vis_nsfs\x86_Release
	)
@if exist ..\vis_nsfs\x64_Debug (
	@echo Delete ..\vis_nsfs\x64_Debug
	@call rd /S /Q ..\vis_nsfs\x64_Debug
	)
@if exist ..\vis_nsfs\x64_Release (
	@echo Delete ..\vis_nsfs\x64_Release
	@call rd /S /Q ..\vis_nsfs\x64_Release
	)
@if exist ..\vis_nsfs\.vs (
	@echo Delete ..\vis_nsfs\.vs
	@call rd /S /Q ..\vis_nsfs\.vs
	)
@if exist ..\vis_nsfs\*.user (
	@echo Delete ..\vis_nsfs\*.user
	@call del ..\vis_nsfs\*.user
	)
@if exist ..\vis_nsfs\*.filters (
	@echo Delete ..\vis_nsfs\*.filters
	@call del ..\vis_nsfs\*.filters
	)
@if exist ..\vis_nsfs\*.htm (
	@echo Delete ..\vis_nsfs\*.htm
	@call del ..\vis_nsfs\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * vlb project
@echo *******************************
@if exist ..\vlb\x86_Debug (
	@echo Delete ..\vlb\x86_Debug
	@call rd /S /Q ..\vlb\x86_Debug
	)
@if exist ..\vlb\x86_Release (
	@echo Delete ..\vlb\x86_Release
	@call rd /S /Q ..\vlb\x86_Release
	)
@if exist ..\vlb\x64_Debug (
	@echo Delete ..\vlb\x64_Debug
	@call rd /S /Q ..\vlb\x64_Debug
	)
@if exist ..\vlb\x64_Release (
	@echo Delete ..\vlb\x64_Release
	@call rd /S /Q ..\vlb\x64_Release
	)
@if exist ..\vlb\.vs (
	@echo Delete ..\vlb\.vs
	@call rd /S /Q ..\vlb\.vs
	)
@if exist ..\vlb\*.user (
	@echo Delete ..\vlb\*.user
	@call del ..\vlb\*.user
	)
@if exist ..\vlb\*.filters (
	@echo Delete ..\vlb\*.filters
	@call del ..\vlb\*.filters
	)
@if exist ..\vlb\*.htm (
	@echo Delete ..\vlb\*.htm
	@call del ..\vlb\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * vp8x project
@echo *******************************
@if exist ..\vp8x\x86_Debug (
	@echo Delete ..\vp8x\x86_Debug
	@call rd /S /Q ..\vp8x\x86_Debug
	)
@if exist ..\vp8x\x86_Release (
	@echo Delete ..\vp8x\x86_Release
	@call rd /S /Q ..\vp8x\x86_Release
	)
@if exist ..\vp8x\x64_Debug (
	@echo Delete ..\vp8x\x64_Debug
	@call rd /S /Q ..\vp8x\x64_Debug
	)
@if exist ..\vp8x\x64_Release (
	@echo Delete ..\vp8x\x64_Release
	@call rd /S /Q ..\vp8x\x64_Release
	)
@if exist ..\vp8x\.vs (
	@echo Delete ..\vp8x\.vs
	@call rd /S /Q ..\vp8x\.vs
	)
@if exist ..\vp8x\*.user (
	@echo Delete ..\vp8x\*.user
	@call del ..\vp8x\*.user
	)
@if exist ..\vp8x\*.filters (
	@echo Delete ..\vp8x\*.filters
	@call del ..\vp8x\*.filters
	)
@if exist ..\vp8x\*.htm (
	@echo Delete ..\vp8x\*.htm
	@call del ..\vp8x\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * Wasabi project
@echo *******************************
@if exist ..\Wasabi\x86_Debug (
	@echo Delete ..\Wasabi\x86_Debug
	@call rd /S /Q ..\Wasabi\x86_Debug
	)
@if exist ..\Wasabi\x86_Release (
	@echo Delete ..\Wasabi\x86_Release
	@call rd /S /Q ..\Wasabi\x86_Release
	)
@if exist ..\Wasabi\x64_Debug (
	@echo Delete ..\Wasabi\x64_Debug
	@call rd /S /Q ..\Wasabi\x64_Debug
	)
@if exist ..\Wasabi\x64_Release (
	@echo Delete ..\Wasabi\x64_Release
	@call rd /S /Q ..\Wasabi\x64_Release
	)
@if exist ..\Wasabi\.vs (
	@echo Delete ..\Wasabi\.vs
	@call rd /S /Q ..\Wasabi\.vs
	)
@if exist ..\Wasabi\*.user ( 
	@echo Delete ..\Wasabi\*.user
	@call del ..\Wasabi\*.user
	)
@if exist ..\Wasabi\*.filters (
	@echo Delete ..\Wasabi\*.filters
	@call del ..\Wasabi\*.filters
	)
@if exist ..\Wasabi\*.htm (
	@echo Delete ..\Wasabi\*.htm
	@call del ..\Wasabi\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * Wasabi-replicant project
@echo *******************************
@if exist ..\replicant\Wasabi\x86_Debug (
	@echo Delete ..\replicant\Wasabi\x86_Debug
	@call rd /S /Q ..\replicant\Wasabi\x86_Debug
	)
@if exist ..\replicant\Wasabi\x86_Release (
	@echo Delete ..\replicant\Wasabi\x86_Release
	@call rd /S /Q ..\replicant\Wasabi\x86_Release
	)
@if exist ..\replicant\Wasabi\x64_Debug (
	@echo Delete ..\replicant\Wasabi\x64_Debug
	@call rd /S /Q ..\replicant\Wasabi\x64_Debug
	)
@if exist ..\replicant\Wasabi\x64_Release (
	@echo Delete ..\replicant\Wasabi\x64_Release
	@call rd /S /Q ..\replicant\Wasabi\x64_Release
	)
@if exist ..\replicant\Wasabi\.vs (
	@echo Delete ..\replicant\Wasabi\.vs
	@call rd /S /Q ..\replicant\Wasabi\.vs
	)
@if exist ..\replicant\Wasabi\*.user (
	@echo Delete ..\replicant\Wasabi\*.user
	@call del ..\replicant\Wasabi\*.user
	)
@if exist ..\replicant\Wasabi\*.filters (
	@echo Delete ..\replicant\Wasabi\*.filters
	@call del ..\replicant\Wasabi\*.filters
	)
@if exist ..\replicant\Wasabi\*.htm (
	@echo Delete ..\replicant\Wasabi\*.htm
	@call del ..\replicant\Wasabi\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * Wasabi2 project
@echo *******************************
@if exist ..\Wasabi2\x86_Debug (
	@echo Delete ..\Wasabi2\x86_Debug
	@call rd /S /Q ..\Wasabi2\x86_Debug
	)
@if exist ..\Wasabi2\x86_Release (
	@echo Delete ..\Wasabi2\x86_Release
	@call rd /S /Q ..\Wasabi2\x86_Release
	)
@if exist ..\Wasabi2\x64_Debug (
	@echo Delete ..\Wasabi2\x64_Debug
	@call rd /S /Q ..\Wasabi2\x64_Debug
	)
@if exist ..\Wasabi2\x64_Release (
	@echo Delete ..\Wasabi2\x64_Release
	@call rd /S /Q ..\Wasabi2\x64_Release
	)
@if exist ..\Wasabi2\.vs (
	@echo Delete ..\Wasabi2\.vs
	@call rd /S /Q ..\Wasabi2\.vs
	)
@if exist ..\Wasabi2\*.user ( 
	@echo Delete ..\Wasabi2\*.user
	@call del ..\Wasabi2\*.user
	)
@if exist ..\Wasabi2\*.filters (
	@echo Delete ..\Wasabi2\*.filters
	@call del ..\Wasabi2\*.filters
	)
@if exist ..\Wasabi2\*.htm (
	@echo Delete ..\Wasabi2\*.htm
	@call del ..\Wasabi2\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * wbm project
@echo *******************************
@if exist ..\wbm\x86_Debug (
	@echo Delete ..\wbm\x86_Debug
	@call rd /S /Q ..\wbm\x86_Debug
	)
@if exist ..\wbm\x86_Release (
	@echo Delete ..\wbm\x86_Release
	@call rd /S /Q ..\wbm\x86_Release
	)
@if exist ..\wbm\x64_Debug (
	@echo Delete ..\wbm\x64_Debug
	@call rd /S /Q ..\wbm\x64_Debug
	)
@if exist ..\wbm\x64_Release (
	@echo Delete ..\wbm\x64_Release
	@call rd /S /Q ..\wbm\x64_Release
	)
@if exist ..\wbm\.vs (
	@echo Delete ..\wbm\.vs
	@call rd /S /Q ..\wbm\.vs
	)
@if exist ..\wbm\*.user ( 
	@echo Delete ..\wbm\*.user
	@call del ..\wbm\*.user
	)
@if exist ..\wbm\*.filters (
	@echo Delete ..\wbm\*.filters
	@call del ..\wbm\*.filters
	)
@if exist ..\wbm\*.htm (
	@echo Delete ..\wbm\*.htm
	@call del ..\wbm\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * Winamp project
@echo *******************************
@if exist ..\Winamp\x86_Debug (
	@echo Delete ..\Winamp\x86_Debug
	@call rd /S /Q ..\Winamp\x86_Debug
	)
@if exist ..\Winamp\x86_Release (
	@echo Delete ..\Winamp\x86_Release
	@call rd /S /Q ..\Winamp\x86_Release
	)
@if exist ..\Winamp\x64_Debug (
	@echo Delete ..\Winamp\x64_Debug
	@call rd /S /Q ..\Winamp\x64_Debug
	)
@if exist ..\Winamp\x64_Release (
	@echo Delete ..\Winamp\x64_Release
	@call rd /S /Q ..\Winamp\x64_Release
	)
@if exist ..\Winamp\.vs (
	@echo Delete ..\Winamp\.vs
	@call rd /S /Q ..\Winamp\.vs
	)
@if exist ..\Winamp\*.user (
	@echo Delete ..\Winamp\*.user
	@call del ..\Winamp\*.user
	)
@if exist ..\Winamp\*.filters (
	@echo Delete ..\Winamp\*.filters
	@call del ..\Winamp\*.filters
	)
@if exist ..\Winamp\*.htm (
	@echo Delete ..\Winamp\*.htm
	@call del ..\Winamp\*.htm
	)	
@echo -------------------------------
@echo *******************************
@echo * winampa project
@echo *******************************
@if exist ..\winampa\x86_Debug (
	@echo Delete ..\winampa\x86_Debug
	@call rd /S /Q ..\winampa\x86_Debug
	)
@if exist ..\winampa\x86_Release (
	@echo Delete ..\winampa\x86_Release
	@call rd /S /Q ..\winampa\x86_Release
	)
@if exist ..\winampa\x64_Debug (
	@echo Delete ..\winampa\x64_Debug
	@call rd /S /Q ..\winampa\x64_Debug
	)
@if exist ..\winampa\x64_Release (
	@echo Delete ..\winampa\x64_Release
	@call rd /S /Q ..\winampa\x64_Release
	)
@if exist ..\winampa\.vs (
	@echo Delete ..\winampa\.vs
	@call rd /S /Q ..\winampa\.vs
	)
@if exist ..\winampa\*.user (
	@echo Delete ..\winampa\*.user
	@call del ..\winampa\*.user
	)
@if exist ..\winampa\*.filters (
	@echo Delete ..\winampa\*.filters
	@call del ..\winampa\*.filters
	)
@if exist ..\winampa\*.htm (
	@echo Delete ..\winampa\*.htm
	@call del ..\winampa\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * xml project
@echo *******************************
@if exist ..\xml\x86_Debug (
	@echo Delete ..\xml\x86_Debug
	@call rd /S /Q ..\xml\x86_Debug
	)
@if exist ..\xml\x86_Release (
	@echo Delete ..\xml\x86_Release
	@call rd /S /Q ..\xml\x86_Release
	)
@if exist ..\xml\x64_Debug (
	@echo Delete ..\xml\x64_Debug
	@call rd /S /Q ..\xml\x64_Debug
	)
@if exist ..\xml\x64_Release (
	@echo Delete ..\xml\x64_Release
	@call rd /S /Q ..\xml\x64_Release
	)
@if exist ..\xml\.vs (
	@echo Delete ..\xml\.vs
	@call rd /S /Q ..\xml\.vs
	)
@if exist ..\xml\*.user (
	@echo Delete ..\xml\*.user
	@call del ..\xml\*.user
	)
@if exist ..\xml\*.filters (
	@echo Delete ..\xml\*.filters
	@call del ..\xml\*.filters
	)
@if exist ..\xml\*.htm (
	@echo Delete ..\xml\*.htm
	@call del ..\xml\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * xspf project
@echo *******************************
@if exist ..\xspf\x86_Debug (
	@echo Delete ..\xspf\x86_Debug
	@call rd /S /Q ..\xspf\x86_Debug
	)
@if exist ..\xspf\x86_Release (
	@echo Delete ..\xspf\x86_Release
	@call rd /S /Q ..\xspf\x86_Release
	)
@if exist ..\xspf\x64_Debug (
	@echo Delete ..\xspf\x64_Debug
	@call rd /S /Q ..\xspf\x64_Debug
	)
@if exist ..\xspf\x64_Release (
	@echo Delete ..\xspf\x64_Release
	@call rd /S /Q ..\xspf\x64_Release
	)
@if exist ..\xspf\.vs (
	@echo Delete ..\xspf\.vs
	@call rd /S /Q ..\xspf\.vs
	)
@if exist ..\xspf\*.user (
	@echo Delete ..\xspf\*.user
	@call del ..\xspf\*.user
	)
@if exist ..\xspf\*.filters (
	@echo Delete ..\xspf\*.filters
	@call del ..\xspf\*.filters
	)
@if exist ..\xspf\*.htm (
	@echo Delete ..\xspf\*.htm
	@call del ..\xspf\*.htm
	)
@echo -------------------------------
@echo *******************************
@echo * zlib project
@echo *******************************
@if exist ..\replicant\zlib\x86_Debug (
	@echo Delete ..\replicant\zlib\x86_Debug
	@call rd /S /Q ..\replicant\zlib\x86_Debug
	)
@if exist ..\replicant\zlib\x86_Release (
	@echo Delete ..\replicant\zlib\x86_Release
	@call rd /S /Q ..\replicant\zlib\x86_Release
	)
@if exist ..\replicant\zlib\x64_Debug (
	@echo Delete ..\replicant\zlib\x64_Debug
	@call rd /S /Q ..\replicant\zlib\x64_Debug
	)
@if exist ..\replicant\zlib\x64_Release (
	@echo Delete ..\replicant\zlib\x64_Release
	@call rd /S /Q ..\replicant\zlib\x64_Release
	)
@if exist ..\replicant\zlib\.vs (
	@echo Delete ..\replicant\zlib\.vs
	@call rd /S /Q ..\replicant\zlib\.vs
	)
@if exist ..\replicant\zlib\*.user (
	@echo Delete ..\replicant\zlib\*.user
	@call del ..\replicant\zlib\*.user
	)
@if exist ..\replicant\zlib\*.filters (
	@echo Delete ..\replicant\zlib\*.filters
	@call del ..\replicant\zlib\*.filters
	)
@if exist ..\replicant\zlib\*.htm (
	@echo Delete ..\replicant\zlib\*.htm
	@call del ..\replicant\zlib\*.htm
	)

@pause	
@exit
goto :eof
::-----------------------------------------------------------------------------
::                               EXIT
::-----------------------------------------------------------------------------
:exit

endlocal & exit /b %rc%
