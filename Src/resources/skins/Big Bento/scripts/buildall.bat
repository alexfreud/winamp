@echo off

pushd %~dp0..\..\..\..\wasabi\
set basedir=%cd%
popd

for %%a in (*.m) do "%basedir%\mc.exe" %%a
pause