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
  this->SizeOfHistogramX = 0;
  this->GradientRange[0] = 0.0;
  this->GradientRange[1] = 1.0;
  this->CollectGradientHistogram = 1;
  this->CollectGradientRange = 1;
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
  os << indent << "CollectGradientHistogram: " << this->CollectGradientHistogram << endl;
  os << indent << "CollectGradientRange: " << this->CollectGradientRange << endl;
  }

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::CopyFromObject(vtkObject* obj)
  {
  if (!obj)
    {
    this->Initialize();
    }

  vtkImageVolumeRepresentation* volumerep = vtkImageVolumeRepresentation::SafeDownCast(
    vtkPVCompositeRepresentation::SafeDownCast(obj)->GetActiveRepresentation());

  if (!volumerep)
    {
    vtkErrorMacro("Cannot downcast to vtkImageVolumeRepresentation.");
    this->Initialize();
    return;
    }

  if (this->CollectGradientHistogram) {

    vtkSmartPointer<vtkPExtractHistogram> histogram = volumerep->getHistogram();

    this->SizeOfHistogramX = histogram->GetOutput()->GetNumberOfRows();
    std::cout << "sizeofx " << this->SizeOfHistogramX << std::endl;

    this->values.resize(this->SizeOfHistogramX);
    for (vtkIdType bin = 0; bin < this->SizeOfHistogramX; ++bin)
      {
      this->values[bin] =  histogram->GetOutput()->GetRow(bin)->GetValue(1).ToInt();
      }

    std::cout << "ended copy" << std::endl;
    this->arrayName = "bin_values";
    }

  if (this->CollectGradientRange) 
    {
    volumerep->GetGradientRange(this->GradientRange);
    // @TODO need to do an MPI gather of the ranges from each node containing gradient data
    }
  }

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::AddInformation(vtkPVImageAccumulateInformation*)
  {
  vtkErrorMacro("AddInformation not implemented.");
  }

//----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::CopyFromStream(const vtkClientServerStream* stream)
  {
  int N = 0;
  if (!stream->GetArgument(0, 0, &this->CollectGradientHistogram) || !stream->GetArgument(0, 1, &this->CollectGradientRange))
    {
    vtkErrorMacro("Error getting vars from message.");
    return;
    }
  if (this->CollectGradientHistogram) 
    {
    char* name = 0;
    if (!stream->GetArgument(0, 2, &name))
      {
      vtkErrorMacro("Error parsing array name from message.");
      return;
      }
    this->arrayName = name;

    int dimx;

    if (!stream->GetArgument(0, 3, &dimx))
      {
      vtkErrorMacro("Error parsing dims from message.");
      return;
      }
    this->SizeOfHistogramX = dimx;
    this->values.resize(dimx);

    for (int i = 0; i<dimx; i++){
      if (!stream->GetArgument(0, 4+i, &(values[i])))
        {
        vtkErrorMacro("Error parsing array name from message.");
        return;
        }
      }
    N = 4+dimx;
    }
  if (this->CollectGradientRange) 
    {
    if (!stream->GetArgument(0, N, &(this->GradientRange[0])) || !stream->GetArgument(0, N+1, &(this->GradientRange[1])))
      {
      vtkErrorMacro("Error getting gradient range from message.");
      return;
      }
    }
  }
//-----------------------------------------------------------------------------
void vtkPVImageAccumulateInformation::CopyToStream(vtkClientServerStream* stream)
  {
  stream->Reset();
  *stream << vtkClientServerStream::Reply; 

  *stream << this->CollectGradientHistogram;
  *stream << this->CollectGradientRange;

  if (this->CollectGradientHistogram) 
    {
    *stream << this->arrayName.c_str();
    *stream << this->SizeOfHistogramX;
    for (int i = 0; i< this->SizeOfHistogramX; i++)
      {
      *stream << values[i];
      }
    }
  if (this->CollectGradientRange) 
    {
    *stream << this->GradientRange[0] << this->GradientRange[1];
    }
  *stream << vtkClientServerStream::End;
  }
//-----------------------------------------------------------------------------
int *vtkPVImageAccumulateInformation::GetHistogramValues() 
  {
  if (this->values.size()>0) { return &this->values[0]; }
  return NULL;
  }
