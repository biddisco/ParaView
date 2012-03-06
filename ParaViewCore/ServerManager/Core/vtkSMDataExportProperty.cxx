/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDataExportProperty.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMDataExportProperty.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkProcessModule.h"
#include "vtkStringList.h"
#include "vtkSMVectorPropertyTemplate.h"

#include <vector>
#include "vtkStdString.h"

#include <vtksys/RegularExpression.hxx>

vtkStandardNewMacro(vtkSMDataExportProperty);

// redefined here so that the compiler is happy about the struct
class vtkSMStringVectorProperty::vtkInternals :
  public vtkSMVectorPropertyTemplate<vtkStdString>
{
public:
  std::vector<int> ElementTypes;
  std::vector<vtkStdString> RegexDefaults;

  vtkInternals(vtkSMStringVectorProperty* ivp):
    vtkSMVectorPropertyTemplate<vtkStdString>(ivp)
  {
  }
};

//---------------------------------------------------------------------------
vtkSMDataExportProperty::vtkSMDataExportProperty()
{
  this->SetNumberOfElements(1);
}

//---------------------------------------------------------------------------
vtkSMDataExportProperty::~vtkSMDataExportProperty()
{
}

//---------------------------------------------------------------------------
void vtkSMDataExportProperty::SetDefaultValue(const char *def) {
  this->Internals->DefaultValues.resize(1);
  this->Internals->DefaultValues[0] = def;
}
