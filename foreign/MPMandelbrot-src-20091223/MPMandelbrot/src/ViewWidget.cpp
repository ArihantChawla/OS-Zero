// Display the fractal and manages zoom
// (c) EB Nov 2009

#include <QtGui/QApplication>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QPainter>
#include <QtGui/QClipboard>
#include <QtCore/QDebug>

#include "ViewWidget.h"
#include "Mandelbrot.h"
#include "SpecialSites.h"

template <int N> void set(FPReal<N> & x,const QString & s)
{
  x.set(s.toAscii().data());
}

ViewWidget::ViewWidget()
{
  mCenterX.set(0);
  mCenterY.set(0);
  mStep.set(0.005);
  mMaxIt = 1024;
  mDragging = false;
  mZooming = false;
  mRunningTime = -1;
  mWidth = 1;
  mHeight = 1;
  mInQueue = mRunningQueue = 0;

  // Use Qt::QueuedConnection to put the message in the receiver's event queue
  // instead of calling directly the receiver object from another thread.
  connect(&mEngine,SIGNAL(queueStateChanged(int,int,int)),this,SLOT(processQueue(int,int,int)),Qt::QueuedConnection);

  // Forward engine messages to our listeners
  connect(&mEngine,SIGNAL(engineChanged(int)),this,SIGNAL(engineChanged(int)),Qt::QueuedConnection);
  connect(&mEngine,SIGNAL(engineUnavailable(int)),this,SIGNAL(engineUnavailable(int)),Qt::QueuedConnection);

  // Setup the timers
  mFullTimer.setSingleShot(true);
  connect(&mFullTimer,SIGNAL(timeout()),this,SLOT(timerFull()));
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::displayCoordinates()
{
  std::string s;
  mCenterX.toString(s);
  emit editedX(QString(s.c_str()));
  mCenterY.toString(s);
  emit editedY(QString(s.c_str()));
  mStep.toString(s);
  emit editedStep(QString(s.c_str()));
}

void ViewWidget::selectEngine(int e)
{
  if (e < 0 || e >= EngineThread::NB_ENGINES) return; // Invalid
  mEngine.selectEngine(e);
}

void ViewWidget::selectSpecialView(int v)
{
  if (v < 0 || v >= NB_SPECIAL_SITES) return; // Invalid
  set(mCenterX,SpecialSites[v].x);
  set(mCenterY,SpecialSites[v].y);
  set(mStep,SpecialSites[v].step);
  displayCoordinates();

  if (mMaxIt != SpecialSites[v].maxIt)
  {
    // Avoid useless updates of the slider
    mMaxIt = SpecialSites[v].maxIt;
    emit editedMaxIt(mMaxIt);
  }
  updateImage();
}

void ViewWidget::enqueueBenchmark(int b)
{
  if (b < 0 || b >= NB_SPECIAL_SITES) return; // Invalid

  FPReal<NWords> cx,cy,step;
  cx.set(SpecialSites[b].x);
  cy.set(SpecialSites[b].y);
  step.set(SpecialSites[b].step); step.mul2k(-1); // div by 2 (for size 2048)
  int maxIt = SpecialSites[b].maxIt;
  int sz = 2048;
  Mandelbrot::RenderingData * data = Mandelbrot::createRenderingData(sz,sz,cx,cy,step,maxIt,1,Mandelbrot::BENCHMARK_TASK);
  data->taskID = b;

  // If we could create a benchmark task, run it and wait for the result
  mEngine.enqueueTask(data);
  mInQueue = data->taskLevel;
  update(); // display queue state
}

void ViewWidget::copy()
{
  QClipboard * clipboard = QApplication::clipboard();
  clipboard->setPixmap(mPixmap);
}

void ViewWidget::paintEvent(QPaintEvent * e)
{
  if (mPixmap.isNull()) return;
  int pw = mPixmap.width();
  int ph = mPixmap.height();
  int w = width();
  int h = height();
  QPainter p(this);
  p.drawPixmap((w-pw)>>1,(h-ph)>>1,mPixmap);

#if 0
  // Add crosshairs
  p.setPen(Qt::blue);
  p.drawLine(w>>1,0,w>>1,h);
  p.drawLine(0,h>>1,w,h>>1);
#endif

  QRect r(0,0,100,16);
  p.setPen(Qt::black);
  p.setBrush(Qt::white);
  p.drawRect(r);

  // Display last full image rendering time
  if (mRunningTime>0)
  {
    p.setPen(Qt::black);
    p.drawText(r,Qt::AlignVCenter | Qt::AlignRight,QString("%1 ms").arg(mRunningTime*1.0e3));
  }

  // Display queue state
  for (int i=0;i<2;i++)
  {
    int s = (i==0)?mInQueue:mRunningQueue;
    r = QRect(2+14*i,2,12,12);
    p.setPen(Qt::black);
    switch (s)
    {
    case Mandelbrot::NO_TASK: p.setBrush(Qt::green); break;
    case Mandelbrot::LOW_RESOLUTION_TASK: p.setBrush(Qt::yellow); break;
    case Mandelbrot::HIGH_RESOLUTION_TASK: p.setBrush(Qt::red); break;
    case Mandelbrot::BENCHMARK_TASK: p.setBrush(Qt::blue); break;
    default: p.setBrush(Qt::lightGray); break;
    }
    p.drawRect(r);
  }
}

void ViewWidget::resizeEvent(QResizeEvent * e)
{
  int width = e->size().width();
  int height = e->size().height();

  // Round up to multiples of 4*C
  const int c = 4*Mandelbrot::BlockSize;
  width = (width+c-1) & ~(c-1);
  height = (height+c-1) & ~(c-1);
  if (width == mWidth && height == mHeight) return; // Already OK
  mWidth = width;
  mHeight = height;

  updateImage(true);
}

void ViewWidget::mouseMoveEvent(QMouseEvent * e)
{
  if (mDragging)
  {
    QPoint u = e->pos() - mDragPos0;
    FPReal<NWords> delta;

    // Update X
    mCenterX = mDragX0;
    delta = mStep;
    delta.mul(-u.x());
    mCenterX.add(delta);

    // Update Y
    mCenterY = mDragY0;
    delta = mStep;
    delta.mul(u.y());
    mCenterY.add(delta);

    // Update display
    displayCoordinates();

    // Enqueue computation (asynchronous)
    updateImage();

#if 0
    // In the meantime, translate what we already have
    int width = mPixmap.width();
    int height = mPixmap.height();
    QPainter p(&mPixmap);
    p.fillRect(0,0,width,height,Qt::gray);
    p.drawPixmap(u.x(),u.y(),mDragPixmap0);
    update();
#endif
  }
}

void ViewWidget::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == Qt::RightButton)
  {
    if (mDragging) return;
    mDragging = true;
    mDragPos0 = e->pos();
    mDragX0 = mCenterX;
    mDragY0 = mCenterY;
    mDragPixmap0 = mPixmap;
  }
  if (e->button() == Qt::LeftButton)
  {
    if (mZooming) return;
    mZooming = true;
    mZoomFactor = 0.95;
    mZoomX = e->x() - (width()>>1);
    mZoomY = (height()>>1) - e->y();
    if (e->modifiers() & Qt::ShiftModifier)
      mZoomFactor = 1.0/mZoomFactor;
    mFullTimer.stop();
    scaleImage(mZoomX,mZoomY,mZoomFactor,true);
  }
}

