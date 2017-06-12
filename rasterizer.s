.global rasterize
@ void rasterize(unsigned int* pixels, Point** p, int pitch, int color);
@ r0 - unsigned int* pixels
@ r1 - Point** p
@ r2 - int pitch
@ r3 - int color
@---------------
@r4, minx
@r5, maxx
@r6, miny
@r7, maxy
@---------------
@r8 x
@r9 y
@---------------
@r10 (x1-x2) * (y-y1) - (y1-y2) * (x - x1)
@r11 (x2-x3) * (y-y2) - (y2-y3) * (x - x2)
@r12 (x3-x4) * (y-y3) - (y3-y4) * (x - x3)
@r1  (x4-x1) * (y-y4) - (y4-y1) * (x - x4)

rasterize:
  stmdb sp!, {r4, r5, r6, r7, r8, r9, r10, r11, r12}
  ldr r4, [r1, #1, lsl #0]@load p[0]->x in r4
  ldr r9, [r1, #1, lsl #1]@compare p[1]->x with r4 -> If less, save in r4
  cmp r4, r9
  bgt x3min
  mov r4, r9
  x3min:
  ldr r9, [r1, #1, lsl #2]@compare p[2]->x with r4 -> If less, save in r4
  cmp r4, r9
  bgt x4min
  mov r4, r9
  x4min:
  ldr r9, [r1, #1, lsl #3]@compare p[3]->x with r4 -> If less, save in r4
  cmp r4, r9
  bgt x1max
  mov r4, r9
  x1max:
  ldr r5, [r1, #1, lsl #0] @r5 - maxx
  ldr r9, [r1, #1, lsl #1] @r9
  cmp r5, r9
  blt x3max
  mov r5, r9
  x3max:
  ldr r9, [r1, #1, lsl #2] @r9
  cmp r5, r9
  blt x4max
  mov r5, r9
  x4max:
  ldr r0, [r1, #1, lsl #3] @r9
  cmp r5 r9
  blt y1min
  mov r5, r9
  y1min:
  @--------------------------------
  ldr r6, [r1, #2, lsl #0] @r6 - miny
  ldr r8, [r1, #2, lsl #1] @r8
  cmp r6, r8
  bgt y3min
  mov r6, r8
  y3min:
  ldr r8, [r1, #2, lsl #2] @r8
  cmp r6, r8
  bgt y4min
  mov r6, r8
  y4min:
  ldr r8, [r1, #2, lsl #3] @r8
  cmp r6, r8
  bgt y1max
  mov r6, r8
  y1max:
  ldr r7, [r1, #2, lsl #0] @r7 - maxy
  ldr r8, [r1, #2, lsl #1] @r8
  cmp r7, r8
  blt y3max
  mov r7, r8
  y3max:
  ldr r8, [r1, #2, lsl #2] @r8
  cmp r7, r8
  blt y4max
  mov r7, r8
  y4max:
  ldr r8, [r1, #2, lsl #3] @r8
  cmp r7, r8
  blt out_min_max
  mov r7, r8
  @--------------------------------
  out_min_max:
  mov r8, #0 @r8, init y
  mov r9, #0 @r9, init x
  @--------------------------------

yloop:
  mov r9, #0 @r9 reset x

xloop:

@NO SOY CAPAZ DE SACAR VARIABLES PARA CALCULAR LOS DELTAS Y LUEGO TENER PARA ACUMULAR TODO
@if complex conditional - if (x1x2y - (y1y2 * (x - x1)) < 0 && x2x3y - (y2y3 * (x - x2)) < 0 && x3x4y - (y3y4 * (x - x3)) < 0 && x4x1y - (y4y1 * (x - x4)) < 0)
str r0, [r3, r8, r2] @pixels [ x + (y * pitch)] = color; //// ESTO ESTÁ MAL; FALTA "x+"


add r9, #1, r9  @r9 x++
cmp r9, r5      @x<maxx
blt xloop       @start x loop again if less than

add r8, #1, r8  @r8 y++
cmp r8, r7      @y<maxy
blt yloop       @start y loop again if less than

ldmia sp!,{r4, r5, r6, r7, r8, r9, r10, r11, r12}
bx lr


int x,y;

  //Use halfspace line equation method
  int y1 = p[0]->y;
  int y2 = p[1]->y;
  int y3 = p[2]->y;
  int y4 = p[3]->y;

  int x1 = p[0]->x;
  int x2 = p[1]->x;
  int x3 = p[2]->x;
  int x4 = p[3]->x;

  int minx = (int) min(x1,x2,x3,x4);
  int maxx = (int) max(x1,x2,x3,x4);
  int miny = (int) min(y1,y2,y3,y4);
  int maxy = (int) max(y1,y2,y3,y4);

  int x1x2 = x1-x2;
  int x2x3 = x2-x3;
  int x3x4 = x3-x4;
  int x4x1 = x4-x1;

  int y1y2 = y1-y2;
  int y2y3 = y2-y3;
  int y3y4 = y3-y4;
  int y4y1 = y4-y1;

  for (y=miny; y< maxy; y++) {
    int x1x2y = x1x2 * (y-y1);
    int x2x3y = x2x3 * (y-y2);
    int x3x4y = x3x4 * (y-y3);
    int x4x1y = x4x1 * (y-y4);

    for (x=minx; x< maxx; x++) {
       {
        setColorPixel(pixels, x, y, color, pitch);
      }
    }
  }
