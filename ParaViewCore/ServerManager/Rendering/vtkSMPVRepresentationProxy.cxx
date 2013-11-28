/*=========================================================================

 Program:   ParaView
 Module:    vtkSMPVRepresentationProxy.cxx

 Copyright (c) Kitware, Inc.
 All rights reserved.
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notice for more information.

 =========================================================================*/
#include "vtkSMPVRepresentationProxy.h"

#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVTemporalDataInformation.h"
#include "vtkPVXMLElement.h"
#include "vtkSMOutputPort.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkPVImageAccumulateInformation.h"

#include <set>
#include <string>

class vtkSMPVRepresentationProxy::vtkStringSet: public std::set<std::string>
  {
  };

vtkStandardNewMacro(vtkSMPVRepresentationProxy);
//----------------------------------------------------------------------------
vtkSMPVRepresentationProxy::vtkSMPVRepresentationProxy()
  {
  this->SetSIClassName("vtkSIPVRepresentationProxy");
  this->RepresentationSubProxies = new vtkStringSet();
  this->InReadXMLAttributes = false;
  }

//----------------------------------------------------------------------------
vtkSMPVRepresentationProxy::~vtkSMPVRepresentationProxy()
  {
  delete this->RepresentationSubProxies;
  }

//----------------------------------------------------------------------------
void vtkSMPVRepresentationProxy::CreateVTKObjects()
  {
  if (this->ObjectsCreated)
	{
	return;
	}
  this->Superclass::CreateVTKObjects();
  if (!this->ObjectsCreated)
	{
	return;
	}

  // Ensure that we update the RepresentationTypesInfo property and the domain
  // for "Representations" property before CreateVTKObjects() is finished. This
  // ensure that all representations have valid Representations domain.
  this->UpdatePropertyInformation();

  // Whenever the "Representation" property is modified, we ensure that the
  // this->InvalidateDataInformation() is called.
  this->AddObserver(vtkCommand::UpdatePropertyEvent, this,
	  &vtkSMPVRepresentationProxy::OnPropertyUpdated);
  }

//----------------------------------------------------------------------------
void vtkSMPVRepresentationProxy::OnPropertyUpdated(vtkObject*, unsigned long,
	void* calldata)
  {
  const char* pname = reinterpret_cast<const char*>(calldata);
  if (pname && strcmp(pname, "Representation") == 0)
	{
	this->InvalidateDataInformation();
	}
  }

//----------------------------------------------------------------------------

bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToCustomRange(
	double minScale, double maxScale, double minGradient, double maxGradient,
	bool extend)
  {

  vtkSMProperty* lutProperty = this->GetProperty("LookupTable");
  vtkSMProperty* sofProperty = this->GetProperty("ScalarOpacityFunction");
  vtkSMProperty* sgaussfProperty = this->GetProperty("ScalarGaussianOpacityFunction");
  vtkSMProperty* gofProperty = this->GetProperty("GradientLinearOpacityFunction");
  vtkSMProperty* gaussfProperty = this->GetProperty("GradientGaussianOpacityFunction");
  vtkSMProperty* TDTransfProperty = this->GetProperty("TwoDTransferFunction");
  if (!lutProperty && !sofProperty)
	{
	vtkWarningMacro("No 'LookupTable' and 'ScalarOpacityFunction' found.");
	return false;
	}
  if (!lutProperty && !sgaussfProperty)
    {
    vtkWarningMacro("No 'ScalarGaussianOpacityFunction' and 'LookupTable' found.");
    }
  if (!lutProperty && !gofProperty)
	{
	vtkWarningMacro("No 'GradientLinearOpacityFunction' and 'LookupTable' found.");
	}
  if (!lutProperty && !gaussfProperty)
	{
	vtkWarningMacro("No 'GradientGaussianOpacityFunction' and 'LookupTable' found.");
	}
  if (!lutProperty && !TDTransfProperty)
	{
	vtkWarningMacro("No 'TwoDTransferFunction' and 'LookupTable' found.");
	}

  vtkSMProxy* lut = vtkSMPropertyHelper(lutProperty).GetAsProxy();
  vtkSMProxy* sof = vtkSMPropertyHelper(sofProperty).GetAsProxy();
  vtkSMProxy* sgaussf = vtkSMPropertyHelper(sgaussfProperty).GetAsProxy();
  vtkSMProxy* gof = vtkSMPropertyHelper(gofProperty).GetAsProxy();
  vtkSMProxy* gaussf = vtkSMPropertyHelper(gaussfProperty).GetAsProxy();
  vtkSMProxy* TwoDTf = vtkSMPropertyHelper(TDTransfProperty).GetAsProxy();

  double range[2] =
	{
	minScale, maxScale
	};
  double gofrange[2] =
	{
	minGradient, maxGradient
	};
  // We need to determine the component number to use from the lut.

  // If data range is too small then we tweak it a bit so scalar mapping
  // produces valid/reproducible results.
  if (lut)
	{
	vtkSMTransferFunctionProxy::RescaleTransferFunction(lut, range, extend);
	vtkSMProxy* sof_lut = vtkSMPropertyHelper(lut, "ScalarOpacityFunction",
		true).GetAsProxy();
	vtkSMProxy* sgaussf_lut = vtkSMPropertyHelper(lut,
	      "ScalarGaussianOpacityFunction", true).GetAsProxy();
	vtkSMProxy* gof_lut = vtkSMPropertyHelper(lut, "GradientLinearOpacityFunction",
		true).GetAsProxy();
	vtkSMProxy* gaussf_lut = vtkSMPropertyHelper(lut, "GradientGaussianOpacityFunction",
		true).GetAsProxy();
	vtkSMProxy* TwoDTf_lut = vtkSMPropertyHelper(lut, "TwoDTransferFunction",
		true).GetAsProxy();
	if (sof_lut && sof != sof_lut)
	  {
	  vtkSMTransferFunctionProxy::RescaleTransferFunction(sof_lut, range,
		  extend);
	  }
	if (sgaussf_lut && sgaussf != sgaussf_lut)
	      {

	      vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(
	        sgaussf_lut, range, true);
	      }
	if (gof_lut && gof != gof_lut)
	  {

	  vtkSMTransferFunctionProxy::RescaleTransferFunction(gof_lut, gofrange,
		  true);
	  }
	if (gaussf_lut && gaussf != gaussf_lut)
	  {

	  vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(gaussf_lut,
		  gofrange, true);
	  }
	if (TwoDTf_lut && TwoDTf != TwoDTf_lut)
	  {

	  vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf_lut, range,
		  gofrange, true);
	  }

	}
  if (sof)
	{
	vtkSMTransferFunctionProxy::RescaleTransferFunction(sof, range, extend);
	}
  if (sgaussf)
      {
      vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(sgaussf,
        range, true);
      }
  if (gof)
	{
	vtkSMTransferFunctionProxy::RescaleTransferFunction(gof, gofrange, true);
	}
  if (gaussf)
	{
	vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(gaussf,
		gofrange, true);
	}
  if (TwoDTf)
	{
	vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf, range,
		gofrange, true);
	}

  return (lut || sof || gof || gaussf || TwoDTf);

  }

//----------------------------------------------------------------------------
void vtkSMPVRepresentationProxy::SetPropertyModifiedFlag(const char* name,
	int flag)
  {
  if (!this->InReadXMLAttributes && name && strcmp(name, "Input") == 0)
	{
	// Whenever the input for the representation is set, we need to setup the
	// the input for the internal selection representation that shows the
	// extracted-selection. This is done at the proxy level so that whenever the
	// selection is changed in the application, the SelectionRepresentation is
	// 'MarkedModified' correctly, so that it updates itself cleanly.
	vtkSMProxy* selectionRepr = this->GetSubProxy("SelectionRepresentation");
	vtkSMPropertyHelper helper(this, name);
	for (unsigned int cc = 0; cc < helper.GetNumberOfElements(); cc++)
	  {
	  vtkSMSourceProxy* input = vtkSMSourceProxy::SafeDownCast(
		  helper.GetAsProxy(cc));
	  if (input && selectionRepr)
		{
		input->CreateSelectionProxies();
		vtkSMSourceProxy* esProxy = input->GetSelectionOutput(
			helper.GetOutputPort(cc));
		if (!esProxy)
		  {
		  vtkErrorMacro("Input proxy does not support selection extraction.");
		  }
		else
		  {
		  vtkSMPropertyHelper(selectionRepr, "Input").Set(esProxy);
		  selectionRepr->UpdateVTKObjects();
		  }
		}
	  }
	}

  this->Superclass::SetPropertyModifiedFlag(name, flag);
}

