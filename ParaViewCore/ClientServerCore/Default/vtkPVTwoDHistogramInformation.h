//the more I think about it, the worse the name of this class gets.

#ifndef __vtkPVTwoDHistogramInformation_h
#define __vtkPVTwoDHistogramInformation_h

#include "vtkPVClientServerCoreDefaultModule.h"  //needed for exports
#include "vtkPVInformation.h"
#include "vtkIntArray.h"
#include "vtkStdString.h"

class vtkClientServerStream;
class vtkMultiProcessStream;
class vtkImageData;


class VTKPVCLIENTSERVERCOREDEFAULT_EXPORT vtkPVTwoDHistogramInformation : public vtkPVInformation
{
public:
  static vtkPVTwoDHistogramInformation* New();
  vtkTypeMacro(vtkPVTwoDHistogramInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object.
  virtual void AddInformation(vtkPVTwoDHistogramInformation*);

  //BTX
  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  // Description:
  // Serialize/Deserialize the parameters that control how/what information is
  // gathered. This are different from the ivars that constitute the gathered
  // information itself. For example, PortNumber on vtkPVDataInformation
  // controls what output port the data-information is gathered from.
  virtual void CopyParametersToStream(vtkMultiProcessStream&) {};
  virtual void CopyParametersFromStream(vtkMultiProcessStream&) {};
  //ETX

  // Description:
  // Set/get whether to gather information only from the root.
  virtual int GetRootOnly() { return 1; }

  // Description:
  // Getters for gathered information about histogram and gradient ranges
  vtkGetMacro(SizeOfHistogram, int);
  int *GetHistogramValues();

  // Description:
  // turn on/off collection of Histogram data
  vtkSetMacro(CollectTwoDHistogram, int);
  vtkGetMacro(CollectTwoDHistogram, int);



  void Initialize();

protected:
  vtkPVTwoDHistogramInformation();
  ~vtkPVTwoDHistogramInformation();



  int CollectTwoDHistogram;
  int SizeOfHistogram;
  vtkStdString arrayName;
  int dimensions[3];

  //BTX
  std::vector<int> values;
  double GradientRange[2];
  //ETX
  vtkSetMacro(RootOnly, int);



  vtkPVTwoDHistogramInformation(const vtkPVTwoDHistogramInformation&); // Not implemented
  void operator=(const vtkPVTwoDHistogramInformation&); // Not implemented
};

#endif
