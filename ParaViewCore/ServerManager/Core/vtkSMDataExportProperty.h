/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDataExportProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMDataExportProperty - property representing a vector of strings
// .SECTION Description
// vtkSMDataExportProperty is a concrete sub-class of vtkSMVectorProperty
// representing a vector of strings. vtkSMDataExportProperty can also
// be used to store double and int values as strings. The strings
// are converted to the appropriate type when they are being passed
// to the stream. This is generally used for calling methods that have mixed 
// type arguments.
// .SECTION See Also
// vtkSMVectorProperty vtkSMDoubleVectorProperty vtkSMIntVectorProperty

#ifndef __vtkSMDataExportProperty_h
#define __vtkSMDataExportProperty_h

#include "vtkSMStringVectorProperty.h"

//BTX
struct vtkSMDataExportPropertyInternals;
//ETX

class vtkStringList;

class VTK_EXPORT vtkSMDataExportProperty : public vtkSMStringVectorProperty
{
public:
  static vtkSMDataExportProperty* New();
  vtkTypeMacro(vtkSMDataExportProperty, vtkSMStringVectorProperty);

  void SetDefaultValue(const char *def);

protected:
   vtkSMDataExportProperty();
  ~vtkSMDataExportProperty();


private:
  vtkSMDataExportProperty(const vtkSMDataExportProperty&); // Not implemented
  void operator=(const vtkSMDataExportProperty&); // Not implemented
};

#endif
