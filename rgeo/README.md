# RGEO (RallySportED)
RGEO is the main module of RallySportED; it allows editing of Rally-Sport tracks' heightmaps, tilemaps, and textures. For instructions on how to use RGEO, see the readme file included with its binary distribution, which is downloadable from [Tarpeeksi Hyvae Soft's website](http://tarpeeksihyvaesoft.com/soft).

This open-source distribution of RGEO is compatible with Linux (Qt) and Windows (Qt and Win32 API). A binary-only version of RGEO is also available for DOS, from the above-mentioned website, but its source code is not included here.

### Building
For the Qt version on Linux, do ```qmake && make```, or load the .pro file in Qt Creator. If you want to build the Win32 versions on Linux, too, find included a few scripts for that purpose, of which ```build_all_win32.sh``` is the master. The scripts use MinGW with Wine, which assumes that you have both on your system.

On Windows, whatever is its equivalent for building the Qt version is what might be best to use. For the Win32 versions of RGEO, you may need to convert the corresponding .sh scripts into .bats or the like, which should be fairly straightforward.

If you use MinGW 4.7.1 (possibly up to 4.7.2), the resulting executables should run on as far back as Windows 95. Later versions of MinGW drop support for Windows 9x, but if you don't mind this, they should work fine, too. I use MinGW 4.7.1 for Win32 builds, and Clang or GCC for Qt.

### Tests
RGEO has rudimentary facilities for testing import/export functionality, i.e. to validate RGEO's ability to import and export a RallySportED project without losing or corrupting its data in the process.

A Bash script is included to compile and run this test using Wine and MinGW. If you want to run the test with a native Qt version, you can do ```qmake 'DEFINES+=VALIDATION_RUN' && make -B```, or equivalent, and run the resulting binary in a directory that has Rally-Sport's files in it. The test will output its result into the console.

### Code flow
RGEO's code is organized into units on the level of individual .cpp files, each of which maintains a local unit-wide state of some size. State is encapsulated within-unit using the C++ ```static``` specifier. This is to say that RGEO's large-scale organization is into encapsulated units rather than classes.

The main program loop is in ```src/core/main/main.cpp```. Its goals are two: (1) accept user input to modify game assets, e.g. the terrain heightmap; and (2) produce for display an image of the assets' current state, usually a 3d scene, based on which the user can choose whether and how to modify those assets further. These two goals involve, among other things, management of asset states (e.g. ```src/core/maasto/``` and ```src/core/palat/```), as well as triangle rasterization (```src/dendrons/render/```).

You'll notice from the paths above that RGEO's codebase is split at the root into two branches, ```core``` and ```dendrons```. The program was designed to run on several platforms, so the divider here is that ```core``` will accept code that's meant to run on all intended platforms, while ```dendrons``` contains platform-specific functionality not included in builds for other platforms.
