#ifndef __pqTwoDHistogramDialog_h
#define __pqTwoDHistogramDialog_h


#include "pqApplicationComponentsModule.h"
#include <QDialog>
#include "vtkType.h"
#include <qframe.h>
#include <vector>
class pqTwoDHistogramWidget;

class pqTwoDHistogramDialogUi;


class PQAPPLICATIONCOMPONENTS_EXPORT pqTwoDHistogramDialog : public QDialog
{
  Q_OBJECT

public :

  pqTwoDHistogramDialog(QWidget *widgetParent, std::vector<int>* hist, int size[2],
      std::vector<bool>* histogramEn, bool* logScal);
  ~pqTwoDHistogramDialog();

  //void setData(int* hist, int size, bool* histogramEn, bool* logScale);

  std::vector<bool>* finalHistogramEnabled;

  bool* temphistogram; //only used so that changes can be undone.
  int histogramSize[2];

  pqTwoDHistogramDialogUi* dialogUi;

protected :


void rejectChanges();
void acceptChanges();

void setHistogramBools();

bool* finallogScale;
bool logScale;




bool getEnabled(int index);
bool getLogScale();


void resizeImage(int width, int height);

    std::vector<int>* histogram;
    std::vector<bool> histogramEnabled;
    float *enabledBarsHeightFraction;
    void resizeEvent(QResizeEvent* event);



protected slots:
void reject();
void accept();

void useLogScale();
void disableLogScale();




};
#endif
