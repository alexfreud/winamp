${SegmentFile}

${Segment.OnInit}
	; Borrowed the following from PAL 2.2, Remove on release of PAL 2.2
		; Work out if it's 64-bit or 32-bit
	System::Call kernel32::GetCurrentProcess()i.s
	System::Call kernel32::IsWow64Process(is,*i.r0)
	${If} $0 == 0
		StrCpy $Bits 32
		Rename "$EXEDIR\App\7-Zip64\Lang" "$EXEDIR\App\7-Zip\Lang"
	${Else}
		StrCpy $Bits 64
		Rename "$EXEDIR\App\7-Zip\Lang" "$EXEDIR\App\7-Zip64\Lang"
	${EndIf}
!macroend

${SegmentInit}
    ${If} $Bits = 64
        ${SetEnvironmentVariablesPath} FullAppDir "$EXEDIR\App\7-Zip64"
	${Else}
        ${SetEnvironmentVariablesPath} FullAppDir "$EXEDIR\App\7-Zip"
	${EndIf}
!macroend

${SegmentPre}
	${Registry::StrToHex} ":" $9 ;$9 now contains the ASCII code for :
	ExpandEnvStrings $0 "%PAL:Drive%"
	${Registry::StrToHex} $0 $1 ;$1 now contains the ASCII code for current drive
	${WordReplace} $1 $9 "" "+" $2
		
	ExpandEnvStrings $3 "%PAL:LastDrive%"
	${Registry::StrToHex} $3 $4 ;$4 now contains the ASCII code for last drive
	${WordReplace} $4 $9 "" "+" $5
	
	System::Call 'Kernel32::SetEnvironmentVariable(t, t) i("PAL:DriveHex", "$2").r0'
	System::Call 'Kernel32::SetEnvironmentVariable(t, t) i("PAL:LastDriveHex", "$5").r0'
!macroend