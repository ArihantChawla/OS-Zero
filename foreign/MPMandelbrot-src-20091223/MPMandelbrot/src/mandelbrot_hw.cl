// Multiprecision Mandelbrot, hardware float or double
// (c) EB Dec 2009

// Kernels compute NX*NY pixels of image A with coordinates
//
// Case 1:
// Xpix = global_id(0)+IX*global_size(0)
// Ypix = global_id(1)+IY*global_size(1)
//
// Case 2:
// Xpix = global_id(0)*NX+IX
// Ypix = global_id(1)*NY+IY
//
// The result is stored in
// A[Offset + Xpix+LDA*Ypix]
// and it corresponds to the real plane point
// Xc = leftX+Xpix*stepX
// Yc = topY-Ypix*stepY
//
// COLORMAP[maxIt] is the color to put in A.
// Iterations reaching the max receive color 0xFF000000 (black with alpha).

#if USE_DOUBLE
// Ref. http://www.khronos.org/opencl/sdk/1.0/docs/man/xhtml/cl_khr_fp64.html
#pragma OPENCL EXTENSION cl_khr_fp64 : enable
typedef double real_t;
#else
typedef float real_t;
#endif

// hardware 'float' or 'double' version
__kernel void compute_hw(__global uint * a,
			 __constant uint * colormap,
			 int nx,int ny,
			 int offset,int lda,
			 real_t leftX,real_t topY,
			 real_t stepX,real_t stepY,
			 int maxIt)
{
  for (int iy=0;iy<ny;iy++) for (int ix=0;ix<nx;ix++)
  {
    int xpix = get_global_id(0)*nx + ix;
    int ypix = get_global_id(1)*ny + iy;

    real_t xc = leftX + (real_t)xpix * stepX;
    real_t yc = topY  - (real_t)ypix * stepY;
    int it;
    real_t x,y;
    x = y = (real_t)0;
    for (it=0;it<maxIt;it++)
    {
      real_t x2 = x*x;
      real_t y2 = y*y;
      if (x2+y2 > (real_t)4) break; // Out!
      real_t twoxy = (real_t)2*x*y;
      x = x2 - y2 + xc;
      y = twoxy + yc;
    }
    uint index = (COLORMAP_SIZE*it)/maxIt;
    uint color = (index<COLORMAP_SIZE)?colormap[index]:0xFF000000;
    a[offset+xpix+lda*ypix] = color;
  }
}
