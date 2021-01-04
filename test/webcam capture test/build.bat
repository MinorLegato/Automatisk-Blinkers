@echo off
clang++ main.cc -o ../bin/capture_test.exe ^
 -std=c++17 -O2 -fno-exceptions -march=haswell -lmsvcrt -llibcmt -lopencv_world411
