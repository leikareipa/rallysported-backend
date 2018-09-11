#!/bin/bash

############################################################################################
##
## Runs validation tests; after which bulds all Win32 versions of RGEO.
##
############################################################################################

# Perform validative tests.
./test_rgeo_import_export.sh &&

# Build the actual program.
./build/build_rgeo_win32_ogl_gcc.sh &&
./build/build_rgeo_win32_sw_gcc.sh &&
./build/build_rgeo_win32_glide_gcc.sh
#./build/build_rgeo_win32_msi_gcc.sh &&		# Not ready yet.
#./build/build_rgeo_win32_s3d_gcc.sh &&		# Not ready yet.


