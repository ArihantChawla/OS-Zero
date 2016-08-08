// Application window
// (c) EB Nov 2009

#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSlider>
#include <QtGui/QMenuBar>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include "AppWindow.h"
#include "HelpDialog.h"
#include "ViewWidget.h"
#include "SpecialSites.h"

AppWindow::AppWindow()
{
  this->setWindowTitle("Multiprecision CPU/GPU Mandelbrot Set");

  // View
  mView = new ViewWidget();
  setCentralWidget(mView);

  // Coordinates palette
  QDockWidget * dock = new QDockWidget("View Parameters");
  QWidget * w = new QWidget();
  QGridLayout * l = new QGridLayout();
  w->setLayout(l);
  l->addWidget(new QLabel("Center X"),0,0);
  l->addWidget(new QLabel("Center Y"),1,0);
  l->addWidget(new QLabel("Pixel"),2,0);
  l->addWidget(new QLabel("Max Iterations"),3,0);
  mXEdit = new QLineEdit(); l->addWidget(mXEdit,0,1,1,2);
  mYEdit = new QLineEdit(); l->addWidget(mYEdit,1,1,1,2);
  mStepEdit = new QLineEdit(); l->addWidget(mStepEdit,2,1,1,2);
  mMaxItLabel = new QLineEdit();
  mMaxItLabel->setReadOnly(true);
  mMaxItLabel->setFixedWidth(80); l->addWidget(mMaxItLabel,3,1);
  mMaxItEdit = new QSlider(Qt::Horizontal); l->addWidget(mMaxItEdit,3,2);
  mMaxItEdit->setRange(100,50000);
  mMaxItEdit->setSingleStep(100);
  mMaxItEdit->setPageStep(500);
  dock->setWidget(w);
  dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::TopDockWidgetArea,dock);

  connect(mView,SIGNAL(editedX(const QString &)),this,SLOT(updateX(const QString &)));
  connect(mView,SIGNAL(editedY(const QString &)),this,SLOT(updateY(const QString &)));
  connect(mView,SIGNAL(editedStep(const QString &)),this,SLOT(updateStep(const QString &)));
  connect(mView,SIGNAL(editedMaxIt(int)),this,SLOT(updateMaxIt(int)));
  connect(mXEdit,SIGNAL(textEdited(const QString &)),mView,SLOT(updateX(const QString &)));
  connect(mYEdit,SIGNAL(textEdited(const QString &)),mView,SLOT(updateY(const QString &)));
  connect(mStepEdit,SIGNAL(textEdited(const QString &)),mView,SLOT(updateStep(const QString &)));
  connect(mMaxItEdit,SIGNAL(valueChanged(int)),this,SLOT(updateMaxItLabel(int)));
  connect(mMaxItEdit,SIGNAL(valueChanged(int)),mView,SLOT(updateMaxIt(int)));
  connect(mView,SIGNAL(engineChanged(int)),this,SLOT(processEngineChange(int)));
  connect(mView,SIGNAL(engineUnavailable(int)),this,SLOT(processEngineUnavailable(int)));
  connect(mView,SIGNAL(benchmarkFinished(const QString &)),this,SLOT(reportBenchmark(const QString &)));

  // Menu
  QMenuBar * mbar = new QMenuBar();
  setMenuBar(mbar);
  QMenu * entry;
  // Menu>File
  entry = new QMenu("File");
  mbar->addMenu(entry);
  entry->addAction("Exit",this,SLOT(close()),QKeySequence::Close);
  // Menu>Engine
  entry = new QMenu("Engine");
  mbar->addMenu(entry);
  mEngineActions.resize(EngineThread::NB_ENGINES,0);
  for (int i=0;i<EngineThread::NB_ENGINES;i++)
  {
    mEngineActions[i] = entry->addAction(EngineThread::getEngineName(i),this,SLOT(selectEngine()));
    mEngineActions[i]->setCheckable(true);
  }
  // Menu>View
  entry = new QMenu("View");
  mbar->addMenu(entry);
  entry->addAction("Refresh",mView,SLOT(refresh()),QKeySequence::Refresh);
  entry->addAction("Copy",mView,SLOT(copy()),QKeySequence::Copy);
  // Menu>Colormap
  entry = new QMenu("Colormap");
  mbar->addMenu(entry);
  mColormapActions.resize(Mandelbrot::NB_COLORMAPS,0);
  for (int i=0;i<Mandelbrot::NB_COLORMAPS;i++)
  {
    mColormapActions[i] = entry->addAction(Mandelbrot::getColormapName(i),this,SLOT(selectColormap()));
    mColormapActions[i]->setCheckable(true);
  }
  // Menu>Sites
  entry = new QMenu("Sites");
  mbar->addMenu(entry);
  mViewActions.resize(NB_SPECIAL_SITES,0);
  for (int i=0;i<NB_SPECIAL_SITES;i++)
  {
    mViewActions[i] = entry->addAction(getSpecialSiteName(i),this,SLOT(selectView()));
  }
  // Menu>Benchmarks
  entry = new QMenu("Benchmarks");
  mbar->addMenu(entry);
  mBenchmarkActions.resize(NB_SPECIAL_SITES,0);
  for (int i=0;i<NB_SPECIAL_SITES;i++)
  {
    mBenchmarkActions[i] = entry->addAction(getSpecialSiteName(i),this,SLOT(selectBenchmark()));
  }
  // Menu>Help
  entry = new QMenu("Help");
  mbar->addMenu(entry);
  entry->addAction("About...",this,SLOT(helpAbout()),QKeySequence::HelpContents);

  // Initial position
  mView->selectSpecialView(0); // Home
  // Initial engine
  mView->selectEngine(EngineThread::NATIVE_DOUBLE_8T);
  mEngineActions[EngineThread::NATIVE_DOUBLE_8T]->setChecked(true);
  // Initial colormap
  Mandelbrot::initColormap(0);
  mColormapActions[0]->setChecked(true);

  resize(QSize(800,600));
}

