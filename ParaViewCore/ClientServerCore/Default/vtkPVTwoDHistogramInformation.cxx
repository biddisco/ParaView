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
#include "vtkPVTwoDHistogramInformation.h"
#include "vtkClientServerStream.h"
#include "vtkMultiProcessStream.h"
#include "vtkImageAccumulate.h"
#include "vtkImageData.h"
#include "vtkPointData.h"
#include "vtkObjectFactory.h" //needed for newmacro
#include "vtkImageVolumeRepresentation.h"
#include "vtkPVCompositeRepresentation.h"
#include "vtkPImageAccumulate.h"
#include "vtkVariantArray.h"
#include "vtkTable.h"



vtkStandardNewMacro(vtkPVTwoDHistogramInformation);

//----------------------------------------------------------------------------
vtkPVTwoDHistogramInformation::vtkPVTwoDHistogramInformation()
  {
  this->SizeOfHistogram = 0;
  this->GradientRange[0] = 0.0;
  this->GradientRange[1] = 1.0;
  this->CollectTwoDHistogram = 1;
  this->RootOnly = 1;
  this->dimensions[0] = 0;
  this->dimensions[1] = 0;
  this->dimensions[2] = 0;
  }

//----------------------------------------------------------------------------
vtkPVTwoDHistogramInformation::~vtkPVTwoDHistogramInformation()
  {
  }

//----------------------------------------------------------------------------

void vtkPVTwoDHistogramInformation::Initialize()
  {
  //TBD
  }

//----------------------------------------------------------------------------
void vtkPVTwoDHistogramInformation::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  os << indent << "RootOnly: " << this->RootOnly << endl;
  os << indent << "CollectTwoDHistogram: " << this->CollectTwoDHistogram << std::endl;
  os << indent << "histogramdimensions index 0 : " << this->dimensions[0] << std::endl;
  os << indent << "histogramdimensions index 1 : " << this->dimensions[1] << std::endl;
  os << indent << "histogramdimensions index 1 : " << this->dimensions[1] << std::endl;
  }

//----------------------------------------------------------------------------
void vtkPVTwoDHistogramInformation::CopyFromObject(vtkObject* obj)
  {
  if (!obj)
    {
    this->Initialize();
    }


  vtkImageVolumeRepresentation* volumerep = vtkImageVolumeRepresentation::SafeDownCast(
    vtkPVCompositeRepresentation::SafeDownCast(obj)->GetActiveRepresentation());

  this->CollectTwoDHistogram = !volumerep->GetTwoDHistogramOutOfDate();

  if (!volumerep)
    {
    vtkErrorMacro("Cannot downcast to vtkImageVolumeRepresentation.");
    this->Initialize();
    return;
    }

  if (this->CollectTwoDHistogram) {

    vtkSmartPointer<vtkPImageAccumulate> histogram = volumerep->getTwoDHistogram();
    histogram->GetOutput()->GetDimensions(this->dimensions);

    this->SizeOfHistogram = this->dimensions[0];
    this->SizeOfHistogram *= this->dimensions[1]>0 ? this->dimensions[1] : 1;
    this->SizeOfHistogram *= this->dimensions[2]>0 ? this->dimensions[2] : 1;

    this->values.resize(this->SizeOfHistogram);
    for (vtkIdType binx = 0; binx < this->dimensions[0]; ++binx)
      {
      for (vtkIdType biny = 0; biny < this->dimensions[1]; ++biny)
        {
        this->values[binx*dimensions[1]+biny] =  *static_cast<int*>(histogram->GetOutput()->GetScalarPointer(binx,biny,0));
        }
      }

    this->arrayName = "bin_values";
    }

  }

//----------------------------------------------------------------------------
void vtkPVTwoDHistogramInformation::AddInformation(vtkPVTwoDHistogramInformation*)
  {
  vtkErrorMacro("AddInformation not implemented.");
  }

//----------------------------------------------------------------------------
void vtkPVTwoDHistogramInformation::CopyFromStream(const vtkClientServerStream* stream)
  {
  int N = 0;
  if (!stream->GetArgument(0, 0, &this->CollectTwoDHistogram))
    {
    vtkErrorMacro("Error getting vars from message.");
    return;
    }
  if (this->CollectTwoDHistogram)
    {
    char* name = 0;
    for (int i = 0; i< 3; i++)
      {
      if (!stream->GetArgument(0, i+1, &this->dimensions[i]))
            {
            vtkErrorMacro("Error parsing histogram dimensions.");
            return;
            }
      }

    this->arrayName = name;

    this->SizeOfHistogram = this->dimensions[0];
    this->SizeOfHistogram *= this->dimensions[1]>0 ? this->dimensions[1] : 1;
    this->SizeOfHistogram *= this->dimensions[2]>0 ? this->dimensions[2] : 1;
    this->values.resize(this->SizeOfHistogram);
    std::cout << "values size " << values.size() << std::endl;

    for (int i = 0; i<this->SizeOfHistogram; i++){
      if (!stream->GetArgument(0, 4+i, &(values[i])))
        {
        vtkErrorMacro("Error parsing array name from message.");
        return;
        }
      }
    N = 4+this->SizeOfHistogram;
    }

  }
//-----------------------------------------------------------------------------
void vtkPVTwoDHistogramInformation::CopyToStream(vtkClientServerStream* stream)
  {
  stream->Reset();
  *stream << vtkClientServerStream::Reply;

  *stream << this->CollectTwoDHistogram;

  if (this->CollectTwoDHistogram)
    {
    *stream << this->dimensions[0];
    *stream << this->dimensions[1];
    *stream << this->dimensions[2];
    this->SizeOfHistogram = this->dimensions[0];
    this->SizeOfHistogram *= this->dimensions[1]>0 ? this->dimensions[1] : 1;
    this->SizeOfHistogram *= this->dimensions[2]>0 ? this->dimensions[2] : 1;
    for (int i = 0; i< this->SizeOfHistogram; i++)
      {
      *stream << values[i];
      }
    }

  *stream << vtkClientServerStream::End;
  }
//-----------------------------------------------------------------------------
int *vtkPVTwoDHistogramInformation::GetHistogramValues()
  {
  if (this->values.size()>0) { return &this->values[0]; }
  return NULL;
  }
