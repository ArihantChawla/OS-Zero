// Mandelbrot engine base class
// (c) EB Nov 2009

#ifndef Mandelbrot_h
#define Mandelbrot_h

#include <stdlib.h>
#include "FPReal.h"

class Mandelbrot
{
public:

  enum Constants
  {
    NB_COLORMAPS = 3,
    COLORMAP_SIZE = 2048
  };

  enum TaskLevel
  {
    NO_TASK = 0,
    LOW_RESOLUTION_TASK,
    HIGH_RESOLUTION_TASK,
    BENCHMARK_TASK
  };

  enum ScalarType
  {
    FLOAT_TYPE = 0,
    DOUBLE_TYPE,
    FP128_TYPE
  };

  // Precision of coordinates for exploration (interface, display)
  static const int NWords = 8;

  // Block size.
  // Image sizes are rounded up to multiples of 4*BlockSize for full, or BlockSize for fast.
  static const int BlockSize = 32; // pix

  struct RenderingData
  {
    // Dimensions of image
    int width,height;
    // Point (X,Y) is A[X+Y*WIDTH]
    int * a;
    // Coordinates of top-left corner in the complex plane
    FPReal<NWords> leftX,topY;
    // Increment for one pixel
    FPReal<NWords> stepX,stepY;
    // Max iterations
    int maxIt;
    // Scale
    int scale;
    // Additional attributes managed by engine or caller
    double runningTime;
    // Task level, one of TaskLevel
    int taskLevel;
    // Task ID, for client use, not modified by the engine
    int taskID;

    // Constructor, allocates A
    RenderingData(int w,int h)
    {
      width = w;
      height = h;
      a = (int *)malloc(w*h*sizeof(a[0]));
      if (a == 0)
      {
	printf("HELP! A allocation failed!\n");
      }
    }

    // Destructor, releases A
    ~RenderingData()
    {
      if (a != 0) free(a);
    }
  };

  // Constructor
  Mandelbrot() { }

  // Destructor
  virtual ~Mandelbrot() { }

  // Create a new instance of rendering data to compute the given region
  static RenderingData * createRenderingData(int width,int height,
    const FPReal<NWords> & centerX,const FPReal<NWords> & centerY,const FPReal<NWords> & step,
    int maxIt,int scale,int taskLevel);

  // Check if the engine is OK
  virtual bool isOK() { return true; }

  // Compute the given DATA.
  // Return TRUE if OK, and FALSE otherwise.
  virtual bool compute(RenderingData * data) = 0;

  // Return real time with as much precision as we can
  static double realTime();

  // Lookup colormap
  static unsigned int getColor(int it,int maxIt);

  // raw table
  static unsigned int getColor(int i) { return mColormap[i]; }

  // Initialize colormap. Must be called at least once in the process.
  static void initColormap(int c);

  // Get colormap name
  static const char * getColormapName(int c);

private:

  static unsigned int mColormap[];
  static const char * mColormapNames[];

};

#endif // #ifndef Mandelbrot_h
