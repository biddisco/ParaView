/*=========================================================================

 Program:   ParaView
 Module:    vtkImageVolumeRepresentation.cxx

 Copyright (c) Kitware, Inc.
 All rights reserved.
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
#include "vtkImageVolumeRepresentation.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCommand.h"
#include "vtkExtentTranslator.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOutlineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkPVCacheKeeper.h"
#include "vtkPVLODVolume.h"
#include "vtkPVRenderView.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkSmartVolumeMapper.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkVolumeProperty.h"
#include "vtkPointData.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkImageAccumulate.h"
#include "vtkPExtractHistogram.h"
#include "vtkGaussianPiecewiseFunction.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"
#include "vtkMultiProcessController.h"


#include <map>
#include <string>

vtkStandardNewMacro(vtkImageVolumeRepresentation);
//----------------------------------------------------------------------------
vtkImageVolumeRepresentation::vtkImageVolumeRepresentation()
{
  this->VolumeMapper = vtkSmartVolumeMapper::New();
  this->Property = vtkVolumeProperty::New();

  this->Actor = vtkPVLODVolume::New();
  this->Actor->SetProperty(this->Property);

  this->CacheKeeper = vtkPVCacheKeeper::New();

  this->OutlineSource = vtkOutlineSource::New();
  this->OutlineMapper = vtkPolyDataMapper::New();

  this->ColorArrayName = 0;
  this->ColorAttributeType = POINT_DATA;
  this->Cache = vtkImageData::New();

  this->CacheKeeper->SetInputData(this->Cache);
  this->Actor->SetLODMapper(this->OutlineMapper);

  vtkMath::UninitializeBounds(this->DataBounds);
  this->GradientVectorComponent = 0;
  this->GradientArrayName = NULL;

  this->histogram = 0;
  this->GradientHistogram = 0;
  this->histogramsize = 0;
  this->UseGradientFunction = 0;
  this->SupportHistogramWidget = true;
  // this->GradientRange = vtkDoubleArray::New();
  //this->GradientRange->SetNumberOfComponents(2);
  // GradientRange[0] = 0.0;
  // GradientRange[1] = 1.0;
  GradientRange[0] = 0;
  GradientRange[1] = 1;

  numbinsX = 100;
  HistogramBins = 100;

  connected = false;
  histogramOutOfDate = true;
  GradientRangeOutOfDate = true;

  GradientRangeFirstTimeStartup = true;
  GradientHistogramFirstTimeStartup = true;
  this->ExecuteOnClient = true;
}

//----------------------------------------------------------------------------
vtkImageVolumeRepresentation::~vtkImageVolumeRepresentation()
{
  this->VolumeMapper->Delete();
  this->Property->Delete();
  this->Actor->Delete();
  this->OutlineSource->Delete();
  this->OutlineMapper->Delete();
  this->CacheKeeper->Delete();

  this->SetColorArrayName(0);

  this->Cache->Delete();
}

//----------------------------------------------------------------------------
int
vtkImageVolumeRepresentation::FillInputPortInformation(int,
    vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int
vtkImageVolumeRepresentation::ProcessViewRequest(
    vtkInformationRequestKey* request_type, vtkInformation* inInfo,
    vtkInformation* outInfo)
{
  if (!this->Superclass::ProcessViewRequest(request_type, inInfo, outInfo))
    {
      return 0;
    }
  if (request_type == vtkPVView::REQUEST_UPDATE())
    {
      vtkPVRenderView::SetPiece(inInfo, this,
          this->OutlineSource->GetOutputDataObject(0));
      outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);

      vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds);

      vtkImageVolumeRepresentation::PassOrderedCompositingInformation(this,
          inInfo);
    }
  else if (request_type == vtkPVView::REQUEST_RENDER())
    {
      this->UpdateMapperParameters();

      vtkAlgorithmOutput* producerPort = vtkPVRenderView::GetPieceProducer(
          inInfo, this);
      if (producerPort)
        {
          this->OutlineMapper->SetInputConnection(producerPort);
        }
    }
  return 1;
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::PassOrderedCompositingInformation(
    vtkPVDataRepresentation* self, vtkInformation* inInfo)
{
  // The KdTree generation code that uses the image cuts needs to be updated
  // bigtime. But due to time shortage, I'm leaving the old code as is. We
  // will get back to it later.
  if (self->GetNumberOfInputConnections(0) == 1)
    {
      vtkAlgorithmOutput* connection = self->GetInputConnection(0, 0);
      vtkAlgorithm* inputAlgo = connection->GetProducer();
      vtkStreamingDemandDrivenPipeline* sddp =
          vtkStreamingDemandDrivenPipeline::SafeDownCast(
              inputAlgo->GetExecutive());
      vtkExtentTranslator* translator = sddp->GetExtentTranslator(
          connection->GetIndex());

      int extent[6] =
        { 1, -1, 1, -1, 1, -1 };
      sddp->GetWholeExtent(sddp->GetOutputInformation(connection->GetIndex()),
          extent);

      double origin[3], spacing[3];
      vtkImageData* image = vtkImageData::SafeDownCast(
          inputAlgo->GetOutputDataObject(connection->GetIndex()));
      image->GetOrigin(origin);
      image->GetSpacing(spacing);
      vtkPVRenderView::SetOrderedCompositingInformation(inInfo, self,
          translator, extent, origin, spacing);
    }
}

//----------------------------------------------------------------------------
int
vtkImageVolumeRepresentation::RequestData(vtkInformation* request,
    vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMath::UninitializeBounds(this->DataBounds);

  // Pass caching information to the cache keeper.
  this->CacheKeeper->SetCachingEnabled(this->GetUseCache());
  this->CacheKeeper->SetCacheTime(this->GetCacheKey());

  if (inputVector[0]->GetNumberOfInformationObjects() == 1)
    {
      this->ExecuteOnClient = true;
      vtkImageData* input = vtkImageData::GetData(inputVector[0], 0);
      if (!this->GetUsingCacheForUpdate())
        {
          this->Cache->ShallowCopy(input);
        }
      this->CacheKeeper->Update();

      this->Actor->SetEnableLOD(0);
      this->VolumeMapper->SetInputConnection(
          this->CacheKeeper->GetOutputPort());

      this->OutlineSource->SetBounds(
          vtkImageData::SafeDownCast(this->CacheKeeper->GetOutputDataObject(0))->GetBounds());
      this->OutlineSource->GetBounds(this->DataBounds);
      this->OutlineSource->Update();
    }
  else
    {
      // when no input is present, it implies that this processes is on a node
      // without the data input i.e. either client or render-server, in which case
      // we show only the outline.
      this->ExecuteOnClient = false;
      this->VolumeMapper->RemoveAllInputs();
      this->Actor->SetEnableLOD(1);
    }
  this->histogramOutOfDate = true;
  this->GradientRangeOutOfDate = true;
  return this->Superclass::RequestData(request, inputVector, outputVector);
}

//----------------------------------------------------------------------------
bool
vtkImageVolumeRepresentation::IsCached(double cache_key)
{
  return this->CacheKeeper->IsCached(cache_key);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::MarkModified()
{
  if (!this->GetUseCache())
    {
      // Cleanup caches when not using cache.
      this->CacheKeeper->RemoveAllCaches();
    }
  this->Superclass::MarkModified();
}

//----------------------------------------------------------------------------
bool
vtkImageVolumeRepresentation::AddToView(vtkView* view)
{
  // FIXME: Need generic view API to add props.
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
      rview->GetRenderer()->AddActor(this->Actor);
      return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool
vtkImageVolumeRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
    {
      rview->GetRenderer()->RemoveActor(this->Actor);
      return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::UpdateMapperParameters()
{
  this->VolumeMapper->SelectScalarArray(this->ColorArrayName);
  switch (this->ColorAttributeType)
    {
  case CELL_DATA:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_CELL_FIELD_DATA);
    break;

  case POINT_DATA:
  default:
    this->VolumeMapper->SetScalarMode(VTK_SCALAR_MODE_USE_POINT_FIELD_DATA);
    break;
    }
  this->Actor->SetMapper(this->VolumeMapper);
}
//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::setInformation()
{
  //if(!this->Information)
  //this->Information = vtkPVImageAccumulateInformation::New();

  //this->Information->CopyFromObject(this->AccumulateFilter);

}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//***************************************************************************
// Forwarded to Actor.

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetOrientation(double x, double y, double z)
{
  this->Actor->SetOrientation(x, y, z);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetOrigin(double x, double y, double z)
{
  this->Actor->SetOrigin(x, y, z);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetPickable(int val)
{
  this->Actor->SetPickable(val);
}
//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetPosition(double x, double y, double z)
{
  this->Actor->SetPosition(x, y, z);
}
//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetScale(double x, double y, double z)
{
  this->Actor->SetScale(x, y, z);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
  this->Actor->SetVisibility(val ? 1 : 0);
}

//***************************************************************************
// Forwarded to vtkVolumeProperty.
//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetInterpolationType(int val)
{
  this->Property->SetInterpolationType(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetColor(vtkColorTransferFunction* lut)
{
  this->Property->SetColor(lut);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetScalarOpacity(vtkPiecewiseFunction* pwf)
{
  this->Property->SetScalarOpacity(pwf);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetGradientLinearOpacity(vtkPiecewiseFunction* pwf)
{
  this->Property->SetGradientLinearOpacity(pwf);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetScalarGaussianOpacity(
    vtkGaussianPiecewiseFunction* pwf)
{
  this->Property->SetScalarGaussianOpacity(pwf);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetGradientGaussianOpacity(
    vtkGaussianPiecewiseFunction* pwf)
{
  this->Property->SetGradientGaussianOpacity(pwf);
}
//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetTwoDTransferFunction(
    vtkTwoDTransferFunction* pwf)
{
  this->Property->SetTwoDTransferFunction(pwf); //remoeveme
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetSwitchGradientOpacity(bool GaussOrPwf)
{
  this->Property->SwitchGradientOpacity(GaussOrPwf);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetSwitchScalarOpacity(bool GaussOrPwf)
{
  this->Property->SwitchScalarOpacity(GaussOrPwf);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetScalarOpacityUnitDistance(double val)
{
  this->Property->SetScalarOpacityUnitDistance(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetAmbient(double val)
{
  this->Property->SetAmbient(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetDiffuse(double val)
{
  this->Property->SetDiffuse(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetSpecular(double val)
{
  this->Property->SetSpecular(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetSpecularPower(double val)
{
  this->Property->SetSpecularPower(val);
}

//----------------------------------------------------------------------------
void
vtkImageVolumeRepresentation::SetShade(bool val)
{
  this->Property->SetShade(val);
}

/*
 void vtkImageVolumeRepresentation::SetHistogramBins(int nbins)
 {
 this->HistogramBins =  nbins;
 if (connected)
 AccumulateFilter->UpdateWholeExtent();
 }
 */

