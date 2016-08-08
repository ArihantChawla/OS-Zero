// Application help dialog
// (c) EB Dec 2009

#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QDialogButtonBox>
#include "HelpDialog.h"

HelpDialog::HelpDialog(QWidget * parent) : QDialog(parent)
{
  setModal(true);
  setFixedSize(600,400);
  QVBoxLayout * l = new QVBoxLayout();
  setLayout(l);
  QTextEdit * e = new QTextEdit();
  e->setReadOnly(true);
  l->addWidget(e);
  QDialogButtonBox * b = new QDialogButtonBox(QDialogButtonBox::Ok);
  connect(b,SIGNAL(accepted()),this,SLOT(accept()));
  l->addWidget(b);

  QString s;
  s.append("<h2>About " + parent->windowTitle() + "</h2>");
  s.append("<p style='color:red'>WARNING! OpenCL fixed-point engines may freeze your computer.  SAVE and CLOSE other applications before selecting them.</p>");

  // s.append("<h2>Controls</h2>");
  s.append("<p><b>Left click</b>: zoom in at mouse position until button is released.");
  s.append("<br><b>Shift+Left click</b>: zoom out at mouse position until button is released.");
  s.append("<br><b>Right drag</b>: translate view.");
  s.append("<br><b>Wheel</b>: zoom in/out at window center.");
  s.append("<br><b>Menu:Engine:...</b>: select rendering engine.");
  s.append("<br><b>Menu:View:Refresh</b>: recompute current view at full resolution.");
  s.append("<br><b>Menu:View:Copy</b>: copy current view to clipboard.");
  s.append("<br><b>Menu:Colormap:...</b>: select colormap.");
  s.append("<br><b>Menu:Sites:...</b>: update view to remarkable site.");
  s.append("<br><b>Menu:Benchmark:...</b>: compute benchmark image for a given site. Result is copied to clipboard.");
  s.append("</p>");

  s.append("<p>In the top-left corner of the image, the first square shows the task in the waiting queue, and"
  " the second square the task currently being processed.  Green: empty slot, Yellow: low-resolution image, Red: full-resolution image, Blue: benchmark image."
  " The time displayed corresponds to the rendering of the last full-resolution image.</p>");
  s.append("<p>During interactive manipulation, low-resolution images are displayed to provide a faster visual feedback to the user. "
    "When interactive manipulation stops, a full-resolution rendering is scheduled.</p>");

  s.append("<p>&copy; 2009 Eric Bainville -- <a href='http://www.bealto.com/'>www.bealto.com</a></p>");

  e->setHtml(s);
}

#if 0
  QDialog w;
  QMessageBox mb(this);

  QString it,dt;
  it.append("<p>About " + windowTitle() + "</p>");
  it.append("<p><b>WARNING!</b> OpenCL fixed-point engines may freeze your computer.  SAVE and CLOSE other applications before selecting them.</p>");

  dt.append("<p>Left click: zoom in at mouse position until button is released.</p>");
  dt.append("<p>Shift+Left click: zoom out at mouse position until button is released.</p>");
  dt.append("<p>Right drag: translate view.</p>");

  mb.setInformativeText(it);
  mb.setDetailedText(dt);
  mb.exec();
#endif
