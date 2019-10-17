@echo off
call vcvarsall x64
cl /std:c++17 main.cc /EHsc /Ox opencv_world411.lib
