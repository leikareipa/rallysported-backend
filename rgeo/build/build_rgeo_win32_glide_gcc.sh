#!/bin/bash

############################################################################################
##
## Uses GCC/MinGW to build RGEO for Win32 w/ Glide rendering.
##
############################################################################################

GCC_PATH=~/compi/mingw471

EXE_NAME="rgeo_win32_glide.exe"
BUILD_OPTIONS="-nostdinc -s -std=c++11 -march=pentium -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++/mingw32/ -I$GCC_PATH/lib/gcc/mingw32/4.7.1/include/c++ -I$GCC_PATH/include -DRSED_ON_WIN32 -DRENDERER_IS_GLIDE"

SRC_FILES="
src\\core\\main\\main.cpp
src\\core\\file\\file.cpp
src\\core\\game_exe\\game_exe.cpp
src\\core\\lookups\\lookups.cpp
src\\core\\maasto\\maasto.cpp
src\\core\\palat\\palat.cpp
src\\core\\palette\\palette.cpp
src\\core\\cmd_line\\cmd_line.cpp
src\\core\\camera\\camera.cpp
src\\core\\ui\\ui_logic.cpp
src\\core\\ui\\ui_layout.cpp
src\\core\\text\\text.cpp
src\\core\\memory\\\memory.cpp
src\\core\\ray_trace\\ray_trace.cpp
src\\core\\frame_timer\\frame_timer.cpp
src\\core\\props\\props.cpp
src\\core\\manifesto\\manifesto.cpp
src\\core\\geometry\\geometry_sw.cpp
src\\core\\geometry\\geometry_sw_matrix.cpp
src\\core\\geometry\\geometry_sw_triangle.cpp
src\\dendrons\\render\\glide\\render_glide.cpp
src\\dendrons\\display\\win32\\glide\\display_win32_glide.cpp
src\\dendrons\\display\\win32\\display_win32_common.cpp
"

echo "Building Glide..."
wine "$GCC_PATH/bin/g++.exe" $BUILD_OPTIONS -o $EXE_NAME $SRC_FILES -lm -lgdi32 -lwinmm -lglide/glide3x

if [ -e $EXE_NAME ]
then
	mv $EXE_NAME bin/$EXE_NAME
else
	echo Build failed.
	exit 1
fi

exit 0

