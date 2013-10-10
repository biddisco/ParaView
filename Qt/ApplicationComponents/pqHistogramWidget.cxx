#include "pqHistogramWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QImage>
#include <iostream>
#include <math.h>

pqHistogramWidget::pqHistogramWidget() {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  logScale = false;
  currentHistogramImage = 0;
  currentMax = 0;
  enabledBarsHeight = 0;
  currentUnEnabledMax = 0;
}

pqHistogramWidget::pqHistogramWidget(QWidget* parentObject) {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  logScale = false;
  currentHistogramImage = 0;
  currentMax = 0;
  enabledBarsHeight = 0;
  currentUnEnabledMax = 0;
}

void pqHistogramWidget::SetData(bool* histogramEnable, int* histogra,
    int histogramSiz, bool logSc, int enabledBarsHeigh) {
  this->histogramEnabled = histogramEnable;
  this->histogram = histogra;
  this->histogramSize = histogramSiz;
  this->logScale = logSc;
  this->enabledBarsHeight = enabledBarsHeigh;
  createPixmap();
}

void pqHistogramWidget::useLogScale() {
  logScale = true;
  createPixmap();
  this->update();
}

void pqHistogramWidget::disableLogScale() {
  logScale = false;
  createPixmap();
  this->update();
}

bool pqHistogramWidget::getEnabled(int index) {
  return histogramEnabled[index];
}

bool pqHistogramWidget::getLogScale() {
  return this->logScale;
}

void pqHistogramWidget::enableAllBins() {
  for (int i = 0; i < histogramSize; i++) {
    this->histogramEnabled[i] = true;
  }
}

int pqHistogramWidget::getBin(int xCoordinate) {
  return int(
      float(xCoordinate) / float(this->contentsRect().width())
          * float(histogramSize));
}

void pqHistogramWidget::setPixmap(QImage* map) {
  if (this->currentHistogramImage)
    delete this->currentHistogramImage;
  this->currentHistogramImage = map;
}

void pqHistogramWidget::createPixmap() {

  int width = histogramSize;
  int height = this->geometry().height();

  //get max value
  int max = 0;
  currentUnEnabledMax = 0;
  for (int i = 0; i < width; i++) {
    if (histogram[i] > max && histogramEnabled[i])
      max = histogram[i];
    if (histogram[i] > this->currentUnEnabledMax && !histogramEnabled[i])
      this->currentUnEnabledMax = histogram[i];
  }
  currentMax = max;

  float scale;
  if (logScale)
    scale = float(height - enabledBarsHeight) / float(log10((double)(max)));
  else
    scale = float(height - enabledBarsHeight) / float((double)(max));

  QImage* image = new QImage(QSize(width, height), QImage::Format_RGB32);

  image->fill(0);

  for (int i = 0; i < width; i++) {
    int end = getTopBinPixel(i, scale);
    QRgb color = histogramEnabled[i] ? qRgb(200, 0, 0) : qRgb(100, 0, 0);
    for (int j = height - 1; j >= end; j--) {
      image->setPixel(i, j, color);
    }
  }

  for (int i = 0; i < width; i++) {
    image->setPixel(i, enabledBarsHeight, qRgb(0, 0, 255));
  }

  unscaledImage = *image;

  *image = image->scaled(this->contentsRect().width(), height,
      Qt::IgnoreAspectRatio, Qt::FastTransformation);

  this->setPixmap(image);

}

void pqHistogramWidget::drawBin(QRgb color, int bin, int endCoord) {
  //first make the bin completely black
  for (int i = this->contentsRect().height() - 1; i >= 0; i--) {
    this->unscaledImage.setPixel(bin, i, 0);
  }

  for (int i = this->contentsRect().height() - 1; i >= endCoord; i--) {
    this->unscaledImage.setPixel(bin, i, color);
  }

  //make sure the blue line doesn't disappear
  this->unscaledImage.setPixel(bin, enabledBarsHeight, qRgb(0, 0, 255));

  scaleAndDraw();

}

