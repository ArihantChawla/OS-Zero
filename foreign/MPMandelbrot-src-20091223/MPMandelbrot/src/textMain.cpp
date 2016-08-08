// Mandelbrot command line
// (c) EB Dec 2009

#include <stdlib.h>
#include <png.h>
#include "MandelbrotOpenCL.h"
#include "SpecialSites.h"

typedef FPReal<Mandelbrot::NWords> fp_real_t;

void usage()
{
  printf("Command line options:\n");
  printf("\n  -float     select scalar type 'hardware float'\n");
  printf("  -double    select scalar type 'hardware double'\n");
  printf("  -fp128     select scalar type 'software fixed-point 128 bits'\n");
  printf("\n  <int>      select site to render:\n");
  for (int i=0;i<NB_SPECIAL_SITES;i++)
    printf("      %d        %s\n",i,getSpecialSiteName(i));
  printf("\n  -save      save the result image\n");
  printf("\n  -h         display this\n");
  exit(1);
}

int main(int argc,char ** argv)
{
  int site = 0;
  int scalarType = Mandelbrot::FP128_TYPE;
  bool saveImage = false;
  // Parse options
  for (int i=1;i<argc;i++)
    {
      const char * s = argv[i];
      if (isdigit(s[0]))
	{
	  site = atoi(s);
	  if (site < 0 || site >= NB_SPECIAL_SITES) site = 0;
	  continue;
	}
      if (strcmp(s,"-float") == 0) { scalarType = Mandelbrot::FLOAT_TYPE; continue; }
      if (strcmp(s,"-double") == 0) { scalarType = Mandelbrot::DOUBLE_TYPE; continue; }
      if (strcmp(s,"-fp128") == 0) { scalarType = Mandelbrot::FP128_TYPE; continue; }
      if (strcmp(s,"-save") == 0) { saveImage = true; continue; }
      usage();
    }

  Mandelbrot::initColormap(0);
  MandelbrotOpenCL * M = new MandelbrotOpenCL(CL_DEVICE_TYPE_GPU,scalarType,"bin");

  fp_real_t centerX(SpecialSites[site].x);
  fp_real_t centerY(SpecialSites[site].y);
  fp_real_t step(SpecialSites[site].step);
  int maxIt = SpecialSites[site].maxIt;
  int size = 2048;

  printf("Mandelbrot Set OpenCL benchmark\n");
  printf("Size: %dx%d\n",size,size);
  printf("Site: %s\n",SpecialSites[site].name);
  printf("MaxIt: %d\n",maxIt);
  switch (scalarType)
    {
    case Mandelbrot::FLOAT_TYPE: printf("Type: float\n"); break;
    case Mandelbrot::DOUBLE_TYPE: printf("Type: double\n"); break;
    case Mandelbrot::FP128_TYPE: printf("Type: fp128\n"); break;
    }

  Mandelbrot::RenderingData * data = Mandelbrot::createRenderingData(size,size,centerX,centerY,step,maxIt,1,Mandelbrot::BENCHMARK_TASK);
  double t = Mandelbrot::realTime();
  M->compute(data);
  t = Mandelbrot::realTime() - t;

  printf("Time: %f ms\n",t*1.0e3);

  // Save the image if requested
  if (saveImage)
    {
      std::string filename;
      filename.append(SpecialSites[site].name);
      switch (scalarType)
	{
	case Mandelbrot::FLOAT_TYPE: filename.append(" - float"); break;
	case Mandelbrot::DOUBLE_TYPE: filename.append(" - double"); break;
	case Mandelbrot::FP128_TYPE: filename.append(" - fp128"); break;
	}
      filename.append(".png");
      png_structp png = 0;
      png_infop info = 0;
      FILE * f = 0;
      std::vector< unsigned char * > rows(size,(unsigned char *)0);
      for (int y=0;y<size;y++) rows[y] = (unsigned char *)(data->a+y*size);

      f = fopen(filename.c_str(),"wb");
      if (f == 0) goto END;
      png = png_create_write_struct(PNG_LIBPNG_VER_STRING,0,0,0);
      if (png == 0) goto END;
      info = png_create_info_struct(png);
      if (info == 0) goto END;

      png_init_io(png,f);
      png_set_compression_level(png,Z_BEST_COMPRESSION);
      png_set_IHDR(png,info,size,size,8,
		   PNG_COLOR_TYPE_RGBA,
		   PNG_INTERLACE_NONE,
		   PNG_COMPRESSION_TYPE_DEFAULT,
		   PNG_FILTER_TYPE_DEFAULT);
      png_set_rows(png,info,&(rows[0]));
      png_write_png(png,info,PNG_TRANSFORM_BGR,0);

      printf("Image saved to: %s\n",filename.c_str());

    END:
      png_destroy_write_struct(&png,&info);
      if (f != 0) fclose(f);
    }

  delete data;
  delete M;
}
