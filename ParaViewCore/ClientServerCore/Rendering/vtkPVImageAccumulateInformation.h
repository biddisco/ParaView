//the more I think about it, the worse the name of this class gets.

#ifndef __vtkPVImageAccumulateInformation_h
#define __vtkPVImageAccumulateInformation_h

#include "vtkPVClientServerCoreRenderingModule.h"
#include "vtkPVInformation.h"
#include "vtkIntArray.h"
#include "vtkStdString.h"

class vtkClientServerStream;
class vtkMultiProcessStream;
class vtkImageData;


class VTKPVCLIENTSERVERCORERENDERING_EXPORT vtkPVImageAccumulateInformation : public vtkPVInformation
{
public:
  static vtkPVImageAccumulateInformation* New();
  vtkTypeMacro(vtkPVImageAccumulateInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);
  int sizeOfX;
  int* values;

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object.
  virtual void AddInformation(vtkPVImageAccumulateInformation*);

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
  vtkGetMacro(RootOnly, int);



protected:
  vtkPVImageAccumulateInformation();
  ~vtkPVImageAccumulateInformation();

  void Initialize();

  int RootOnly;
  vtkSetMacro(RootOnly, int);

  vtkStdString arrayName;


  vtkPVImageAccumulateInformation(const vtkPVImageAccumulateInformation&); // Not implemented
  void operator=(const vtkPVImageAccumulateInformation&); // Not implemented
};

#endif
