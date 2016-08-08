// Thread for Mandelbrot computation on the CPU
// (c) EB Dec 2009

#ifndef MandelbrotNativeThread_h
#define MandelbrotNativeThread_h

#include <vector>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include "Mandelbrot.h"

// N is the number of words in fixed point reals to use, or 0 for double.
class MandelbrotNativeThread : public QThread
{
  Q_OBJECT

public:

  // DATA describes the global image to compute.
  // BLOCKS is a list of block coordinates (16,16) bits packed in an uint32: (x<<16|y).
  // BLOCKMUTEX is a mutex used to lock access to the block list, shared by all threads.
  MandelbrotNativeThread(Mandelbrot::RenderingData * data,std::vector<unsigned int> & blocks,QMutex & blockMutex,int scalarType) :
    mData(data), mBlocks(blocks), mMutex(blockMutex), mScalarType(scalarType)
  {
    start();
  }

  // Base class function
  void run();

private:

  // Get a new block to process from mBlocks.
  // Return TRUE if block found, FALSE otherwise.
  bool getBlock(unsigned int & block)
  {
    bool found = true;
    mMutex.lock();
    if (mBlocks.empty()) found = false;
    else { block = mBlocks.back(); mBlocks.pop_back(); }
    mMutex.unlock();
    return found;
  }

  Mandelbrot::RenderingData * mData;
  std::vector<unsigned int> & mBlocks;
  QMutex & mMutex;
  int mScalarType;

  template <typename real_t> friend void runHW(MandelbrotNativeThread * mt);
  template <int N> friend void runFP(MandelbrotNativeThread * mt);

};

#endif // #ifndef MandelbrotNativeThread_h
