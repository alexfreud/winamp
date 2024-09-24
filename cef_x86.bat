set GN_DEFINES=is_official_build=true
set GYP_MSVS_VERSION=2019
set CEF_ARCHIVE_FORMAT=tar.bz2
set DEPOT_TOOLS_UPDATE=0
python3 automate-git.py --download-dir=D:\CEF --branch=5414 --minimal-distrib --client-distrib --force-clean --with-pgo-profiles

pause