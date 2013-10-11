#ifndef __pqHistogramWidget_h
#define __pqHistogramWidget_h



#include <QWidget>
#include "pqApplicationComponentsModule.h"
#include "vtkType.h"

class QPixmap;

class PQAPPLICATIONCOMPONENTS_EXPORT pqHistogramWidget : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;





  public :
  void SetData(bool* histogramEnable, int* histogra, int histogramSiz, bool logSc, float enabledBarsFrac);
  pqHistogramWidget();
  pqHistogramWidget(QWidget* parentObject);

  void useLogScale();
  void disableLogScale();

  bool getEnabled(int index);
  bool getLogScale();

  void display();

  void resizeImage(int width, int height);

  void paintPixMap();


  protected :
  void   mousePressEvent(QMouseEvent*);
  void   mouseDoubleClickEvent(QMouseEvent*);
  void   paintEvent(QPaintEvent *e);
  void calculateEnabledBarsHeight();
  int histogramSize;
    int* histogram;
    bool* histogramEnabled;
    void enableAllBins();
    void createPixmap();
    void updatePixmap(int bin);
    int getBin(int);
    void setPixmap(QImage* map);
    void drawBin(QRgb color, int bin, int endCoord);
    void updateAllBinColumns();
    void scaleAndDraw();
    int enabledBarsHeight;
    int getTopBinPixel(int bin, float scale);

    float enabledBarsHeightFraction; //this is the portion of the image that the enabled bars will take up.
    //don't get confused by the fact that the origin is in the top left -> the larger the fraction the smaller the higher.


    void resizeEvent(QResizeEvent* event);

    bool logScale;

    QImage unscaledImage;

    QImage* currentHistogramImage;

    int currentMax;
    int currentUnEnabledMax;


};

#endif