void pqHistogramWidget::scaleAndDraw() {
  QImage* image = new QImage(
      unscaledImage.scaled(this->contentsRect().width(),
          this->contentsRect().height(), Qt::IgnoreAspectRatio,
          Qt::FastTransformation));
  this->setPixmap(image);
}

void pqHistogramWidget::updatePixmap(int bin) {
  QRgb color;
  if (histogramEnabled[bin])
    color = qRgb(200, 0, 0);
  else
    color = qRgb(100, 0, 0);

  float scale;
  if (logScale)
    scale = float(this->contentsRect().height() - enabledBarsHeight)
        / float(log10((double)(currentMax)));
  else
    scale = float(this->contentsRect().height() - enabledBarsHeight)
        / float(currentMax);

  int endCoord = getTopBinPixel(bin, scale);

  drawBin(color, bin, endCoord);

}

int pqHistogramWidget::getTopBinPixel(int bin, float scale) {

  int finalheight = 0;

  if (histogram[bin] == 0) {
    return this->contentsRect().height();
  }

  if (!histogramEnabled[bin] && histogram[bin] > currentMax) {
    if (logScale)
      finalheight =
          float(enabledBarsHeight)
              - float(enabledBarsHeight)
                  * std::max(
                      (double)(log10(float(histogram[bin] - currentMax))
                          / log10(float(currentUnEnabledMax - currentMax))),
                      0.0);
    else
      finalheight = float(enabledBarsHeight)
          - float(enabledBarsHeight)
              * (float(histogram[bin] - currentMax)
                  / float(currentUnEnabledMax - currentMax));
  } else {
    if (logScale)
      finalheight = int(
          scale * std::max((double)log10(float(histogram[bin])), 0.0));
    else
      finalheight = int(scale * float(histogram[bin]));

    finalheight = this->contentsRect().height() - finalheight;

  }

  return finalheight;
}

void pqHistogramWidget::updateAllBinColumns() {
  createPixmap();
}

void pqHistogramWidget::mousePressEvent(QMouseEvent *e) {
	//switch enabled disabled
	int width = this->contentsRect().width();
	int selectedBin = int(
			float(e->x()) * (float(this->histogramSize) / float(width)));

	if (histogramSize > selectedBin) {
		histogramEnabled[selectedBin] = !histogramEnabled[selectedBin];
	}
	if (selectedBin >= histogramSize || selectedBin < 0) {
		std::cout << "selectedbin error" << std::endl;
		if(selectedBin > histogramSize){
			selectedBin = histogramSize-1;
			std::cout << "selected bin too high. Bin set to histogramSize-1" << std::endl;
		}
		else{
			selectedBin = 0;
			std::cout << "selected bin too low. Bin set to 0" << std::endl;
		}
	}

	if ((histogramEnabled[selectedBin] && histogram[selectedBin] > currentMax)
			|| (!histogramEnabled[selectedBin]
					&& histogram[selectedBin] == currentMax)) //TBD fix bug (what if 2 histograms the same
		updateAllBinColumns();
	else
		updatePixmap(selectedBin);

	this->repaint();

}

void pqHistogramWidget::mouseDoubleClickEvent(QMouseEvent *e) {


this->mousePressEvent(e);

int y = e->y();


float scale = float(this->contentsRect().height() - enabledBarsHeight) / float(currentMax);
  for (int i = 0; i<histogramSize; i++){
      int top = getTopBinPixel(i, scale);
      if (top<y){
          this->histogramEnabled[i] = false;
      }
      else{
          this->histogramEnabled[i] = true;
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


void pqHistogramWidget::paintEvent(QPaintEvent *e) {

  QPainter painter(this);
  QRect dirtyRect(QPoint(0, 0),
      QPoint(this->contentsRect().width() - 1,
          this->contentsRect().height() - 1));

  painter.drawImage(dirtyRect, *currentHistogramImage, dirtyRect);
}

