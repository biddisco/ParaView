/*=========================================================================

 Program:   ParaView
 Module:    vtkImageVolumeRepresentation.h

 Copyright (c) Kitware, Inc.
 All rights reserved.
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
// .NAME vtkImageVolumeRepresentation - representation for showing image
// datasets as a volume.
// .SECTION Description
// vtkImageVolumeRepresentation is a representation for volume rendering
// vtkImageData. Unlike other data-representations used by ParaView, this
// representation does not support delivery to client (or render server) nodes.
// In those configurations, it merely delivers a outline for the image to the
// client and render-server and those nodes simply render the outline.
#ifndef __vtkImageVolumeRepresentation_h
#define __vtkImageVolumeRepresentation_h

#include "vtkPVClientServerCoreDefaultModule.h" //needed for exports
#include "vtkPVDataRepresentation.h"
#include "vtkSmartPointer.h"                    //needed for cleanup

class vtkColorTransferFunction;
class vtkFixedPointVolumeRayCastMapper;
class vtkImageData;
class vtkOutlineSource;
class vtkPiecewiseFunction;
class vtkGaussianPiecewiseFunction;
class vtkTwoDTransferFunction;
class vtkPolyDataMapper;
class vtkPVCacheKeeper;
class vtkPVLODVolume;
class vtkSmartVolumeMapper;
class vtkVolumeProperty;
class vtkPImageAccumulate;
class vtkImageAccumulate;
class vtkPExtractHistogram;
class vtkImageGradientMagnitude;
class vtkIntArray;
class vtkDoubleArray;
class vtkVariantArray;

class VTKPVCLIENTSERVERCOREDEFAULT_EXPORT vtkImageVolumeRepresentation: public vtkPVDataRepresentation
  {
public:
  static vtkImageVolumeRepresentation* New();vtkTypeMacro(vtkImageVolumeRepresentation, vtkPVDataRepresentation)
  ;
  void PrintSelf(ostream& os, vtkIndent indent);

  // This is same a vtkDataObject::FieldAssociation types so you can use those
  // as well.
  enum AttributeTypes
	{
	POINT_DATA = 0,
	CELL_DATA = 1
	};

  // Description:
  // Methods to control scalar coloring. ColorAttributeType defines the
  // attribute type.
  vtkSetMacro(ColorAttributeType, int);
  vtkGetMacro(ColorAttributeType, int);

  vtkGetMacro(SupportHistogramWidget,bool);
  // Description:
  // Pick the array to color with.
  vtkSetStringMacro(ColorArrayName);
  vtkGetStringMacro(ColorArrayName);

  vtkGetMacro(GradientRangeOutOfDate,bool);
  vtkGetMacro(HistogramOutOfDate,bool);
  vtkGetMacro(TwoDHistogramOutOfDate,bool);


  vtkGetVector2Macro(GradientRange, double);

  bool GetIsScalarGaussianFunction();
  bool GetIsGradientGaussianFunction();

  /*
   virtual double *GetGradientRange ()
   {
   return this->GradientRange;
   }
   virtual void GetGradientRange (double &_arg1, double &_arg2)
   {
   _arg1 = this->GradientRange[0];
   _arg2 = this->GradientRange[1];
   };
   virtual void GetGradientRange (double _arg[2])
   {
   this->GetGradientRange (_arg[0], _arg[1]);
   }
   */

  vtkSmartPointer<vtkPExtractHistogram> getHistogram()
	{
	//UpdateHistogram();
	return AccumulateFilter;
	}

  vtkSmartPointer<vtkPImageAccumulate> getTwoDHistogram()
    {
    //UpdateHistogram();
    return this->TwoDAccumulateFilter;
    }

  // Description:
  // vtkAlgorithm::ProcessRequest() equivalent for rendering passes. This is
  // typically called by the vtkView to request meta-data from the
  // representations or ask them to perform certain tasks e.g.
  // PrepareForRendering.
  virtual int ProcessViewRequest(vtkInformationRequestKey* request_type,
	  vtkInformation* inInfo, vtkInformation* outInfo);

  // Description:
  // This needs to be called on all instances of vtkGeometryRepresentation when
  // the input is modified. This is essential since the geometry filter does not
  // have any real-input on the client side which messes with the Update
  // requests.
  virtual void MarkModified();

  // Description:
  // Get/Set the visibility for this representation. When the visibility of
  // representation of false, all view passes are ignored.
  virtual void SetVisibility(bool val);

  //***************************************************************************
  // Forwarded to Actor.
  void SetOrientation(double, double, double);
  void SetOrigin(double, double, double);
  void SetPickable(int val);
  void SetPosition(double, double, double);
  void SetScale(double, double, double);

  //***************************************************************************
  // Forwarded to vtkVolumeProperty.
  void SetInterpolationType(int val);
  void SetColor(vtkColorTransferFunction* lut);
  void SetScalarOpacity(vtkPiecewiseFunction* pwf);
  void SetGradientLinearOpacity(vtkPiecewiseFunction* pwf);
  void SetScalarGaussianOpacity(vtkGaussianPiecewiseFunction* pwf);
  void SetGradientGaussianOpacity(vtkGaussianPiecewiseFunction* pwf);
  void SetTwoDTransferFunction(vtkTwoDTransferFunction* pwf);
  void SetSwitchGradientOpacity(bool GaussOrPwf);
  void SetSwitchScalarOpacity(bool GaussOrPwf);
  void SetScalarOpacityUnitDistance(double val);
  void SetAmbient(double);
  void SetDiffuse(double);
  void SetSpecular(double);
  void SetSpecularPower(double);
  void SetShade(bool);
  void SetIndependantComponents(bool);

  void SetDisableGradientOpacity(bool disable);
  void SetDisableTwoDTransferFunction(bool disable);
  void EnableUseAdjustMapperGradientRangeFactor();
  void DisableUseAdjustMapperGradientRangeFactor();

  //***************************************************************************
  // Forwarded to vtkSmartVolumeMapper.
  void SetRequestedRenderMode(int);

  void UpdateGradientRange();
  void UpdateHistogram();
  void UpdateTwoDHistogram();

  bool GetDisableGradientOpacity();
  bool GetDisableTwoDTransferFunction();

  // Description:
  // Provides access to the actor used by this representation.
  vtkPVLODVolume* GetActor()
	{
	return this->Actor;
	}
  //vtkDoubleArray* GetGradientRange() { return this->GradientRange;}

  // Description:
  // Helper method to pass input image extent information to the view to use in
  // determining the cuts for ordered compositing.
  static void PassOrderedCompositingInformation(vtkPVDataRepresentation* self,
	  vtkInformation* inInfo);

  // Description:
  // Set/Get the name of the array which will be used for gradient opacity mapping
  vtkSetStringMacro(GradientArrayName);
  vtkGetStringMacro(GradientArrayName);

  // Description:
  // If gradient is a vector filed, then specify the component (0=magnitude)
  vtkSetMacro(GradientVectorComponent, int);
  vtkGetMacro(GradientVectorComponent, int);

