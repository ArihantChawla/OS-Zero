// Multiprecision Mandelbrot, fixed-point 128 bits
// (c) EB Dec 2009

// Requires fp128.cl

__kernel void compute_fp128(__global uint * a,
			    __constant uint * colormap,
			    __constant uint * coords,
			    int nx,int ny,
			    int offset,int lda,
			    int leftXSign,int topYSign,
			    int maxIt)
{
  // Convert inputs
  uint4 leftX = vload4(0,coords);
  uint4 topY  = vload4(1,coords);
  uint4 stepX = vload4(2,coords);
  uint4 stepY = vload4(3,coords);
  if (leftXSign < 0) leftX = neg128(leftX);
  if (topYSign < 0) topY = neg128(topY);

  for (int iy=0;iy<ny;iy++) for (int ix=0;ix<nx;ix++)
  {
    int xpix = get_global_id(0)*nx + ix;
    int ypix = get_global_id(1)*ny + iy;
    uint4 xc = add128(leftX,mul128(stepX,xpix)); // xc = leftX + xpix * stepX;
    uint4 yc = add128(topY,neg128(mul128(stepY,ypix))); // yc = topY - ypix * stepY;

    int it = 0;
    uint4 x = set128(0);
    uint4 y = set128(0);
    for (it=0;it<maxIt;it++)
    {
      uint4 x2 = sqrfp(x); // x2 = x^2
      uint4 y2 = sqrfp(y); // y2 = y^2
      uint4 aux = add128(x2,y2); // x^2+y^2
      if (aux.x >= 4) break; // Out!
      uint4 twoxy = shl128(mulfp(x,y)); // 2*x*y
      x = add128(xc,add128(x2,neg128(y2))); // x' = xc+x^2-y^2
      y = add128(yc,twoxy); // y' = yc+2*x*y
    }
    uint index = (COLORMAP_SIZE*it)/maxIt;
    uint color = (index<COLORMAP_SIZE)?colormap[index]:0xFF000000;
    a[offset+xpix+lda*ypix] = color;
  }
}
