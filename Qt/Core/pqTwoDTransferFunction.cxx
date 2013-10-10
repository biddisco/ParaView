

#include "pqTwoDTransferFunction.h"

#include "pqSMAdaptor.h"
#include <QPair>
#include "vtkSMDoubleVectorProperty.h"


pqTwoDTransferFunction::pqTwoDTransferFunction(const QString& group,
  const QString& name, vtkSMProxy* proxy, pqServer* server,
  QObject* parentObject)
: pqProxy(group, name, proxy, server, parentObject)
{
}

pqTwoDTransferFunction::~pqTwoDTransferFunction()
{
}



void pqTwoDTransferFunction::setRange(double xmin, double xmax, double ymin, double ymax)
{

	//FIXME

  vtkSMProxy* TwoDTransferFunction = this->getProxy();

  vtkSMDoubleVectorProperty* rvp = vtkSMDoubleVectorProperty::SafeDownCast(
      TwoDTransferFunction->GetProperty("Range"));

   vtkSMDoubleVectorProperty* dvp = vtkSMDoubleVectorProperty::SafeDownCast(
    TwoDTransferFunction->GetProperty("Points"));

  //TBD get property "range" and then set it through that

   QList<QVariant> range = pqSMAdaptor::getMultipleElementProperty(rvp); //dunno what checked or unchecked does

  QList<QVariant> controlPoints = pqSMAdaptor::getMultipleElementProperty(dvp);
  if (controlPoints.size() == 0)
    {
    return;
    }

  int max_index = dvp->GetNumberOfElementsPerCommand() * (
    (controlPoints.size()-1)/ dvp->GetNumberOfElementsPerCommand());

  //controlpoints don't control range...
  QPair<double, double> current_x_range(range[0].toDouble(),
      range[1].toDouble());

  QPair<double, double> current_y_range(range[2].toDouble(),
      range[3].toDouble());

  // Adjust vtkPiecewiseFunction points to the new range.
  double doldx = (current_x_range.second - current_x_range.first);
  doldx = (doldx > 0) ? doldx : 1;

  double doldy = (current_y_range.second - current_y_range.first);
    doldy = (doldy > 0) ? doldy : 1;

  double dnewx = (xmax -xmin);

  double dnewy = (ymax -ymin);

  if (dnewx > 0)
    {
    double xscale = dnewx/doldx;

    for (int cc=0; cc < controlPoints.size();
         cc+= 4)
      {
      controlPoints[cc] =
        xscale * (controlPoints[cc].toDouble()-current_x_range.first) + xmin;
      cc+= 2;

      controlPoints[cc] = xscale * controlPoints[cc].toDouble();

      }

    }
  if(dnewy >0){
	  double yscale = dnewy/doldy;
	  for (int cc=0; cc < controlPoints.size();
	          cc+= 4)
	       {
					controlPoints[cc] = yscale * (controlPoints[cc].toDouble()-current_y_range.first) + ymin;
					cc+=2;

					controlPoints[cc] = yscale * controlPoints[cc].toDouble();
	       }

  }
  if(dnewx <= 0 && dnewy <= 0)
    {
    controlPoints << 0.5 << 0.5 << 0.25 << 0.25 << 0.0 <<1.0;
    }

  range[0] = xmin;
  range[1] = xmax;
  range[2] = ymin;
  range[3] = ymax;

  pqSMAdaptor::setMultipleElementProperty(dvp, controlPoints);


  pqSMAdaptor::setMultipleElementProperty(rvp,range);

  TwoDTransferFunction->UpdateVTKObjects();
}
