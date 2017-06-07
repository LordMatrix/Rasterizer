#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <SDL/SDL.h>

#include "sdl_funcs.h"


#define LCF (200.0f)    // En flotantes
#define PI 3.1416f
#define DEBUG 0



//Encapsulated point
typedef struct {
  int x,y,z;
  int used;
} Point;

typedef struct {
  float x,y,z;
  int used;
} Pointf;


//Point "constructor"
static Point* makePoint(int x, int y, int z) {
  Point* p = malloc(sizeof(Point));
  p->x = x;
  p->y = y;
  p->z = z;
  return p;
}


// Floats
static float cubef [24] = {
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
void drawLine(unsigned int* pixels, int pitch, Point start, Point end, int color) {
    //Bresenham's algorithm
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for (;;) {
      setColorPixel(pixels, start.x, start.y, color);

      if (start.x==end.x && start.y==end.y)
        break;

      e2 = err;
      if (e2 >-dx) { err -= dy; start.x += sx; }
      if (e2 < dy) { err += dx; start.y += sy; }
    }
}


/// Draws a straight line given a pair of points
void drawLineF(unsigned int* pixels, int pitch, Pointf start, Pointf end, int color) {
    //Bresenham's algorithm
    int dx = abs(end.x - start.x), sx = start.x < end.x ? 1 : -1;
    int dy = abs(end.y - start.y), sy = start.y < end.y ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

    for (;;) {
      setColorPixel(pixels, start.x, start.y, color);

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

  trans.used = 0;

  return trans;
}



int min(int n1, int n2, int n3, int n4) {
  float minn = n1;

  if (n2 < minn) minn = n2;
  if (n3 < minn) minn = n3;
  if (n4 < minn) minn = n4;

  return minn;
}


int max(int n1, int n2, int n3, int n4) {
  float maxn = n1;

  if (n2 > maxn) maxn = n2;
  if (n3 > maxn) maxn = n3;
  if (n4 > maxn) maxn = n4;

  return maxn;
}


static void rasterize(unsigned int* pixels, Point** p, int pitch, int color) {
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

  for (y=miny; y< maxy; y++) {
    for (x=minx; x< maxx; x++) {
      //When all half-space functions are positive, point is in quad

      if ((x1 - x2) * (y - y1) - (y1 - y2) * (x - x1) < 0 &&
          (x2 - x3) * (y - y2) - (y2 - y3) * (x - x2) < 0 &&
          (x3 - x4) * (y - y3) - (y3 - y4) * (x - x3) < 0 &&
          (x4 - x1) * (y - y4) - (y4 - y1) * (x - x4) < 0 
          ) {
        setColorPixel(pixels, x, y, color);
      }
    }
  }
}


static void PaintCubeInFloat ( unsigned int* pixels, float w, float h, int pitch, float trans_x, float trans_z, float proy, float rot) {
  int i;

  Point* points[8];
  //Indices for drawing lines to connect edges
  int indices[24] = { 0,1, 1,2, 2,3, 3,0,
                      4,5, 5,6, 6,7, 4,7,
                      0,4, 1,5, 2,6, 3,7};

  //Transform and paint all vertices in square
  for ( i=0; i<8; i++) {
    float xp, yp;
    float x = cubef [ i * 3 + 0];
    float y = cubef [ i * 3 + 1];
    float z = cubef [ i * 3 + 2];

    //Construct point
    Point* p = makePoint(x, y, z);

    //Rotate in all axis
    *p = rotatePoint(*p, rot*1.5f, 0);
    *p = rotatePoint(*p, rot*2.0f, 1);
    *p = rotatePoint(*p, rot, 2);

    //project Z axis on 2 dimensions
    p->z -= trans_z;
    xp = (p->x * proy) / p->z;
    yp = (p->y * proy) / p->z;

    //translate points to the middle of the screen
    xp += w * 0.5f;
    yp += h * 0.5f;

    //Save point
    points[i] = makePoint(xp,yp,p->z);
  }

  //DRAW LINE PRIMITIVES
  for (i=0; i<24; i+=2) {
    Point* start = points[indices[i]];
    Point* end = points[indices[i+1]];
    drawLine(pixels, pitch, *start, *end, 0xffffff);
  }


  
  //RASTERIZE POLYGONS
//  for (i=0; i<2; i+=4) {
    Point* sqp[4] = { points[0], points[1], points[2], points[3] };
    rasterize(pixels, sqp, pitch, 0xFF0000);

    Point* sqp_back[4] = { points[7], points[6], points[5], points[4] };
    rasterize(pixels, sqp_back, pitch, 0x00FF00);

    Point* sqp_bottom[4] = { points[0], points[4], points[5], points[1] };
    rasterize(pixels, sqp_bottom, pitch, 0x0000FF);

    Point* sqp_top[4] = { points[2], points[6], points[7], points[3] };
    rasterize(pixels, sqp_top, pitch, 0x00FFFF);



    Point* sqp_left[4] = { points[1], points[5], points[6], points[2] };
    rasterize(pixels, sqp_left, pitch, 0xFF00FF);

    Point* sqp_right[4] = { points[3], points[7], points[4], points[0] };
    rasterize(pixels, sqp_right, pitch, 0xFFFF00);
//  }

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

  float ang = 0;

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


    
    if (g_keydown == 1)
      ang += 0.05f;

    frame_SDL();

    input_SDL();

    SDL_Delay(40.0f);
  }

  return 0;
}
