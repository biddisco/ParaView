/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "QvisGaussianOpacityBar.h"
#include "vtkGaussianPiecewiseFunction.h"

#include <qpainter.h>
#include <qpolygon.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qnamespace.h>
#include <QMouseEvent>

#include <iostream>
#include <cmath>
#include <cstdlib>









// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::QvisGaussianOpacityBar
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************

QvisGaussianOpacityBar::QvisGaussianOpacityBar(QWidget *parentObject, const char *name)
    : QvisAbstractOpacityBar(parentObject, name)
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
    setLineWidth( 2 );
    setMinimumHeight(50);
    setMinimumWidth(128);
    ngaussian = 0;
    currentGaussian = 0;
    currentMode     = modeNone;
    maximumNumberOfGaussians = -1; // unlimited
    minimumNumberOfGaussians =  0;

    lasty = -1;
    lastx = -1;

    // set a default:
    //addGaussian(0.5f, 0.5f, 0.1f, 0.0f, 0);
    //if we add a default in the constructor, it currently crashes

    mousedown = false;
    setMouseTracking(true);
}




// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::~QvisGaussianOpacityBar
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************

QvisGaussianOpacityBar::~QvisGaussianOpacityBar()
{
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::drawControlPoints
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************


void QvisGaussianOpacityBar::initialize(vtkGaussianPiecewiseFunction* gpwf){
	if(gpwf){
		this->gaussianFunctionGroup = gpwf;
		this->ngaussian = this->gaussianFunctionGroup->GetSize();
	}
}


/*
void QvisGaussianOpacityBar::preparePointsForDrawing(std::vector<Gaussian> &gaussians){
	//there are 2
	int ngaussians = gaussianFunctionGroup->GetSize();
	Gaussian gauss;
	for (int i = 0; i<ngaussians; i++){convertGaussianToDataSpace(i, gauss);
	gaussians.push_back(gauss);//the vector should be making copies when it pushes back -> we can reuse gauss
	}

}
*/


void
QvisGaussianOpacityBar::drawControlPoints(QPainter &painter)
{
	//there are 2 ways for drawing the widget. The first is by getting the context view and rendering and stuff like in the pqtransferfunctionwidget.
	//Because I don't really care to figure out how that works at the moment (and I want to get done with this) it's not being done like that.
	//Instead, I'm choosing the less efficient way of moving converting all the gaussians into "image space".
    int pw = pix->width();
    int ph = pix->height();
    QPen bluepen(QColor(100,100,255), 2);
    QPen greenpen(QColor(100,255,0),  2);;
    QPen cyanpen(QColor(100,255,255), 2);;
    QPen graypen(QColor(100,100,100), 2);
    QPolygon pts;

   // preparePointsForDrawing(gaussian);
    if (!gaussianFunctionGroup)
    	return;
    ngaussian = gaussianFunctionGroup->GetSize();

    for (int p=0; p<ngaussian; p++)
    {
        int _x  = int(float(getGaussValue(p,gaussX)+getGaussValue(p,gaussBx))*float(pw));
        int xr = int(float(getGaussValue(p,gaussX)+getGaussValue(p,gaussW))*float(pw));
        int xl = int(float(getGaussValue(p,gaussX)-getGaussValue(p,gaussW))*float(pw));
        int _y  = int(float(1-getGaussValue(p,gaussH))*float(ph));
        int y0 = int(float(1-0)*float(ph));
        int yb = int(float(1-getGaussValue(p,gaussH)/4. - getGaussValue(p,gaussBy)*getGaussValue(p,gaussH)/4.)*float(ph));

        // lines:
        painter.setPen(graypen);
        painter.drawLine(_x,y0-2, _x,_y);
        painter.drawLine(xl,y0-2, xr,y0-2);

        // square: position
        if (currentGaussian == p && currentMode == modeX)
        {
            if (mousedown)
                painter.setPen(greenpen);
            else
                painter.setPen(cyanpen);
        }
        else
            painter.setPen(bluepen);
        pts.setPoints(4, _x-4,y0, _x-4,y0-4, _x+4,y0-4, _x+4,y0);
        painter.drawPolyline(pts);

        // diamond: bias (horizontal and vertical)
        if (currentGaussian == p && currentMode == modeB)
        {
            if (mousedown)
                painter.setPen(greenpen);
            else
                painter.setPen(cyanpen);
        }
        else
            painter.setPen(bluepen);
        float bx = getGaussValue(p,gaussBx);
        float by = getGaussValue(p,gaussBy);
        painter.drawLine(_x,yb, _x,yb+5);
        if (bx > 0)
        {
            painter.drawLine(_x,yb-5, _x+5,yb);
            painter.drawLine(_x,yb+5, _x+5,yb);
        }
        else
        {
            painter.drawLine(_x,yb, _x+5,yb);
        }
        if (bx < 0)
        {
            painter.drawLine(_x,yb-5, _x-5,yb);
            painter.drawLine(_x,yb+5, _x-5,yb);
        }
        else
        {
            painter.drawLine(_x-5,yb, _x,yb);
        }
        if (by > 0)
        {
            painter.drawLine(_x,yb-5, _x-5,yb);
            painter.drawLine(_x,yb-5, _x+5,yb);
        }
        else
        {
            painter.drawLine(_x,yb-5, _x,yb);
        }

        // up triangle: height
        if (currentGaussian == p && currentMode == modeH)
        {
            if (mousedown)
                painter.setPen(greenpen);
            else
                painter.setPen(cyanpen);
        }
        else
            painter.setPen(bluepen);
        pts.setPoints(4, _x+5,_y, _x,_y-5, _x-5,_y, _x+5,_y);
        painter.drawPolyline(pts);

        // triangle: width (R)
        if (currentGaussian == p && (currentMode == modeWR || currentMode == modeW))
        {
            if (mousedown)
                painter.setPen(greenpen);
            else
                painter.setPen(cyanpen);
        }
        else
            painter.setPen(bluepen);
        pts.setPoints(3, xr,y0, xr,y0-6, xr+6,y0);
        painter.drawPolyline(pts);

        // triangle: width (L)
        if (currentGaussian == p && (currentMode == modeWL  || currentMode == modeW))
        {
            if (mousedown)
                painter.setPen(greenpen);
            else
                painter.setPen(cyanpen);
        }
        else
            painter.setPen(bluepen);
        pts.setPoints(3, xl,y0, xl,y0-6, xl-6,y0);
        painter.drawPolyline(pts);
    }
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::paintToPixmap
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::paintToPixmap(int w,int h)
{
  float *values = new float[w];
  getRawOpacities(w,values);

  QColor white(255, 255, 255 );
  QColor black(0,   0,   0 );
  QPen   whitepen(Qt::white, 2);

  QPainter painter(pix);
  this->paintBackground(painter,w,h);

  float dy = 1.0/float(h-1);
  for (int _x=0; _x<w; _x++)
  {
    float yval1 = values[_x];
    float yval2 = values[_x+1];
    painter.setPen(whitepen);
    for (int _y=0; _y<h; _y++)
    {
      float yvalc = 1 - float(_y)/float(h-1);
      if (yvalc >= qMin(yval1,yval2)-dy && yvalc < qMax(yval1,yval2))
      {
        painter.drawPoint(_x,_y);
      }
    }
  }
  delete[] values;

  this->drawControlPoints(painter);
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::mousePressEvent
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::mousePressEvent(QMouseEvent *e)
{

    int _x = e->x();
    int _y = e->y();

    if (e->button() == Qt::RightButton)
    {
        if (findGaussianControlPoint(_x,_y, &currentGaussian, &currentMode))
        {
            if (getNumberOfGaussians()>minimumNumberOfGaussians)
            {
                removeGaussian(currentGaussian);
            }
        }
    }
    else if (e->button() == Qt::LeftButton)
    {
        if (! findGaussianControlPoint(_x,_y,
                                       &currentGaussian, &currentMode))
        {
            //currentGaussian = ngaussian;
            currentMode     = modeW;
            if (maximumNumberOfGaussians==-1 || getNumberOfGaussians()<maximumNumberOfGaussians)
            {
            	currentGaussian = addGaussian(double(x2val(_x)),double(y2val(_y)), 0.001, 0,0);
            }
        }
        lastx = _x;
        lasty = _y;
        mousedown = true;
    }


//    this->paintToPixmap(contentsRect().width(), contentsRect().height());
    this->repaint();
//    QPainter p(this);
//    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::mouseMoveEvent
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::mouseMoveEvent(QMouseEvent *e)
{
    int _x = e->x();
    int _y = e->y();

    if (!mousedown)
    {
        int  oldGaussian = currentGaussian;
        Mode oldMode     = currentMode;
        findGaussianControlPoint(_x,_y, &currentGaussian, &currentMode);
        if (oldGaussian != currentGaussian ||
            oldMode     != currentMode)
        {
//            drawControlPoints();
    this->update();
//            QPainter p(this);
//            p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);
        }
        return;
    }

    switch (currentMode)
    {
      case modeX:
        setGaussValue(currentGaussian,x2val(_x) - getGaussValue(currentGaussian,gaussBx),gaussX);
        emit currentPointChanged(currentGaussian);
        break;
      case modeH:
        setGaussValue(currentGaussian,y2val(_y),gaussH);
        emit currentPointChanged(currentGaussian);
        break;
      case modeW:
        setGaussValue(currentGaussian,qMax((float)fabs(x2val(_x) - getGaussValue(currentGaussian,gaussX)),(float)0.01),gaussW);
        emit currentPointChanged(currentGaussian);
        break;
      case modeWR:
        setGaussValue(currentGaussian,qMax((double)(x2val(_x) - getGaussValue(currentGaussian,gaussX)),0.01),gaussW);
        if (getGaussValue(currentGaussian,gaussW) < fabs(getGaussValue(currentGaussian,gaussBx)))
            setGaussValue(currentGaussian,fabs(getGaussValue(currentGaussian,gaussBx)),gaussW);
        emit currentPointChanged(currentGaussian);
        break;
      case modeWL:
        setGaussValue(currentGaussian,qMax((float)(getGaussValue(currentGaussian,gaussX) - x2val(_x)),(float)0.01),gaussW);
        if (getGaussValue(currentGaussian,gaussW) < fabs(getGaussValue(currentGaussian,gaussBx)))
            setGaussValue(currentGaussian,fabs(getGaussValue(currentGaussian,gaussBx)),gaussW);
        emit currentPointChanged(currentGaussian);
        break;
      case modeB:
        setGaussValue(currentGaussian,x2val(_x) - getGaussValue(currentGaussian,gaussX),gaussBx);
        if (getGaussValue(currentGaussian,gaussBx) > getGaussValue(currentGaussian,gaussW))
            setGaussValue(currentGaussian,getGaussValue(currentGaussian,gaussW),gaussBx);
        if (getGaussValue(currentGaussian,gaussBx) < -getGaussValue(currentGaussian,gaussW))
            setGaussValue(currentGaussian,-getGaussValue(currentGaussian,gaussW),gaussBx);
        if (fabs(getGaussValue(currentGaussian,gaussBx)) < .001)
            setGaussValue(currentGaussian,0,gaussBx);

        setGaussValue(currentGaussian,4*(y2val(_y) - getGaussValue(currentGaussian,gaussH)/4.)/getGaussValue(currentGaussian,gaussH),gaussBy);
        if (getGaussValue(currentGaussian,gaussBy) > 2)
            setGaussValue(currentGaussian,2,gaussBy);
        if (getGaussValue(currentGaussian,gaussBy) < 0)
            setGaussValue(currentGaussian,0,gaussBy);
        emit currentPointChanged(currentGaussian);
        break;
      default:
        break;
    }
    lastx = _x;
    lasty = _y;

//    this->paintToPixmap(contentsRect().width(), contentsRect().height());
    this->repaint();
//    QPainter p(this);
//    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);

//    emit mouseMoved();
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::mouseReleaseEvent
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::mouseReleaseEvent(QMouseEvent *)
{
    mousedown = false;

//    this->paintToPixmap(contentsRect().width(), contentsRect().height());
    this->repaint();
//    QPainter p(this);
//    p.drawPixmap(contentsRect().left(),contentsRect().top(),*pix);

    emit mouseReleased();
    emit controlPointsModified();
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::getRawOpacities
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::getRawOpacities(int n, float *opacity)
{
    for (int i=0; i<n; i++) opacity[i] = 0;

    ngaussian = gaussianFunctionGroup->GetSize();
    for (int p=0; p<ngaussian; p++)
    {
        float _pos    = getGaussValue(p,gaussX);
        float _width  = getGaussValue(p,gaussW);
        float _height = getGaussValue(p,gaussH);
        float xbias  = getGaussValue(p,gaussBx);
        float ybias  = getGaussValue(p,gaussBy);
        for (int i=0; i<n/*+1*/; i++)
        {
            float _x = float(i)/float(n-1);

            // clamp non-zero values to _pos +/- _width
            if (_x > _pos+_width || _x < _pos-_width)
            {
                opacity[i] = qMax(opacity[i],(float)0);
                continue;
            }

            // non-zero _width
            if (_width == 0)
                _width = .00001f;

            // translate the original x to a new x based on the xbias
            float x0;
            if (xbias==0 || _x == _pos+xbias)
            {
                x0 = _x;
            }
            else if (_x > _pos+xbias)
            {
                if (_width == xbias)
                    x0 = _pos;
                else
                    x0 = _pos+(_x-_pos-xbias)*(_width/(_width-xbias));
            }
            else // (_x < _pos+xbias)
            {
                if (-_width == xbias)
                    x0 = _pos;
                else
                    x0 = _pos-(_x-_pos-xbias)*(_width/(_width+xbias));
            }

            // center around 0 and normalize to -1,1
            float x1 = (x0-_pos)/_width;

            // do a linear interpolation between:
            //    a gaussian and a parabola        if 0<ybias<1
            //    a parabola and a step function   if 1<ybias<2
            float h0a = exp(-(4*x1*x1));
            float h0b = 1. - x1*x1;
            float h0c = 1.;
            float h1;
            if (ybias < 1)
                h1 = ybias*h0b + (1-ybias)*h0a;
            else
                h1 = (2-ybias)*h0b + (ybias-1)*h0c;
            float h2 = _height * h1;

            // perform the MAX over different guassians, not the sum
            opacity[i] = qMax(opacity[i], h2);
        }
    }
}



//---------------------------------------------------------------------------


int QvisGaussianOpacityBar::getTopBinPixel(int bin, float scale, int* histogram, int currentMax, int currentUnEnabledMax, bool logScale, float enabledBarsHeight, bool* histogramEnabled){

int finalheight = 0;

if (histogram[bin] == 0){
	return this->contentsRect().height();
}


		if(!histogramEnabled[bin] && histogram[bin]>currentMax){
				if(logScale)
					finalheight = float(enabledBarsHeight) - float(enabledBarsHeight)*std::max((double)(log10(float(histogram[bin]-currentMax))/log10(float(currentUnEnabledMax-currentMax))),0.0);
				else
					finalheight = float(enabledBarsHeight) - float(enabledBarsHeight)*(float(histogram[bin]-currentMax)/float(currentUnEnabledMax-currentMax));
		}
		else{
			if(logScale)
				finalheight = int(scale*std::max((double)log10(float(histogram[bin])),0.0));
			else
				finalheight = int(scale*float(histogram[bin]));

			finalheight = this->contentsRect().height()-finalheight;

		}

		return finalheight;
		}





void QvisGaussianOpacityBar::generateBackgroundHistogram(int* values, int size, bool useLogScale, bool* histogramEnabled){


int enabledBarsHeight = 8;

	int width = size;
		int height = this->contentsRect().height();

		//get max value
		int max = 0;
		int currentUnEnabledMax = 0;
		for (int i = 0; i < width; i++) {
			if (values[i] > max && histogramEnabled[i])
				max = values[i];
			if (values[i] > currentUnEnabledMax && !histogramEnabled[i])
				currentUnEnabledMax = values[i];
		}


		float scale;
		if (useLogScale)
			scale = float(height - enabledBarsHeight) / float(log10((float)max));
		else
			scale = float(height - enabledBarsHeight) / float(max);

		QImage* image = new QImage(QSize(width, height), QImage::Format_RGB32);

		image->fill(0);

		for (int i = 0; i < width; i++) {
			int end = getTopBinPixel(i,scale,values,max,currentUnEnabledMax,useLogScale,enabledBarsHeight,histogramEnabled);
			QRgb color = histogramEnabled[i] ? qRgb(200, 0, 0) : qRgb(100, 0, 0);
			for (int j = height - 1; j >= end; j--) {
				image->setPixel(i, j, color);
			}
		}


		QPixmap* background =new QPixmap(QPixmap::fromImage(image->scaled(this->contentsRect().width(), height,
				Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));



	this->SetBackgroundPixmap(background);


	this->update();






}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::removeGaussian
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::removeGaussian(int n)
{
	gaussianFunctionGroup->RemoveGaussianAtIndex(n);
}

// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::setMaximumNumberOfGaussians
//
//  Purpose: Limit the number of Gaussians the user can add/remove
//
//
//  Programmer:  John Biddiscombe
//  Creation:    January 31, 2005
//
// ****************************************************************************
void
QvisGaussianOpacityBar::setMaximumNumberOfGaussians(int n)
{
  this->maximumNumberOfGaussians = n;
}


//-------------------------------------------------------------------------------


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::addGaussian
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
int
QvisGaussianOpacityBar::addGaussian(float _x,float h,float w,float bx,float by)
{
	double rangemagnitude = gaussianFunctionGroup->GetRangeAtIndex(1)-gaussianFunctionGroup->GetRangeAtIndex(0);
		return gaussianFunctionGroup->AddGaussian(std::min(std::max(_x*rangemagnitude, gaussianFunctionGroup->GetRangeAtIndex(0)),gaussianFunctionGroup->GetRangeAtIndex(1))
		,h,w*rangemagnitude,bx,by);
}



QvisGaussianOpacityBar::Gaussian QvisGaussianOpacityBar::getNode(int index){
	double values[5];
	double rangemagnitude = gaussianFunctionGroup->GetRangeAtIndex(1)-gaussianFunctionGroup->GetRangeAtIndex(0);
	double shift = gaussianFunctionGroup->GetRangeAtIndex(0);
	Gaussian gauss;
	gaussianFunctionGroup->GetNodeValue(index,values);
	gauss.x=values[0]/rangemagnitude-shift/rangemagnitude;
	gauss.h=values[1];
	gauss.w=values[2]/rangemagnitude;
	gauss.bx=values[3];
	gauss.by=values[4];
	return gauss;
}


//TBD convert space
double QvisGaussianOpacityBar::getGaussValue(int index, gaussvalue v){
	double rangemagnitude = gaussianFunctionGroup->GetRangeAtIndex(1)-gaussianFunctionGroup->GetRangeAtIndex(0);
	double shift = gaussianFunctionGroup->GetRangeAtIndex(0);
	if (gaussX == v)
		return gaussianFunctionGroup->getX(index)/rangemagnitude - shift/rangemagnitude;
	else if (gaussH ==v)
			return gaussianFunctionGroup->getH(index);
	else if (gaussW ==v)
		return gaussianFunctionGroup->getW(index)/rangemagnitude;
	else if (gaussBx ==v)
		return gaussianFunctionGroup->getBx(index);
	else if (gaussBy ==v)
		return gaussianFunctionGroup->getBy(index);
}

void  QvisGaussianOpacityBar::setNode(int index, Gaussian &gauss){
	double values[5];
	double rangemagnitude = gaussianFunctionGroup->GetRangeAtIndex(1)-gaussianFunctionGroup->GetRangeAtIndex(0);
	double shift = gaussianFunctionGroup->GetRangeAtIndex(0);
	values[0] = std::min(std::max(gauss.x*rangemagnitude+shift, gaussianFunctionGroup->GetRangeAtIndex(0)),gaussianFunctionGroup->GetRangeAtIndex(1));
	values[1] = gauss.h;
	values[2] = gauss.w*rangemagnitude;
	values[3] = gauss.bx;
	values[4] = gauss.by;
	gaussianFunctionGroup->SetNodeValue(index,values);

}


//TBD convert space
void QvisGaussianOpacityBar::setGaussValue(int index, double value, gaussvalue v){
	double rangemagnitude = gaussianFunctionGroup->GetRangeAtIndex(1)-gaussianFunctionGroup->GetRangeAtIndex(0);
	double shift = gaussianFunctionGroup->GetRangeAtIndex(0);
	if (gaussX == v)
		gaussianFunctionGroup->setX(index,std::min(std::max(value*rangemagnitude+shift,gaussianFunctionGroup->GetRangeAtIndex(0)),gaussianFunctionGroup->GetRangeAtIndex(1)));
	else if (gaussH ==v)
			gaussianFunctionGroup->setH(index,value);
	else if (gaussW ==v)
			gaussianFunctionGroup->setW(index,value*rangemagnitude);
	else if (gaussBx ==v)
			gaussianFunctionGroup->setBx(index,value);
	else if (gaussBy ==v)
			gaussianFunctionGroup->setBy(index,value);
}


void QvisGaussianOpacityBar::setFunctionRange(double range[2]){
	this->gaussianFunctionGroup->UpdateRange(false, range);
}




// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::setMaximumNumberOfGaussians
//
//  Purpose: Limit the number of Gaussians the user can add/remove
//
//
//  Programmer:  John Biddiscombe
//  Creation:    January 31, 2005
//
// ****************************************************************************
void
QvisGaussianOpacityBar::setMinimumNumberOfGaussians(int n)
{
  this->minimumNumberOfGaussians = n;
}

void QvisGaussianOpacityBar::setCurrentGaussian(int index){
	if (index < ngaussian && index >=0)
		currentGaussian = index;
}

#define dist2(x1,y1,x2,y2) (((x2)-(x1))*((x2)-(x1)) + ((y2)-(y1))*((y2)-(y1)))
// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::findGaussianControlPoint
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
bool
QvisGaussianOpacityBar::findGaussianControlPoint(int _x,int _y,
                                             int *newgaussian, Mode *newmode)
{
    *newgaussian = -1;
    *newmode     = modeNone;
    bool  found   = false;
    float mindist = 100000;  // it's okay, it's pixels
    for (int p=0; p<ngaussian; p++)
    {
        int xc = val2x(getGaussValue(p,gaussX)+getGaussValue(p,gaussBx));
        int xr = val2x(getGaussValue(p,gaussX)+getGaussValue(p,gaussW));
        int xl = val2x(getGaussValue(p,gaussX)-getGaussValue(p,gaussW));
        int yc = val2y(getGaussValue(p,gaussH));
        int y0 = val2y(0);
        int yb = val2y(getGaussValue(p,gaussH)/4. + getGaussValue(p,gaussBy)*getGaussValue(p,gaussH)/4.);

        float d1 = dist2(_x,_y, xc,y0);
        float d2 = dist2(_x,_y, xc,yc);
        float d3 = dist2(_x,_y, xr,y0);
        float d4 = dist2(_x,_y, xl,y0);
        float d5 = dist2(_x,_y, xc,yb);

        float rad = 8*8;

        if (d1 < rad && mindist > d1)
        {
            *newgaussian = p;
            *newmode     = modeX;
            mindist          = d1;
            found = true;
        }
        if (d2 < rad && mindist > d2)
        {
            *newgaussian = p;
            *newmode     = modeH;
            mindist          = d2;
            found = true;
        }
        if (d3 < rad && mindist > d3)
        {
            *newgaussian = p;
            *newmode     = modeWR;
            mindist          = d3;
            found = true;
        }
        if (d4 < rad && mindist > d4)
        {
            *newgaussian = p;
            *newmode     = modeWL;
            mindist          = d4;
            found = true;
        }
        if (d5 < rad && mindist > d5)
        {
            *newgaussian = p;
            *newmode     = modeB;
            mindist          = d5;
            found = true;
        }
    }
    return found;
}



// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::getNumberOfGaussians
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
int
QvisGaussianOpacityBar::getNumberOfGaussians()
{
    return ngaussian;
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::getGaussian
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::getGaussian(int i,
                                float *_x,
                                float *h,
                                float *w,
                                float *bx,
                                float *by)
{
    *_x  = gaussian[i].x;
    *h  = gaussian[i].h;
    *w  = gaussian[i].w;
    *bx = gaussian[i].bx;
    *by = gaussian[i].by;
}

// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::setGaussian
//
//  Purpose: Set the parameters of a particular Gaussian
//
//
//  Programmer:  John Biddiscombe
//  Creation:    January 31, 2005
//
// ****************************************************************************
void
QvisGaussianOpacityBar::setGaussian(int i,
                                float *_x,
                                float *h,
                                float *w,
                                float *bx,
                                float *by)
{
    gaussian[i].x = *_x;
    gaussian[i].h = *h;
    gaussian[i].w = *w;
    gaussian[i].bx = *bx;
    gaussian[i].by = *by;
}


// ****************************************************************************
//  Method:  QvisGaussianOpacityBar::setAllGaussians
//
//  Purpose:
//
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 31, 2001
//
// ****************************************************************************
void
QvisGaussianOpacityBar::setAllGaussians(int n, float *gaussdata)
{
    ngaussian = 0;
    for (int i=0; i<n; i++)
    {
        addGaussian(gaussdata[i*5 + 0],
                    gaussdata[i*5 + 1],
                    gaussdata[i*5 + 2],
                    gaussdata[i*5 + 3],
                    gaussdata[i*5 + 4]);
    }
    this->update();
}
