/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDataExportDomain.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMDataExportDomain.h"

#include "vtkObjectFactory.h"
#include "vtkSMDataExportProperty.h"
#include "vtkPVXMLElement.h"

vtkStandardNewMacro(vtkSMDataExportDomain);

//---------------------------------------------------------------------------
vtkSMDataExportDomain::vtkSMDataExportDomain()
{
}

//---------------------------------------------------------------------------
vtkSMDataExportDomain::~vtkSMDataExportDomain()
{
}

//---------------------------------------------------------------------------
const char *vtkSMDataExportDomain::getCommandProperty()
{
  if (command_property.size()>0) {
    return command_property.c_str();
  }
  return NULL;
}
//---------------------------------------------------------------------------
int vtkSMDataExportDomain::SetDefaultValues(vtkSMProperty* property)
{
  vtkSMStringVectorProperty* svp = 
    vtkSMStringVectorProperty::SafeDownCast(property);
  if (svp && this->GetNumberOfStrings() > 0)
    {
    svp->SetElement(0, this->GetString(0));
    return 1;
    }

  return this->Superclass::SetDefaultValues(property);
}

//---------------------------------------------------------------------------
void vtkSMDataExportDomain::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
int vtkSMDataExportDomain::ReadXMLAttributes(
  vtkSMProperty* prop, vtkPVXMLElement* element)
{
  this->Superclass::ReadXMLAttributes(prop, element);
  //
  const char *attrib;
  std::string temp = "";
  // Loop over the top-level elements.
  if (attrib=element->GetAttribute("full_path")) {
    temp += ":full_path:" + std::string(attrib);
  }
  if (attrib=element->GetAttribute("geometry_path")) {
    temp += ":geometry_path:" + std::string(attrib);
  }
  if (attrib=element->GetAttribute("topology_path")) {
    temp += ":topology_path:" + std::string(attrib);
  }
  if (attrib=element->GetAttribute("field_path")) {
    temp += ":field_path:" + std::string(attrib);
  }
  if (attrib=element->GetAttribute("command_property")) {
    this->command_property = attrib;
  }
  temp += ":";
  if (temp!="") {
    this->AddString(temp.c_str());
    vtkSMDataExportProperty::SafeDownCast(prop)->SetDefaultValue(temp.c_str());
  }
  return 1;
}
