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
#include "vtkImageVolumeRepresentation.h"
#include "vtkPVCompositeRepresentation.h"
#include "vtkPExtractHistogram.h"
#include "vtkVariantArray.h"
#include "vtkTable.h"



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


	//std::cout << obj->GetClassName() << std::endl;

	vtkImageVolumeRepresentation* volumerep = vtkImageVolumeRepresentation::SafeDownCast(
			vtkPVCompositeRepresentation::SafeDownCast(obj)->GetActiveRepresentation());

	if (!volumerep)
		    {
		    vtkErrorMacro("Cannot downcast to vtkImageVolumeRepresentation.");
		    this->Initialize();
		    return;
		    }




	vtkSmartPointer<vtkPExtractHistogram> histogram = volumerep->getHistogram();

	int dims[3];
	//histogram->GetOutput()->GetDimensions(dims);

	sizeOfX = histogram->GetOutput()->GetNumberOfRows();
	std::cout << "sizeofx " << sizeOfX << std::endl;

	values = new int[sizeOfX];


	//vtkVariantArray * row =
	for(vtkIdType bin = 0; bin < sizeOfX; ++bin)
			   {
				  values[bin] =  histogram->GetOutput()->GetRow(bin)->GetValue(1).ToInt();
			   }

	std::cout << "ended copy" << std::endl;
	arrayName = "bin_values";



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





