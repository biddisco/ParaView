#include "pqHistogramWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include <iostream>
#include <math.h>

pqHistogramWidget::pqHistogramWidget()
  {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  this->logScale = false;
  this->currentHistogramImage = 0;
  this->currentMax = 0;
  this->enabledBarsHeight = 0;
  this->currentUnEnabledMax = 0;
  this->enabledBarsHeightFraction = 0;
  }

pqHistogramWidget::pqHistogramWidget(QWidget* parentObject)
  {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  this->logScale = false;
  this->currentHistogramImage = 0;
  this->currentMax = 0;
  this->enabledBarsHeight = 0;
  this->currentUnEnabledMax = 0;
  this->enabledBarsHeightFraction = 0;
  }

void pqHistogramWidget::SetData(std::vector<bool>* histogramEnable, std::vector<int>* histogra,
	int histogramSiz, bool logSc, float enabledBarsFrac)
  {
  this->histogramEnabled = histogramEnable;
  this->histogram = histogra;
  this->histogramSize = histogramSiz;
  this->logScale = logSc;
  this->enabledBarsHeightFraction = enabledBarsFrac;
  calculateEnabledBarsHeight();
  createPixmap();
  }
void pqHistogramWidget::calculateEnabledBarsHeight()
  {
  this->enabledBarsHeight = this->contentsRect().height()
	  - int(enabledBarsHeightFraction * float(this->contentsRect().height()));

  }

void pqHistogramWidget::useLogScale()
  {
  logScale = true;
  createPixmap();
  this->update();
  }

void pqHistogramWidget::disableLogScale()
  {
  this->logScale = false;
  createPixmap();
  this->update();
  }

bool pqHistogramWidget::getEnabled(int index)
  {
  return this->histogramEnabled->at(index);
  }

bool pqHistogramWidget::getLogScale()
  {
  return this->logScale;
  }

void pqHistogramWidget::enableAllBins()
  {
  for (int i = 0; i < histogramSize; i++)
	{
	this->histogramEnabled->at(i) = true;
	}
  }

int pqHistogramWidget::getBin(int xCoordinate)
  {
  return int(
	  float(xCoordinate) / float(this->contentsRect().width())
		  * float(histogramSize));
  }

void pqHistogramWidget::setPixmap(QImage* map)
  {
  if (this->currentHistogramImage)
	delete this->currentHistogramImage;
  this->currentHistogramImage = map;
  }

void pqHistogramWidget::createPixmap()
  {

  int width = histogramSize;
  int height = this->contentsRect().height();

  //get max value
  int max = 0;
  this->currentUnEnabledMax = 0;
  for (int i = 0; i < width; i++)
	{
	if (this->histogram->at(i) > max && this->histogramEnabled->at(i))
	  max = this->histogram->at(i);
	if (this->histogram->at(i) > this->currentUnEnabledMax && !this->histogramEnabled->at(i))
	  this->currentUnEnabledMax = this->histogram->at(i);
	}
  currentMax = max;

  float scale;
  if (this->logScale)
	scale = float(height - this->enabledBarsHeight) / float(log10((double) (max)));
  else
	scale = float(height - this->enabledBarsHeight) / float((double) (max));

  QImage* image = new QImage(QSize(width, height), QImage::Format_RGB32);

  image->fill(0);

  for (int i = 0; i < width; i++)
	{
	int end = getTopBinPixel(i, scale);
	QRgb color = this->histogramEnabled->at(i) ? qRgb(200, 0, 0) : qRgb(100, 0, 0);
	int value = this->histogram->at(i);
	for (int j = height - 1; j >= end; j--)
	  {
	  image->setPixel(i, j, color);
	  }
	}

  for (int i = 0; i < width; i++)
	{
	image->setPixel(i, this->enabledBarsHeight, qRgb(0, 0, 255));
	}

  this->unscaledImage = *image;

  *image = image->scaled(this->contentsRect().width(), height,
	  Qt::IgnoreAspectRatio, Qt::FastTransformation);

  this->setPixmap(image);

  }

void pqHistogramWidget::drawBin(QRgb color, int bin, int endCoord)
  {
  //first make the bin completely black
  for (int i = this->contentsRect().height() - 1; i >= 0; i--)
	{
	this->unscaledImage.setPixel(bin, i, 0);
	}

  for (int i = this->contentsRect().height() - 1; i >= endCoord; i--)
	{
	this->unscaledImage.setPixel(bin, i, color);
	}

  //make sure the blue line doesn't disappear
  this->unscaledImage.setPixel(bin, this->enabledBarsHeight, qRgb(0, 0, 255));

  scaleAndDraw();

  }

