#!/bin/bash

############################################################################################
##
## Uses GCC/MinGW to build RGEO for Win32 as a standalone test of project exporting - will import
## a project into RGEO, have RGEO export it, then check that the exported data is as expected.
##
############################################################################################

GCC_PATH=~/compi/mingw471

EXE_NAME="rgeo_test_imp_exp.exe"
BUILD_OPTIONS="-nostdinc -std=c++11 -O2 -march=pentium -I/home/op/compi/mingw471/lib/gcc/mingw32/4.7.1/include -I/home/op/compi/mingw471/lib/gcc/mingw32/4.7.1/include/c++/mingw32/ -I/home/op/compi/mingw471/lib/gcc/mingw32/4.7.1/include/c++ -I/home/op/compi/mingw471/include -DRSED_ON_WIN32 -DVALIDATION_RUN"

SRC_FILES="
src\\core\\tests\\test_import_export.cpp
src\\core\\file\\file.cpp
src\\core\\game_exe\\game_exe.cpp
src\\core\\lookups\\lookups.cpp
src\\core\\maasto\\maasto.cpp
src\\core\\palat\\palat.cpp
src\\core\\palette\\palette.cpp
src\\core\\camera\\camera.cpp
src\\core\\props\\props.cpp
src\\core\\memory\\memory.cpp
src\\core\\ui\\ui_logic.cpp
src\\core\\ui\\ui_layout.cpp
src\\core\\cmd_line\\cmd_line.cpp
src\\core\\text\\text.cpp
src\\core\\frame_timer\\frame_timer.cpp
src\\core\\ray_trace\\ray_trace.cpp
src\\core\\manifesto\\manifesto.cpp
src\\core\\geometry\\geometry_sw.cpp
src\\core\\geometry\\geometry_sw_matrix.cpp
src\\core\\geometry\\geometry_sw_triangle.cpp
src\\dendrons\\render\\null\\render_null.cpp
src\\dendrons\\display\\null\\display_null.cpp
"

wine "$GCC_PATH/bin/g++.exe" $BUILD_OPTIONS -o $EXE_NAME $SRC_FILES -lm -lgdi32 -lwinmm

if [ -e $EXE_NAME ]
then
	mv $EXE_NAME bin/$EXE_NAME
else
	echo "Build of import/export validation failed."
	exit 1
fi

exit 0

