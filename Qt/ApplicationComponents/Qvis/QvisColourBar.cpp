
#include "QvisColourBar.h"

#include <qpainter.h>
#include <qpolygon.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qnamespace.h>
#include <QMouseEvent>

#include <iostream>
#include <cmath>
#include <cstdlib>

//---------------------------------------------------------------------------
QvisColourBar::QvisColourBar(QWidget *parentObject, const char *name)
    : QvisAbstractOpacityBar(parentObject, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);
}
//---------------------------------------------------------------------------
QvisColourBar::~QvisColourBar()
{
}
//---------------------------------------------------------------------------
void QvisColourBar::setBackgroundColourData(int N, int c, const unsigned char *data)
{
  if (N>0 && data) {
    QImage image( QSize(N,1), QImage::Format_RGB32);
    for (int i=0; i<N; i++) {
      image.setPixel(i, 0, qRgb(data[c*i+0],  data[c*i+1], data[c*i+2]));
    }
    QPixmap pixmap = QPixmap::fromImage(image);
    this->SetShowBackgroundPixmap(true, true);
    this->SetBackgroundPixmap(&pixmap);
  }
  else {
    this->SetBackgroundPixmap(NULL);
    this->SetShowBackgroundPixmap(false, false);
  }
  this->repaint();
}
//---------------------------------------------------------------------------
void QvisColourBar::paintToPixmap(int w,int h)
{
  QPainter painter(pix);
  this->paintBackground(painter,w,h);
}
//---------------------------------------------------------------------------
void QvisColourBar::getRawOpacities(int, float *)
{
  // pure virtual function has to be overidden
}
//---------------------------------------------------------------------------
