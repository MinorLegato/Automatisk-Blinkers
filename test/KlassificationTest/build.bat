@echo off
clang++ main.cc -o ../bin/klassTest.exe ^
 -std=c++17 -O2 -fno-exceptions -march=haswell -lmsvcrt -llibcmt -lopencv_world411