//BTX
  void updateGradRange();
  void updateGradientHistogram();
  void createTwoDHistogram();


protected:
  vtkImageVolumeRepresentation();
  ~vtkImageVolumeRepresentation();

  // Description:
    // Used to determine if the histogram has not been created or belongs to a different data set.
  bool HistogramOutOfDate;
  // Description:
  // Used to determine if the gradient range has not been determined or belongs to a different data set.
  bool GradientRangeOutOfDate;

  bool TwoDHistogramOutOfDate;

  // Description:
  // Fill input port information.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  virtual int RequestData(vtkInformation*, vtkInformationVector**,
	  vtkInformationVector*);

  void SaveScalarData();

  int numbinsX;
  int histogramsize;
  vtkTable* GradientHistogram;
  int* histogram;

  //used to set up the imageaccumulateinformation to send the histogram data to the client.
  void setInformation();

  //vtkDataArray* grads;

  // Description:
  // Adds the representation to the view.  This is called from
  // vtkView::AddRepresentation().  Subclasses should override this method.
  // Returns true if the addition succeeds.
  virtual bool AddToView(vtkView* view);

  // Description:
  // Removes the representation to the view.  This is called from
  // vtkView::RemoveRepresentation().  Subclasses should override this method.
  // Returns true if the removal succeeds.
  virtual bool RemoveFromView(vtkView* view);

  // Description:
  // Overridden to check with the vtkPVCacheKeeper to see if the key is cached.
  virtual bool IsCached(double cache_key);

  // Description:
  // Passes on parameters to the active volume mapper
  virtual void UpdateMapperParameters();

  vtkImageData* Cache;
  vtkPVCacheKeeper* CacheKeeper;
  vtkSmartVolumeMapper* VolumeMapper;
  vtkVolumeProperty* Property;
  vtkPVLODVolume* Actor;

  vtkOutlineSource* OutlineSource;
  vtkPolyDataMapper* OutlineMapper;
//BTX
  vtkSmartPointer<vtkImageGradientMagnitude> GradientFilter;
  vtkSmartPointer<vtkPExtractHistogram> AccumulateFilter;
  vtkSmartPointer<vtkPImageAccumulate> TwoDAccumulateFilter;
  vtkSmartPointer<vtkImageData> GradientAndScalarData;
//ETX

  int ColorAttributeType;
  char* ColorArrayName;
  char *GradientArrayName;
  int GradientVectorComponent;
  double DataBounds[6];
  double GradientRange[2];
  int HistogramBins;
  int UseGradientFunction;
  bool connected;
  bool SupportHistogramWidget;
  bool GradientRangeFirstTimeStartup;
  bool GradientHistogramFirstTimeStartup;
  bool TwoDHistogramFirstTimeStartup;
  int  ExecuteOnClient;

private:
  vtkImageVolumeRepresentation(const vtkImageVolumeRepresentation&); // Not implemented
  void operator=(const vtkImageVolumeRepresentation&); // Not implemented

//ETX
  };

#endif
