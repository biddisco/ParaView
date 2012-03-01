/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCommandProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMCommandProperty - abstract superclass for all vector properties
// .SECTION Description
// vtkSMCommandProperty defines an interface common to all vector properties
// as well as some common settings. A vector property contains a list
// of values passed to one or more invocations of a command. How the
// values are distributed to the different invocations is controlled
// by several parameters.

#ifndef __vtkSMCommandProperty_h
#define __vtkSMCommandProperty_h

#include "vtkPVServerManagerCoreModule.h" //needed for exports
#include "vtkSMProperty.h"

class VTKPVSERVERMANAGERCORE_EXPORT vtkSMCommandProperty : public vtkSMProperty
{
public:
  static vtkSMCommandProperty* New();
  vtkTypeMacro(vtkSMCommandProperty, vtkSMProperty);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSMCommandProperty();
  ~vtkSMCommandProperty();

private:
  vtkSMCommandProperty(const vtkSMCommandProperty&); // Not implemented
  void operator=(const vtkSMCommandProperty&); // Not implemented
};

#endif
