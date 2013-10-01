#ifndef __pqHistogramWidget_h
#define __pqHistogramWidget_h



#include <QWidget>
#include "pqComponentsModule.h"
#include "vtkType.h"

class QPixmap;

class PQCOMPONENTS_EXPORT pqHistogramWidget : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;





  public :
  void SetData(bool* histogramEnable, int* histogra, int histogramSiz, bool logSc, int enabledBarsHeigh);
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
      void   paintEvent(QPaintEvent *e);
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

    bool logScale;

    QImage unscaledImage;

    QImage* currentHistogramImage;

    int currentMax;
    int currentUnEnabledMax;


};

#endif