//----------------------------------------------------------------------------
int vtkSMPVRepresentationProxy::ReadXMLAttributes(vtkSMSessionProxyManager* pm,
	vtkPVXMLElement* element)
  {
  this->InReadXMLAttributes = true;
  for (unsigned int cc = 0; cc < element->GetNumberOfNestedElements(); ++cc)
	{
	vtkPVXMLElement* child = element->GetNestedElement(cc);
	if (child->GetName() &&
	strcmp(child->GetName(), "RepresentationType") == 0 &&
	child->GetAttribute("subproxy") != NULL)
	  {
	  this->RepresentationSubProxies->insert(child->GetAttribute("subproxy"));
	  }
	}

  int retVal = this->Superclass::ReadXMLAttributes(pm, element);
  this->InReadXMLAttributes = false;

  // Setup property links for sub-proxies. This ensures that whenever the
  // this->GetProperty("Input") changes (either checked or un-checked values),
  // all the sub-proxy's "Input" is also changed to the same value. This ensures
  // that the domains are updated correctly.
  vtkSMProperty* inputProperty = this->GetProperty("Input");
  if (inputProperty)
    {
    for (vtkStringSet::iterator iter = this->RepresentationSubProxies->begin();
      iter != this->RepresentationSubProxies->end(); ++iter)
      {
      vtkSMProxy* subProxy = this->GetSubProxy((*iter).c_str());
      vtkSMProperty* subProperty = subProxy? subProxy->GetProperty("Input") : NULL;
      if (subProperty)
        {
        this->LinkProperty(inputProperty, subProperty);
        }
      }
    }
  return retVal;
  }

//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::GetUsingScalarColoring()
  {
  if (this->GetProperty("ColorArrayName"))
	{
	vtkSMPropertyHelper helper(this->GetProperty("ColorArrayName"));
	return (helper.GetNumberOfElements() == 1 && helper.GetAsString(0) != NULL
		&& strcmp(helper.GetAsString(0), "") != 0);
	}
  else
	{
	vtkWarningMacro("Missing 'ColorArrayName' property.");
	}
  return false;
  }

//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(bool extend)
  {
  if (!this->GetUsingScalarColoring())
	{
	// we are not using scalar coloring, nothing to do.
	return false;
	}

  if (!this->GetProperty("ColorAttributeType"))
	{
	vtkWarningMacro("Missing 'ColorAttributeType' property.");
	return false;
	}

  return this->RescaleTransferFunctionToDataRange(
	  vtkSMPropertyHelper(this->GetProperty("ColorArrayName")).GetAsString(0),
	  vtkSMPropertyHelper(this->GetProperty("ColorAttributeType")).GetAsInt(0),
	  extend);
  }

//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionToDataRange(bool extend)
  {
  if (!this->GetUsingScalarColoring())
	{
	// we are not using scalar coloring, nothing to do.
	return false;
	}

  if (!this->GetProperty("ColorAttributeType"))
	{
	vtkWarningMacro("Missing 'ColorAttributeType' property.");
	return false;
	}

  return this->RescaleGradientTransferFunctionToDataRange(
	  vtkSMPropertyHelper(this->GetProperty("ColorArrayName")).GetAsString(0),
	  vtkSMPropertyHelper(this->GetProperty("ColorAttributeType")).GetAsInt(0),
	  extend);
  }


//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(
	const char* arrayname, int attribute_type, bool extend)
  {
  vtkSMPropertyHelper inputHelper(this->GetProperty("Input"));
  vtkSMSourceProxy* inputProxy = vtkSMSourceProxy::SafeDownCast(
	  inputHelper.GetAsProxy());
  int port = inputHelper.GetOutputPort();
  if (!inputProxy)
	{
	// no input.
	vtkWarningMacro("No input present. Cannot determine data ranges.");
	return false;
	}

  vtkPVDataInformation* dataInfo = inputProxy->GetDataInformation(port);
  vtkPVArrayInformation* info = dataInfo->GetArrayInformation(arrayname,
	  attribute_type);
  if (!info)
	{
	vtkPVDataInformation* representedDataInfo =
		this->GetRepresentedDataInformation();
	info = representedDataInfo->GetArrayInformation(arrayname, attribute_type);
	}


  return this->RescaleTransferFunctionToDataRange(info, extend);
  }


