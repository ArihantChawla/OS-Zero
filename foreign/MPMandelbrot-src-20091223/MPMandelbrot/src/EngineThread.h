// Mandelbrot engine thread (Qt)
// (c) EB Dec 2009

#ifndef EngineThread_h
#define EngineThread_h

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QSemaphore>

#include "MandelbrotNative.h"
#include "MandelbrotOpenCL.h"

class EngineThread : public QThread
{
  Q_OBJECT

public:
  
  // All available engines
  enum Engines {
    // Special value
    NO_ENGINE = -1,
    // Engines
    NATIVE_FLOAT_1T = 0,
    NATIVE_FLOAT_2T,
    NATIVE_FLOAT_4T,
    NATIVE_FLOAT_8T,
    NATIVE_DOUBLE_1T,
    NATIVE_DOUBLE_2T,
    NATIVE_DOUBLE_4T,
    NATIVE_DOUBLE_8T,
    NATIVE_FP128_1T,
    NATIVE_FP128_2T,
    NATIVE_FP128_4T,
    NATIVE_FP128_8T,
    OPENCL_CPU_FLOAT,
    OPENCL_CPU_DOUBLE,
    OPENCL_CPU_FP128,
    OPENCL_GPU_FLOAT,
    OPENCL_GPU_DOUBLE,
    OPENCL_GPU_FP128,
    NB_ENGINES
  };

  // Return engine name corresponding to E
  static const char * getEngineName(int e);

  // Constructor.  Selects the native engine 1T.
  EngineThread();

  // Destructor
  ~EngineThread();

  // Select engine.  Engine will be switched asap in the worker thread.
  void selectEngine(int e) { QMutexLocker locker(&mMutex); mRequestedEngine = e; }

  // Return name of current engine
  const char * getName() const;

  // Enqueue a task in the input queue.
  // DATA is owned by THIS after the call.
  // DATA may be 0.
  void enqueueTask(Mandelbrot::RenderingData * data);

  // Dequeue a task from the output queue.
  // DATA is owned by the caller after the call.
  // Returns 0 if no task is available.
  Mandelbrot::RenderingData * dequeueTask();

signals:

  // Emitted when the engine is changed
  void engineChanged(int e);

  // Emitted when an engine was requested and could not be
  // activated.  In that case, we signal this engine as
  // unavailable, and fallback to the native 1T cpu engine.
  void engineUnavailable(int e);

  // Emitted when the queue state changes. Values are
  // 0 (no task), 1 (small scale), 2 (full scale).
  // Not emitted when enqueueTask / dequeueTask is called,
  // since the caller already knows how he changed the queues.
  void queueStateChanged(int inputQueue,int running,int outputQueue);

protected:

  // Allocate a new engine if needed.
  void allocEngine();

  // Base class function
  void run();

private:

  // Emit queueStateChanged message
  void emitQueueStateChanged();

  // Mandelbrot set computation engine
  Mandelbrot * mEngine;
  // Mutex controlling access to the rendering data
  QMutex mMutex;
  // Semaphore controlling the state of the thread
  QSemaphore mTasks;
  // Tasks to render / running / rendered.
  Mandelbrot::RenderingData * mToRender;
  Mandelbrot::RenderingData * mRunning;
  Mandelbrot::RenderingData * mRendered;
  // Current / requested engine
  int mCurrentEngine;
  int mRequestedEngine;
};

#endif // #ifndef EngineThread_h
