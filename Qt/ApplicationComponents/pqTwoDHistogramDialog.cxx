#include "pqTwoDHistogramDialog.h"
#include "ui_pqTwoDHistogramDialog.h"
#include <iostream>
#include <QPainter>

#include <QMouseEvent>
#include <math.h>

class pqTwoDHistogramDialogUi: public Ui::pqTwoDHistogramDialog
  {
  };

pqTwoDHistogramDialog::pqTwoDHistogramDialog(QWidget *widgetParent,
    std::vector<int>* hist, int size[2], std::vector<bool>* histogramEn,
    bool* logScal) :
    QDialog(widgetParent)
  {

  this->temphistogram = 0;
  this->histogramSize[0] = size[0];
  this->histogramSize[1] = size[1];
  this->finalHistogramEnabled = 0;
  this->histogram = hist;

  this->dialogUi = new pqTwoDHistogramDialogUi;
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
  this->histogramEnabled.resize(size[0] * size[1]);
  for (int i = 0; i < size[0] * size[1]; i++)
    {
    this->histogramEnabled[i] = this->finalHistogramEnabled->at(i);
    }
  this->finallogScale = logScal;
  this->logScale = *logScal;

  this->dialogUi->TwoDHistogramWidget->SetData(&this->histogramEnabled,
      this->histogram, this->histogramSize[0], this->histogramSize[1],
      *logScal);

  this->dialogUi->stackedWidget->setCurrentIndex(0);

  }

void pqTwoDHistogramDialog::resizeEvent(QResizeEvent* event)
  {

  QWidget::resizeEvent(event);

  }

void pqTwoDHistogramDialog::setHistogramBools()
  {
  int tempsize = this->histogramSize[0] * this->histogramSize[1];
  for (int i = 0; i < tempsize; i++)
    {
    this->finalHistogramEnabled->at(i) = this->histogramEnabled[i];
    }
  }

void pqTwoDHistogramDialog::rejectChanges()
  {
  }

void pqTwoDHistogramDialog::acceptChanges()
  {
  setHistogramBools();

  }

void pqTwoDHistogramDialog::reject()
  {
  rejectChanges();
  QDialog::reject();
  }

void pqTwoDHistogramDialog::accept()
  {
  acceptChanges();
  *(this->finallogScale) = this->logScale;
  QDialog::accept();
  }

void pqTwoDHistogramDialog::reset()
  {
  this->dialogUi->TwoDHistogramWidget->reset();
  }

pqTwoDHistogramDialog::~pqTwoDHistogramDialog()
  {
  delete temphistogram;
  delete dialogUi;

  }

//drawing related stuff

void pqTwoDHistogramDialog::useLogScale()
  {
  this->logScale = true;
  this->dialogUi->TwoDHistogramWidget->useLogScale();
  this->dialogUi->stackedWidget->setCurrentIndex(1);
  }

void pqTwoDHistogramDialog::disableLogScale()
  {
  this->logScale = false;
  this->dialogUi->TwoDHistogramWidget->disableLogScale();
  this->dialogUi->stackedWidget->setCurrentIndex(0);
  }
