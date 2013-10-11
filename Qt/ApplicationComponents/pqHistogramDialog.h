#ifndef __pqHistogramDialog_h
#define __pqHistogramDialog_h


#include "pqApplicationComponentsModule.h"
#include <QDialog>
#include "vtkType.h"
#include <qframe.h>

class pqHistogramWidget;

class pqHistogramDialogUi;


class PQAPPLICATIONCOMPONENTS_EXPORT pqHistogramDialog : public QDialog
{
  Q_OBJECT

public :

  pqHistogramDialog(QWidget *widgetParent, int* hist, int size, bool* histogramEn, bool* logScal, float* enabledBarsHeighfrac);
  ~pqHistogramDialog();

  //void setData(int* hist, int size, bool* histogramEn, bool* logScale);

  bool* finalHistogramEnabled;

  bool* temphistogram; //only used so that changes can be undone.
  int histogramSize;

  pqHistogramDialogUi* dialogUi;

protected :


void rejectChanges();
void acceptChanges();

void setHistogramBools();

bool* finallogScale;
bool logScale;




bool getEnabled(int index);
bool getLogScale();


void resizeImage(int width, int height);

    int* histogram;
    bool* histogramEnabled;
    float *enabledBarsHeightFraction;
    void resizeEvent(QResizeEvent* event);



protected slots:
void reject();
void accept();

void useLogScale();
void disableLogScale();




};
#endif