//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::SetIndependantComponents(bool val)
{
  this->Property->SetIndependentComponents(val);
}

//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::SetRequestedRenderMode(int mode)
{
  this->VolumeMapper->SetRequestedRenderMode(mode);
}

//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::SetDisableGradientOpacity(bool disable)
{
  if (!disable)
    {
      this->Property->DisableGradientLinearOpacityOff(0);
      this->Property->DisableGradientGaussianOpacityOff(0);
    }
  else
    {
      this->Property->DisableGradientLinearOpacityOn(0);
      this->Property->DisableGradientGaussianOpacityOn(0);
    }
}
//----------------------------------------------------------------------------
bool vtkImageVolumeRepresentation::GetDisableGradientOpacity()
{
  return this->Property->gradientOpacityDisabled(0);
}
//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::updateGradientHistogram()
{
  if (this->ExecuteOnClient) {
    AccumulateFilter->SetInputConnection(this->GradientFilter->GetOutputPort());
    AccumulateFilter->SetCustomBinRanges(GradientRange[0],GradientRange[1]);
    AccumulateFilter->SetBinCount(HistogramBins);
  /*  AccumulateFilter->SetComponentExtent(0, HistogramBins - 1, 0, 0, 0, 0);
    AccumulateFilter->SetComponentOrigin(GradientRange[0], 0, 0);
    AccumulateFilter->SetComponentSpacing(
        double(
            (GradientRange[1] - GradientRange[0]) / (double(HistogramBins - 1))),
        0.0, 0.0);*/
    AccumulateFilter->Update();
    std::cout << "updated accumulatefilter " <<std::endl;

   // int dims[3];
   // AccumulateFilter->GetOutput()->GetDimensions(dims);

    GradientHistogram = AccumulateFilter->GetOutput();
  }
  else {

  }
}
//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::updateGradRange()
{
  if (this->ExecuteOnClient) {
    this->GradientFilter->SetInputConnection(this->CacheKeeper->GetOutputPort());
    this->GradientFilter->SetDimensionality(3);
    this->GradientFilter->SetInputArrayToProcess(0, 0, 0,
        vtkDataObject::FIELD_ASSOCIATION_POINTS, this->ColorArrayName);
    std::cout << "updating whole extent" << std::endl;
    this->GradientFilter->UpdateWholeExtent();
    std::cout << "finished updating whole extent" << std::endl;
    // Get the gradient output
    vtkImageData *gradient = this->GradientFilter->GetOutput();
    // Get the gradient array
    vtkDataArray *grads = gradient->GetPointData()->GetArray(ColorArrayName); // this->GradientArrayName);
    if (!grads)
      {
      grads = gradient->GetPointData()->GetScalars();
      }
    // get the range, local to this process
    double gradient_range_local[2];
    grads->GetRange(gradient_range_local);
    // now do a parallel reduction to get the global min/max
    vtkMultiProcessController *controller = vtkMultiProcessController::GetGlobalController();
    std::cout << "controller " << controller << std::endl;
    if (controller != NULL) {
      std::cout << "reducing" << std::endl;
      controller->AllReduce(&gradient_range_local[0], &this->GradientRange[0], 1, vtkCommunicator::MIN_OP);
      controller->AllReduce(&gradient_range_local[1], &this->GradientRange[1], 1, vtkCommunicator::MAX_OP);
      std::cout << "finished reducing" << std::endl;
    }
  }
}
//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::UpdateGradientRange()
{
  std::cout << "startinggradupdate" << std::endl;
  if (GradientRangeFirstTimeStartup)
    {
	std::cout << "gradfirststartup" << std::endl;
      GradientRangeFirstTimeStartup = false;
      return;
    }
std::cout << "this->ExecuteOnClient && !GradientFilter" << std::endl;
  if (this->ExecuteOnClient && !GradientFilter)
    {
    this->GradientFilter = vtkSmartPointer<vtkImageGradientMagnitude>::New();
    }
  std::cout << "GradientRangeOutOfDate" << !GradientRangeOutOfDate << std::endl;
  if (!GradientRangeOutOfDate)//
    {
    return;
    }
  std::cout << "upgradinggradrange" << std::endl;
  updateGradRange();

  GradientRangeOutOfDate = false;
  histogramOutOfDate = true;

}
//----------------------------------------------------------------------------
void vtkImageVolumeRepresentation::UpdateHistogram()
{
std::cout << "updateHistogram" << std::endl;
  if (GradientHistogramFirstTimeStartup)
    {
      GradientHistogramFirstTimeStartup = false;
      return;
    }
  std::cout << "updateHistogram2" << std::endl;
  if (!histogramOutOfDate)//
    {
    return;
    }
  std::cout << "updateHistogram3" << std::endl;
  if (this->ExecuteOnClient && !AccumulateFilter)
    {
    this->AccumulateFilter = vtkSmartPointer<vtkPExtractHistogram>::New();
    }
  std::cout << "updateHistogram4" << std::endl;
  if (GradientRangeOutOfDate)//
    {
      UpdateGradientRange();
      histogramOutOfDate = true;
    }
  std::cout << "updateHistogram5" << std::endl;
  if (histogramOutOfDate)//
    {
    updateGradientHistogram();
    }
  std::cout << "updateHistogram6" << std::endl;
  histogramOutOfDate = false;
}
//----------------------------------------------------------------------------
bool vtkImageVolumeRepresentation::GetIsScalarGaussianFunction(){
  return this->Property->GetuseScalarGaussian();
}
//----------------------------------------------------------------------------
bool vtkImageVolumeRepresentation::GetIsGradientGaussianFunction(){
  return this->Property->GetuseGradientGaussian();
}

