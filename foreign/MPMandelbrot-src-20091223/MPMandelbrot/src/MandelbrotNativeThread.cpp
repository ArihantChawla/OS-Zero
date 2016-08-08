// Thread for Mandelbrot computation on the CPU
// (c) EB Dec 2009

#include "MandelbrotNativeThread.h"

template <typename real_t> void runHW(MandelbrotNativeThread * mt)
{
  Mandelbrot::RenderingData * data = mt->mData;
  int * a = data->a;
  real_t ox,oy,cstepx,cstepy;
  ox = (real_t)(double)(data->leftX);
  oy = (real_t)(double)(data->topY);
  cstepx = (real_t)(double)(data->stepX);
  cstepy = (real_t)(double)(data->stepY);

  while (1)
  {
    // Get a new block to process
    unsigned int block = 0;
    if (!mt->getBlock(block)) break;

    // Process one block

    // Coordinates of top-left pixel of block
    int bx = (block & 0xFFFF) * Mandelbrot::BlockSize;
    int by = ((block>>16) & 0xFFFF) * Mandelbrot::BlockSize;

    // Process
    for (int iy=0;iy<Mandelbrot::BlockSize;iy++) for (int ix=0;ix<Mandelbrot::BlockSize;ix++)
    {
      int xpix = ix+bx;
      int ypix = iy+by;
      real_t cx = ox + xpix*cstepx;
      real_t cy = oy - ypix*cstepy;
      real_t x = 0;
      real_t y = 0;
      int it;
      for (it=0;it<data->maxIt;it++)
      {
	real_t x2 = x*x;
	real_t y2 = y*y;
	if (x2 + y2 > 4.0) break; // Out of disc
	real_t xy = x*y;
	x = x2 - y2 + cx;
	y = 2.0*xy + cy;
      }
      unsigned int color = Mandelbrot::getColor(it,data->maxIt);
      a[xpix+ypix*data->width] = color;
    }
  }
}

template <int N> void runFP(MandelbrotNativeThread * mt)
{
  Mandelbrot::RenderingData * data = mt->mData;
  int * a = data->a;
  // Convert inputs to N words
  FPReal<N> ox,oy,cstepx,cstepy;
  ox.set(data->leftX);
  oy.set(data->topY);
  cstepx.set(data->stepX);
  cstepy.set(data->stepY);

  while (1)
  {
    // Get a new block to process
    unsigned int block = 0;
    if (!mt->getBlock(block)) break;

    // Process one block

    // Coordinates of top-left pixel of block
    int bx = (block & 0xFFFF) * Mandelbrot::BlockSize;
    int by = ((block>>16) & 0xFFFF) * Mandelbrot::BlockSize;

    // Process
    for (int iy=0;iy<Mandelbrot::BlockSize;iy++) for (int ix=0;ix<Mandelbrot::BlockSize;ix++)
    {
      int xpix = ix+bx;
      int ypix = iy+by;
      FPReal<N> cx,cy,aux,x,y,x2,y2,twoxy;
      cx.set(ox);
      aux.set(cstepx); aux.mul(xpix);
      cx.add(aux);
      cy.set(oy);
      aux.set(cstepy); aux.mul(-ypix);
      cy.add(aux);
      x.zero();
      y.zero();
      int it;
      for (it=0;it<data->maxIt;it++)
      {
	mul(x2,x,x);
	mul(y2,y,y);
	aux.set(x2); aux.add(y2);
	if (aux.intPart() >= 4) break; // Out of disc
	mul(twoxy,x,y);
	twoxy.mul2k(1);
	x.set(cx); x.add(x2); x.sub(y2);
	y.set(cy); y.add(twoxy);
      }
      unsigned int color = Mandelbrot::getColor(it,data->maxIt);
      a[xpix+ypix*data->width] = color;
    }
  }
}

void MandelbrotNativeThread::run()
{
  switch (mScalarType)
    {
    case Mandelbrot::FLOAT_TYPE: runHW<float>(this); break;
    case Mandelbrot::DOUBLE_TYPE: runHW<double>(this); break;
    case Mandelbrot::FP128_TYPE: runFP<4>(this); break;
    }
}
