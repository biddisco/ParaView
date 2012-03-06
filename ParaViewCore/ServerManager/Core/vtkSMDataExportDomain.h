/*=========================================================================

  Program:   ParaView
  Module:    vtkSMDataExportDomain.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMDataExportDomain - double interval specified by min and max
// .SECTION Description
// vtkSMDataExportDomain represents an interval in real space (using
// double precision) specified using a min and a max value.
// Valid XML attributes are:
// @verbatim
// * min 
// * max
// @endverbatim
// Both min and max attributes can have one or more space space
// separated (double) arguments.
// Optionally, a Required Property may be specified (which typically is a
// information property) which can be used to obtain the range for the values as
// follows:
// @verbatim
// <DoubleRangeDomain ...>
//    <RequiredProperties>
//      <Property name="<InfoPropName>" function="RangeInfo" />
//    </RequiredProperties>
// </DoubleRangeDomain>
// @endverbatim
// .SECTION See Also
// vtkSMDomain 

#ifndef __vtkSMDataExportDomain_h
#define __vtkSMDataExportDomain_h

#include "vtkSMStringListDomain.h"

class VTK_EXPORT vtkSMDataExportDomain : public vtkSMStringListDomain
{
public:
  static vtkSMDataExportDomain* New();
  vtkTypeMacro(vtkSMDataExportDomain, vtkSMStringListDomain);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // A vtkSMProperty is often defined with a default value in the
  // XML itself. However, many times, the default value must be determined
  // at run time. To facilitate this, domains can override this method
  // to compute and set the default value for the property.
  // Note that unlike the compile-time default values, the
  // application must explicitly call this method to initialize the
  // property.
  // Returns 1 if the domain updated the property.
  virtual int SetDefaultValues(vtkSMProperty*);

  virtual int ReadXMLAttributes(vtkSMProperty* prop, vtkPVXMLElement* element);
  const char *getCommandProperty();

protected:
  vtkSMDataExportDomain();
  ~vtkSMDataExportDomain();

  std::string command_property;

private:
  vtkSMDataExportDomain(const vtkSMDataExportDomain&); // Not implemented
  void operator=(const vtkSMDataExportDomain&); // Not implemented
};

#endif