//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionToDataRange(const char* arrayname, int attribute_type, bool extend)
  {


  vtkPVImageAccumulateInformation * gradientInfo = NULL;
    if (this->GetProperty("GradientRange")){
  	gradientInfo = this->GetRepresentedGradientDataInformation();
    }
    else{
  	return true; //nothing to scale
    }

  vtkSMPropertyHelper inputHelper(this->GetProperty("Input"));
    vtkSMSourceProxy* inputProxy = vtkSMSourceProxy::SafeDownCast(
  	  inputHelper.GetAsProxy());
    int port = inputHelper.GetOutputPort();
    if (!inputProxy)
  	{
  	// no input.
  	vtkWarningMacro("No input present. Cannot determine data ranges.");
  	return false;
  	}

    vtkPVDataInformation* dataInfo = inputProxy->GetDataInformation(port);
    vtkPVArrayInformation* info = dataInfo->GetArrayInformation(arrayname,
  	  attribute_type);
    if (!info)
  	{
  	vtkPVDataInformation* representedDataInfo =
  		this->GetRepresentedDataInformation();
  	info = representedDataInfo->GetArrayInformation(arrayname, attribute_type);
  	}




  return this->RescaleGradientTransferFunctionToDataRange(gradientInfo, info, extend);
  }



//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRangeOverTime()
  {
  if (!this->GetUsingScalarColoring())
	{
	// we are not using scalar coloring, nothing to do.
	return false;
	}

  if (!this->GetProperty("ColorAttributeType"))
	{
	vtkWarningMacro("Missing 'ColorAttributeType' property.");
	return false;
	}

  return this->RescaleTransferFunctionToDataRangeOverTime(
	  vtkSMPropertyHelper(this->GetProperty("ColorArrayName")).GetAsString(0),
	  vtkSMPropertyHelper(this->GetProperty("ColorAttributeType")).GetAsInt(0));
  }

//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRangeOverTime(
	const char* arrayname, int attribute_type)
  {
  vtkSMPropertyHelper inputHelper(this->GetProperty("Input"));
  vtkSMSourceProxy* inputProxy = vtkSMSourceProxy::SafeDownCast(
	  inputHelper.GetAsProxy());
  int port = inputHelper.GetOutputPort();
  if (!inputProxy || !inputProxy->GetOutputPort(port))
	{
	// no input.
	vtkWarningMacro("No input present. Cannot determine data ranges.");
	return false;
	}

  vtkPVTemporalDataInformation* dataInfo =
	  inputProxy->GetOutputPort(port)->GetTemporalDataInformation();
  vtkPVArrayInformation* info = dataInfo->GetArrayInformation(arrayname,
	  attribute_type);
  return info ? this->RescaleTransferFunctionToDataRange(info) : false;
  }



