/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCommandProperty.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMCommandProperty.h"

#include "vtkObjectFactory.h"
#include "vtkSMVectorPropertyTemplate.h"
#include <vector>

vtkStandardNewMacro(vtkSMCommandProperty);
//---------------------------------------------------------------------------
vtkSMCommandProperty::vtkSMCommandProperty()
{
  this->IsInternal = 0;
  this->ImmediateUpdate = 1;
}
//---------------------------------------------------------------------------
vtkSMCommandProperty::~vtkSMCommandProperty()
{
}
//---------------------------------------------------------------------------
void vtkSMCommandProperty::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
