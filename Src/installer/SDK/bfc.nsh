; BFC
SetOutPath $INSTDIR\Wasabi\bfc
File ${PROJECTS}\Wasabi\bfc\dispatch.h
File ${PROJECTS}\Wasabi\bfc\multipatch.h
File ${PROJECTS}\Wasabi\bfc\std_mkncc.h
File ${PROJECTS}\Wasabi\bfc\nsguid.h

SetOutPath $INSTDIR\Wasabi\bfc\platform
File ${PROJECTS}\Wasabi\bfc\platform\types.h
File ${PROJECTS}\Wasabi\bfc\platform\guid.h
File ${PROJECTS}\Wasabi\bfc\platform\platform.h
File ${PROJECTS}\Wasabi\bfc\platform\win32.h
File ${PROJECTS}\Wasabi\bfc\platform\export.h