void ViewWidget::mouseReleaseEvent(QMouseEvent * e)
{
  mDragging = false;
  mDragPixmap0 = QPixmap();
  mZooming = false;
}

void ViewWidget::wheelEvent(QWheelEvent * e)
{
  double factor = exp(-e->delta()/200.0);
  // scaleImage(e->x() - width()/2,height()/2 - e->y(),factor,true);
  scaleImage(0,0,factor,true); // Maintain center
}

void ViewWidget::updateImage(bool interactive)
{
  // Setup rendering data for the task
  Mandelbrot::RenderingData * data = 0;

  if (interactive)
  {
    FPReal<Mandelbrot::NWords> step = mStep;
    step.mul2k(2); // *4
    data = Mandelbrot::createRenderingData(mWidth>>2,mHeight>>2,mCenterX,mCenterY,step,mMaxIt,4,Mandelbrot::LOW_RESOLUTION_TASK);
  }
  else
    data = Mandelbrot::createRenderingData(mWidth,mHeight,mCenterX,mCenterY,mStep,mMaxIt,1,Mandelbrot::HIGH_RESOLUTION_TASK);

  // qDebug() << "enqueue" << data->taskID << ":" << mWidth << "x" << mHeight;
  mEngine.enqueueTask(data);
  mInQueue = data->taskLevel;

  // In interactive mode, schedule a full resolution computation later
  if (interactive)
  {
    mFullTimer.stop(); // Is it needed?
    mFullTimer.start(FULL_COMPUTE_DELAY); // ms
  }

  update(); // Update queue state
}

