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

  trans.used = 0;

  return trans;
}



static void rasterize(unsigned int* pixels, Point** p, int pitch) {


//PROPUESTA DE TRAMEO: DESPLAZAR VÉRTICES QUE REPITEN COORDENADA UN PIXEL PARA QUE NO SEAN IGUALES

  Point* temp_top = makePoint(0, 639, 0);
  Point* temp_bottom = makePoint(0, 0, 0);
  Point* temp_left = makePoint(479, 0, 0);
  Point* temp_right = makePoint(0, 0, 0);

  Point* top = temp_top;
  Point* bottom = temp_bottom;
  Point* left = temp_left;
  Point* right = temp_right;

  int i,j;
  float diff_x;

  // Calculate top, bottom, left, right
  for(i=0; i<4;i++) {
    // Obtain actual point
    Point* actual = p[i];

    if(actual->y < top->y) { top = actual; actual->used++; }
    if(actual->y > bottom->y) { bottom = actual; actual->used++; }
    if(actual->x > right->x) { right = actual; actual->used++; }
    if(actual->x < left->x) { left = actual; actual->used++; }

  }

  //free (temp_top);
  //free (temp_bottom);
  //free (temp_left);
  //free (temp_right);

  int usages[4] = {0,0,0,0};

  int k,repeated=0;
  for(k=0; k<4; k++) {
    if (p[k] == top) usages[k]++;
    if (p[k] == bottom) usages[k]++;
    if (p[k] == right) usages[k]++;
    if (p[k] == left) usages[k]++;
  }
  printf("USAGES : %d, %d, %d, %d\n", usages[0], usages[1], usages[2], usages[3]);


    if (usages[0] == 2 && usages[3]==0)  
      right = p[3];

    if (usages[1] == 0 && usages[2]==2)  
      left = p[1];

    if (usages[2] == 0 && usages[3]==2)  
      top = p[2];
  

  if (repeated < 4) {
    //Indexes are screen Ys, values are X line points
    //2 arrays indicating left and right limits of the polygon
    int xs_left[1024];
    int xs_right[1024];
    memset(xs_left, 0, 1024);
    memset(xs_right, 0, 1024);

    //TOP-LEFT EDGE
    diff_x = (float)(top->x - left->x) / (float)(left->y - top->y);
    for (i=0; i< left->y - top->y; i++) {
      xs_left[top->y + i] = top->x - (int)(diff_x * i);
    }

    //LEFT-BOTTOM EDGE
    diff_x = (float)(bottom->x - left->x) / (float)(bottom->y - left->y);
    for (i=0; i< bottom->y - left->y; i++) {
      xs_left[left->y + i] = left->x + (int)(diff_x * i);
    }

    //TOP-RIGHT EDGE
    diff_x = (float)(right->x - top->x) / (float)(right->y - top->y);
    for (i=0; i< right->y - top->y; i++) {
      xs_right[top->y + i] = top->x + (int)(diff_x * i);
    }

    //RIGHT-BOTTOM EDGE
    diff_x = (float)(right->x - bottom->x) / (float)(bottom->y - right->y);
    for (i=0; i< bottom->y - right->y; i++) {
      xs_right[right->y + i] = right->x - (int)(diff_x * i);
    }

    for (i=0; i<480; i++) {
      if (xs_left[i] > 0 && xs_left[i] < 640) {
        for (j=0; j<xs_right[i]-xs_left[i]; j++) {
          //printf("%d\n", xs_left[i]+j);
          setColorPixel(pixels, xs_left[i]+j, i, 0xFF0000);
        }
      }
    }
  } else {
    Point start;
    Point end;
    for(i=0; i< abs(top->y - bottom->y); i++) {
      start.x = left->x;
      start.y = top->y;
      end.x = right->x;
      end.y = bottom->y;
//      printf("Start: %i,%i - End: %i,%i\n", start.x, start.y, end.x, end.y);
      //drawLine(pixels, pitch, start, end, 0xff0000);

      for (j=0; j<abs(right->x - left->x); j++) {
        setColorPixel(pixels, left->x+j, start.y+i, 0xFF0000);
      }

    }
  }
}


static void PaintCubeInFloat ( unsigned int* pixels, float w, float h, int pitch, float trans_x, float trans_z, float proy, float rot) {
  int i;

  Point* points[4];
  //Indices for drawing lines to connect edges
  int indices[8] = { 0,1, 1,2, 2,3, 3,0};

  //Transform and paint all vertices in square
  for ( i=0; i<4; i++) {
    float xp, yp;
    float x = cubef [ i * 3 + 0];
    float y = cubef [ i * 3 + 1];
    float z = cubef [ i * 3 + 2];

    //Construct point
    Point* p = makePoint(x, y, z);

    //Rotate in all axis
    *p = rotatePoint(*p, rot, 0);
    *p = rotatePoint(*p, rot, 1);
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

    /*if (( xp >= 0) && (xp < w) && (yp >= 0) && (yp < h)) {
        int color = getColorByIndex(i);
        setColorPixel(pixels, (int)xp, (int)yp, color);
    }*/
  }

  //DRAW LINE PRIMITIVES
  for (i=0; i<8; i+=2) {
    Point* start = points[indices[i]];
    Point* end = points[indices[i+1]];
    drawLine(pixels, pitch, *start, *end, 0xffffff);
  }

  //RASTERIZE POLYGONS
  rasterize(pixels, points, pitch);
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
      ang += 0.01f;

    frame_SDL();

    input_SDL();

    SDL_Delay(40.0f);
  }

  return 0;
}