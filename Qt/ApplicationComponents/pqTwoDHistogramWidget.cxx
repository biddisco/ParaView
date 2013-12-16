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
  this->logScale = false;
  this->currentHistogramImage = 0;
  this->currentMax = 0;
  this->currentUnEnabledMax = 0;
  }

pqTwoDHistogramWidget::pqTwoDHistogramWidget(QWidget* parentObject)
  {
  this->histogramEnabled = 0;
  this->histogram = 0;
  this->histogramSize = 0;
  this->logScale = false;
  this->currentHistogramImage = 0;
  this->currentMax = 0;
  this->currentUnEnabledMax = 0;
  }

void pqTwoDHistogramWidget::SetData(std::vector<bool>* histogramEnable,
    std::vector<int>* histogra, int histogramxSiz, int histogramySiz,
    bool logSc)
  {
  this->histogramEnabled = histogramEnable;
  this->histogram = histogra;
  this->histogramXSize = histogramxSiz;
  this->histogramYSize = histogramySiz;
  this->histogramSize = histogramxSiz * histogramySiz;
  this->logScale = logSc;
  createPixmap();
  }

void pqTwoDHistogramWidget::useLogScale()
  {
  this->logScale = true;
  createPixmap();
  this->update();
  }

void pqTwoDHistogramWidget::disableLogScale()
  {
  this->logScale = false;
  createPixmap();
  this->update();
  }

bool pqTwoDHistogramWidget::getEnabled(int index)
  {
  return this->histogramEnabled->at(index);
  }

bool pqTwoDHistogramWidget::getLogScale()
  {
  return this->logScale;
  }

void pqTwoDHistogramWidget::enableAllBins()
  {
  for (int i = 0; i < this->histogramSize; i++)
    {
    this->histogramEnabled->at(i) = true;
    }
  }

void pqTwoDHistogramWidget::getBin(int xCoordinate, int yCoordinate,
    int* xresult, int* yresult)
  {
  *xresult = int(
      float(xCoordinate) / float(this->contentsRect().width())
          * float(this->histogramXSize));
  *yresult = int(
      ((float(this->contentsRect().height()) - float(yCoordinate))
          / float(this->contentsRect().height()))
          * float(this->histogramYSize));
  }

void pqTwoDHistogramWidget::setPixmap(QImage* map)
  {
  if (this->currentHistogramImage)
    delete this->currentHistogramImage;
  this->currentHistogramImage = map;
  }

void pqTwoDHistogramWidget::createPixmap()
  {

  int width = this->histogramSize;
  int height = this->contentsRect().height();

  //get max value
  int max = 0;
  int maxXIndex = -1, maxYIndex = -1, maxXUnenIndex = -1, maxYUnenIndex = -1;
  this->currentUnEnabledMax = 0;

  for (int i = 0; i < this->histogramXSize; i++)
    {
    for (int j = 0; j < this->histogramYSize; j++)
      {
      if (this->histogram->at(i * this->histogramYSize + j) > max
          && this->histogramEnabled->at(i * this->histogramYSize + j))
        {

        max = this->histogram->at(i * this->histogramYSize + j);
        maxXIndex = i;
        maxYIndex = j;
        }
      else if (this->histogram->at(i * this->histogramYSize + j)
          > this->currentUnEnabledMax
          && !this->histogramEnabled->at(i * this->histogramYSize + j))
        {
        this->currentUnEnabledMax = this->histogram->at(
            i * this->histogramYSize + j);
        maxXUnenIndex = i;
        maxYUnenIndex = j;
        }
      }
    }

  this->currentMax = max;

  float scale, unenabledScale;
  if (this->logScale)
    {
    scale = 1 / log10(float(this->currentMax));
    unenabledScale = 1 / log10(float(this->currentUnEnabledMax));
    }
  else
    {
    scale = 1.0f / float(this->currentMax);
    unenabledScale = 1 / float(this->currentUnEnabledMax);
    }

  unenabledScale = scale > unenabledScale ? scale : unenabledScale;

  QImage* image = new QImage(QSize(this->histogramXSize, this->histogramYSize),
      QImage::Format_RGB32);

  image->fill(qRgb(255, 255, 255));

  for (int i = 0; i < this->histogramXSize; i++)
    {
    for (int j = 0; j < this->histogramYSize; j++)
      {
      float value = float(this->histogram->at(i * histogramYSize + j));
      if (this->logScale && value != 0)
        value = log10(value);
      if (this->histogramEnabled->at(i * this->histogramYSize + j))
        {
        image->setPixel(i, this->histogramYSize - j - 1,
            qRgb(255 - int(255 * (value * scale)),
                255 - int(255 * (value * scale)),
                255 - int(255 * (value * scale))));
        }
      else
        {
        if (value != 0)
          image->setPixel(i, this->histogramYSize - j - 1,
              qRgb(0, 0,
                  std::max(255 - int(255 * (value * unenabledScale)), 100)));
        else
          image->setPixel(i, this->histogramYSize - j - 1, qRgb(255, 255, 255));
        }
      }
    }

  if (maxXIndex >= 0)
    image->setPixel(maxXIndex, this->histogramYSize - maxYIndex - 1,
        qRgb(255, 0, 0));
  if (maxXUnenIndex >= 0)
    image->setPixel(maxXUnenIndex, this->histogramYSize - maxYUnenIndex - 1,
        qRgb(255, 0, 255));

  this->unscaledImage = *image;

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
      this->unscaledImage.scaled(this->contentsRect().width(),
          this->contentsRect().height(), Qt::IgnoreAspectRatio,
          Qt::FastTransformation));
  this->setPixmap(image);
  }

void pqTwoDHistogramWidget::updatePixmap(int binx, int biny)
  {

  //bugged, don't use
  QRgb color;
  if (this->histogramEnabled->at(binx * this->histogramYSize + biny))
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

  if (false)
    drawBin(color, binx);

  }

void pqTwoDHistogramWidget::reset()
  {
  int size = this->histogramEnabled->size();
  for (int i = 0; i < size; i++)
    {
    this->histogramEnabled->at(i) = true;
    }
  createPixmap();
  this->update();
  }

void pqTwoDHistogramWidget::mousePressEvent(QMouseEvent *e)
  {
  //switch enabled disabled
  int width = this->contentsRect().width();
  int selectedBinx, selectedBiny;
  getBin(e->x(), e->y(), &selectedBinx, &selectedBiny);
  int selectedBin = selectedBinx * this->histogramYSize + selectedBiny;

  if (this->histogramXSize * this->histogramYSize > selectedBin)
    {
    this->histogramEnabled->at(selectedBin) = !this->histogramEnabled->at(
        selectedBin);
    }
  if (selectedBin >= this->histogramXSize * this->histogramYSize
      || selectedBin < 0)
    {
    std::cout << "selectedbin error" << std::endl;
    if (selectedBin > this->histogramXSize * this->histogramYSize)
      {
      selectedBin = this->histogramSize - 1;
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

  int value = this->histogram->at(xresult * this->histogramYSize + yresult);

  for (int i = 0; i < this->histogramSize; i++)
    {
    if (this->histogram->at(i) >= value)
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
      QPoint(this->contentsRect().width() - 1,
          this->contentsRect().height() - 1));

  painter.drawImage(dirtyRect, *this->currentHistogramImage, dirtyRect);
  }

void pqTwoDHistogramWidget::resizeEvent(QResizeEvent* event)
  {
  // std::cout << "widget window resized" << std::endl;
  createPixmap();
  QWidget::resizeEvent(event);
  update();

  }
