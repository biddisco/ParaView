

#include "pqGaussianOpacityFunction.h"

#include "pqSMAdaptor.h"
#include <QPair>
#include "vtkSMDoubleVectorProperty.h"


pqGaussianOpacityFunction::pqGaussianOpacityFunction(const QString& group,
  const QString& name, vtkSMProxy* proxy, pqServer* server,
  QObject* parentObject)
: pqProxy(group, name, proxy, server, parentObject)
{
}

pqGaussianOpacityFunction::~pqGaussianOpacityFunction()
{
}

void pqGaussianOpacityFunction::setScalarRange(double min, double max)
{
  vtkSMProxy* GaussianopacityFunction = this->getProxy();
  vtkSMDoubleVectorProperty* dvp = vtkSMDoubleVectorProperty::SafeDownCast(
    GaussianopacityFunction->GetProperty("Points"));

  //TBD get property "range" and then set it through that

  QList<QVariant> controlPoints = pqSMAdaptor::getMultipleElementProperty(dvp);
  if (controlPoints.size() == 0)
    {
    return;
    }


  //TBD controlpoints don't control range
  //BUG

  int max_index = dvp->GetNumberOfElementsPerCommand() * (
    (controlPoints.size()-1)/ dvp->GetNumberOfElementsPerCommand());
  QPair<double, double> current_range(controlPoints[0].toDouble(),
    controlPoints[max_index].toDouble());

  // Adjust vtkPiecewiseFunction points to the new range.
  double dold = (current_range.second - current_range.first);
  dold = (dold > 0) ? dold : 1;

  double dnew = (max -min);

  if (dnew > 0)
    {
    double scale = dnew/dold;
    for (int cc=0; cc < controlPoints.size(); 
         cc+= 3)
      {
      controlPoints[cc] = 
        scale * (controlPoints[cc].toDouble()-current_range.first) + min;
      cc+= 2;
      controlPoints[cc] =
              scale * (controlPoints[cc].toDouble());
      }
    }
  else
    {
    // allowing an opacity transfer function with a scalar range of 0.
    // In this case, the piecewise function only contains the endpoints.
    // We are new setting defaults for midPoint (0.5) and sharpness(0.0) 
    controlPoints << 0.5 << 1.0 << 0.5 << 0.0 << 0.0 ;
  //  controlPoints << 1.0 << 1.0 << 0.5 << 0.0 << 0.0;
    }


  //TBD set range
  pqSMAdaptor::setMultipleElementProperty(dvp, controlPoints);
  GaussianopacityFunction->UpdateVTKObjects();
}


