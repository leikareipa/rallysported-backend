#!/bin/bash

############################################################################################
##
## Builds and runs a data validity test in RGEO for project import/export.
##
############################################################################################

./build/build_rgeo_test_imp_exp_gcc.sh && cd bin && wine ./rgeo_test_imp_exp.exe

