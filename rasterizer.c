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
} Point;

typedef struct {
  float x,y,z;
} Pointf;


//Point "constructor"
static Point makePoint(int x, int y, int z) {
  Point p;
  p.x = x;
  p.y = y;
  p.z = z;
  return p;
}


// Floats
static float cubef [12] = {
    -LCF,-LCF, LCF,
     LCF,-LCF, LCF,
     LCF, LCF, LCF,
    -LCF, LCF, LCF,
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


  return trans;
}



/// Hides sides that are hidden behind others
static void doCull(unsigned int* pixels) {
  //There will only be a maximum of 3 sides showing simultaneously

}



static void rasterize(unsigned int* pixels, Point* p, int pitch) {
  int i;

  Point top = p[0];
  Point bottom = p[0];
  Point left = p[0];
  Point right = p[0];

  int repeated = 0;
  int j;

  // Calculate top, bottom, left, right
  for(i=0; i<4;i++) {
    // Obtain actual point
    Point actual = {p[i].x, p[i].y, p[i].z};
    if(actual.y > top.y) { top = actual; }
    if(actual.y < bottom.y) { bottom = actual; }
    if(actual.x > right.x) { right = actual; }
    if(actual.x < left.x) { left = actual; }

    for(j=0; j<4 && repeated<2; j++) {
      if(p[i].x == p[j].x && i != j) {
        repeated++;
      }
    }
  }

#if DEBUG
  // Show top, bottom, right, left after calculate - DEBUG
  printf("Top: %i, %i\n", top.x, top.y);
  printf("Bottom: %i, %i\n", bottom.x, bottom.y);
  printf("Right: %i, %i\n", right.x, right.y);
  printf("Left: %i, %i\n", left.x, left.y);
#endif


  if(repeated < 2) {
    Pointf start;
    Pointf end;

    // Calculate dx
    float dx = 0.0f;

    dx = (right.x - left.x) / (top.y - bottom.y);
    for(i=0; i<top.y - bottom.y; ++i) {
      // Calculate y
      start.y = top.y - i;
      end.y = top.y - i;

      // Calculate x
      start.x = left.x + top.x - dx;
      end.x = right.x - top.x + dx;
      //printf("%f, %f\n", start.x, end.x);
      drawLineF(pixels, pitch, start, end, 0xff0000);
    }
  } else {
    Point start;
    Point end;
    for(i=0; i<top.y-bottom.y; i++) {
      start.y = i;
      start.x = left.x;
      end.y = i;
      end.x = right.x;
//      printf("Start: %i,%i - End: %i,%i\n", start.x, start.y, end.x, end.y);
      drawLine(pixels, pitch, start, end, 0xff0000);
    }
  }
}


static void rasterizeChuster(unsigned int* pixels, Point* p, int pitch) {

  Point top = p[0];
  Point bottom = p[0];
  Point left = p[0];
  Point right = p[0];
  int i,j;
  float diff_x;

  // Calculate top, bottom, left, right
  for(i=0; i<4;i++) {
    // Obtain actual point
    Point actual = {p[i].x, p[i].y, p[i].z};
    if(actual.y < top.y) { top = actual; }
    if(actual.y > bottom.y) { bottom = actual; }
    if(actual.x > right.x) { right = actual; }
    if(actual.x < left.x) { left = actual; }
  }

  //Indexes are screen Ys, values are X line points
  //2 arrays indicating left and right limits of the polygon
  int xs_left[1024];
  int xs_right[1024];
  memset(xs_left, 0, 1024);
  memset(xs_right, 0, 1024);

  //TOP-LEFT EDGE
  diff_x = (float)(top.x - left.x) / (float)(left.y - top.y);
  for (i=0; i< left.y - top.y; i++) {
    xs_left[top.y + i] = top.x - (int)(diff_x * i);
  }

  //LEFT-BOTTOM EDGE
  diff_x = (float)(bottom.x - left.x) / (float)(bottom.y - left.y);
  for (i=0; i< bottom.y - left.y; i++) {
    xs_left[left.y + i] = left.x + (int)(diff_x * i);
  }

  //TOP-RIGHT EDGE
  diff_x = (float)(right.x - top.x) / (float)(right.y - top.y);
  for (i=0; i< right.y - top.y; i++) {
    xs_right[top.y + i] = top.x + (int)(diff_x * i);
  }

  //RIGHT-BOTTOM EDGE
  diff_x = (float)(right.x - bottom.x) / (float)(bottom.y - right.y);
  for (i=0; i< bottom.y - right.y; i++) {
    xs_right[right.y + i] = right.x - (int)(diff_x * i);
  }

  for (i=0; i<400; i++) {
    if (xs_left[i] > 0 && xs_left[i] < 800) {
      for (j=0; j<xs_right[i]-xs_left[i]; j++) {
        //printf("%d\n", xs_left[i]+j);
        setColorPixel(pixels, xs_left[i]+j, i, 0xFF0000);
      }
    }
  }
}


static void PaintCubeInFloat ( unsigned int* pixels, float w, float h, int pitch, float trans_x, float trans_z, float proy, float rot) {
  int i;

  Point points[4];
  //Indices for drawing lines to connect edges
  int indices[8] = { 0,1, 1,2, 2,3, 3,0};

  //Transform and paint all vertices in square
  for ( i=0; i<4; i++) {
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
    points[i] = makePoint(xp,yp,p.z);

    /*if (( xp >= 0) && (xp < w) && (yp >= 0) && (yp < h)) {
        int color = getColorByIndex(i);
        setColorPixel(pixels, (int)xp, (int)yp, color);
    }*/
  }

  //DRAW LINE PRIMITIVES
  for (i=0; i<8; i+=2) {
    Point start = points[indices[i]];
    Point end = points[indices[i+1]];
    drawLine(pixels, pitch, start, end, 0xffffff);
  }

  //CULL
  doCull(pixels);

  //RASTERIZE POLYGONS
  rasterizeChuster(pixels, points, pitch);
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

  float ang = 0.3f;

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

