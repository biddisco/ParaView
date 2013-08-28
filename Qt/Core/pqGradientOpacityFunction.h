/*=========================================================================



=========================================================================*/

#ifndef _pqGradientOpacityFunction_h
#define _pqGradientOpacityFunction_h

#include "pqScalarOpacityFunction.h"


class PQCORE_EXPORT pqGradientOpacityFunction : public pqScalarOpacityFunction
{
  Q_OBJECT

public:
  pqGradientOpacityFunction(const QString& group, const QString& name,
    vtkSMProxy* proxy, pqServer* server, QObject* parent=NULL);
  virtual ~pqGradientOpacityFunction();

  /// Set the scalar range for the opacity function. This
  /// will rescale all the control points to fit the 
  /// scalar range.
  void setScalarRange(double min, double max);
};

#endif
