// Application window
// (c) EB Nov 2009

#ifndef AppWindow_h
#define AppWindow_h

#include <vector>
#include <QtGui/QMainWindow>
class ViewWidget;
class QLineEdit;
class QSlider;
class QLabel;
class QAction;

class AppWindow : public QMainWindow
{
  Q_OBJECT

public:

  // Constructor
  AppWindow();

public slots:

  // Update X,Y,Step,maxIt
  void updateX(const QString & s);
  void updateY(const QString & s);
  void updateStep(const QString & s);
  void updateMaxIt(int i); // updates label and slider

  // Process engine change / unavailable
  void processEngineChange(int e);
  void processEngineUnavailable(int e);

  // Display benchmark report
  void reportBenchmark(const QString & report);

private slots:

  // Update the maxit label only
  void updateMaxItLabel(int i);
  // Select engine/view/colormap (menu actions)
  void selectEngine();
  void selectView();
  void selectColormap();
  void selectBenchmark();

  // Help menu
  void helpAbout();

private:

  // Run benchmark B and display report
  void runBenchmark(int b);

  ViewWidget * mView;
  QLineEdit * mXEdit;
  QLineEdit * mYEdit;
  QLineEdit * mStepEdit;
  QSlider * mMaxItEdit;
  QLineEdit * mMaxItLabel;
  std::vector<QAction *> mEngineActions;
  std::vector<QAction *> mViewActions;
  std::vector<QAction *> mColormapActions;
  std::vector<QAction *> mBenchmarkActions;

};

#endif // #ifndef AppWindow_h
