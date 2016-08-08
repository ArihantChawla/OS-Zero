// Mandelbrot engine base class
// (c) EB Nov 2009

#include "Mandelbrot.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

double Mandelbrot::realTime()
{
  LARGE_INTEGER freq,value;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&value);
  return (double)value.QuadPart/(double)freq.QuadPart;
}
#endif

#ifdef Linux
#include <sys/time.h>

double Mandelbrot::realTime()
{
  struct timeval tv;
  gettimeofday(&tv,0);
  return (double)tv.tv_sec + 1.0e-6*(double)tv.tv_usec;
}
#endif

// HSV to RGB.  H in 0..359, S and V in 0..255
unsigned int hsv(double h,double s,double v)
{
  h /= 120.0;
  s /= 255.0;
  v /= 255.0;
  double r,g,b,i,p,q,t,f;
  int rr,gg,bb;
  if (s<=0.0001)
    r=g=b=v;
  else
    {
      h=2.0*h;
      i=floor(h);
      f=h-i;
      p=v*(1.0-s);
      q=v*(1.0-s*f);
      t=v*(1-s*(1.0-f));
      if (i==0.0) { r=v;g=t;b=p; }
      else if (i==1.0) { r=q;g=v;b=p; }
      else if (i==2.0) { r=p;g=v;b=t; }
      else if (i==3.0) { r=p;g=q;b=v; }
      else if (i==4.0) { r=t;g=p;b=v; }
      else if (i==5.0) { r=v;g=p;b=q; }
      else { r=g=b=0; }
    }
  rr=(int)(r*256.0);if (rr>255) rr=255;
  gg=(int)(g*256.0);if (gg>255) gg=255;
  bb=(int)(b*256.0);if (bb>255) bb=255;
  return ((rr<<16) | (gg<<8) << bb) | 0xFF000000;
}

const char * Mandelbrot::mColormapNames[NB_COLORMAPS] = { "All Colors","Blue-Red-Yellow","Grayscale" };
unsigned int Mandelbrot::mColormap[COLORMAP_SIZE];

Mandelbrot::RenderingData * Mandelbrot::createRenderingData(int width,int height,
							    const FPReal<NWords> & centerX,const FPReal<NWords> & centerY,const FPReal<NWords> & step,
							    int maxIt,int scale,int taskLevel)
{
  RenderingData * data = new RenderingData(width,height);
  FPReal<NWords> aux;

  // X
  data->leftX = centerX;
  aux = step;
  aux.mul(-width/2);
  data->leftX.add(aux);
  data->stepX = step;

  // Y
  data->topY = centerY;
  aux = step;
  aux.mul(height/2);
  data->topY.add(aux);
  data->stepY = step;

  // Iterations
  data->maxIt = maxIt;
  // Scale
  data->scale = scale;
  // Level
  data->taskLevel = taskLevel;
  // Running time
  data->runningTime = -1; // will be set by engine

  return data;
}

unsigned int Mandelbrot::getColor(int it,int maxIt)
{
  if (it<0) return 0xFFFFFFFF;
  if (it>=maxIt) return 0xFF000000;

  int index = (it*COLORMAP_SIZE)/maxIt;
  if (index < 0) index = 0;
  else if (index >= COLORMAP_SIZE) index = COLORMAP_SIZE-1;

  return mColormap[index];
}

void Mandelbrot::initColormap(int c)
{
  int n = Mandelbrot::COLORMAP_SIZE;
  for (int i=0;i<n;i++)
  {
    int h,s,v;
    switch (c)
    {
    default:
    case 0:
      h = (360*i)/n;
      s = 255;
      v = 255;
      break;
    case 1:
      h = 240 + (180*i)/n; // blue-yellow
      s = 255;
      v = 155+(100*i)/n;
      break;
    case 2:
      h = 0;
      s = 0;
      v = (255*i)/n;
      break;
    }
    mColormap[i] = hsv(h%360,s,v);
  }
}

const char * Mandelbrot::getColormapName(int c)
{
  if (c < 0 || c >= NB_COLORMAPS) return "Invalid";
  return mColormapNames[c];
}
