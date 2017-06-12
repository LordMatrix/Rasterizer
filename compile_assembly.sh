
gcc -c -g -Wall -O0 -o buildobjs/rasterizer.o rasterizer.c
gcc -c -g -Wall -O0 -o buildobjs/chrono.o chrono.c
gcc -c -g -Wall -O0 -o buildobjs/rasterizer_inner.o rasterizer.s


cd buildobjs
gcc -o ../rasterizer.elf rasterizer.o chrono.o rasterizer_inner.o -lSDL -lm
