
#define _USE_MATH_DEFINES
#include "Qvis2DTransferFunctionWidget.h"


#include <qpainter.h>
#include <qpolygon.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qnamespace.h>
#include <QMouseEvent>
#include "vtkEventQtSlotConnect.h"
#include "vtkColorTransferFunction.h"
#include "vtkScalarsToColors.h"
#include "vtkMath.h"

#include <iostream>
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
    currentbackgroundOpacityValuesSize = 0;
    this->backgroundOpacityValues = 0;
    this->paintScalarColorBackground = true;
    this->colortransferfunction = 0;
    lasty = 0;
    lastx = 0;
    this->histogramBackground = NULL;

    // set a default:
   // this->addRegion(0.25f, 0.25f, 0.5f, 0.5f, this->defaultTFMode, 1.0);

    this->mousedown = false;
    this->setMouseTracking(true);
}
//---------------------------------------------------------------------------
Qvis2DTransferFunctionWidget::~Qvis2DTransferFunctionWidget()
{
  if (this->UnderlayColourPixmap) delete this->UnderlayColourPixmap;
}

//---------------------------------------------------------------------------

void Qvis2DTransferFunctionWidget::initialize(vtkTwoDTransferFunction* function, vtkScalarsToColors* stc){
	if(function){
			this->transferFunction = function;
			this->nregion = this->transferFunction->GetSize();
		}
	if(stc)
	  {
	  this->colortransferfunction = vtkColorTransferFunction::SafeDownCast(stc);
	  this->paintScalarColorBackground = true;
	  this->showBackgroundPixmap = true;

	  this->VTKConnect->Connect(
	    colortransferfunction, vtkCommand::ModifiedEvent, this, SLOT(updateImage()));
	  }

}

void Qvis2DTransferFunctionWidget::updateImage(){
  this->update();
}

//----------------------------------------------------------------------------

//this Section TBD

int
Qvis2DTransferFunctionWidget::addRegion(double _x,double _y, double _w, double _h, TransferFnMode mode, double max)
{
  double xshift = this->transferFunction->GetRangeAtIndex(0);
  double yshift = this->transferFunction->GetRangeAtIndex(2);
	double functionX = _x*this->transferFunction->getXRange() + xshift;
	double functionY = _y*this->transferFunction->getYRange() + yshift;
	double functionW = _w*this->transferFunction->getXRange();
	double functionH = _h*this->transferFunction->getYRange();
	return this->transferFunction->AddRegion(functionX,functionY,functionW,functionH,mode,max);

}



Qvis2DTransferFunctionWidget::Region Qvis2DTransferFunctionWidget::getRegion(int index){
  double xshift = this->transferFunction->GetRangeAtIndex(0);
    double yshift = this->transferFunction->GetRangeAtIndex(2);
    double regionX = this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_X)/this->transferFunction->getXRange() - xshift/this->transferFunction->getXRange();
    double regionY = this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_Y)/this->transferFunction->getYRange() - yshift/this->transferFunction->getYRange();
    double regionW = this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_W)/this->transferFunction->getXRange();
    double regionH = this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_H)/this->transferFunction->getYRange();
    double max = this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_MAX);
    TransferFnMode mode = this->transferFunction->getRegionMode(index);


	return Region(regionX,regionY,regionW,regionH,mode,max);

}


/*
 *   ((p-r1Min)/(r1Max-r1Min))*(r2Max-r2Min)+r2Min
      ((p-0)/(1))*(r2Max-r2Min)+r2Min
 */

//TBD convert space
double Qvis2DTransferFunctionWidget::getRegionValue(int index, vtkTwoDTransferFunction::regionvalue value){
  double xshift = this->transferFunction->GetRangeAtIndex(0);
      double yshift = this->transferFunction->GetRangeAtIndex(2);
	if (value == vtkTwoDTransferFunction::REGION_MODE)
			return this->transferFunction->getRegionMode(index);
		else if(value == vtkTwoDTransferFunction::REGION_X)
			return this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_X)/this->transferFunction->getXRange()-xshift/this->transferFunction->getXRange();///this->transferFunction->getXRange()*double(this->contentsRect().width());
		else if (value == vtkTwoDTransferFunction::REGION_Y)
			return this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_Y)/this->transferFunction->getYRange()- yshift/this->transferFunction->getYRange();///this->transferFunction->getYRange()*double(this->contentsRect().height());
		else if (value == vtkTwoDTransferFunction::REGION_W)
			return this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_W)/this->transferFunction->getXRange();///this->transferFunction->getXRange()*double(this->contentsRect().width());
		else if (value == vtkTwoDTransferFunction::REGION_H)
			return this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_H)/this->transferFunction->getYRange();///this->transferFunction->getYRange()*double(this->contentsRect().height());
		else if (value == vtkTwoDTransferFunction::REGION_MAX)
			return this->transferFunction->getValue(index,vtkTwoDTransferFunction::REGION_MAX);
		else
			return -1;

}

