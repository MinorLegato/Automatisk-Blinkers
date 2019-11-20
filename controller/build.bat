@echo off
gcc -static client.c -o client.exe -s -std=gnu11 -O2 -march=haswell -flto -lmingw32 -lglfw3 -lopengl32 -lgdi32 -lws2_32
gcc -static server.c -o server.exe -s -std=gnu11 -O2 -march=haswell -flto -lws2_32
