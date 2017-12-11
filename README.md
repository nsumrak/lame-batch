Multicore batch encoding
========================
This project is used to batch convert WAV files to MP3 using LAMP library. It is designed to be compiled
for Windows/Win32 target using Visual Studio (tested with Visual Studio 2013 Express for Desktop and
Visual Studio 2015 Community) or Linux with standard GCC dev toolkit.

Building for Windows
--------------------
Open lametest.sln with Visual Studio and press F5.

Building for Linux
------------------
To build:
    $> make

To clean:
    $> make clean

To run:
    $> ./lametest <path>
<path>: file or directory to process

### Example:
    ./lametest testcase
This will encode provided test case WAV's into MP3

Options
-------
You can opt to recurse subdirectories for the given path by changing:
    const bool gc_recursive = false;
to true.

By default, VBR with standard quality is used for encoding. You can adjust settings by changing open()
mathod of LameEnc class withing lame_enc.h file.

Notes:
------
* Project is designed for little endian systems (eg. Intel).
* Only 8, 16, 24, 32-bit signed PCM WAV files are supported.
* Tested on Linux (lubuntu), but should work on any system with make and gcc.


Additional Open Source code included
------------------------------------
This project uses following open source code:

pthreads-win32, version 2.9.1, precompiled, LGPL
[link: http://sourceware.org/pthreads-win32/](http://sourceware.org/pthreads-win32/)

Windows Dirent inteface, MIT
[link: https://github.com/tronkko/dirent](https://github.com/tronkko/dirent)

LAME, version 3.100, LGPL
[link: http://lame.sourceforge.net/](http://lame.sourceforge.net/)

Test case WAVs are excerpt based on "Divan Dan" song by E-Play band
[link: http://www.eplaymusic.com/](http://www.eplaymusic.com/)
