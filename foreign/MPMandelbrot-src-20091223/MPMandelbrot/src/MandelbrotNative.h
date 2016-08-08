// Native Mandelbrot engine, trivial implementation
// (c) EB Nov 2009

#ifndef MandelbrotNative_h
#define MandelbrotNative_h

#include "Mandelbrot.h"

class MandelbrotNative : public Mandelbrot
{
public:

  // Constructor
  // scalarType is the type to use, one of Mandelbrot::ScalarType.
  MandelbrotNative(int scalarType,int nThreads) : mScalarType(scalarType), mNThreads(nThreads) { }

  // Base class implementation
  bool compute(RenderingData * data);

private:

  // Scalar type
  int mScalarType;
  // Number of threads
  int mNThreads;
};

#endif // #ifndef MandelbrotNative_h
