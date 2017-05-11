
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <SDL/SDL.h>

#include "sdl_funcs.h"


#define LCF (200.0f)    // En flotantes
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


// Floats
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



/// Sets the color of a pixel
void setColorPixel(unsigned int* pixels, int x, int y, int color) {
  int pitch = g_SDLSrf->pitch >> 2;
  pixels [ x + (y * pitch)] = color;
}



/// Retrieves the color of a pixel
int getColorPixel(unsigned int* pixels, int x, int y) {
  int pitch = g_SDLSrf->pitch >> 2;
  return pixels[ x + (y * pitch)];
}



/// Somewhat randomizes the color returned
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



/// Draws a straight line given a pair of points
void drawLine(unsigned int* pixels, int pitch, Point start, Point end) {
    //Bresenham's algorithm
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for (;;) {
      setColorPixel(pixels, start.x, start.y, 0xffffff);

      if (start.x==end.x && start.y==end.y)
        break;

      e2 = err;
      if (e2 >-dx) { err -= dy; start.x += sx; }
      if (e2 < dy) { err += dx; start.y += sy; }
    }
}



/// Shortcuts for the rotation matrix for a given axis. Axis => 0=x, 1=y, 2=z
/// Returns the rotated point
static Point rotatePoint(Point p, float rads, int axis) {
  float sinr = sin(rads);
  float cosr = cos(rads);
  Point trans;

  switch(axis) {
    // X-AXIS ROTATION
    case 0:
      trans.x = p.x;
      trans.y = p.y*cosr - p.z*sinr;
      trans.z = p.y*sinr + p.z*cosr;
      break;
    // Y-AXIS ROTATION
    case 1:
      trans.x = p.z*sinr + p.x*cosr;
      trans.y = p.y;
      trans.z = p.z*cosr - p.x*sinr;
      break;
    // Z-AXIS ROTATION (2D)
    case 2:
      trans.x = p.x*cosr - p.y*sinr;
      trans.y = p.x*sinr + p.y*cosr;
      trans.z = p.z;
      break;
    default:
      trans = p;
      break;
  }


  return trans;
}



/// Hides sides that are hidden behind others
static void doCull(unsigned int* pixels) {
  //There will only be a maximum of 3 sides showing simultaneously
  
}



/// Scans the pixels looking for line primitives. Colors the space between them
static void rasterize(unsigned int* pixels) {
  int i,j;
  int siding = 0;

  for (i = 0; i < g_SDLSrf->w; i++) {
    siding =0;
    for (j = 0; j < g_SDLSrf->h; j++) {
      int color = getColorPixel(pixels, i, j);

      if (color == 0x000000) {
        if (siding) {
          setColorPixel(pixels, i, j, 0xff0000);
        }
      } else {
        siding = (siding == 0) ? 1 : 0;
      }
    }
  }

}



static void PaintCubeInFloat ( unsigned int* pixels, float w, float h, int pitch, float trans_x, float trans_z, float proy, float rot) {   
  int i;

  Point points[8];
  //Indices for drawing lines to connect edges
  int indices[24] = { 0,1, 1,2, 2,3, 0,3,
                      4,5, 5,6, 6,7, 4,7,
                      0,4, 1,5, 2,6, 3,7};

  //Transform and paint all vertices in cube
  for ( i=0; i<8; i++) { 
    float xp, yp;
    float x = cubef [ i * 3 + 0];
    float y = cubef [ i * 3 + 1];
    float z = cubef [ i * 3 + 2];

    //Construct point
    Point p = makePoint(x, y, z);

    //Rotate in all axis
    p = rotatePoint(p, rot, 0);
    p = rotatePoint(p, rot, 1);
    p = rotatePoint(p, rot, 2);

    //project Z axis on 2 dimensions
    p.z -= trans_z;
    xp = (p.x * proy) / p.z;
    yp = (p.y * proy) / p.z;

    //translate points to the middle of the screen
    xp += w * 0.5f;
    yp += h * 0.5f;

    //Save point
    points[i] = makePoint(xp,yp,0);

    if (( xp >= 0) && (xp < w) && (yp >= 0) && (yp < h)) {
        int color = getColorByIndex(i);
        setColorPixel(pixels, (int)xp, (int)yp, color);
    }
  }

  //DRAW LINE PRIMITIVES
  for (i=0; i<24; i+=2) {
    Point start = points[indices[i]];
    Point end = points[indices[i+1]];
    drawLine(pixels, pitch, start, end);
  }
 
  //CULL
  doCull(pixels);

  //RASTERIZE POLYGONS
  rasterize(pixels); 
}

 
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


    // Girar los cubos en pantalla
    float x, z, offs_z = 1200.0f;
    float radius = 100.0f;
    x = 0.0f   + radius;
    z = offs_z + radius;
    PaintCubeInFloat ( g_screen_pixels, (float)g_SDLSrf->w, (float)g_SDLSrf->h, pitch, 
                       x, z, 
                       projection, ang);

    ang += 0.01f;

    frame_SDL();

    input_SDL();

    SDL_Delay(40.0f);
  }

  return 0;
}


