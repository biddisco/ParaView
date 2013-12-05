#include "pqTwoDHistogramWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include <iostream>
#include <math.h>

pqTwoDHistogramWidget::pqTwoDHistogramWidget()
  {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  logScale = false;
  currentHistogramImage = 0;
  currentMax = 0;
  currentUnEnabledMax = 0;
  }

pqTwoDHistogramWidget::pqTwoDHistogramWidget(QWidget* parentObject)
  {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  logScale = false;
  currentHistogramImage = 0;
  currentMax = 0;
  currentUnEnabledMax = 0;
  }

void pqTwoDHistogramWidget::SetData(std::vector<bool>* histogramEnable, std::vector<int>* histogra,
	int histogramxSiz, int histogramySiz,  bool logSc)
  {
  this->histogramEnabled = histogramEnable;
  this->histogram = histogra;
  this->histogramXSize = histogramxSiz;
  this->histogramYSize = histogramySiz;
  this->histogramSize = histogramxSiz*histogramySiz;
  this->logScale = logSc;
  createPixmap();
  }


void pqTwoDHistogramWidget::useLogScale()
  {
  logScale = true;
  createPixmap();
  this->update();
  }

void pqTwoDHistogramWidget::disableLogScale()
  {
  logScale = false;
  createPixmap();
  this->update();
  }

bool pqTwoDHistogramWidget::getEnabled(int index)
  {
  return histogramEnabled->at(index);
  }

bool pqTwoDHistogramWidget::getLogScale()
  {
  return this->logScale;
  }

void pqTwoDHistogramWidget::enableAllBins()
  {
  for (int i = 0; i < histogramSize; i++)
	{
	this->histogramEnabled->at(i) = true;
	}
  }

void pqTwoDHistogramWidget::getBin(int xCoordinate, int yCoordinate, int* xresult, int* yresult)
  {
  *xresult = int(
	  float(xCoordinate) / float(this->contentsRect().width())
		  * float(histogramXSize));
*yresult = int(
	  ((float(this->contentsRect().height()) - float(yCoordinate)) / float(this->contentsRect().height()))
		  * float(histogramYSize));
  }

void pqTwoDHistogramWidget::setPixmap(QImage* map)
  {
  if (this->currentHistogramImage)
	delete this->currentHistogramImage;
  this->currentHistogramImage = map;
  }

void pqTwoDHistogramWidget::createPixmap()
  {

  int width = histogramSize;
  int height = this->contentsRect().height();

  //get max value
  int max = 0;
  int maxXIndex = -1, maxYIndex = -1, maxXUnenIndex = -1, maxYUnenIndex = -1;
  currentUnEnabledMax = 0;


  for (int i = 0; i< this->histogramXSize; i++)
      {
      for (int j = 0; j< this->histogramYSize; j++)
        {
        if (histogram->at(i*histogramYSize + j) > max && histogramEnabled->at(i*histogramYSize + j))
            {

            max = histogram->at(i*histogramYSize + j);
            maxXIndex = i;
            maxYIndex = j;
            }
        else if (histogram->at(i*histogramYSize + j) > currentUnEnabledMax && !histogramEnabled->at(i*histogramYSize + j))
              {
          currentUnEnabledMax = histogram->at(i*histogramYSize + j);
          maxXUnenIndex = i;
          maxYUnenIndex = j;
              }
        }
      }



  currentMax = max;


  float scale, unenabledScale;
  if (logScale)
    {
    scale = 1/log10(float(currentMax));
    unenabledScale = 1/log10(float(currentUnEnabledMax));
    }
  else
    {
    scale =1.0f/float(currentMax);
    unenabledScale = 1/float(currentUnEnabledMax);
    }

  unenabledScale = scale  >unenabledScale ? scale : unenabledScale;

  QImage* image = new QImage(QSize(this->histogramXSize, this->histogramYSize), QImage::Format_RGB32);

  image->fill(qRgb(255,255,255));

  for (int i = 0; i< this->histogramXSize; i++)
    {
    for (int j = 0; j< this->histogramYSize; j++)
      {
      int value = histogram->at(i*histogramYSize + j);
      if (logScale && value != 0)
        value = log10(value);
      if (histogramEnabled->at(i*histogramYSize + j))
        {
        image->setPixel(i,histogramYSize-j-1,qRgb(255-int(255*(float(value)*scale)),
            255-int(255*(float(value)*scale)),255-int(255*(float(value)*scale))));
        }
      else
        {
        if (value != 0)
          image->setPixel(i,histogramYSize-j-1,qRgb(0,0,std::max(255-int(255*(float(value)*unenabledScale)),100)));
        else
          image->setPixel(i,histogramYSize-j-1,qRgb(255,255,255));
        }
      }
    }

  if (maxXIndex >= 0)
  image->setPixel(maxXIndex,histogramYSize-maxYIndex-1,qRgb(255,0,0));
  if (maxXUnenIndex >= 0)
    image->setPixel(maxXUnenIndex,histogramYSize-maxYUnenIndex-1,qRgb(255,0,255));


 
  unscaledImage = *image;

  *image = image->scaled(this->contentsRect().width(), height,
	  Qt::IgnoreAspectRatio, Qt::FastTransformation);

  this->setPixmap(image);

  }

