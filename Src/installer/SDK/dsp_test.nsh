; Sample DSP Plugin
SetOutPath $INSTDIR\dsp_test

; project files
File ${SDKPlugins}\dsp_test\dsp_test.sln
File ${SDKPlugins}\dsp_test\dsp_test.vcxproj
File ${SDKPlugins}\dsp_test\dsp_test.vcxproj.filters

; source
File ${SDKPlugins}\dsp_test\dsp_test.c


; documentation
File ${SDKPlugins}\dsp_test\dsp_ns.txt

; resources
File ${SDKPlugins}\dsp_test\RESOURCE.H
File ${SDKPlugins}\dsp_test\SCRIPT1.RC
