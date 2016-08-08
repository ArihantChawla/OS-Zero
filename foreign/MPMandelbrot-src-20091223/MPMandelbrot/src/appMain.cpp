// Multi-Precision Mandelbrot
// (c) EB Nov 2009

#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <BealtoOpenCL.h>

#ifndef CONFIG_UNIT_TESTS
#define CONFIG_UNIT_TESTS 0
#endif

#if CONFIG_UNIT_TESTS
#include "UnitTests.h"
#endif
#include "AppWindow.h"
#include "Mandelbrot.h"

int main(int argc,char ** argv)
{
  QApplication app(argc,argv);

#if CONFIG_UNIT_TESTS // Tests
  runTests();
  exit(0);
#endif

#if 0
  cl::Context * c = cl::Context::create();
  if (c == 0) exit(1);
  qDebug() << c->getDeviceImageSupport();
  qDebug() << c->getDeviceImage2DMaxWidth() << "x" << c->getDeviceImage2DMaxHeight();
  delete c;
#endif

#if 0
  double t0 = Mandelbrot::realTime();
  double t1 = 0;
  int nv = 0;
  while (1)
  {
    double t2 = Mandelbrot::realTime() - t0;
    if (t2>t1) { nv++; t1 = t2; }
    if (t2>1.0) break;
  }
  qDebug() << "Clock resolution" << nv << "per second";
#endif
  AppWindow * w = new AppWindow();
  w->show();
  qApp->exec();
}