void  Qvis2DTransferFunctionWidget::setRegion(int index, Qvis2DTransferFunctionWidget::Region &reg){
	double xshift = this->transferFunction->GetRangeAtIndex(0);
        double yshift = this->transferFunction->GetRangeAtIndex(2);
        double values[5];
	values[0] = reg.x*this->transferFunction->getXRange()+xshift;
	values[1] = reg.y*this->transferFunction->getYRange()+yshift;
	values[2] = reg.w*this->transferFunction->getXRange();
	values[3] = reg.h*this->transferFunction->getYRange();
	//
	values[4] = reg.maximum;
	this->transferFunction->SetRegionValues(index,values,reg.TFMode);

}


//TBD convert space
void Qvis2DTransferFunctionWidget::setRegionValue(int index, double value, vtkTwoDTransferFunction::regionvalue v){

  double xshift = this->transferFunction->GetRangeAtIndex(0);
  double yshift = this->transferFunction->GetRangeAtIndex(2);

	if(v == vtkTwoDTransferFunction::REGION_X)
		this->transferFunction->SetRegionValue(index, value*this->transferFunction->getXRange() + xshift, v);
	else if (v == vtkTwoDTransferFunction::REGION_Y)
		this->transferFunction->SetRegionValue(index, value*this->transferFunction->getYRange() + yshift, v);
	else if (v == vtkTwoDTransferFunction::REGION_W)
		this->transferFunction->SetRegionValue(index, value*this->transferFunction->getXRange(), v);
	else if (v == vtkTwoDTransferFunction::REGION_H)
		this->transferFunction->SetRegionValue(index, value*this->transferFunction->getYRange(), v);
	else if (v == vtkTwoDTransferFunction::REGION_MAX)
		this->transferFunction->SetRegionValue(index, value, v);
	else if (v == vtkTwoDTransferFunction::REGION_MODE)
    this->transferFunction->SetRegionMode(index, TransferFnMode(vtkMath::Round(value)));
}

void Qvis2DTransferFunctionWidget::setRegionMode(int index, TransferFnMode mo){
	this->transferFunction->SetRegionMode(index,mo);



}


