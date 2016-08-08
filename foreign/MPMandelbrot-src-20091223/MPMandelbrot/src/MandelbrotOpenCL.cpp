// OpenCL Mandelbrot engine
// (c) EB Nov 2009

#include "MandelbrotOpenCL.h"

#ifdef Linux
#define _snprintf snprintf
#endif

inline void report(const char * msg)
{
  printf("%s\n",msg);
}

MandelbrotOpenCL::MandelbrotOpenCL(cl_device_type deviceType,int scalarType,const char * srcDir) : mSrcDir(srcDir), mScalarType(scalarType)
{
  // Setup OpenCL objects
  mContext = 0;
  mProgram = 0;
  mQueue = 0;
  mImageBuffer = 0;
  mColormapBuffer = 0;
  mCoordBuffer = 0;

  mContext = cl::Context::create(deviceType);
  if (mContext == 0)
  {
    report("Context failed");
    return;
  }
  // Use the application path to search OpenCL sources.
  std::vector<std::string> programFiles;
  std::string s;
  if (mScalarType == FP128_TYPE)
  {
    s.assign(mSrcDir); s.append("/fp128.cl"); programFiles.push_back(s);
    s.assign(mSrcDir); s.append("/mandelbrot_fp128.cl"); programFiles.push_back(s);
  }
  else
  {
    s.assign(mSrcDir); s.append("/mandelbrot_hw.cl"); programFiles.push_back(s);
  }
  mProgram = mContext->createProgramWithFiles(programFiles);
  if (mProgram == 0)
  {
    report("Program failed");
    return;
  }
  mQueue = mContext->createCommandQueue(0,0);
  if (mQueue == 0)
  {
    report("Queue failed");
    return;
  }
  std::string e,options("-cl-unsafe-math-optimizations -cl-finite-math-only -cl-mad-enable");
  int fpWords = 0;
  char aux[200];
  if (mScalarType == FP128_TYPE) fpWords = 4;
  if (mScalarType == DOUBLE_TYPE)
    options.append(" -DUSE_DOUBLE=1");
  else
    options.append(" -DUSE_DOUBLE=0");
  _snprintf(aux,200," -DCOLORMAP_SIZE=%d",Mandelbrot::COLORMAP_SIZE);
  options.append(aux);
  mProgramOK = mProgram->build(options.c_str(),e);
  if (!mProgramOK)
  {
    report("Build failed");
    report(e.c_str()); // Build failed
  }
}

MandelbrotOpenCL::~MandelbrotOpenCL()
{
  delete mImageBuffer;
  delete mColormapBuffer;
  delete mCoordBuffer;
  delete mQueue;
  delete mProgram;
  delete mContext;
}

