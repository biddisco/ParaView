#ifndef __pqTwoDHistogramWidget_h
#define __pqTwoDHistogramWidget_h

#include <QWidget>
#include "pqApplicationComponentsModule.h"
#include "vtkType.h"

class QPixmap;

class PQAPPLICATIONCOMPONENTS_EXPORT pqTwoDHistogramWidget: public QWidget
  {
Q_OBJECT
  typedef QWidget Superclass;

public:
  void SetData(std::vector<bool>* histogramEnable, std::vector<int>* histogra,
      int histogramxSiz, int histogramySiz,  bool logSc);
  pqTwoDHistogramWidget();
  pqTwoDHistogramWidget(QWidget* parentObject);

  void useLogScale();
  void disableLogScale();

  bool getEnabled(int index);
  bool getLogScale();

  void display();

  void resizeImage(int width, int height);

  void paintPixMap();

protected:
  void mousePressEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void paintEvent(QPaintEvent *e);
  int histogramSize;
  std::vector<int>* histogram;
  std::vector<bool>* histogramEnabled;
  void enableAllBins();
  void createPixmap();
  void updatePixmap(int binx, int biny);
  void getBin(int xCoordinate, int yCoordinate, int* xresult, int* yresult);
  void setPixmap(QImage* map);
  void drawBin(QRgb color, int bin);
  void updateAllBinColumns();
  void scaleAndDraw();
  int enabledBarsHeight;
  int histogramXSize;
  int histogramYSize;

 
  void resizeEvent(QResizeEvent* event);

  bool logScale;

  QImage unscaledImage;

  QImage* currentHistogramImage;

  int currentMax;
  int currentUnEnabledMax;

  };

#endif
