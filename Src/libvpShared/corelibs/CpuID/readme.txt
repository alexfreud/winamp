This library contains functions
that will determine the type of CPU that is in your system.  See cpuidlib.h for
a more detailed description of the functions that are avaliable.

If you want to use the library all you need to do is to fetch

 - cpuidlib.h
 - cpuidlib.lib


October 14 1999
Jong Chen
    
    This is the initial revision of the library.
    
    At the moment the code is not fully tested.  The code that tests for OS support
    of Pentium III instructions has only been tested on systems with OS that
    support the Pentium III instructions.  It has not been tested in a
    configuration where we will detect the the OS will not support the Pentium III
    instructions.


