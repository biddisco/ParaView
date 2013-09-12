/*

  This is mostly adapted from QvisGaussianOpacityBar, with a bunch of changes
  to handle rectanlges instead of Gaussians.

  Note : Background ColourData is used to show the gradient as a background
         Underlay ColourData shows the colour lookuptable inside the shapes

*/
//---------------------------------------------------------------------------
#ifndef QVIS_2D_TRANSFER_FUNCTION_WIDGET_H
#define QVIS_2D_TRANSFER_FUNCTION_WIDGET_H
//---------------------------------------------------------------------------
#include "QvisAbstractOpacityBar.h"
//---------------------------------------------------------------------------
class QPixmap;
//---------------------------------------------------------------------------
class Qvis2DTransferFunctionWidget : public QvisAbstractOpacityBar
{
    Q_OBJECT
  public:
                  Qvis2DTransferFunctionWidget(QWidget *parent=NULL, const char *name=NULL);
                 ~Qvis2DTransferFunctionWidget();
    void          getRawOpacities(int, float*);
    int           getNumberOfRegions();
    void          getRegion(int, float*,float*,float*,float*,float*,float*);
    void          setRegion(int, float*,float*,float*,float*,float*,float*);
    void          setActiveRegionMode(int index);
    int           getActiveRegionMode();
    void          setActiveRegionMaximum(float mx);
    float         getActiveRegionMaximum();
    void          setAllRegions(int, float*);
    void          setMaximumNumberOfRegions(int);
    void          setMinimumNumberOfRegions(int);

    void          setBackgroundGradientData(int x, int y, int *data);
    void          setUnderlayColourData(int N, int c, const unsigned char *data);

    void          createRGBAData(unsigned char *data);

  protected:
    void          mouseMoveEvent(QMouseEvent*);
    void          mousePressEvent(QMouseEvent*);
    void          mouseReleaseEvent(QMouseEvent*);
    void          paintToPixmap(int,int);
    void          drawControlPoints(QPainter &painter);
    void          drawColourBars(QPainter &painter);
    void          drawRegions(QPainter &painter);

  signals:
    void          mouseMoved();
    void          activeRegionChanged(int);

  private:
    enum SelectMode     {modeNone, modeC0, modeC1, modeC2, modeC3, modeAll};
    enum TransferFnMode {Uniform, Gaussian, RightHalf, LeftHalf, TopHalf, BottomHalf, Sine, RampRight, RampLeft};
    // encapsulation of region parameters, if any are added or removed, make sure that
    // the number REGION_VARS is incremented accordingly as the streaming to XML
    // from the gui makes use of it
    #define REGION_VARS 6
    class Region
    {
      public:
        float x;
        float y;
        float w;
        float h;
        TransferFnMode TFMode;
        float maximum;
      public:
        Region(float x_,float y_,float w_,float h_, float mo_, float mx_) 
          : x(x_),y(y_),w(w_),h(h_), TFMode(TransferFnMode(static_cast<int>(mo_))), maximum(mx_) {};
        Region() {};
       ~Region() {};
    };

    // the list of regions
    int       nregion;
    Region    region[32];

    // the current interaction mode and the current region
    SelectMode  movingMode;
    int         highlitRegion;
    int         activeRegion;
    int         defaultTFMode;

    // Colour data for overlay
    QPixmap    *UnderlayColourPixmap;

    // GUI interaction variables
    bool        mousedown;
    int         lastx;
    int         lasty;

    // User specified constraints
    int         maximumNumberOfRegions;
    int         minimumNumberOfRegions;

    // helper functions
    bool findRegionControlPoint(int,int, int*, SelectMode*);
    void removeRegion(int);
    void addRegion(float,float,float,float,float,float);
    bool InRegion(Region &r, float *pt);
};

#endif
