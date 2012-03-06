/*=========================================================================

   Program:   ParaView
   Module:    pqDataExportWidget.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqDataExportWidget.h"
#include "ui_pqDataExportWidget.h"

#include "vtkPVConfig.h"
#include "vtkSmartPointer.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMProperty.h"
#include "vtkPVDataInformation.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataSetAttributesInformation.h"


// Qt includes
#include <QLineEdit>
#include <QSlider>
#include <QHBoxLayout>
#include <QDoubleValidator>

// ParaView Includes.
#include "pqApplicationCore.h"
#include "pqPropertyLinks.h"
#include "pqServerManagerModelItem.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqFieldSelectionAdaptor.h"
#include "pqRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "pqActiveObjects.h"
#include "pqPipelineFilter.h"

//
#include <vtksys/SystemTools.hxx>
#include <vtksys/RegularExpression.hxx>
#include <vtksys/ios/iostream>
#include <vtksys/ios/sstream>
#include <vtksys/stl/stdexcept>

//-----------------------------------------------------------------------------
class pqDataExportWidget::pqImplementation : public Ui::pqDataExportWidget
{
public:
  pqPropertyLinks Links;
  vtkSmartPointer<vtkSMSourceProxy> ActiveSourceProxy;
  int                               ActiveSourcePort;
  QIcon* CellDataIcon;
  QIcon* PointDataIcon;
};
//-----------------------------------------------------------------------------
pqDataExportWidget::pqDataExportWidget(QWidget* p)
  : QWidget(p) 
{
  this->Implementation = new pqImplementation();
  this->Implementation->setupUi(this);

    this->Implementation->CellDataIcon = new QIcon(
        ":/pqWidgets/Icons/pqCellData16.png");
    this->Implementation->PointDataIcon = new QIcon(
        ":/pqWidgets/Icons/pqPointData16.png");

  this->BlockUpdate = false;
  this->Value = "";

  
  QObject::connect(this->Implementation->full, SIGNAL(stateChanged(int)),
                   this, SLOT(fullChanged(int)));
  QObject::connect(this->Implementation->geometry, SIGNAL(stateChanged(int)),
                   this, SLOT(geometryChanged(int)));
  QObject::connect(this->Implementation->topology, SIGNAL(stateChanged(int)),
                   this, SLOT(topologyChanged(int)));
  QObject::connect(this->Implementation->field, SIGNAL(stateChanged(int)),
                   this, SLOT(fieldChanged(int)));

  QObject::connect(this->Implementation->assignSource, SIGNAL(clicked(bool)),
                   this, SLOT(chooseSource(bool)));

  
  //QObject::connect(this->LineEdit, SIGNAL(textChanged(const QString&)),
  //                 this, SLOT(textChanged(const QString&)));
  //QObject::connect(this->LineEdit, SIGNAL(editingFinished()),
  //                 this, SLOT(editingFinished()));
  
}

//-----------------------------------------------------------------------------
pqDataExportWidget::~pqDataExportWidget()
{
  delete this->Implementation->CellDataIcon;
  delete this->Implementation->PointDataIcon;
}

//-----------------------------------------------------------------------------
QString pqDataExportWidget::value() const
{
  return this->Value;
}

//-----------------------------------------------------------------------------
void pqDataExportWidget::setValue(QString val)
{
  if(this->Value == val)
    {
    return;
    }
  
  this->Value = val;

  if(!this->BlockUpdate)
    {
    //// set the slider 
    //this->updateSlider();

    this->ShowValue();

    //// set the text
    //this->BlockUpdate = true;
    //this->LineEdit->setText(QString().setNum(
    //  val,'g',DEFAULT_DOUBLE_PRECISION_VALUE));
    //this->BlockUpdate = false;
    }

  emit this->valueChanged(this->Value);
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::setCommandProperty(const char *cp)
{
  this->CommandProperty = cp;
}
//-----------------------------------------------------------------------------
QString pqDataExportWidget::getCommandProperty()
{
  return this->CommandProperty;
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::fullChanged(int)
{
  this->FullPressed = true;
  this->setValue(this->GenerateValue());
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::geometryChanged(int)
{
  this->FullPressed = false;
  this->setValue(this->GenerateValue());
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::topologyChanged(int)
{
  this->FullPressed = false;
  this->setValue(this->GenerateValue());
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::fieldChanged(int)
{
  this->FullPressed = false;
  this->setValue(this->GenerateValue());
}
//-----------------------------------------------------------------------------
QString pqDataExportWidget::GenerateValue()
{
  if (this->BlockUpdate) {
    return this->value();
  }
  //
  QString temp;
  if (this->FullPressed && this->Implementation->full->isChecked()) {
    temp += ":full_path:";
    temp += this->Implementation->fullPath->text();
  }
  else {
    if (this->Implementation->geometry->isChecked()) {
      temp += ":geometry_path:";
      temp += this->Implementation->geometryPath->text();
    }
    if (this->Implementation->topology->isChecked()) {
      temp += ":topology_path:";
      temp += this->Implementation->topologyPath->text();
    }
    if (this->Implementation->field->isChecked()) {
      temp += ":field_path:";
      temp += this->Implementation->fieldPath->text();
    }
  }
  temp += ":";
  return temp;
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::ShowValue()
{
  this->BlockUpdate = true;
  std::string str = this->Value.toAscii().data();
  vtksys::RegularExpression regex_f(":full_path:([^:]*):");
  vtksys::RegularExpression regex_g(":geometry_path:([^:]*):");
  vtksys::RegularExpression regex_t(":topology_path:([^:]*):");
  vtksys::RegularExpression regex_a(":field_path:([^:]*):");
  regex_f.find(str.c_str());
  regex_g.find(str.c_str());
  regex_t.find(str.c_str());
  regex_a.find(str.c_str());
  if (regex_f.match(1).size()>0) {
    this->Implementation->fullPath->setText(regex_f.match(1).c_str());
    this->Implementation->full->setChecked(true);
    // disable these
    this->Implementation->geometry->setChecked(false);
    this->Implementation->topology->setChecked(false);
    this->Implementation->field->setChecked(false);
  }
  if (regex_g.match(1).size()>0) {
    this->Implementation->geometryPath->setText(regex_g.match(1).c_str());
    this->Implementation->geometry->setChecked(true);
    // disable this
    this->Implementation->full->setChecked(false);
  }
  if (regex_t.match(1).size()>0) {
    this->Implementation->topologyPath->setText(regex_t.match(1).c_str());
    this->Implementation->topology->setChecked(true);
    // disable this
    this->Implementation->full->setChecked(false);
  }
  if (regex_a.match(1).size()>0) {
    this->Implementation->fieldPath->setText(regex_a.match(1).c_str());
    this->Implementation->field->setChecked(true);
    // disable this
    this->Implementation->full->setChecked(false);
  }
  this->BlockUpdate = false;
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::chooseSource(bool)
{
  //find the active filter
  pqServerManagerModelItem* item =
    pqActiveObjects::instance().activeSource();  
  if (item)
  {
    pqOutputPort* port = qobject_cast<pqOutputPort*>(item);
    this->Implementation->ActiveSourcePort = port ? port->getPortNumber() : 0;
    pqPipelineSource *source = port ? port->getSource() : 
      qobject_cast<pqPipelineSource*>(item);
    if (source)
    {

      this->Implementation->ActiveSourceProxy = vtkSMSourceProxy::SafeDownCast(source->getProxy());
      this->Implementation->sourceName->clear();
      pqPipelineRepresentation *dr = dynamic_cast<pqPipelineRepresentation*>(source->getRepresentation(0, pqActiveObjects::instance().activeView()));
      this->Implementation->sourceName->insert(source->getSMName());
      this->updateVariables();
/*      
      //
      // make sure the field arrays are propagated to our panel
      //
      if (this->Implementation->DSMProxyHelper) {
        vtkSMProperty *ip = this->Implementation->DSMProxyHelper->GetProperty("Input");
        pqSMAdaptor::setInputProperty(
          ip,
          this->Implementation->ActiveSourceProxy,
          this->Implementation->ActiveSourcePort
        );
        // This updates the ArrayListDomain domain 
        ip->UpdateDependentDomains();
      }
      //
      // make sure the Steering Writer knows where to get data from
      //
      if (this->Implementation->SteeringWriter) {
        vtkSMProperty *ip = this->Implementation->SteeringWriter->GetProperty("Input");
        pqSMAdaptor::setInputProperty(
          ip,
          this->Implementation->ActiveSourceProxy,
          this->Implementation->ActiveSourcePort
        );
      }
*/
    }
    else {
      this->Implementation->ActiveSourceProxy = NULL;
      this->Implementation->sourceName->clear();
    }
  }
  else {
    this->Implementation->ActiveSourceProxy = NULL;
    this->Implementation->sourceName->clear();
  }
}
//-----------------------------------------------------------------------------
vtkSMProxy *pqDataExportWidget::getControlledProxy()
{
  return this->Implementation->ActiveSourceProxy;
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::updateVariables()
{
  this->Implementation->fieldSelect->clear();

  vtkPVDataInformation *di = this->Implementation->ActiveSourceProxy->GetDataInformation();

  vtkPVDataSetAttributesInformation* fdi = NULL;
  if(!di)
    {
    return;
    }

  pqVariableType tt;
  for (int x=0; x<2; x++) {
    if (x==0) {  
      fdi = di->GetPointDataInformation();
      tt = VARIABLE_TYPE_NODE;
    }
    else {
      fdi = di->GetCellDataInformation();
      tt = VARIABLE_TYPE_CELL;
    }
    //
    for(int i=0; i<fdi->GetNumberOfArrays(); i++)
      {
      vtkPVArrayInformation* arrayInfo = fdi->GetArrayInformation(i);
      if (arrayInfo->GetDataType() == VTK_STRING
          || arrayInfo->GetDataType() == VTK_VARIANT )
        {
        continue;
        }
      QString name = arrayInfo->GetName();
      this->addVariable(VARIABLE_TYPE_CELL, name, false);
      }
    }
}
//-----------------------------------------------------------------------------
void pqDataExportWidget::addVariable(pqVariableType type,
    const QString& arg_name,
    bool is_partial)
{
  QString name = arg_name;
  if (is_partial)
    {
    name += " (partial)";
    }

  switch (type)
    {
    case VARIABLE_TYPE_NODE:
      this->Implementation->fieldSelect->addItem(*this->Implementation->PointDataIcon, name, arg_name);
      break;

    case VARIABLE_TYPE_CELL:
      this->Implementation->fieldSelect->addItem(*this->Implementation->CellDataIcon, name, arg_name);
      break;
    }
}