void AppWindow::updateX(const QString & s)
{
  mXEdit->setText(s);
}

void AppWindow::updateY(const QString & s)
{
  mYEdit->setText(s);
}

void AppWindow::updateStep(const QString & s)
{
  mStepEdit->setText(s);
}

void AppWindow::updateMaxIt(int i)
{
  if (mMaxItEdit->value() == i) return; // To prevent message loops

  mMaxItEdit->setValue(i);
  updateMaxItLabel(i);
}

void AppWindow::processEngineChange(int e)
{
  for (int i=0;i<EngineThread::NB_ENGINES;i++)
  {
    mEngineActions[i]->setChecked(i==e);
  }
  mView->updateImage(true);
}

void AppWindow::processEngineUnavailable(int e)
{
  if (e >= 0 && e < EngineThread::NB_ENGINES)
  {
    mEngineActions[e]->setEnabled(false);
  }
}

void AppWindow::reportBenchmark(const QString & report)
{
  QMessageBox::information(this,"Benchmark Result",report);
}

void AppWindow::updateMaxItLabel(int i)
{
  mMaxItLabel->setText(QString("%1").arg(i));
}

void AppWindow::selectEngine()
{
  for (int i=0;i<EngineThread::NB_ENGINES;i++)
  {
    if (sender() == mEngineActions[i])
    {
      mView->selectEngine(i);
      mEngineActions[i]->setChecked(true);
    }
    else
    {
      mEngineActions[i]->setChecked(false);
    }
  }
}

void AppWindow::selectView()
{
  for (int i=0;i<NB_SPECIAL_SITES;i++)
  {
    if (sender() == mViewActions[i])
    {
      mView->selectSpecialView(i);
      return;
    }
  }
}

void AppWindow::selectBenchmark()
{
  for (int i=0;i<NB_SPECIAL_SITES;i++)
  {
    if (sender() == mBenchmarkActions[i])
    {
      mView->enqueueBenchmark(i);
      return;
    }
  }
}

void AppWindow::selectColormap()
{
  for (int i=0;i<Mandelbrot::NB_COLORMAPS;i++)
  {
    if (sender() == mColormapActions[i])
    {
      Mandelbrot::initColormap(i);
      mView->updateImage();
      mColormapActions[i]->setChecked(true);
    }
    else
    {
      mColormapActions[i]->setChecked(false);
    }
  }
}

void AppWindow::helpAbout()
{
  HelpDialog h(this);
  h.exec();
}