//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionToDataRange(
	  vtkPVImageAccumulateInformation * gradientInfo, vtkPVArrayInformation* info, bool extend){
  if (!info)
 	{
 	vtkWarningMacro("Could not determine array range.");
 	return false;
 	}

   vtkSMProperty* lutProperty = this->GetProperty("LookupTable");
   vtkSMProperty* gofProperty = this->GetProperty("GradientLinearOpacityFunction");
   vtkSMProperty* ggaussfProperty = this->GetProperty("GradientGaussianOpacityFunction");
   vtkSMProperty* TDTransfProperty = this->GetProperty("TwoDTransferFunction");

   if (!lutProperty && !gofProperty)
 	{
 	vtkWarningMacro("No 'GradientOpacityFunction' and 'LookupTable' found.");
 	}
   if (!lutProperty && !ggaussfProperty)
 	{
 	vtkWarningMacro("No 'GaussianOpacityFunction' and 'LookupTable' found.");
 	}
   if (!lutProperty && !TDTransfProperty)
 	{
 	vtkWarningMacro("No 'TwoDTransferFunction' and 'LookupTable' found.");
 	}

   vtkSMProxy* lut = vtkSMPropertyHelper(lutProperty).GetAsProxy();
   vtkSMProxy* gof = vtkSMPropertyHelper(gofProperty).GetAsProxy();
   vtkSMProxy* ggaussf = vtkSMPropertyHelper(ggaussfProperty).GetAsProxy();
   vtkSMProxy* TwoDTf = vtkSMPropertyHelper(TDTransfProperty).GetAsProxy();

 ///  vtkSMProperty *prop = repr->getProxy()->GetProperty("GradientRange");

   double gofrange[2] =
 	{
 	0, 1
 	};

   if(gradientInfo != 0)
 	{
 	gradientInfo->GetGradientRange(gofrange);
 	}

   /*
   bool hasgradrange = this->GetProperty("GradientRange");
   if(hasgradrange){
   this->UpdatePropertyInformation(this->GetProperty("GradientRange"));
   vtkSMPropertyHelper(this, "GradientRange").Get(gofrange, 2);
   }
   */
   // We need to determine the component number to use from the lut.
   int component = -1;
   if (lut && vtkSMPropertyHelper(lut, "VectorMode").GetAsInt() != 0)
 	{
 	component = vtkSMPropertyHelper(lut, "VectorComponent").GetAsInt();
 	}

   if (component < info->GetNumberOfComponents())
 	{
 	double range[2];
 	info->GetComponentRange(component, range);
 	if (range[1] >= range[0])
 	  {
 	  if ((range[1] - range[0] < 1e-5))
 		{
 		range[1] = range[0] + 1e-5;
 		}
 	  if ((gofrange[1] - gofrange[0] < 1e-5))
 		{
 		gofrange[1] = gofrange[0] + 1e-5;
 		}
 	  // If data range is too small then we tweak it a bit so scalar mapping
 	  // produces valid/reproducible results.
 	  if (lut)
 		{
 		vtkSMTransferFunctionProxy::RescaleTransferFunction(lut, range, extend);
 		vtkSMProxy* sof_lut = vtkSMPropertyHelper(lut, "ScalarOpacityFunction",
 			true).GetAsProxy();
 		vtkSMProxy* gof_lut = vtkSMPropertyHelper(lut,
 			"GradientLinearOpacityFunction", true).GetAsProxy();
 		vtkSMProxy* ggaussf_lut = vtkSMPropertyHelper(lut,
 			"GradientGaussianOpacityFunction", true).GetAsProxy();
 		vtkSMProxy* TwoDTf_lut = vtkSMPropertyHelper(lut,
 			"TwoDTransferFunction", true).GetAsProxy();
 		if (gof_lut && gof != gof_lut)
 		  {

 		  vtkSMTransferFunctionProxy::RescaleTransferFunction(gof_lut, gofrange,
 			  true);
 		  }
 		if (ggaussf_lut && ggaussf != ggaussf_lut)
 		  {

 		  vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(
 			  ggaussf_lut, gofrange, true);
 		  }
 		if (TwoDTf_lut && TwoDTf != TwoDTf_lut)
 		  {

 		  vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf_lut,
 			  range, gofrange, true);
 		  }

 		}
 	  if (gof)
 		{
 		vtkSMTransferFunctionProxy::RescaleTransferFunction(gof, gofrange,
 			true);
 		}
 	  if (ggaussf)
 		{
 		vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(ggaussf,
 			gofrange, true);
 		}
 	  if (TwoDTf)
 		{
 		vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf, range,
 			gofrange, true);
 		}

 	  return (lut || gof || ggaussf || TwoDTf);
 	  }

 	}
   return false;




}



