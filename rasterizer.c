
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <SDL/SDL.h>

#include "sdl_funcs.h"


/*
 Este ejemplo muestra como mover y proyectar puntos 3D
 en diferentes formatos numericos.
 Para vuestros trabajos debeis fijaros primero en la
 version de coma flotante
*/

// Definicion de un cubo en 3 formatos numericos diferentes
// Entero, coma fija 8 bits y flotante

#define LCF (100.0f)    // En flotantes
#define PI 3.1416f


//Encapsulated point
typedef struct {
  int x,y,z;
} Point;


//Point "constructor"
static Point makePoint(int x, int y, int z) {
  Point p; 
  p.x = x;
  p.y = y;
  p.z = z;
  return p;
}


// Flotantes

static float cubef [8*3] = {
    -LCF,-LCF, LCF,
     LCF,-LCF, LCF,
     LCF, LCF, LCF,
    -LCF, LCF, LCF,

    -LCF,-LCF, -LCF,
     LCF,-LCF, -LCF,
     LCF, LCF, -LCF,
     -LCF, LCF, -LCF,
};


int getColorByIndex(int i) {
    int color;
    switch (i) {
        case 0:
        case 4:
            color = 0xff0000;
            break;
        case 1:
        case 5:
            color = 0x00ff00;
            break;
        case 2:
        case 6:
            color = 0x0000ff;
            break;
        case 3:
        case 7:
            color = 0xffffff;
            break;
        default:
            color = 0xaaaaaa;
            break;
    }
    return color;
}


void drawLine(unsigned int* pixels, int pitch, Point start, Point end) {
    //Bresenham's algorithm
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for (;;) {
      pixels [ (start.x) + (start.y * pitch)] = 0xffffff;

      if (start.x==end.x && start.y==end.y)
        break;

      e2 = err;
      if (e2 >-dx) { err -= dy; start.x += sx; }
      if (e2 < dy) { err += dx; start.y += sy; }
    }
}



static void PaintCubeInFloat ( unsigned int* pixels, float w, float h, int pitch, float trans_x, float trans_z, float proy, float rot) {   
  int i;

  float angle = 0.0f;

  //Point prev_point;
  //Point first_point;
  Point points[8];
  int indexes[24] = { 0,1, 1,3, 2,3, 0,2,
                      4,5, 5,7, 6,7, 4,6,
                      0,4, 1,5, 2,6, 3,7};


  for ( i=0; i<8; i++) { 
    float xp, yp;
    float x = cubef [ i * 3 + 0];
    float y = cubef [ i * 3 + 1];
    float z = cubef [ i * 3 + 2];

    angle = i * ((2*PI) / 4.0f);

    x = cos(rot+angle) * x;
//    y = sin(rot+angle) * y;

    z += trans_z;

    xp = (x * proy) / z;
    yp = (y * proy) / z;
    xp += w * 0.5f;
    yp += h * 0.5f;

    //DRAW LINES
    Point current_point = makePoint(xp,yp,0);
/*
    if (i>0) {
       drawLine(pixels, pitch, prev_point, current_point);
    } else {
      first_point = current_point;
    }
    prev_point = current_point;
*/
    points[i] = current_point;

    if (( xp >= 0) && (xp < w) && (yp >= 0) && (yp < h)) {
        int color = getColorByIndex(i);
        pixels [ ((int)xp) + (((int)yp) * pitch)] = color;
    }
  }
  //drawLine(pixels, pitch, prev_point, first_point);

  for (i=0; i<24; i+=2) {
    Point start = points[indexes[i]];
    Point end = points[indexes[i+1]];
    drawLine(pixels, pitch, start, end);
  }
  
}

//0,1 1,2 2,3 3,0

//4,5 5,6 6,7 7,4
 
/*******************************/
/********* MAIN CODE  **********/
/*******************************/


int main ( int argc, char **argv) {
  
  init_SDL();

  // Horizontal field of view
  float hfov = 60.0f * ((PI * 2.0f) / 360.0f);  // De grados a radianes
  // Proyeccion usando la tangente
  float half_scr_w = (float)(g_SDLSrf->w >> 1);
  float projection = (1.0f / tan ( hfov * 0.5f)) * half_scr_w;

  float ang = 0.0f;

  g_end = 0;
  while (!g_end) { 
    clear_SDL();

    int pitch = g_SDLSrf->pitch >> 2;

    // Borrar pantalla
    memset ( g_screen_pixels, 0, g_SDLSrf->h * g_SDLSrf->pitch);  

    // Girar los cubos en pantalla
    float x, z, offs_z = 1200.0f;
    float radius = 100.0f;
    x = 0.0f   + radius * cos( ang + ((0.0f * PI * 2.0f) / 3.0f));
    z = offs_z + radius * sin( ang + ((0.0f * PI * 2.0f) / 3.0f));
    PaintCubeInFloat ( g_screen_pixels, (float)g_SDLSrf->w, (float)g_SDLSrf->h, pitch, 
                       x, z, 
                       projection, ang);

    ang += 0.05f;

    frame_SDL();

    input_SDL();

    SDL_Delay(40.0f);
  }

  return 0;
}


