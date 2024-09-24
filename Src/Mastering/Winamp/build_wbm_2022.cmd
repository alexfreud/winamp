@echo off
REM copy this file to root of output dir (where winamp.exe and wbm.exe reside) and run to generate .wbm files
REM cd /D %CURSANDBOX%\output\Winamp
REM wbm auto "System\a52.wbm" "System\a52.w5s"
REM wbm auto "System\aacdec.wbm" "System\aacdec.w5s"
wbm auto "System\adpcm.wbm" "System\adpcm.w5s"
REM wbm auto "System\alac.wbm" "System\alac.w5s"
REM wbm auto "System\dca.wbm" "System\dca.w5s"
wbm auto "System\f263.wbm" "System\f263.w5s"
REM wbm auto "System\h264.wbm" "System\h264.w5s"
wbm auto "System\mp4v.wbm" "System\mp4v.w5s"
wbm auto "System\pcm.wbm" "System\pcm.w5s"
wbm auto "System\theora.wbm" "System\theora.w5s"
wbm auto "System\vlb.wbm" "System\vlb.w5s"
wbm auto "System\vp6.wbm" "System\vp6.w5s"
wbm auto "System\vp8.wbm" "System\vp8.w5s"
wbm auto "System\jnetlib.wbm" "System\jnetlib.w5s"

pause