//----------------------------------------------------------------------------
bool vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(
	vtkPVArrayInformation* info, bool extend)
  {

  double gofrange[2] = {0,1};

  vtkPVImageAccumulateInformation * gradientInfo = NULL;
     if (this->GetProperty("GradientRange")){
   	gradientInfo = this->GetRepresentedGradientDataInformation();
   	gradientInfo->GetGradientRange(gofrange);
     }






  if (!info)
	{
	vtkWarningMacro("Could not determine array range.");
	return false;
	}

  vtkSMProperty* lutProperty = this->GetProperty("LookupTable");
  vtkSMProperty* sofProperty = this->GetProperty("ScalarOpacityFunction");
  vtkSMProperty* sgaussfProperty = this->GetProperty(
	  "ScalarGaussianOpacityFunction");
  vtkSMProperty* gofProperty = this->GetProperty("GradientLinearOpacityFunction");
     vtkSMProperty* ggaussfProperty = this->GetProperty("GradientGaussianOpacityFunction");
     vtkSMProperty* TDTransfProperty = this->GetProperty("TwoDTransferFunction");

  if (!lutProperty && !sofProperty)
	{
	vtkWarningMacro("No 'LookupTable' and 'ScalarOpacityFunction' found.");
	return false;
	}
  if (!lutProperty && !sgaussfProperty)
	{
	vtkWarningMacro(
		"No 'ScalarGaussianOpacityFunction' and 'LookupTable' found.");
	}

  if (!lutProperty && !gofProperty)
  	{
  	vtkWarningMacro("No 'GradientOpacityFunction' and 'LookupTable' found.");
  	}
    if (!lutProperty && !ggaussfProperty)
  	{
  	vtkWarningMacro("No 'GaussianOpacityFunction' and 'LookupTable' found.");
  	}
    if (!lutProperty && !TDTransfProperty)
  	{
  	vtkWarningMacro("No 'TwoDTransferFunction' and 'LookupTable' found.");
  	}


  vtkSMProxy* lut = vtkSMPropertyHelper(lutProperty).GetAsProxy();
  vtkSMProxy* sof = vtkSMPropertyHelper(sofProperty).GetAsProxy();
  vtkSMProxy* sgaussf = vtkSMPropertyHelper(sgaussfProperty).GetAsProxy();
  vtkSMProxy* gof = vtkSMPropertyHelper(gofProperty).GetAsProxy();
     vtkSMProxy* ggaussf = vtkSMPropertyHelper(ggaussfProperty).GetAsProxy();
     vtkSMProxy* TwoDTf = vtkSMPropertyHelper(TDTransfProperty).GetAsProxy();

///  vtkSMProperty *prop = repr->getProxy()->GetProperty("GradientRange");


  // We need to determine the component number to use from the lut.
  int component = -1;
  if (lut && vtkSMPropertyHelper(lut, "VectorMode").GetAsInt() != 0)
	{
	component = vtkSMPropertyHelper(lut, "VectorComponent").GetAsInt();
	}

  if (component < info->GetNumberOfComponents())
	{
	double range[2];
	info->GetComponentRange(component, range);
	if (range[1] >= range[0])
	  {
	  if ((range[1] - range[0] < 1e-5))
		{
		range[1] = range[0] + 1e-5;
		}
	  // If data range is too small then we tweak it a bit so scalar mapping
	  // produces valid/reproducible results.
	  if (lut)
		{
		vtkSMTransferFunctionProxy::RescaleTransferFunction(lut, range, extend);
		vtkSMProxy* sof_lut = vtkSMPropertyHelper(lut, "ScalarOpacityFunction",
			true).GetAsProxy();

		vtkSMProxy* sgaussf_lut = vtkSMPropertyHelper(lut,
			"ScalarGaussianOpacityFunction", true).GetAsProxy();
		vtkSMProxy* gof_lut = vtkSMPropertyHelper(lut,
			  "GradientLinearOpacityFunction", true).GetAsProxy();
		  vtkSMProxy* ggaussf_lut = vtkSMPropertyHelper(lut,
			  "GradientGaussianOpacityFunction", true).GetAsProxy();
		  vtkSMProxy* TwoDTf_lut = vtkSMPropertyHelper(lut,
			  "TwoDTransferFunction", true).GetAsProxy();
		if (sof_lut && sof != sof_lut)
		  {
		  vtkSMTransferFunctionProxy::RescaleTransferFunction(sof_lut, range,
			  extend);
		  }
		if (sgaussf_lut && sgaussf != sgaussf_lut)
		  {

		  vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(
			  sgaussf_lut, range, true);
		  }
		if (gof_lut && gof != gof_lut)
		  {

		  vtkSMTransferFunctionProxy::RescaleTransferFunction(gof_lut, gofrange,
			  true);
		  }
		if (ggaussf_lut && ggaussf != ggaussf_lut)
		  {

		  vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(
			  ggaussf_lut, gofrange, true);
		  }
		if (TwoDTf_lut && TwoDTf != TwoDTf_lut)
		  {

		  vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf_lut,
			  range, gofrange, true);
		  }

		}
	  if (sof)
		{
		vtkSMTransferFunctionProxy::RescaleTransferFunction(sof, range, extend);
		}
	  if (sgaussf)
		{
		vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(sgaussf,
			range, true);
		}
	  if (gof)
		{
		vtkSMTransferFunctionProxy::RescaleTransferFunction(gof, gofrange,
			true);
		}
	  if (ggaussf)
		{
		vtkSMTransferFunctionProxy::RescaleGaussianTransferFunction(ggaussf,
			gofrange, true);
		}
	  if (TwoDTf)
		{
		vtkSMTransferFunctionProxy::RescaleTwoDTransferFunction(TwoDTf, range,
			gofrange, true);
		}

	  return (lut || sof || sgaussf || gof || ggaussf || TwoDTf);
	  }

	}
  return false;
  }

//----------------------------------------------------------------------------
void vtkSMPVRepresentationProxy::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os, indent);
  }
