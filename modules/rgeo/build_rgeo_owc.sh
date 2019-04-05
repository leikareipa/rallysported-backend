# Build RGEO for DOS using Open Watcom C/C++ in Linux.

OW_PATH=~/compi/watcom
OUTPUT_EXE_FILENAME=../bin/rgeo.exe
BUILD_OPTIONS="-bcl=dos -cc -ml -oneatx -oh -ei -zp2 -4 -fpi87 -sg -k16384 -fe=$OUTPUT_EXE_FILENAME"

cd src
export WATCOM=$OW_PATH
export INCLUDE=$WATCOM/h
PATH=$PATH:$OW_PATH/binl
wcl $BUILD_OPTIONS *.c

rm *.o
