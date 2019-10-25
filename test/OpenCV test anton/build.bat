@echo off
clang++ main.cc -std=c++17 -O2 -fno-exceptions -march=haswell -lmsvcrt -llibcmt -lopencv_world411
REM call vcvarsall x64
REM cl /std:c++17 main.cc /EHsc /Ox opencv_world411.lib