bool MandelbrotOpenCL::compute(RenderingData * data)
{
  bool ok = true;
  if (data == 0) return false; // Invalid
  // qDebug() << "Entered GPU compute" << data->taskID;
  if (mContext == 0 || mProgram == 0 || mQueue == 0 || !mProgramOK) return false; // Can't do it!
  cl::Kernel * kernel = 0;
  int fpWords = 0;
  if (mScalarType == FP128_TYPE) fpWords = 4;

  // Addresses to map our buffers
  int * xBuf = 0;
  unsigned int * xColormap = 0;
  unsigned int * xCoords = 0;

  // Global size. Total number of threads is GLOBAL_SIZE*GLOBAL_SIZE. Each thread
  // computes a rectangle of NX*NY pixels.
  int global_size = 128;

  // Block size (to split the kernel execution into several calls to the OpenCL runtime).
  // Each kernel executes BLOCK_SIZE*BLOCK_SIZE threads.
  // Must divide global sizes, and be a multiple of workgroup_size.
  int block_size = global_size;
  // if (mScalarType == DOUBLE_TYPE) block_size = global_size / 4;
  if (mScalarType == FP128_TYPE) block_size = global_size / 4;

  // Workgroup size. Must divide block size.
  int workgroup_size = 8;

  // Number of pixels to compute in each kernel
  // Total computed image size is NX*global_size,NY*global_size
  // We may compute a few pixels more because of rounding to multiples of global_size and global_size
  int nx = (data->width / global_size); if (nx*global_size<data->width) nx++;
  int ny = (data->height / global_size); if (ny*global_size<data->height) ny++;

  int maxIt = data->maxIt;
  size_t bufSize = nx*global_size*ny*global_size * sizeof(int); // bytes
  size_t colormapSize = Mandelbrot::COLORMAP_SIZE * sizeof(int); // bytes
  size_t coordsSize = fpWords * 4 * sizeof(int); // bytes
  int argc;
  int lda = global_size*nx; // pixels on a line of the computed buffer

  switch (mScalarType)
  {
  case FLOAT_TYPE:
  case DOUBLE_TYPE:
    kernel = mProgram->createKernel("compute_hw");
    break;
  case FP128_TYPE:
    kernel = mProgram->createKernel("compute_fp128");
    break;
  default:
    kernel = 0;
    break;
  }
  if (kernel == 0)
  {
    report("create kernel failed\n");
    ok = false; goto END;
  }

  // Allocate buffers if needed
  if (mImageBuffer == 0 || mImageBuffer->getSize() < bufSize)
  {
    if (mImageBuffer != 0) delete mImageBuffer;
    mImageBuffer = mContext->createBuffer(CL_MEM_WRITE_ONLY,bufSize);
    if (mImageBuffer == 0) { ok = false; goto END; }
  }
  if (mColormapBuffer == 0 || mColormapBuffer->getSize() < colormapSize)
  {
    if (mColormapBuffer != 0) delete mColormapBuffer;
    mColormapBuffer = mContext->createBuffer(CL_MEM_READ_ONLY,colormapSize);
    if (mColormapBuffer == 0) { ok = false; goto END; }
  }
  if (coordsSize > 0 && (mCoordBuffer == 0 || mCoordBuffer->getSize() < coordsSize))
  {
    mCoordBuffer = mContext->createBuffer(CL_MEM_READ_ONLY,coordsSize);
    if (mCoordBuffer == 0) { ok = false; goto END; }
  }

  // Setup colormap
  if ( !mQueue->mapBuffer(mColormapBuffer,xColormap,CL_TRUE,CL_MAP_WRITE).isValid() )
  {
    report("map colormap failed");
    ok = false; goto END;
  }
  for (int i=0;i<Mandelbrot::COLORMAP_SIZE;i++) xColormap[i] = getColor(i);
  if (!mQueue->unmapMemoryObject(mColormapBuffer,xColormap).isValid())
  {
    report("unmap colormap failed");
    ok = false; goto END;
  }

  // Loop on blocks
  for (int yy=0;yy<global_size;yy+=block_size) for (int xx=0;xx<global_size;xx+=block_size)
  {
    int leftXSign=1,topYSign=1;

    // Setup coordinates
    if (coordsSize > 0)
    {
      if ( !mQueue->mapBuffer(mCoordBuffer,xCoords,CL_TRUE,CL_MAP_WRITE).isValid() )
      {
	report("map coords failed");
	ok = false; goto END;
      }
      FPReal<Mandelbrot::NWords> leftX,topY,aux;

      leftX.set(data->leftX);
      aux.set(data->stepX);
      aux.mul(xx*nx);
      leftX.add(aux);
      leftXSign = leftX.sgn();

      topY.set(data->topY);
      aux.set(data->stepY);
      aux.mul(-yy*ny);
      topY.add(aux);
      topYSign = topY.sgn();

      leftX.getWords(fpWords,xCoords);
      topY.getWords(fpWords,xCoords + fpWords);
      data->stepX.getWords(fpWords,xCoords + 2*fpWords);
      data->stepY.getWords(fpWords,xCoords + 3*fpWords);

      if (!mQueue->unmapMemoryObject(mCoordBuffer,xCoords).isValid())
      {
	report("unmap coords failed");
	ok = false; goto END;
      }
    }

    // Set kernel args
    argc = 0;
    ok &= kernel->setArg(argc++,mImageBuffer); // a
    ok &= kernel->setArg(argc++,mColormapBuffer);
    if (coordsSize > 0)
    {
      ok &= kernel->setArg(argc++,mCoordBuffer);
    }
    ok &= kernel->setArg(argc++,nx);
    ok &= kernel->setArg(argc++,ny);
    ok &= kernel->setArg(argc++,(xx*nx)+lda*(yy*ny)); // offset
    ok &= kernel->setArg(argc++,lda);
    if (coordsSize > 0)
    {
      // Sign of leftX and topY
      ok &= kernel->setArg(argc++,leftXSign);
      ok &= kernel->setArg(argc++,topYSign);
    }
    else
    {
      double leftX = (double)data->leftX;
      double topY = (double)data->topY;
      double stepX = (double)data->stepX;
      double stepY = (double)data->stepY;
      leftX += (double)(xx*nx)*stepX;
      topY -= (double)(yy*ny)*stepY;
      if (mScalarType == FLOAT_TYPE)
      {
	ok &= kernel->setArg(argc++,(float)leftX);
	ok &= kernel->setArg(argc++,(float)topY);
	ok &= kernel->setArg(argc++,(float)stepX);
	ok &= kernel->setArg(argc++,(float)stepY);
      }
      else
      {
	ok &= kernel->setArg(argc++,leftX);
	ok &= kernel->setArg(argc++,topY);
	ok &= kernel->setArg(argc++,stepX);
	ok &= kernel->setArg(argc++,stepY);
      }
    }
    ok &= kernel->setArg(argc++,maxIt);
    if (!ok)
    {
      report("setArg failed");
      goto END;
    }
    cl::Event e = mQueue->execKernel2(kernel,block_size,block_size,workgroup_size,workgroup_size);
    if (!e.isValid())
    {
      report("exec kernel failed");
      ok = false; goto END;
    }

  } // XX,YY block loop
  mQueue->finish();

  // Copy the result to DATA
  if (!mQueue->mapBuffer(mImageBuffer,xBuf,CL_TRUE,CL_MAP_READ).isValid())
  {
    report("map buf failed");
    ok = false; goto END;
  }
  // Copy lines to DATA
  for (int y=0;y<data->height;y++)
  {
    memcpy(data->a+data->width*y,xBuf+lda*y,data->width*sizeof(int));
  }
  if (!mQueue->unmapMemoryObject(mImageBuffer,xBuf).isValid())
  {
    report("unmap buf failed");
    ok = false; goto END;
  }

END:
  if (!ok) { report("RUN FAILED"); }
  return ok;
}