void ViewWidget::scaleImage(int x,int y,double factor,bool interactive)
{
  FPReal<NWords> auxX = mStep;
  FPReal<NWords> auxY = mStep;
  double u = (double)mStep * factor;
  mStep.set(u);
  auxX.sub(mStep);
  auxX.mul(x);
  mCenterX.add(auxX);
  auxY.sub(mStep);
  auxY.mul(y);
  mCenterY.add(auxY);

  // Update coord display
  displayCoordinates();

  updateImage(interactive);
}

void ViewWidget::processQueue(int in,int run,int out)
{
  mInQueue = in;
  mRunningQueue = run;

  // Schedule a new zoom task when input queue is empty
  if (mZooming && in == Mandelbrot::NO_TASK)
  {
    scaleImage(mZoomX,mZoomY,mZoomFactor,true);
  }

  // Get task from output queue
  Mandelbrot::RenderingData * data = mEngine.dequeueTask();
  if (data != 0)
  {
    if (data->taskLevel == Mandelbrot::BENCHMARK_TASK)
    {
      int b = data->taskID; // special view index
      QString report;
      report += QString("<p>");
      report += QString("Engine: <b>%1</b>").arg(mEngine.getName());
      report += QString("<br>Site: <b>%1</b>").arg(SpecialSites[b].name);
      report += QString("<br>Dimensions: <b>%1x%2</b>").arg(data->width).arg(data->height);
      report += QString("<br>Iterations: <b>%1</b>").arg(data->maxIt);
      report += QString("<br>Running time: <b>%1 s</b>").arg(data->runningTime);
      report += QString("</p>");
      // Copy image to clipboard
      int width = data->width;
      int height = data->height;
      QImage image = QImage((const unsigned char *)data->a,width,height,width*4,QImage::Format_ARGB32).copy(); // force deep copy, otherwise won't work
      QClipboard * clipboard = QApplication::clipboard();
      clipboard->setImage(image);

      // Terminate task and signal
      delete data;
      emit benchmarkFinished(report);
    }
    else
    {
      // Convert to Image
      // qDebug() << "dequeue" << data->taskID << ":" << data->width << "x" << data->height;

      int width = data->width;
      int height = data->height;
      QImage image((const unsigned char *)data->a,width,height,width*4,QImage::Format_ARGB32);
      if (data->scale > 1)
      {
	image = image.scaled(width*data->scale,height*data->scale); // rescale
      }
      else
      {
	mRunningTime = data->runningTime;
      }
      mPixmap = QPixmap::fromImage(image);

      // Terminate task
      delete data;
    }
  }

  update();
}

void ViewWidget::updateX(const QString & s)
{
  set(mCenterX,s);
  updateImage();
}

void ViewWidget::updateY(const QString & s)
{
  set(mCenterY,s);
  updateImage();
}

void ViewWidget::updateStep(const QString & s)
{
  set(mStep,s);
  updateImage();
}

void ViewWidget::updateMaxIt(int i)
{
  if (mMaxIt == i) return;

  mMaxIt = i;
  updateImage();
}

void ViewWidget::timerFull()
{
  updateImage(false);
}
