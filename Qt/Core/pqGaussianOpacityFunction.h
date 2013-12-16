
#ifndef _pqGaussianOpacityFunction_h
#define _pqGaussianOpacityFunction_h

#include "pqProxy.h"

class PQCORE_EXPORT pqGaussianOpacityFunction: public pqProxy
  {
Q_OBJECT

public:
  pqGaussianOpacityFunction(const QString& group, const QString& name,
      vtkSMProxy* proxy, pqServer* server, QObject* parent = NULL);
  virtual ~pqGaussianOpacityFunction();

  /// Set the range for the opacity function. This
  /// will rescale all the control points to fit the 
  /// new range.
  void setScalarRange(double min, double max);
  };

#endif
