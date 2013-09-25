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
#include "vtkPVImageAccumulateInformation.h"
#include "vtkClientServerStream.h"
#include "vtkMultiProcessStream.h"
#include "vtkImageAccumulate.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkObjectFactory.h" //needed for newmacro



vtkStandardNewMacro(vtkPVImageAccumulateInformation);

//----------------------------------------------------------------------------
vtkPVImageAccumulateInformation::vtkPVImageAccumulateInformation()
{
	values = 0;
	sizeOfX = 0;
  this->RootOnly = 1;
}

//----------------------------------------------------------------------------
vtkPVImageAccumulateInformation::~vtkPVImageAccumulateInformation()
{
}

//----------------------------------------------------------------------------

void vtkPVImageAccumulateInformation::Initialize()
{
	//TBD
}

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "RootOnly: " << this->RootOnly << endl;
}

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::CopyFromObject(vtkObject* obj)
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




	int dims[3];
	histogram->GetOutput()->GetDimensions(dims);

	values = new int[dims[0]];

	for(vtkIdType bin = 0; bin < dims[0]; ++bin)
			   {
				  values[bin] =  *(static_cast<int*>(histogram->GetOutput()->GetScalarPointer(bin, 0, 0)));
			   }


	if(arrayName){
		delete arrayName;
	}
	arrayName = "bin_values";

	sizeOfX = dims[0];

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
void vtkPVImageAccumulateInformation::AddInformation(vtkPVImageAccumulateInformation*)
{
  vtkErrorMacro("AddInformation not implemented.");
}

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::CopyFromStream(const vtkClientServerStream* stream)
{
	char* name = 0;
		  if (!stream->GetArgument(0, 0, &name))
		     {
		     vtkErrorMacro("Error parsing array name from message.");
		     return;
		     }
		   this->arrayName = name;

	   int dimx;

	   if (!stream->GetArgument(0, 1, &dimx))
	   		     {
	   		     vtkErrorMacro("Error parsing array name from message.");
	   		     return;
	   		     }
	   		   this->sizeOfX = dimx;


	   		   if(this->values != NULL){
	   			   delete values;
	   		   }
	   		this->values = new int[dimx];

		for (int i = 2; i< dimx+2; i++){
		if (!stream->GetArgument(0, 2, &(values[i-1])))
			 {
			 vtkErrorMacro("Error parsing array name from message.");
			 return;
			 }
		}





}

void vtkPVImageAccumulateInformation::CopyToStream(vtkClientServerStream* stream){
	*stream << this->arrayName;
	  *stream << this->sizeOfX;
	  for (int i = 0; i< sizeOfX; i++)
	  {
		  *stream << values[i];
	  }




}





