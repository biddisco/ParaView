#include "pqHistogramDialog.h"
#include "ui_pqHistogramDialog.h"
#include <iostream>
#include <QPainter>

#include <QMouseEvent>
#include <math.h>

class pqHistogramDialogUi: public Ui::pqHistogramDialog
  {
  };

pqHistogramDialog::pqHistogramDialog(QWidget *widgetParent,
    std::vector<int>* hist, int size, std::vector<bool>* histogramEn,
    bool* logScal, float* enabledBarsHeighfrac) :
    QDialog(widgetParent)
  {

  this->temphistogram = 0;
  this->histogramSize = size;
  this->finalHistogramEnabled = 0;
  this->histogram = hist;

  this->enabledBarsHeightFraction = enabledBarsHeighfrac;
  *(this->enabledBarsHeightFraction) = 0.8; //the lower the number is, the higher the bars

  this->dialogUi = new pqHistogramDialogUi;
  this->dialogUi->setupUi(this);
  QPushButton* applyButton = this->dialogUi->Confirmation->button(
      QDialogButtonBox::Ok);
  this->connect(applyButton, SIGNAL(clicked()), this, SLOT(accept()));
  QPushButton* cancelButton = this->dialogUi->Confirmation->button(
      QDialogButtonBox::Cancel);
  this->connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  this->connect(this->dialogUi->UseLogScale, SIGNAL(clicked()), this,
      SLOT(useLogScale()));
  this->connect(this->dialogUi->DisableLogScale, SIGNAL(clicked()), this,
      SLOT(disableLogScale()));
  this->connect(this->dialogUi->Reset, SIGNAL(clicked()), this, SLOT(reset()));

  this->finalHistogramEnabled = histogramEn;
  this->histogramEnabled.resize(size);
  for (int i = 0; i < size; i++)
    {
    this->histogramEnabled[i] = this->finalHistogramEnabled->at(i);
    }
  this->finallogScale = logScal;
  this->logScale = *logScal;

  this->histogramSize = size;

  this->dialogUi->HistogramWidget->SetData(&this->histogramEnabled,
      this->histogram, this->histogramSize, *logScal,
      *this->enabledBarsHeightFraction);

  this->dialogUi->stackedWidget->setCurrentIndex(0);

  }

void pqHistogramDialog::resizeEvent(QResizeEvent* event)
  {

  QWidget::resizeEvent(event);

  }

void pqHistogramDialog::setHistogramBools()
  {
  for (int i = 0; i < this->histogramSize; i++)
    {
    this->finalHistogramEnabled->at(i) = this->histogramEnabled[i];
    }
  }

void pqHistogramDialog::rejectChanges()
  {
  }

void pqHistogramDialog::acceptChanges()
  {
  setHistogramBools();

  }

void pqHistogramDialog::reject()
  {
  rejectChanges();
  QDialog::reject();
  }

void pqHistogramDialog::accept()
  {
  acceptChanges();
  *(this->finallogScale) = this->logScale;
  QDialog::accept();
  }

void pqHistogramDialog::reset()
  {
  this->dialogUi->HistogramWidget->reset();
  }

pqHistogramDialog::~pqHistogramDialog()
  {
  delete temphistogram;
  delete dialogUi;

  }

//drawing related stuff

void pqHistogramDialog::useLogScale()
  {
  this->logScale = true;
  this->dialogUi->HistogramWidget->useLogScale();
  this->dialogUi->stackedWidget->setCurrentIndex(1);
  }

void pqHistogramDialog::disableLogScale()
  {
  this->logScale = false;
  this->dialogUi->HistogramWidget->disableLogScale();
  this->dialogUi->stackedWidget->setCurrentIndex(0);
  }
