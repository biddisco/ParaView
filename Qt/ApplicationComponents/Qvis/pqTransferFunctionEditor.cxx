/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: pqTransferFunctionEditor.cxx,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME pqTransferFunctionEditor
// .SECTION Thanks
// <verbatim>
//
//  This file is part of the PointSprites plugin developed and contributed by
//
//  Copyright (c) CSCS - Swiss National Supercomputing Centre
//
//  John Biddiscombe
//
// </verbatim>

#include "pqTransferFunctionEditor.h"
#include "ui_pqTransferFunctionEditor.h"


#include <QTimer>
#include <QChar>

#include <iostream>
using namespace std;

// Qvis includes
#include "Qvis2DTransferFunctionWidget.h"
#include "ui_pqTransferFunctionEditor.h"

// Paraview includes
#include "pqPipelineRepresentation.h"
#include "pqPropertyLinks.h"
#include "pqScalarsToColors.h"
#include "pqSMAdaptor.h"
#include "vtkSMProxy.h"
#include "vtkSMProperty.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMStringVectorProperty.h"
//
#include "vtkPVDataInformation.h"

// vtk includes
#include "vtkSmartPointer.h"
#include "vtkEventQtSlotConnect.h"
//-----------------------------------------------------------------------------
class pqTransferFunctionEditor::pqInternals: public Ui::pqTransferFunctionEditor
{
public:
  vtkSmartPointer<vtkEventQtSlotConnect> VTKConnect;
  pqPropertyLinks Links;
  int Updating;

  pqInternals()
  {
    VTKConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    Updating = 0;
  }

};
//-----------------------------------------------------------------------------
pqTransferFunctionEditor::pqTransferFunctionEditor(QWidget *parent, Qt::WindowFlags f)
: QFrame(parent)
{
  //QWidget::QWidget(parent,f);
  this->Internals = new pqTransferFunctionEditor::pqInternals();
  this->Internals->setupUi(this);

  //this->initialize();

  // We are usinging Queued slot execution where ever possible,
  // This ensures that the updateAllViews() slot is called
  // only after the vtkSMProperty has been changed by the pqPropertyLinks.


  //
  // Connect the transfer function widgets
  //




  this->connect(this->Internals->TransferFunctionMode, SIGNAL(currentIndexChanged(int)), this, 
    SLOT(onTFModeChanged(int)), Qt::QueuedConnection);

  this->connect(this->Internals->OpacityLevel, SIGNAL(valueChanged(int)), this, 
    SLOT(onOpacityLevelChanged(int)), Qt::QueuedConnection);


  this->connect(this->Internals->TransferFunction, SIGNAL(activeRegionChanged(int)), this, 
    SLOT(onActiveRegionChanged(int)), Qt::QueuedConnection);





}
//-----------------------------------------------------------------------------
pqTransferFunctionEditor::~pqTransferFunctionEditor()
{

}

//-----------------------------------------------------------------------------
void pqTransferFunctionEditor::initialize(vtkTwoDTransferFunction* function, vtkScalarsToColors* stc)
  {
  if(function || stc){
      this->Internals->TransferFunction->initialize(function, stc);
      this->connect(this->Internals->TransferFunction, SIGNAL(activeRegionChanged(int)), this,
            SIGNAL(activeRegionChanged(int)));

        this->connect(this->Internals->TransferFunction, SIGNAL(controlPointsModified()), this,
              SIGNAL(controlPointsModified()));
    }


  }
//-----------------------------------------------------------------------------
QList<QVariant> pqTransferFunctionEditor::regionControlPoints()
{
  QList<QVariant> list;
  for (int i = 0; i < this->Internals->TransferFunction->getNumberOfRegions(); ++i)
    {
    float g[REGION_VARS];
    this->Internals->TransferFunction->getRegion(i, &g[0], &g[1], &g[2], &g[3], &g[4], &g[5]);

    for (int j = 0; j < REGION_VARS; j++)
      {
      list.append(QVariant(static_cast<double> (g[j])));
      }
    }
  return list;
}
//-----------------------------------------------------------------------------
void pqTransferFunctionEditor::setRegionControlPoints(const QList<QVariant>& values)
{
  this->Internals->TransferFunction->blockSignals(true);
  //
  this->Internals->TransferFunction->setAllRegions(0, NULL);
  int n = values.size();
  if (n > 0)
    {
    float gcpts[1024];
    for (int i = 0; i < n; ++i)
      gcpts[i] = static_cast<float> (values[i].toDouble());
    // Set all of the regions into the widget.
    this->Internals->TransferFunction->setAllRegions(n / REGION_VARS, gcpts);
    }
  this->Internals->TransferFunction->blockSignals(false);
}


void pqTransferFunctionEditor::setCurrentRegion(int index)
  {
  this->Internals->TransferFunction->setCurrentRegion(index);
  }


void pqTransferFunctionEditor::removeHistogram()
  {
  this->Internals->TransferFunction->removeHistogram();
  }


//-----------------------------------------------------------------------------
void pqTransferFunctionEditor::onTFModeChanged(int index)
{
  this->Internals->TransferFunction->setActiveRegionMode(index);
}
//-----------------------------------------------------------------------------
void pqTransferFunctionEditor::onOpacityLevelChanged(int value)
{
  double val = (double)value/4999.0;
  this->Internals->TransferFunction->setActiveRegionMaximum(val);
}

//-----------------------------------------------------------------------------
void pqTransferFunctionEditor::onActiveRegionChanged(int region)
{

  //
  if (this->Internals->TransferFunction->getActiveRegionMaximum()>-1) {
    float val = this->Internals->TransferFunction->getActiveRegionMaximum();
    int     m = this->Internals->TransferFunction->getActiveRegionMode();
    this->Internals->OpacityLevel->setValue(val*this->Internals->OpacityLevel->maximum());
    this->Internals->OpacityLevel->update();
    this->Internals->TransferFunctionMode->setCurrentIndex(m);
  }
}

void pqTransferFunctionEditor::generateHistogramBackground(int width, int height, std::vector<int> &array, std::vector<bool> &enabledBins,bool logScale)
  {
  this->Internals->TransferFunction->generateHistogramBackground(width, height, array, enabledBins, logScale);
  }

