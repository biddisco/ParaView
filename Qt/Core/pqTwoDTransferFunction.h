
#ifndef _pqTwoDTransferFunction_h
#define _pqTwoDTransferFunction_h

#include "pqProxy.h"

class PQCORE_EXPORT pqTwoDTransferFunction: public pqProxy
  {
Q_OBJECT

public:
  pqTwoDTransferFunction(const QString& group, const QString& name,
      vtkSMProxy* proxy, pqServer* server, QObject* parent = NULL);
  virtual ~pqTwoDTransferFunction();

  /// Set the gradient and scalar range for the opacity function. This
  /// will rescale all the control points to fit the
  /// new ranges.
  void setRange(double xmin, double xmax, double ymin, double ymax);
  };

#endif
