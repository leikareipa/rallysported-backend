# Build RGEO using Digital Mars C/C++ 8 (DMC)

DMC_PATH=~/compi/dmc
OUTPUT_EXE_FILENAME="bin\rgeo.exe"
BUILD_OPTIONS="-a4 -A -f -mld -o+speed -wx -4 -o $OUTPUT_EXE_FILENAME"

wine "$DMC_PATH/bin/dmc.exe" src/*.c $BUILD_OPTIONS -I$DMC_PATH\\include
rm *.obj
