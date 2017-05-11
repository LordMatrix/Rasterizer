
gcc -c -g -Wall -O2 -o buildobjs/rasterizer.o rasterizer.c
gcc -c -g -Wall -O2 -o buildobjs/chrono.o chrono.c


cd buildobjs
gcc -o ../rasterizer.elf rasterizer.o chrono.o -lSDL -lm
