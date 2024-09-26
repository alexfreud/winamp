
# Winamp

## About

Winamp is a multimedia player launched in 1997, iconic for its flexibility and wide compatibility with audio formats. Originally developed by Nullsoft, it gained massive popularity with still millions of users. Its development slowed down, but now, its source code was opened to the community, allowing developers to improve and modernize the player to meet current user needs.

It really whips the llama's ass.

## Usage

Building of the Winamp desktop client is currently based around Visual Studio 2019 (VS2019) and Intel IPP libs (You need to use exactly v6.1.1.035). There are different options of how to build Winamp:

1. Use the `build_winampAll_2019.cmd` script file that makes 4 versions x86/x64 (Debug and Release). In this case, Visual Studio IDE is not required.
2. Use the `winampAll_2019.sln` file to build and debug in Visual Studio IDE.

### Dependencies

#### libvpx
We take libvpx from [https://github.com/ShiftMediaProject/libvpx](https://github.com/ShiftMediaProject/libvpx), modify it, and pack it to archive.
Run `unpack_libvpx_v1.8.2_msvc16.cmd` to unpack.

#### libmpg123
We take libmpg123 from [https://www.mpg123.de/download.shtml](https://www.mpg123.de/download.shtml), modify it, and pack it to archive.
Run `unpack_libmpg123.cmd` to unpack and process the DLLs.

#### OpenSSL
You need to use `openssl-1.0.1u`. For that, you need to build a static version of these libs.
Run `build_vs_2019_openssl_x86.cmd` and `build_vs_2019_openssl_64.cmd`.

To build OpenSSL, you need to install:

- 7-Zip ([https://www.7-zip.org/](https://www.7-zip.org/)) – Licensed under the GNU LGPL.
- NASM ([https://www.nasm.us/](https://www.nasm.us/)) – Licensed under the 2-Clause BSD License.
- Perl ([https://www.perl.org/](https://www.perl.org/)) – Licensed under the Artistic License or GPL.

#### DirectX 9 SDK
We take DirectX 9 SDK (June 2010) from Microsoft, modify it, and pack it to archive.
Run `unpack_microsoft_directx_sdk_2010.cmd` to unpack it.

#### Microsoft ATLMFC lib fix
In file `C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\atlmfc\include\atltransactionmanager.h`

Go to line 427 and change from:
```cpp
return ::DeleteFile((LPTSTR)lpFileName);
```
to:
```cpp
return DeleteFile((LPTSTR)lpFileName);
```

#### Intel IPP 6.1.1.035
We take Intel IPP 6.1.1.035, modify it, and pack it to archive.

Run `unpack_intel_ipp_6.1.1.035.cmd` to unpack it.

### Build Tools

Several external build tools are required to build Winamp. These tools are not bundled directly into the repository to comply with their respective licenses. You will need to download them separately from the following links:

- **7-Zip Portable**: Download from [https://www.7-zip.org/](https://www.7-zip.org/)  
  License: GNU LGPL

- **Git**: Download from [https://git-scm.com/download/win](https://git-scm.com/download/win)  
  License: GNU GPL v2

- **TortoiseSVN**: Download from [https://tortoisesvn.net/downloads.html](https://tortoisesvn.net/downloads.html)  
  License: GNU GPL v2

Make sure to install these tools as part of your build environment. You may need to modify the build scripts to reflect the correct paths to these tools on your system.
