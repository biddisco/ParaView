/*=========================================================================

  Program:   ParaView
  Module:    vtkPVInformation.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPVInformationhistogram.h"
#include "vtkImageAccumulate.h"


//----------------------------------------------------------------------------
vtkPVInformationHistogram::vtkPVInformationHistogram()
{
  this->RootOnly = 0;
}

//----------------------------------------------------------------------------
vtkPVInformationHistogram::~vtkPVInformationHistogram()
{
}

//----------------------------------------------------------------------------

void vtkPVInformationHistogram::Inilialize()
{
	//TBD
}

//----------------------------------------------------------------------------
void vtkPVInformationHistogram::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "RootOnly: " << this->RootOnly << endl;
}

//----------------------------------------------------------------------------
void vtkPVInformationHistogram::CopyFromObject(vtkObject* obj)
{
	if (!obj)
		    {
		    this->Initialize();
		    }

	vtkImageAccumulate* histogram = vtkImageAccumulate::SafeDownCast(obj);

	if (!histogram)
		    {
		    vtkErrorMacro("Cannot downcast to histogram.");
		    this->Initialize();
		    return;
		    }


	/*
	if (!obj)
	    {
	    this->Initialize();
	    }

	  vtkAbstractArray* const array = vtkAbstractArray::SafeDownCast(obj);
	  if (!array)
	    {
	    vtkErrorMacro("Cannot downcast to abstract array.");
	    this->Initialize();
	    return;
	    }

	  this->SetName(array->GetName());
	  this->DataType = array->GetDataType();
	  this->SetNumberOfComponents(array->GetNumberOfComponents());
	  this->SetNumberOfTuples(array->GetNumberOfTuples());

	  if (array->HasAComponentName())
	    {
	    const char *name;
	    //copy the component names over
	    for (int i = 0; i < this->GetNumberOfComponents(); ++i)
	      {
	      name = array->GetComponentName(i);
	      if (name)
	        {
	        //each component doesn't have to be named
	        this->SetComponentName(i, name);
	        }
	      }
	    }

	  if (vtkDataArray* const data_array = vtkDataArray::SafeDownCast(obj))
	    {
	    double range[2];
	    double *ptr;
	    int idx;

	    ptr = this->Ranges;
	    if (this->NumberOfComponents > 1)
	      {
	      // First store range of vector magnitude.
	      data_array->GetRange(range, -1);
	      *ptr++ = range[0];
	      *ptr++ = range[1];
	      }
	    for (idx = 0; idx < this->NumberOfComponents; ++idx)
	      {
	      data_array->GetRange(range, idx);
	      *ptr++ = range[0];
	      *ptr++ = range[1];
	      }
	    }

	  if(this->InformationKeys)
	    {
	    this->InformationKeys->clear();
	    delete this->InformationKeys;
	    this->InformationKeys = 0;
	    }
	  if (array->HasInformation())
	    {
	    vtkInformation* info = array->GetInformation();
	    vtkInformationIterator* it = vtkInformationIterator::New();
	    it->SetInformationWeak(info);
	    it->GoToFirstItem();
	    while (!it->IsDoneWithTraversal())
	      {
	      vtkInformationKey* key = it->GetCurrentKey();
	      this->AddInformationKey(key->GetLocation(), key->GetName());
	      it->GoToNextItem();
	      }
	    it->Delete();
	    }
	    */
}

//----------------------------------------------------------------------------
void vtkPVInformationHistogram::AddInformation(vtkPVInformationHistogram*)
{
  vtkErrorMacro("AddInformation not implemented.");
}

//----------------------------------------------------------------------------
void vtkPVInformationHistogram::CopyFromStream(const vtkClientServerStream*)
{
  vtkErrorMacro("CopyFromStream not implemented.");
}


int vtkPVInformationHistogram::GetRootOnly(){
	return 1;

}
