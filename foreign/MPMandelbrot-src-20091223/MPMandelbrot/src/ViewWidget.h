// Display the fractal and manages zoom
// (c) EB Nov 2009

#ifndef ViewWidget_h
#define ViewWidget_h

#include <QtGui/QWidget>
#include <QtGui/QPixmap>
#include <QtCore/QTimer>
#include "FPReal.h"
#include "EngineThread.h"
class Mandelbrot;

class ViewWidget : public QWidget
{
  Q_OBJECT

public:

  enum Constants
  {
    // Delay to euqueue a full resolution image
    // after the last low resolution request.
    FULL_COMPUTE_DELAY = 1000, // ms
  };

  // Constructor
  ViewWidget();

  // Destructor
  ~ViewWidget();

  // Send the editedXXX signals
  void displayCoordinates();

  // Select engine. E is one of EngineThread::Engines.
  void selectEngine(int e);

  // Select special view. V is in 0..NB_SPECIAL_SITES-1.
  void selectSpecialView(int v);

  // Update the image contents. Enqueue a task in the engine thread.
  // If INTERACTIVE is TRUE enqueue a shorter task with a reduced resolution.
  void updateImage(bool interactive = true);

  // Enqueue benchmark B.  Benchmark results will be signaled back on completion.
  void enqueueBenchmark(int b);

protected:

  // Base class functions
  void paintEvent(QPaintEvent * e);
  void resizeEvent(QResizeEvent * e);
  void mouseMoveEvent(QMouseEvent * e);
  void mousePressEvent(QMouseEvent * e);
  void mouseReleaseEvent(QMouseEvent * e);
  void wheelEvent(QWheelEvent * e);

public slots:

  // Called by the engine to signal a queue state change
  void processQueue(int in,int run,int out);

  // Update X,Y,Step,MaxIt
  void updateX(const QString & s);
  void updateY(const QString & s);
  void updateStep(const QString & s);
  void updateMaxIt(int i);

  // Timer slots
  void timerFull();

  // Refresh (at full resolution)
  void refresh() { updateImage(false); }
  // Copy to clipboard
  void copy();

signals:

  // Emitted when X,Y,Step,MaxIt have been edited here
  void editedX(const QString &);
  void editedY(const QString &);
  void editedStep(const QString &);
  void editedMaxIt(int);

  // Emitted when the engine is changed
  void engineChanged(int);

  // Emitted when an engine was requested and could not be
  // activated.  In that case, we signal this engine as
  // unavailable, and fallback to the native 1T cpu engine.
  void engineUnavailable(int);

  // Emitted when benchmark terminates. String is the benchmark report.
  void benchmarkFinished(const QString &);

private:

  static const int NWords = 8;

  // Coordinates of the current center,
  // and step for one pixel.
  FPReal<NWords> mCenterX,mCenterY,mStep;
  // Max iterations
  int mMaxIt;

  // Scale the image by FACTOR, maintaining pixel (X,Y), int coordinates
  // with respect to center.
  // If INTERACTIVE is TRUE enqueue a shorter task with a reduced resolution.
  void scaleImage(int x,int y,double factor,bool interactive);

  // Computation thread (computing engine is running in an external thread)
  EngineThread mEngine;

  // Image to display
  int mWidth,mHeight; // Dimensions corresponding to current window size
  QPixmap mPixmap;

  // Timer to generate full-resolution images
  QTimer mFullTimer;

  // Last full running time, or -1
  double mRunningTime;

  // Dragging state
  bool mDragging;
  QPoint mDragPos0;
  FPReal<NWords> mDragX0,mDragY0;
  QPixmap mDragPixmap0;

  // Zooming state
  bool mZooming;
  double mZoomFactor;
  int mZoomX,mZoomY;

  // Queue state (Mandelbrot::TaskLevel)
  int mInQueue;
  int mRunningQueue;
};

#endif // #ifndef ViewWidget_h