void pqHistogramWidget::scaleAndDraw()
  {
  QImage* image = new QImage(
      this->unscaledImage.scaled(this->contentsRect().width(), this->contentsRect().height(),
		  Qt::IgnoreAspectRatio, Qt::FastTransformation));
  this->setPixmap(image);
  }

void pqHistogramWidget::updatePixmap(int bin)
  {
  QRgb color;
  if (this->histogramEnabled->at(bin))
	color = qRgb(200, 0, 0);
  else
	color = qRgb(100, 0, 0);

  float scale;
  if (this->logScale)
	scale = float(this->contentsRect().height() - this->enabledBarsHeight)
		/ float(log10((double) (this->currentMax)));
  else
	scale = float(this->contentsRect().height() - this->enabledBarsHeight)
		/ float(this->currentMax);

  int endCoord = getTopBinPixel(bin, scale);

  drawBin(color, bin, endCoord);

  }

int pqHistogramWidget::getTopBinPixel(int bin, float scale)
  {

  int finalheight = 0;

  if (this->histogram->at(bin) == 0)
	{
	return this->contentsRect().height();
	}

  if (!this->histogramEnabled->at(bin) && this->histogram->at(bin) > this->currentMax)
	{
	if (this->logScale)
	  finalheight = float(this->enabledBarsHeight)
		  - float(this->enabledBarsHeight)
			  * std::max(
				  (double) (log10(float(this->histogram->at(bin) - this->currentMax))
					  / log10(float(this->currentUnEnabledMax - this->currentMax))), 0.0);
	else
	  finalheight = float(this->enabledBarsHeight)
		  - float(this->enabledBarsHeight)
			  * (float(this->histogram->at(bin) - this->currentMax)
				  / float(this->currentUnEnabledMax - this->currentMax));
	}
  else
	{
	if (this->logScale)
	  finalheight = int(
		  scale * std::max((double) log10(float(this->histogram->at(bin))), 0.0));
	else
	  finalheight = int(scale * float(this->histogram->at(bin)));

	finalheight = this->contentsRect().height() - finalheight;

	}

  return finalheight;
  }

void pqHistogramWidget::updateAllBinColumns()
  {
  createPixmap();
  }

void pqHistogramWidget::reset()
  {
  for (int i = 0; i< this->histogramSize; i++)
    {
    this->histogramEnabled->at(i) = true;
    }
  createPixmap();
  this->update();
  }

void pqHistogramWidget::mousePressEvent(QMouseEvent *e)
  {
  //switch enabled disabled
  int width = this->contentsRect().width();
  int selectedBin = int(
	  float(e->x()) * (float(this->histogramSize) / float(width)));

  if (this->histogramSize > selectedBin)
	{
    this->histogramEnabled->at(selectedBin) = !this->histogramEnabled->at(selectedBin);
	}
  if (selectedBin >= this->histogramSize || selectedBin < 0)
	{
	std::cout << "selectedbin error" << std::endl;
	if (selectedBin > this->histogramSize)
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

  if ((this->histogramEnabled->at(selectedBin) && this->histogram->at(selectedBin) > this->currentMax)
	  || (!this->histogramEnabled->at(selectedBin) && this->histogram->at(selectedBin) == this->currentMax)) //TBD fix bug (what if 2 histograms the same
	updateAllBinColumns();
  else
	updatePixmap(selectedBin);

  this->repaint();

  }

void pqHistogramWidget::mouseDoubleClickEvent(QMouseEvent *e)
  {

  this->mousePressEvent(e);

  int y = e->y();

  float scale = float(this->contentsRect().height() - this->enabledBarsHeight)
	  / float(this->currentMax);
  for (int i = 0; i < this->histogramSize; i++)
	{
	int top = getTopBinPixel(i, scale);
	if (top < y)
	  {
	  this->histogramEnabled->at(i) = false;
	  }
	else
	  {
	  this->histogramEnabled->at(i) = true;
	  }
	}

  /*
   else{
   float scale = float(currentMax)/float(this->enabledBarsHeight);
   for (int i = 0; i<histogramSize; i++){
   if (!histogramEnabled[i]) {
   int top = getTopBinPixel(i, scale);
   if (top > y)
   histogramEnabled[i] = true;
   }
   }
   }
   */
  createPixmap();
  this->update();

  }

void pqHistogramWidget::paintEvent(QPaintEvent *e)
  {

  QPainter painter(this);
  QRect dirtyRect(QPoint(0, 0),
	  QPoint(this->contentsRect().width() - 1, this->contentsRect().height() - 1));

  painter.drawImage(dirtyRect, *this->currentHistogramImage, dirtyRect);
  }

void pqHistogramWidget::resizeEvent(QResizeEvent* event)
  {
  // std::cout << "widget window resized" << std::endl;
  calculateEnabledBarsHeight();
  createPixmap();
  QWidget::resizeEvent(event);
  update();

  }

