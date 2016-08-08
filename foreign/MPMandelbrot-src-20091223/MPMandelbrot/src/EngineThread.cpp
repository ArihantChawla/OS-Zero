// Mandelbrot engine thread (Qt)
// (c) EB Dec 2009

#include "EngineThread.h"
#include <QtGui/QApplication>
#include <QtCore/QDebug>

const char * EngineThread::getEngineName(int e)
{
  switch (e)
  {
  case NO_ENGINE: return "No engine";
  case NATIVE_FLOAT_1T: return "C float, single thread";
  case NATIVE_FLOAT_2T: return "C float, 2 threads";
  case NATIVE_FLOAT_4T: return "C float, 4 threads";
  case NATIVE_FLOAT_8T: return "C float, 8 threads";
  case NATIVE_DOUBLE_1T: return "C double, single thread";
  case NATIVE_DOUBLE_2T: return "C double, 2 threads";
  case NATIVE_DOUBLE_4T: return "C double, 4 threads";
  case NATIVE_DOUBLE_8T: return "C double, 8 threads";
  case NATIVE_FP128_1T: return "C fixed-point 128 bits, single thread";
  case NATIVE_FP128_2T: return "C fixed-point 128 bits, 2 threads";
  case NATIVE_FP128_4T: return "C fixed-point 128 bits, 4 threads";
  case NATIVE_FP128_8T: return "C fixed-point 128 bits, 8 threads";
  case OPENCL_CPU_FLOAT: return "OpenCL CPU float";
  case OPENCL_CPU_DOUBLE: return "OpenCL CPU double";
  case OPENCL_CPU_FP128: return "OpenCL CPU fixed-point 128 bits";
  case OPENCL_GPU_FLOAT: return "OpenCL GPU float";
  case OPENCL_GPU_DOUBLE: return "OpenCL GPU double";
  case OPENCL_GPU_FP128: return "OpenCL GPU fixed-point 128 bits";
  default: return "Invalid engine";
  }
}

EngineThread::EngineThread() : mTasks(0), mToRender(0), mRunning(0), mRendered(0)
{
  mCurrentEngine = mRequestedEngine = NO_ENGINE;
  mEngine = 0;
  selectEngine(NATIVE_DOUBLE_1T);
  // Start our thread
  start();
}

EngineThread::~EngineThread()
{
  // Shut down engine
  selectEngine(NO_ENGINE);
  // When engine is 0, the worker thread will terminate, wait for it
  bool ok = wait(5000);
  if (!ok)
  {
    qDebug() << "Engine thread did not terminate";
  }
  // We may still have tasks do destroy in the queues
  if (mToRender != 0) delete mToRender;
  if (mRunning != 0) delete mRunning;
  if (mRendered != 0) delete mRendered;
}

void EngineThread::enqueueTask(Mandelbrot::RenderingData * data)
{
  QMutexLocker locker(&mMutex);
  int newLevel = (data != 0)?(data->taskLevel):Mandelbrot::NO_TASK;
  int oldLevel = (mToRender != 0)?(mToRender->taskLevel):Mandelbrot::NO_TASK;
  if (mToRender != 0)
  {
    // Replace existing task or drop incoming task
    bool replace = true;
    if (newLevel <= oldLevel || newLevel == Mandelbrot::BENCHMARK_TASK) replace = true;
    if (oldLevel == Mandelbrot::BENCHMARK_TASK) replace = false; // Can't remove benchmark task

    if (replace)
    {
      delete mToRender;
      // Drop input
      // qDebug() << "** Input task dropped";
      mToRender = data;
    }
    else
    {
      if (data != 0) delete data; // Drop new task
    }
  }
  else
  {
    // No conflict
    mToRender = data;
    mTasks.release(1);
  }
}

Mandelbrot::RenderingData * EngineThread::dequeueTask()
{
  Mandelbrot::RenderingData * data = 0;
  mMutex.lock();
  data = mRendered;
  mRendered = 0;
  mMutex.unlock();
  return data;
}

