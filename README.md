# Winamp

## About

Winamp is a multimedia player launched in 1997, iconic for its flexibility and wide compatibility with audio formats. Originally developed by Nullsoft, it gained massive popularity with still millions of users. Its development slowed down, but now, its source code was opened to the community, allowing developers to improve and modernize the player to meet current user needs.

## Usage

Building of the Winamp desktop client is currently based around Visual Studio 2019 (VS2019) and Intel IPP libs (You need to use exactly v6.1.1.035).
There are different options of how to build Winamp:

1. Use a build_winampAll_2019.cmd script file that makes 4 versions x86/x64 (Debug and Release). In this case Visual Studio IDE not running.
2. Use a winampAll_2019.sln file to build and debug in Visual Studio IDE.

### Dependencies

#### libvpx
We take libvpx from https://github.com/ShiftMediaProject/libvpx, modify it and pack to archive.
Run unpack_libvpx_v1.8.2_msvc16.cmd to unpack.

#### libmpg123
We take libmpg123 from https://www.mpg123.de/download.shtml, modify it and pack to archive.
Run unpack_libmpg123.cmd to unpack and process dlls.

#### OpenSSL
You need to use openssl-1.0.1u. For that you need to build a static version of these libs.
Run build_vs_2019_openssl_x86.cmd and build_vs_2019_openssl_64.cmd.

To build OpenSSL you need to install

7-Zip, NASM and Perl.

#### DirectX 9 SDK 
We take DirectX 9 SDK (June 2010) from Microsoft, modify it and pack to archive.
Run unpack_microsoft_directx_sdk_2010.cmd to unpack it.

#### Microsoft ATLMFC lib fix
In file C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\atlmfc\include\atltransactionmanager.h

goto line 427 and change from 'return ::DeleteFile((LPTSTR)lpFileName);' to 'return DeleteFile((LPTSTR)lpFileName);'

#### Intel IPP 6.1.1.035
We take Intel IPP 6.1.1.035, modify it and pack to archive.
Run unpack_intel_ipp_6.1.1.035.cmd to unpack it.
