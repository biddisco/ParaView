
#define _USE_MATH_DEFINES
#include "Qvis2DTransferFunctionWidget.h"

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
Qvis2DTransferFunctionWidget::Qvis2DTransferFunctionWidget(QWidget *parentObject, const char *name)
    : QvisAbstractOpacityBar(parentObject, name)
{
    this->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    this->setLineWidth( 2 );
    this->setMinimumHeight(50);
    this->setMinimumWidth(128);
    this->nregion       = 0;
    this->highlitRegion =-1;
    this->activeRegion  =-1;
    this->defaultTFMode = 0;
    this->movingMode    = modeNone;
    this->maximumNumberOfRegions = -1; // unlimited
    this->minimumNumberOfRegions =  0;
    this->UnderlayColourPixmap   =  NULL;

    // set a default:
    this->addRegion(0.25f, 0.25f, 0.5f, 0.5f, this->defaultTFMode, 1.0);

    this->mousedown = false;
    this->setMouseTracking(true);
}
//---------------------------------------------------------------------------
Qvis2DTransferFunctionWidget::~Qvis2DTransferFunctionWidget()
{
  if (this->UnderlayColourPixmap) delete this->UnderlayColourPixmap;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setBackgroundGradientData(int x, int y, int *data)
{
  QImage image( QSize(x,y), QImage::Format_RGB32);
  int index = 0;
  double maxval = 0;
  for (int j=0; j<y; j++) {
    for (int i=0; i<x; i++) {
      if (data[index]>maxval) maxval = data[index];
      index++;
    }
  }
  double lmax = log(maxval);
  index = 0;
  for (int j=0; j<y; j++) {
    for (int i=0; i<x; i++) {
      if (data[index]>0) {
        int val = 255.0 * log((double)data[index])/lmax;
        image.setPixel(i, j, qRgb(val,  val, val));
      }
      else {
        image.setPixel(i, j, qRgb(0,0,0));
      }
      index++;
    }
  }
  QPixmap pixmap = QPixmap::fromImage(image);
  this->SetShowBackgroundPixmap(true, true);
  this->SetBackgroundPixmap(&pixmap);
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setUnderlayColourData(int N, int c, const unsigned char *data)
{
  if (N>0 && data) {
    QImage image( QSize(N,1), QImage::Format_RGB32);
    for (int i=0; i<N; i++) {
      image.setPixel(i, 0, qRgb(data[c*i+0],  data[c*i+1], data[c*i+2]));
    }
    if (this->UnderlayColourPixmap) delete this->UnderlayColourPixmap;
    this->UnderlayColourPixmap = new QPixmap(QPixmap::fromImage(image));
  }
}
//---------------------------------------------------------------------------
int Val2x(QPixmap *p, float val)
{
  int w = p->width();
  int h = p->height();
  int _x = val*float(w);
  return _x;
}
//---------------------------------------------------------------------------
int Val2y(QPixmap *p, float val)
{
  int w = p->width();
  int h = p->height();
  int _y = (1-val)*float(h);
  return _y;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::drawColourBars(QPainter &painter)
{
  if (!this->UnderlayColourPixmap) {
    std::cout << "UnderlayColourPixmap is NULL, why? " << std::endl;
    return;
  }
  QImage Image = this->UnderlayColourPixmap->toImage();
  double xx, yy, intensity;
  for (int p=0; p<nregion; p++)
  {
    double x1 = Val2x(pix,region[p].x);
    double y1 = Val2y(pix,region[p].y);
    double x2 = Val2x(pix,region[p].x+region[p].w);
    double y2 = Val2y(pix,region[p].y+region[p].h);
    double wx = x2-x1;
    double wy = y1-y2;
    double mx = (double)this->UnderlayColourPixmap->width()/this->contentsRect().width();
    double xl = mx*(double)x1;
    double cx = wx/2.0;
    double sx = wx*wx;
    double cy = wy/2.0;
    double sy = wy*wy;
    QRect screenBox(x1, y2, wx, wy);
    QRect sourceBox(xl, 0, mx*wx, 0);
    QImage boxmap(wx, wy, QImage::Format_ARGB32);
    //
    for (double i=0; i<wx; i++) {
      QColor pixel = Image.pixel(xl + i*mx, 0);
      for (double j=0; j<wy; j++) {
        switch (region[p].TFMode) {
          case Uniform:
            intensity = 1.0;
            break;
          case Gaussian:
            xx = (i-cx)*(i-cx)/sx;
            yy = (j-cy)*(j-cy)/sy;
            intensity = exp( (-8.0) * (xx + yy) );
            break;
          case Sine:
						intensity = 0.5 * (1.0 + cos(7.0*M_PI*(i-cx)/cx));
            break;
          case RightHalf:
            xx = (i-2*cx)*(i-2*cx)/sx;
            intensity = exp( -4.0 * xx );
            break;
          case LeftHalf:
            xx = (i)*(i)/sx;
            intensity = exp( -4.0 * xx );
            break;
          case TopHalf:
            yy = (j)*(j)/sy;
            intensity = exp( -4.0 * yy );
            break;
          case BottomHalf:
            yy = (j-2*cy)*(j-2*cy)/sy;
            intensity = exp( -4.0 * yy );
            break;
          case RampRight:
            intensity = 1.0+(i-wx)/wx;
            break;
          case RampLeft:
            intensity = (wx-i)/wx;
            break;
        }
        intensity = region[p].maximum * intensity;
        pixel.setAlpha(static_cast<int>(255.0*intensity + 0.49999));
        boxmap.setPixel(i, j, pixel.rgba());
      }
    }
    painter.drawImage(screenBox, boxmap);
  }
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::drawControlPoints(QPainter &painter)
{
  QPen bluepen (QColor(100,100,255), 2);
  QPen greenpen(QColor(100,255,0),  2);;
  QPen cyanpen (QColor(100,255,255), 2);;
  QPen graypen (QColor(100,100,100), 2);
  QPolygon pts;
  for (int p=0; p<nregion; p++)
  {
    int x1 = Val2x(pix,region[p].x);
    int y1 = Val2y(pix,region[p].y);
    int x2 = Val2x(pix,region[p].x+region[p].w);
    int y2 = Val2y(pix,region[p].y+region[p].h);

    // x1, y1
    if (highlitRegion == p && (movingMode == modeC0 || movingMode == modeAll)) {
        if (mousedown) painter.setPen(greenpen);
        else           painter.setPen(cyanpen);
    }
    else painter.setPen(bluepen);
    pts.setPoints(4, x1-2,y1-2, x1-2,y1+2, x1+2,y1+2, x1+2,y1-2);
    painter.drawPolygon(pts);

    // x1, y2
    if (highlitRegion == p && (movingMode == modeC1 || movingMode == modeAll)) {
        if (mousedown) painter.setPen(greenpen);
        else           painter.setPen(cyanpen);
    }
    else painter.setPen(bluepen);
    pts.setPoints(4, x1-2,y2-2, x1-2,y2+2, x1+2,y2+2, x1+2,y2-2);
    painter.drawPolygon(pts);

    // x2, y2
    if (highlitRegion == p && (movingMode == modeC2 || movingMode == modeAll)) {
        if (mousedown) painter.setPen(greenpen);
        else           painter.setPen(cyanpen);
    }
    else painter.setPen(bluepen);
    pts.setPoints(4, x2-2,y2-2, x2-2,y2+2, x2+2,y2+2, x2+2,y2-2);
    painter.drawPolygon(pts);

    // x2, y1
    if (highlitRegion == p && (movingMode == modeC3 || movingMode == modeAll)) {
        if (mousedown) painter.setPen(greenpen);
        else           painter.setPen(cyanpen);
    }
    else painter.setPen(bluepen);
    pts.setPoints(4, x2-2,y1-2, x2-2,y1+2, x2+2,y1+2, x2+2,y1-2);
    painter.drawPolygon(pts);
  }
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::drawRegions(QPainter &painter)
{
  QColor   white(255, 255, 255 );
  QPen     whitepen(Qt::white, 2);
  QPen     redpen(Qt::red, 2);
  QPen     cyanpen (QColor(100,255,255), 2);;
  QPen     greenpen(QColor(100,255,0),  2);;
  QPolygon pts;
  //
  for (int p=0; p<nregion; p++)
  {
    if (this->activeRegion == p) {
      painter.setPen(redpen);
    }
    else if (highlitRegion == p && movingMode == modeAll) {
      if (mousedown) painter.setPen(greenpen);
      else           painter.setPen(cyanpen);
    }
    else {
      painter.setPen(whitepen);
    }
    int x1 = Val2x(pix,region[p].x);
    int y1 = Val2y(pix,region[p].y);
    int x2 = Val2x(pix,region[p].x+region[p].w);
    int y2 = Val2y(pix,region[p].y+region[p].h);
    pts.setPoints(4, x1,y1, x1,y2, x2,y2, x2,y1);
    painter.drawPolygon(pts);
  }
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::paintToPixmap(int w,int h)
{
  QPainter painter(pix);
  //
  this->paintBackground(painter,w,h);
  //
  this->drawRegions(painter);
  //
  this->drawColourBars(painter);
  //
  this->drawControlPoints(painter);
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::createRGBAData(unsigned char *data)
{
  QImage image(QSize(256, 256), QImage::Format_ARGB32);
  QBrush empty(QColor(0,0,0,0));
  QPainter painter(&image);
  //
  image.fill(0);
  //
  this->drawColourBars(painter);
  //
  if (image.numBytes() == 256*256*4) {
    unsigned char *bits = image.bits();
    int i1 = 0, i2;
    for (int y=0; y<256; y++) {
      for (int x=0; x<256; x++) {
        i2 = y*256*4 + x*4;
        data[i1++] = bits[i2+2];
        data[i1++] = bits[i2+1];
        data[i1++] = bits[i2+0];
        data[i1++] = bits[i2+3];
      }
    }
  }
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::mousePressEvent(QMouseEvent *e)
{
  int _x = e->x();
  int _y = e->y();
  bool activeChanged = false;

  if (e->button() == Qt::RightButton) {
    if (findRegionControlPoint(_x,_y, &highlitRegion, &movingMode)) {
      if (getNumberOfRegions()>minimumNumberOfRegions) {
        removeRegion(highlitRegion);
      }
    }
  }
  else if (e->button() == Qt::LeftButton) {
    int testregion = this->activeRegion;
    if (!findRegionControlPoint(_x,_y, &testregion, &movingMode)) {
      // default is C2, top right corner movement
      this->activeRegion = nregion;
      activeChanged = true;
      highlitRegion = this->activeRegion;
      movingMode    = modeC2;
      if (maximumNumberOfRegions==-1 || getNumberOfRegions()<maximumNumberOfRegions) {
        addRegion(x2val(_x), y2val(_y), 0.01f, 0.01f, this->defaultTFMode, 1.0);
      }
    }
    else if (testregion != this->activeRegion) {
      this->activeRegion = testregion;
      activeChanged = true;
    }
    lastx = _x;
    lasty = _y;
    mousedown = true;
  }
  if (activeChanged) {
    emit this->activeRegionChanged(this->activeRegion);
  }
  this->repaint();
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::mouseMoveEvent(QMouseEvent *e)
{
  int _x = e->x();
  int _y = e->y();

  if (!mousedown) {
    int oldRegion = highlitRegion;
    SelectMode oldMode = movingMode;
    findRegionControlPoint(_x,_y, &highlitRegion, &movingMode);
    if (oldRegion != highlitRegion || oldMode   != movingMode) {
      this->update();
    }
    return;
  }
  int r = this->activeRegion;
  float dx, nx = x2val(_x), lx = x2val(lastx);
  float dy, ny = y2val(_y), ly = y2val(lasty);
  float xmax = 1.0; // x2val(this->contentsRect().width());
  float ymax = 1.0; // y2val(this->contentsRect().height());
  switch (movingMode) {
    case modeC0:
      dx = nx - region[r].x;
      dy = ny - region[r].y;
      region[r].x = nx;
      region[r].y = ny;
      region[r].w = region[r].w-dx;
      region[r].h = region[r].h-dy;
      break;
    case modeC1:
      dx = nx - region[r].x;
      dy = ny - (region[r].y+region[r].h);
      region[r].x = nx;
      region[r].w = region[r].w-dx;
      region[r].h = region[r].h+dy;
      break;
    case modeC2:
      dx = nx - (region[r].x + region[r].w);
      dy = ny - (region[r].y + region[r].h);
      region[r].w = region[r].w+dx;
      region[r].h = region[r].h+dy;
      break;
    case modeC3:
      dx = nx - (region[r].x+region[r].w);
      dy = ny - region[r].y;
      region[r].y = ny;
      region[r].w = region[r].w+dx;
      region[r].h = region[r].h-dy;
      break;
    case modeAll:
      dx = nx - lx;
      dy = ny - ly;
      if ((region[r].x+region[r].w+dx)<xmax &&
          (region[r].y+region[r].h+dy)<ymax &&
          (region[r].x+dx)>=0 &&
          (region[r].y+dy)>=0)
      {
        region[r].x = region[r].x + dx;
        region[r].y = region[r].y + dy;
      }
      break;
    default:
      break;
  }
  // make sure it works when the rectangle is inside out
  if (region[r].w<0) {
    region[r].x += region[r].w;
    region[r].w  =-region[r].w;
    if      (movingMode == modeC0) movingMode = modeC3;  
    else if (movingMode == modeC1) movingMode = modeC2;  
    else if (movingMode == modeC2) movingMode = modeC1;  
    else if (movingMode == modeC3) movingMode = modeC0;  
  }
  if (region[r].h<0) {
    region[r].y += region[r].h;
    region[r].h  =-region[r].h;
    if      (movingMode == modeC0) movingMode = modeC1;  
    else if (movingMode == modeC1) movingMode = modeC0;  
    else if (movingMode == modeC2) movingMode = modeC3;  
    else if (movingMode == modeC3) movingMode = modeC2;  
  }

  lastx = _x;
  lasty = _y;

  this->repaint();
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::mouseReleaseEvent(QMouseEvent *)
{
  mousedown = false;
  this->repaint();
  emit mouseReleased();
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::getRawOpacities(int n, float *opacity)
{
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setActiveRegionMode(int index)
{
  if (this->activeRegion!=-1) {
    region[this->activeRegion].TFMode = TransferFnMode(index);
    this->defaultTFMode = index;
    this->repaint();
  }
}
//-----------------------------------------------------------------------------
int Qvis2DTransferFunctionWidget::getActiveRegionMode()
{
  if (this->activeRegion==-1) return -1;
  return region[this->activeRegion].TFMode;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setActiveRegionMaximum(float mx)
{
  if (this->activeRegion!=-1) {
    region[this->activeRegion].maximum = mx;
    this->repaint();
  }
}
//---------------------------------------------------------------------------
float Qvis2DTransferFunctionWidget::getActiveRegionMaximum()
{
  if (this->activeRegion==-1) return -1;
  return region[this->activeRegion].maximum;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::addRegion(
  float x,float y,float w,float h,float mo,float mx)
{
    region[nregion++] = Region(x,y,w,h,mo,mx);
}
//---------------------------------------------------------------------------
void
Qvis2DTransferFunctionWidget::removeRegion(int n)
{
  for (int i=n; i<nregion-1; i++) region[i] = region[i+1];
  nregion--;
  if (this->activeRegion >= nregion) {
    this->activeRegion = nregion-1;
    emit this->activeRegionChanged(this->activeRegion);
  }
}
//---------------------------------------------------------------------------
void
Qvis2DTransferFunctionWidget::setMaximumNumberOfRegions(int n)
{
  this->maximumNumberOfRegions = n;
}
//---------------------------------------------------------------------------
void
Qvis2DTransferFunctionWidget::setMinimumNumberOfRegions(int n)
{
  this->minimumNumberOfRegions = n;
}
//---------------------------------------------------------------------------
bool Qvis2DTransferFunctionWidget::InRegion(Region &r, float *pt)
{
  if (pt[0]<r.x) return false;
  if (pt[0]>(r.x+r.w)) return false;
  if (pt[1]<r.y) return false;
  if (pt[1]>(r.y+r.h)) return false;
  return true;
}
//---------------------------------------------------------------------------
#define dist2(x1,y1,x2,y2) (((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)))
//---------------------------------------------------------------------------
bool Qvis2DTransferFunctionWidget::findRegionControlPoint(
  int _x,int _y, int *newregion, SelectMode *newmode)
{
  if (this->highlitRegion!=-1) {
//    this->activeRegion = this->highlitRegion;
  }
  *newregion = -1;
  *newmode   = modeNone;
  bool  found   = false;
  float mindist = 100000;  // it's okay, it's pixels
  for (int p=0; p<nregion; p++)
  {
    int x1 = val2x(region[p].x);
    int y1 = val2y(region[p].y);
    int x2 = val2x(region[p].x+region[p].w);
    int y2 = val2y(region[p].y+region[p].h);
    int y0 = val2y(0);
    float pt[2] = { x2val(_x), y2val(_y) };

    float d1 = dist2(_x,_y, x1,y1);
    float d2 = dist2(_x,_y, x1,y2);
    float d3 = dist2(_x,_y, x2,y2);
    float d4 = dist2(_x,_y, x2,y1);
    float rad = 8*8;

    if (d1 < rad && mindist > d1)
    {
      *newregion = p;
      *newmode   = modeC0;
      mindist    = d1;
      found      = true;
    }
    if (d2 < rad && mindist > d2)
    {
      *newregion = p;
      *newmode   = modeC1;
      mindist    = d2;
      found      = true;
    }
    if (d3 < rad && mindist > d3)
    {
      *newregion = p;
      *newmode   = modeC2;
      mindist    = d3;
      found      = true;
    }
    if (d4 < rad && mindist > d4)
    {
      *newregion = p;
      *newmode   = modeC3;
      mindist    = d4;
      found      = true;
    }
    else if (InRegion(region[p], pt)) 
    {
      *newregion = p;
      *newmode   = modeAll;
      found      = true;
    }
  }
  return found;
}
//---------------------------------------------------------------------------
int Qvis2DTransferFunctionWidget::getNumberOfRegions()
{
  return nregion;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::getRegion(int i,
  float *x, float *y, float *w, float *h, float *mo, float *mx)
{
  *x  = region[i].x;
  *y  = region[i].y;
  *w  = region[i].w;
  *h  = region[i].h;
  *mo = region[i].TFMode;
  *mx = region[i].maximum;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setRegion(int i,
  float *x, float *y, float *w, float *h, float *mo, float *mx)
{
  region[i].x = *x;
  region[i].y = *y;
  region[i].w = *w;
  region[i].h = *h;
  region[i].TFMode  = TransferFnMode(static_cast<int>(*mo));
  region[i].maximum = *mx;
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setAllRegions(int n, float *regiondata)
{
  nregion = 0;
  for (int i=0; i<n; i++)
  {
    addRegion(regiondata[i*5 + 0], regiondata[i*5 + 1],
              regiondata[i*5 + 2], regiondata[i*5 + 3],
              regiondata[i*5 + 4], regiondata[i*5 + 5]);
  }
  this->update();
}
//-----------------------------------------------------------------------------