void EngineThread::allocEngine()
{
  if (mCurrentEngine == mRequestedEngine) return; // Nothing to do

  QMutexLocker locker(&mMutex);
  if (mEngine != 0)
  {
    delete mEngine; mEngine = 0;
    qDebug() << "Destroyed engine" << getEngineName(mCurrentEngine);
  }
  mCurrentEngine = mRequestedEngine;
  if (mRequestedEngine == NO_ENGINE) return;

  std::string appDir = QApplication::applicationDirPath().toStdString();

  switch (mRequestedEngine)
  {
  case NATIVE_FLOAT_1T: mEngine = new MandelbrotNative(Mandelbrot::FLOAT_TYPE,1); break;
  case NATIVE_FLOAT_2T: mEngine = new MandelbrotNative(Mandelbrot::FLOAT_TYPE,2); break;
  case NATIVE_FLOAT_4T: mEngine = new MandelbrotNative(Mandelbrot::FLOAT_TYPE,4); break;
  case NATIVE_FLOAT_8T: mEngine = new MandelbrotNative(Mandelbrot::FLOAT_TYPE,8); break;
  case NATIVE_DOUBLE_1T: mEngine = new MandelbrotNative(Mandelbrot::DOUBLE_TYPE,1); break;
  case NATIVE_DOUBLE_2T: mEngine = new MandelbrotNative(Mandelbrot::DOUBLE_TYPE,2); break;
  case NATIVE_DOUBLE_4T: mEngine = new MandelbrotNative(Mandelbrot::DOUBLE_TYPE,4); break;
  case NATIVE_DOUBLE_8T: mEngine = new MandelbrotNative(Mandelbrot::DOUBLE_TYPE,8); break;
  case NATIVE_FP128_1T: mEngine = new MandelbrotNative(Mandelbrot::FP128_TYPE,1); break;
  case NATIVE_FP128_2T: mEngine = new MandelbrotNative(Mandelbrot::FP128_TYPE,2); break;
  case NATIVE_FP128_4T: mEngine = new MandelbrotNative(Mandelbrot::FP128_TYPE,4); break;
  case NATIVE_FP128_8T: mEngine = new MandelbrotNative(Mandelbrot::FP128_TYPE,8); break;
  case OPENCL_CPU_FLOAT: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_CPU,Mandelbrot::FLOAT_TYPE,appDir.c_str()); break;
  case OPENCL_CPU_DOUBLE: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_CPU,Mandelbrot::DOUBLE_TYPE,appDir.c_str()); break;
  case OPENCL_CPU_FP128: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_CPU,Mandelbrot::FP128_TYPE,appDir.c_str()); break;
  case OPENCL_GPU_FLOAT: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_GPU,Mandelbrot::FLOAT_TYPE,appDir.c_str()); break;
  case OPENCL_GPU_DOUBLE: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_GPU,Mandelbrot::DOUBLE_TYPE,appDir.c_str()); break;
  case OPENCL_GPU_FP128: mEngine = new MandelbrotOpenCL(CL_DEVICE_TYPE_GPU,Mandelbrot::FP128_TYPE,appDir.c_str()); break;
  };

  // Destroy if initialization failed
  if (mEngine != 0 && !mEngine->isOK())
  {
    delete mEngine;
    mEngine = 0;
    emit engineUnavailable(mRequestedEngine);
  }

  // Fallback to native
  if (mEngine == 0)
  {
    mEngine = new MandelbrotNative(0,1);
    mCurrentEngine = mRequestedEngine = NATIVE_DOUBLE_1T;
  }
  emit engineChanged(mCurrentEngine);
  qDebug() << "Created engine" << getEngineName(mCurrentEngine);
}

const char * EngineThread::getName() const
{
  return getEngineName(mCurrentEngine);
}

void EngineThread::run()
{
  qDebug() << "Worker thread running...";

  while (1)
  {
    bool hasTask = mTasks.tryAcquire(1,400);
    // Even if no task is inserted, check if we should change the engine
    // (so the engine is eventually destroyed when we terminate)
    allocEngine();
    if (mEngine == 0) break; // Terminate thread
    if (!hasTask) continue; // No task, only timeout

    // Check if we have a task to render
    mMutex.lock();
    mRunning = mToRender;
    mToRender = 0;
    mMutex.unlock();
    emitQueueStateChanged();
    if (mRunning == 0) continue; // Nothing to do

    // Render task
    double t0 = Mandelbrot::realTime();
    mEngine->compute(mRunning);
    mRunning->runningTime = Mandelbrot::realTime() - t0;

    // Move to rendered
    mMutex.lock();
    if (mRendered != 0)
    {
      // Drop output
      delete mRendered;
      // qDebug() << "** Rendered task dropped";
    }
    mRendered = mRunning;
    mRunning = 0;
    mMutex.unlock();
    emitQueueStateChanged();
  }

  qDebug() << "Worker thread terminating...";
}

void EngineThread::emitQueueStateChanged()
{
  int in,run,out;
  in = run = out = Mandelbrot::NO_TASK;
  if (mToRender != 0) in = mToRender->taskLevel;
  if (mRunning != 0) run = mRunning->taskLevel;
  if (mRendered != 0) out = mRendered->taskLevel;
  emit queueStateChanged(in,run,out);
}
