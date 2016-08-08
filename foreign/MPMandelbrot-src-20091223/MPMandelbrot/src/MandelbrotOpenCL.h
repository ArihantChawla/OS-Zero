// OpenCL Mandelbrot engine
// (c) EB Nov 2009

#ifndef MandelbrotOpenCL_h
#define MandelbrotOpenCL_h

#include <BealtoOpenCL.h>
#include "Mandelbrot.h"

class MandelbrotOpenCL : public Mandelbrot
{
public:

  // deviceType is the CL_DEVICE_TYPE_XXX to use.
  // scalarType is the type to use in OpenCL kernels, one of Mandelbrot::ScalarType.
  // srcDir is the directory containing the OpenCL code.
  MandelbrotOpenCL(cl_device_type deviceType,int scalarType,const char * srcDir);
  ~MandelbrotOpenCL();

  // Base class implementation
  bool compute(RenderingData * data);
  bool isOK()
  {
    return (mContext != 0 && mProgram != 0 && mQueue != 0 && mProgramOK);
  }

private:

  cl::Context * mContext;
  cl::Program * mProgram;
  cl::CommandQueue * mQueue;
  cl::Buffer * mImageBuffer;
  cl::Buffer * mColormapBuffer;
  cl::Buffer * mCoordBuffer;
  std::string mSrcDir;
  bool mProgramOK;
  int mScalarType;
};

#endif // #ifndef MandelbrotOpenCL_h
