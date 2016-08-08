// Native Mandelbrot engine, trivial implementation
// (c) EB Nov 2009

#include <stdio.h>
#include "MandelbrotNative.h"
#include "MandelbrotNativeThread.h"
#include "FPReal.h"

bool MandelbrotNative::compute(RenderingData * data)
{
  if (data == 0) return false; // Invalid

  // Generate the list of blocks
  std::vector<unsigned int> blocks;
  QMutex mutex;

  int bw = data->width / Mandelbrot::BlockSize;
  int bh = data->height / Mandelbrot::BlockSize;
  blocks.reserve(bw*bh);
  for (int x=0;x<bw;x++) for (int y=0;y<bh;y++) blocks.push_back( (unsigned int)x | ((unsigned int)y<<16) );

  // Create the threads (they start working immediately)
  std::vector<MandelbrotNativeThread *> threads(mNThreads,(MandelbrotNativeThread *)0);
  for (int i=0;i<mNThreads;i++)
  {
    threads[i] = new MandelbrotNativeThread(data,blocks,mutex,mScalarType);
  }
  // Wait for all threads to terminate
  for (int i=0;i<mNThreads;i++)
  {
    threads[i]->wait();
    delete threads[i];
  }

  return true; // OK
}
