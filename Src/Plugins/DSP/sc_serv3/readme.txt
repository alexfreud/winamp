Building sc_serv2
-----------------

The build dependencies for sc_serv2 are expat and zlib and these already have a pre-built
library stored in the 'libs' folder. Otherwise everything else is built from the contents
of the sc_serv3 folder so the libs only need to be updated rarely.

It is assumed in all cases that there is a valid gcc + g++ tool chain in place along with
all standard libraries for building tools on the platform being used.


Win32 / Win64
-------------

The MSVC 2008 project builds straight from here with it using the pre-built libraries.

As from DNAS v2.6, additional libcurl, libssl & libcrypto .lib files are required.
External zip files can be downloaded from the Nullsoft github repo:
https://github.com/Radionomy/Nullsoft/blob/master/Shoutcast/sc_serv3/deps/win32.7z
https://github.com/Radionomy/Nullsoft/blob/master/Shoutcast/sc_serv3/deps/win64.7z

Extract locally to Shoutcast\sc_serv3\deps


BSD / Mac OS X / Linux
----------------------

The following is only needed if there is an update of the dependency library current
setup requires building expat before you can build sc_serv2 itself (not ideal but as it
is usually a build once event on the dependencies then it isn't too much of an issue...).

EXPAT
-----

In most cases just running ./unix_build_expat in the aolxml folder will build it.


ZLIB
----

./configure --static && make

May also need to set (or applicable):

    export CC="/usr/bin/gcc44"

in order to get it using the desired version of GCC on the system used to build this.


-----------

Once all of the dependencies have been built then you just need to do "(g)make release"
to get a build. The make stage accepts the following modes:

    clean
    release
    fire
    debug


Building on Centos 7 (on x64)
-----------------------------
Packages to install to enable compilation
    sudo yum install gcc-c++
    sudo yum install openssl-devel
    sudo yum install libstdc++-static
    
    curl http://mirror.centos.org/centos/8/BaseOS/x86_64/os/Packages/libcurl-devel-7.61.1-22.el8.x86_64.rpm > libcurl-devel.rpm
    sudo rpm install libcur-devel.rpm

If the directory deps/x86_64/lib does not exists create it
    mkdir -p deps/x86_64/lib
Copy the static dependencies to this directory
    cp libs/Aol_XML/Linux_x86_64/libexpat.a deps/x86_64/lib/
    cp libs/zlib/Linux_x86_64/libz.a deps/x86_64/lib/

Notes
-----

Building assumes that both the C and C++ compilers are correctly setup on the machine.
When doing the linux build on ubuntu 10.10 the g++ compiler was not available leading to
"error trying to exec 'cc1plus' execvp" errors during building. The fix is:

    sudo apt-get install g++

When doing the linux build on Centos 5.5 the g++ compiler was not available leading to
"gcc: error trying to exec 'cc1plus': execvp: No such file or directory". The fix is:

    yum install gcc-c++