void Qvis2DTransferFunctionWidget::setFunctionRange(double range[2]){
	//this->transf->UpdateRange(false, range);
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

void Qvis2DTransferFunctionWidget::setCurrentRegion(int index){
  this->activeRegion = index;
}

//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::drawColourBars(QPainter &painter)
{
  if (!this->UnderlayColourPixmap) {
    std::cout << "UnderlayColourPixmap is NULL, why? " << std::endl;
    return;
  }

  if (this->transferFunction->GetSize() <= 0)
	  return;

  QImage Image = this->UnderlayColourPixmap->toImage();
  double xx, yy, intensity;
  for (int p=0; p<nregion; p++)
  {
    double x1 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X));
    double y1 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y));
    double x2 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X)+getRegionValue(p,vtkTwoDTransferFunction::REGION_W));
    double y2 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(p,vtkTwoDTransferFunction::REGION_H));
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
        switch ((TransferFnMode) int(getRegionValue(p,vtkTwoDTransferFunction::REGION_MODE)+0.1)) {
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
        intensity = getRegionValue(p,vtkTwoDTransferFunction::REGION_MAX) * intensity;
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
    int x1 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X));
    int y1 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y));
    int x2 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X)+getRegionValue(p,vtkTwoDTransferFunction::REGION_W));
    int y2 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(p,vtkTwoDTransferFunction::REGION_H));

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
  QColor   black(0, 0, 0 );
  QPen     blackpen(Qt::black, 2);
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
      painter.setPen(blackpen);
    }
    int x1 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X));
    int y1 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y));
    int x2 = Val2x(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_X)+getRegionValue(p,vtkTwoDTransferFunction::REGION_W));
    int y2 = Val2y(pix,getRegionValue(p,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(p,vtkTwoDTransferFunction::REGION_H));
    pts.setPoints(4, x1,y1, x1,y2, x2,y2, x2,y1);
    painter.drawPolygon(pts);
  }
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::paintToPixmap(int w,int h)
{
  QPainter painter(pix);
  //
  this->createScalarColorBackground();

  this->paintBackground(painter,w,h);
  //
  this->drawRegions(painter);
  //
 // this->drawColourBars(painter);
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
void Qvis2DTransferFunctionWidget::createScalarColorBackground()
  {
  double currentRangeMinX, currentRangeMaxX, currentRangeMinY, currentRangeMaxY;
  this->transferFunction->GetRange(currentRangeMinX, currentRangeMaxX, currentRangeMinY, currentRangeMaxY);


  const unsigned char * c = this->colortransferfunction->GetTable(currentRangeMinX, currentRangeMaxX,this->contentsRect().width());

  int size = this->contentsRect().width()*this->contentsRect().height();

  if (size != this->currentbackgroundOpacityValuesSize){
    if(this->backgroundOpacityValues)
      delete backgroundOpacityValues;
    this->backgroundOpacityValues = new double[size];
  }

  //we definitely need something to check if the function has changed
  this->transferFunction->GetTable(currentRangeMinX,currentRangeMaxX,currentRangeMinY,currentRangeMaxY,
      this->contentsRect().width(),this->contentsRect().height(),this->backgroundOpacityValues);




  QImage image(QSize(this->contentsRect().width(),this->contentsRect().height()), QImage::Format_RGB32);
  image.fill(Qt::white);

  if (this->histogramBackground)
    {
  QImage scaledhisto = QImage(this->histogramBackground->scaled(this->contentsRect().width(),this->contentsRect().height(),
      Qt::IgnoreAspectRatio,Qt::FastTransformation));

    for (int i = 0; i< this->contentsRect().width(); i++)
      {
      for (int j = 0; j< this->contentsRect().height(); j++)
        {
        int r = (int)c[i*3]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
            255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
        r = int (double(255-qRed(scaledhisto.pixel(i,j)))/255.0*double(r));/* +
            this->backgroundOpacityValues[i*this->contentsRect().height()+j]*double(r);*/
        /*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j])*double(qRed(scaledhisto.pixel(i,j)));/* +
            this->backgroundOpacityValues[i*this->contentsRect().height()+j]*double(r);*/

        int g = (int)c[i*3+1]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
            255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
        g = int (double(255-qGreen(scaledhisto.pixel(i,j)))/255.0*double(g));/* +
            this->backgroundOpacityValues[i*this->contentsRect().height()+j]*double(g);*/

        int b = (int)c[i*3+2]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
            255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
        b = int (double(255-qBlue(scaledhisto.pixel(i,j)))/255.0*double(b));/* +
            this->backgroundOpacityValues[i*this->contentsRect().height()+j]*double(b);*/

        image.setPixel(i,this->contentsRect().height()-j-1,qRgb(r,g,b));
        }
      }
    }
  else
    {
    for (int i = 0; i< this->contentsRect().width(); i++)
         {
         for (int j = 0; j< this->contentsRect().height(); j++)
           {
            int r = (int)c[i*3]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
                        255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
            int g = (int)c[i*3+1]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
                255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
            int b = (int)c[i*3+2]*this->backgroundOpacityValues[i*this->contentsRect().height()+j]+
                255*(1-this->backgroundOpacityValues[i*this->contentsRect().height()+j]);
            image.setPixel(i,this->contentsRect().height()-j-1,qRgb(r,g,b));
            }
         }
    }


  QPixmap* background = new QPixmap(
      QPixmap::fromImage(
        image));

  this->stretchBackgroundPixmap = false;

    this->SetBackgroundPixmap(background);

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
        emit controlPointsModified();
      }
    }
    else if (testregion != this->activeRegion) {
      this->activeRegion = testregion;
      activeChanged = true;
      emit activeRegionChanged(this->activeRegion);
    }
    lastx = _x;
    lasty = _y;
    mousedown = true;
  }
  if (activeChanged) {
    emit this->activeRegionChanged(this->activeRegion);
  }
  nregion = this->transferFunction->GetSize();
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
      dx = nx - getRegionValue(r,vtkTwoDTransferFunction::REGION_X);
      dy = ny - getRegionValue(r,vtkTwoDTransferFunction::REGION_Y);
      setRegionValue(r,nx, vtkTwoDTransferFunction::REGION_X);
      setRegionValue(r,ny, vtkTwoDTransferFunction::REGION_Y);
      setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_W)-dx, vtkTwoDTransferFunction::REGION_W);
      setRegionValue(r, getRegionValue(r,vtkTwoDTransferFunction::REGION_H)-dy, vtkTwoDTransferFunction::REGION_H);
      break;
    case modeC1:
      dx = nx - getRegionValue(r,vtkTwoDTransferFunction::REGION_X);
      dy = ny - (getRegionValue(r,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(r,vtkTwoDTransferFunction::REGION_H));
      setRegionValue(r,nx, vtkTwoDTransferFunction::REGION_X);
      setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_W)-dx, vtkTwoDTransferFunction::REGION_W);
      setRegionValue(r, getRegionValue(r,vtkTwoDTransferFunction::REGION_H)+dy, vtkTwoDTransferFunction::REGION_H);
      break;
    case modeC2:
      dx = nx - (getRegionValue(r,vtkTwoDTransferFunction::REGION_X) + getRegionValue(r,vtkTwoDTransferFunction::REGION_W));
      dy = ny - (getRegionValue(r,vtkTwoDTransferFunction::REGION_Y) + getRegionValue(r,vtkTwoDTransferFunction::REGION_H));
      setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_W)+dx, vtkTwoDTransferFunction::REGION_W);
      setRegionValue(r, getRegionValue(r,vtkTwoDTransferFunction::REGION_H)+dy, vtkTwoDTransferFunction::REGION_H);
      break;
    case modeC3:
      dx = nx - (getRegionValue(r,vtkTwoDTransferFunction::REGION_X) + getRegionValue(r,vtkTwoDTransferFunction::REGION_W));
      dy = ny - getRegionValue(r,vtkTwoDTransferFunction::REGION_Y);
      setRegionValue(r,ny, vtkTwoDTransferFunction::REGION_Y);
      setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_W)+dx, vtkTwoDTransferFunction::REGION_W);
      setRegionValue(r, getRegionValue(r,vtkTwoDTransferFunction::REGION_H)-dy, vtkTwoDTransferFunction::REGION_H);
      break;
    case modeAll:
      dx = nx - lx;
      dy = ny - ly;
      if ((getRegionValue(r,vtkTwoDTransferFunction::REGION_X)+getRegionValue(r,vtkTwoDTransferFunction::REGION_W)+dx)<xmax &&
          (getRegionValue(r,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(r,vtkTwoDTransferFunction::REGION_H)+dy)<ymax &&
          (getRegionValue(r,vtkTwoDTransferFunction::REGION_X)+dx)>=0 &&
          (getRegionValue(r,vtkTwoDTransferFunction::REGION_Y)+dy)>=0)
      {
    	  setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_X)+ dx, vtkTwoDTransferFunction::REGION_X);
    	  setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_Y)+ dy, vtkTwoDTransferFunction::REGION_Y);
      }
      break;
    default:
      break;
  }
  // make sure it works when the rectangle is inside out
  if (getRegionValue(r,vtkTwoDTransferFunction::REGION_W)<0) {
	  setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_X)+ getRegionValue(r,vtkTwoDTransferFunction::REGION_W),
			  vtkTwoDTransferFunction::REGION_X);
	  setRegionValue(r,-getRegionValue(r,vtkTwoDTransferFunction::REGION_W),vtkTwoDTransferFunction::REGION_W);
    if      (movingMode == modeC0) movingMode = modeC3;  
    else if (movingMode == modeC1) movingMode = modeC2;  
    else if (movingMode == modeC2) movingMode = modeC1;  
    else if (movingMode == modeC3) movingMode = modeC0;  
  }
  if (getRegionValue(r,vtkTwoDTransferFunction::REGION_H)<0) {
	  setRegionValue(r,getRegionValue(r,vtkTwoDTransferFunction::REGION_Y)+ getRegionValue(r,vtkTwoDTransferFunction::REGION_H),
	  			  vtkTwoDTransferFunction::REGION_Y);
	  setRegionValue(r,-getRegionValue(r,vtkTwoDTransferFunction::REGION_H),vtkTwoDTransferFunction::REGION_H);
    if      (movingMode == modeC0) movingMode = modeC1;  
    else if (movingMode == modeC1) movingMode = modeC0;  
    else if (movingMode == modeC2) movingMode = modeC3;  
    else if (movingMode == modeC3) movingMode = modeC2;  
  }

  emit controlPointsModified();

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
    setRegionValue(this->activeRegion,TransferFnMode(index),vtkTwoDTransferFunction::REGION_MODE);
    this->defaultTFMode = index;
    emit controlPointsModified();
    this->repaint();
  }
}
//-----------------------------------------------------------------------------
int Qvis2DTransferFunctionWidget::getActiveRegionMode()
{
  if (this->activeRegion==-1) return -1;
  return getRegionValue(this->activeRegion,vtkTwoDTransferFunction::REGION_MODE);
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setActiveRegionMaximum(float mx)
{
  if (this->activeRegion!=-1) {
    setRegionValue(this->activeRegion,mx,vtkTwoDTransferFunction::REGION_MAX);
    emit controlPointsModified();
    this->repaint();
  }
}
//---------------------------------------------------------------------------
float Qvis2DTransferFunctionWidget::getActiveRegionMaximum()
{
  if (this->activeRegion <0 || this->activeRegion > this->transferFunction->GetSize())
    return -1;
  return this->transferFunction->getValue(this->activeRegion,vtkTwoDTransferFunction::REGION_MAX);
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::addRegion(
  float x,float y,float w,float h,float mo,float mx)
{
	addRegion(double(x),double(y),double(w),double(h),(TransferFnMode) int(mo), double(mx));
    //region[nregion++] = Region(x,y,w,h,mo,mx);
}
//---------------------------------------------------------------------------
void
Qvis2DTransferFunctionWidget::removeRegion(int n)
{
	this->transferFunction->RemoveRegionAtIndex(n);
	if (this->transferFunction->GetSize()>0)
	emit this->activeRegionChanged(0);
/*
  for (int i=n; i<nregion-1; i++) region[i] = region[i+1];
  nregion--;
  if (this->activeRegion >= nregion) {
    this->activeRegion = nregion-1;
    emit this->activeRegionChanged(this->activeRegion);
  }*/
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
bool Qvis2DTransferFunctionWidget::InRegion(int index, float *pt)
{
  if (pt[0]<getRegionValue(index,vtkTwoDTransferFunction::REGION_X)) return false;
  if (pt[0]>getRegionValue(index,vtkTwoDTransferFunction::REGION_X) + getRegionValue(index,vtkTwoDTransferFunction::REGION_W)) return false;
  if (pt[1]<getRegionValue(index,vtkTwoDTransferFunction::REGION_Y)) return false;
  if (pt[1]>getRegionValue(index,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(index,vtkTwoDTransferFunction::REGION_H)) return false;
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
    int x1 = val2x(getRegionValue(p,vtkTwoDTransferFunction::REGION_X));
    int y1 = val2y(getRegionValue(p,vtkTwoDTransferFunction::REGION_Y));
    int x2 = val2x(getRegionValue(p,vtkTwoDTransferFunction::REGION_X)+getRegionValue(p,vtkTwoDTransferFunction::REGION_W));
    int y2 = val2y(getRegionValue(p,vtkTwoDTransferFunction::REGION_Y)+getRegionValue(p,vtkTwoDTransferFunction::REGION_H));
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
    else if (InRegion(p, pt))
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
  *x  = getRegionValue(i,vtkTwoDTransferFunction::REGION_X);
  *y  = getRegionValue(i,vtkTwoDTransferFunction::REGION_Y);
  *w  = getRegionValue(i,vtkTwoDTransferFunction::REGION_W);
  *h  = getRegionValue(i,vtkTwoDTransferFunction::REGION_H);
  *mo = this->transferFunction->getRegionMode(i);
  *mx = getRegionValue(i,vtkTwoDTransferFunction::REGION_MAX);
}
//---------------------------------------------------------------------------
void Qvis2DTransferFunctionWidget::setRegion(int i,
  float *x, float *y, float *w, float *h, float *mo, float *mx)
{
	Region r(*x,*y,*w,*h,TransferFnMode(static_cast<int>(*mo)),*mx);
	setRegion(i,r);
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
void Qvis2DTransferFunctionWidget::generateHistogramBackground(int width, int height, int* array)
{
  //find max
  int size = width * height;
  int max = 0;
  for (int i = 0; i< size; i++)
    {
    max = array[i]>max ? array[i] : max;
    }
  float maxf = float(max);

  if (histogramBackground)
    delete histogramBackground;
  histogramBackground = new QImage(width, height, QImage::Format_RGB32);
  histogramBackground->fill(qRgb(255,255,255));
  for (int i = 0; i< width; i++)
    {
    for (int j = 0; j<height; j++)
      {
      int v = int(255.0f*float(array[i*height+j])/maxf);
      histogramBackground->setPixel(i,j,qRgb(v,v,v));
      }
    }

 // QPixmap* bg = new QPixmap(QPixmap::fromImage(*(this->histogramBackground)));

 // this->SetBackgroundPixmap(bg);
}