void pqTwoDHistogramWidget::drawBin(QRgb color, int bin)
  {

  //make sure the blue line doesn't disappear
 // this->unscaledImage.setPixel(bin, enabledBarsHeight, qRgb(0, 0, 255));

  scaleAndDraw();

  }

void pqTwoDHistogramWidget::scaleAndDraw()
  {
  QImage* image = new QImage(
	  unscaledImage.scaled(this->contentsRect().width(), this->contentsRect().height(),
		  Qt::IgnoreAspectRatio, Qt::FastTransformation));
  this->setPixmap(image);
  }

void pqTwoDHistogramWidget::updatePixmap(int binx, int biny)
  {

  //bugged, don't use
  QRgb color;
  if (histogramEnabled->at(binx*this->histogramYSize +biny))
	color = qRgb(200, 0, 0);
  else
	color = qRgb(100, 0, 0);

  float scale;
  if (logScale)
	scale = float(this->contentsRect().height() - enabledBarsHeight)
		/ float(log10((double) (currentMax)));
  else
	scale = float(this->contentsRect().height() - enabledBarsHeight)
		/ float(currentMax);


  if (false)
  drawBin(color, binx);

  }

void pqTwoDHistogramWidget::reset()
  {
  int size = histogramEnabled->size();
  for (int i = 0; i< size; i++)
    {
    histogramEnabled->at(i) = true;
    }
  createPixmap();
  this->update();
  }


void pqTwoDHistogramWidget::mousePressEvent(QMouseEvent *e)
  {
  //switch enabled disabled
  int width = this->contentsRect().width();
  int selectedBinx,selectedBiny;
  getBin(e->x(),e->y(), &selectedBinx, &selectedBiny);
  int selectedBin = selectedBinx*histogramYSize + selectedBiny;

  if (histogramXSize*histogramYSize > selectedBin)
	{
	histogramEnabled->at(selectedBin) = !histogramEnabled->at(selectedBin);
	}
  if (selectedBin >= histogramXSize*histogramYSize || selectedBin < 0)
	{
	std::cout << "selectedbin error" << std::endl;
	if (selectedBin > histogramXSize*histogramYSize)
	  {
	  selectedBin = histogramSize - 1;
	  std::cout << "selected bin too high. Bin set to histogramSize-1"
		  << std::endl;
	  }
	else
	  {
	  selectedBin = 0;
	  std::cout << "selected bin too low. Bin set to 0" << std::endl;
	  }
	}


	createPixmap();


  this->repaint();

  }

void pqTwoDHistogramWidget::mouseDoubleClickEvent(QMouseEvent *e)
  {

  this->mousePressEvent(e);

  int xresult, yresult;
  getBin(e->x(), e->y(), &xresult, &yresult);

  int value = histogram->at(xresult*this->histogramYSize + yresult);


  for (int i = 0; i < histogramSize; i++)
	{
	if (histogram->at(i) >= value)
	  {
	  this->histogramEnabled->at(i) = false;
	  }
	else
	  {
	  this->histogramEnabled->at(i) = true;
	  }
	}


  createPixmap();
  this->update();

  }

void pqTwoDHistogramWidget::paintEvent(QPaintEvent *e)
  {

  QPainter painter(this);
  QRect dirtyRect(QPoint(0, 0),
	  QPoint(this->contentsRect().width() - 1, this->contentsRect().height() - 1));

  painter.drawImage(dirtyRect, *currentHistogramImage, dirtyRect);
  }

void pqTwoDHistogramWidget::resizeEvent(QResizeEvent* event)
  {
  // std::cout << "widget window resized" << std::endl;
  createPixmap();
  QWidget::resizeEvent(event);
  update();

  }

