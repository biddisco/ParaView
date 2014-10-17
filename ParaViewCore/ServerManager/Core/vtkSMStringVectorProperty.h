/*=========================================================================

  Program:   ParaView
  Module:    vtkSMStringVectorProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/**
 * @class   vtkSMStringVectorProperty
 * @brief   property representing a vector of strings
 *
 * vtkSMStringVectorProperty is a concrete sub-class of vtkSMVectorProperty
 * representing a vector of strings. vtkSMStringVectorProperty can also
 * be used to store double and int values as strings. The strings
 * are converted to the appropriate type when they are being passed
 * to the stream. This is generally used for calling methods that have mixed
 * type arguments.
 * @sa
 * vtkSMVectorProperty vtkSMDoubleVectorProperty vtkSMIntVectorProperty
*/

#ifndef vtkSMStringVectorProperty_h
#define vtkSMStringVectorProperty_h

#include "vtkPVServerManagerCoreModule.h" //needed for exports
#include "vtkSMVectorProperty.h"

#include <string> // needed for std::string
#include <vector> // needed for std::vector

class vtkStringList;
class vtkSMStateLocator;

class VTKPVSERVERMANAGERCORE_EXPORT vtkSMStringVectorProperty : public vtkSMVectorProperty
{
public:
  static vtkSMStringVectorProperty* New();
  vtkTypeMacro(vtkSMStringVectorProperty, vtkSMVectorProperty);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /**
   * Returns the size of the vector.
   */
  virtual unsigned int GetNumberOfElements() VTK_OVERRIDE;

  /**
   * Sets the size of the vector. If num is larger than the current
   * number of elements, this may cause reallocation and copying.
   */
  virtual void SetNumberOfElements(unsigned int num) VTK_OVERRIDE;

  /**
   * Set the value of 1 element. The vector is resized as necessary.
   * Returns 0 if Set fails either because the property is read only
   * or the value is not in all domains. Returns 1 otherwise.
   */
  int SetElement(unsigned int idx, const char* value);

  /**
   * Sets multiple elements. The size of the property is changed to match count.
   */
  int SetElements(const char* values[], unsigned int count);

  //@{
  /**
   * Sets multiple elements. The size of the property is changed to match count.
   */
  int SetElements(vtkStringList* newvalue);
  int SetElements(const std::vector<std::string>& newvalue);
  //@}

  //@{
  /**
   * Sets the values of all the unchecked elements.
   */
  int SetUncheckedElements(const char* values[], unsigned int count);
  int SetUncheckedElements(const std::vector<std::string>& newvalue);
  //@}

  /**
   * Fills up the vtkStringList instance with the current value.
   */
  void GetElements(vtkStringList* list);

  /**
   * Returns the value of 1 element.
   */
  const char* GetElement(unsigned int idx);

  /**
   * Returns the index of an element with a particular value.
   * exists is set to false if element does not exist.
   */
  unsigned int GetElementIndex(const char* value, int& exists);

  //@{
  /**
   * Set the cast type used when passing a value to the stream.
   * For example, if the type is INT, the string is converted
   * to an int (with atoi()) before being passed to stream.
   * Note that representing scalar values as strings can result
   * in loss of accuracy.
   * Possible values are: INT, DOUBLE, STRING.
   */
  void SetElementType(unsigned int idx, int type);
  int GetElementType(unsigned int idx);
  //@}

  /**
   * Returns the value of 1 unchecked element. These are used by
   * domains. SetElement() first sets the value of 1 unchecked
   * element and then calls IsInDomain and updates the value of
   * the corresponding element only if IsInDomain passes.
   */
  const char* GetUncheckedElement(unsigned int idx);

  /**
   * Set the value of 1 unchecked element. This can be used to
   * check if a value is in all domains of the property. Call
   * this and call IsInDomains().
   */
  void SetUncheckedElement(unsigned int idx, const char* value);

  //@{
  /**
   * Get/Set unchecked elements.
   */
  void GetUncheckedElements(vtkStringList* list);
  int SetUncheckedElements(vtkStringList* list);
  //@}

  /**
   * Returns the size of unchecked elements. Usually this is
   * the same as the number of elements but can be different
   * before a domain check is performed.
   */
  virtual unsigned int GetNumberOfUncheckedElements() VTK_OVERRIDE;

  enum ElementTypes
  {
    INT,
    DOUBLE,
    STRING
  };

  /**
   * Copy all property values.
   */
  virtual void Copy(vtkSMProperty* src) VTK_OVERRIDE;

  /**
   * Returns the default value, if any, specified in the XML.
   */
  const char* GetDefaultValue(int idx);

  virtual void ClearUncheckedElements() VTK_OVERRIDE;

  virtual bool IsValueDefault() VTK_OVERRIDE;

  /**
   * For properties that support specifying defaults in XML configuration, this
   * method will reset the property value to the default values specified in the
   * XML.
   */
  virtual void ResetToXMLDefaults() VTK_OVERRIDE;

  // Description:
  // If this string vector has regular expression searches attached
  // then provide a hook so that the a string list domain can pass in a list
  // of possible strings from which a default can be selected
  bool GetDefaultUsesRegex();

  // Description:
  // Using the regular expression list, pick one of the possible string
  // to be used as the default value
  const char* GetDefaultValue(vtkStringList *list);

protected:
  vtkSMStringVectorProperty();
  ~vtkSMStringVectorProperty();

  /**
   * Sets the size of unchecked elements. Usually this is
   * the same as the number of elements but can be different
   * before a domain check is performed.
   */
  virtual void SetNumberOfUncheckedElements(unsigned int num) VTK_OVERRIDE;

  /**
   * Manage additional attribute from the XML
   * -default_values_delimiter:
   * char used to split the "default_values" into a vector.
   * -element_types:
   * StringVectorProperty may be used to store non homogeneous vector,
   * therefore we store for each element its type. [INT, DOUBLE, STRING]
   */
  virtual int ReadXMLAttributes(vtkSMProxy* parent, vtkPVXMLElement* element) VTK_OVERRIDE;

  /**
   * Let the property write its content into the stream
   */
  virtual void WriteTo(vtkSMMessage*) VTK_OVERRIDE;

  /**
   * Let the property read and set its content from the stream
   */
  virtual void ReadFrom(const vtkSMMessage*, int msg_offset, vtkSMProxyLocator*) VTK_OVERRIDE;

  /**
   * Load the XML state.
   */
  virtual int LoadState(vtkPVXMLElement* element, vtkSMProxyLocator* loader) VTK_OVERRIDE;

  // Save concrete property values into the XML state property declaration
  virtual void SaveStateValues(vtkPVXMLElement* propElement) VTK_OVERRIDE;

private:
  vtkSMStringVectorProperty(const vtkSMStringVectorProperty&) VTK_DELETE_FUNCTION;
  void operator=(const vtkSMStringVectorProperty&) VTK_DELETE_FUNCTION;

  class vtkInternals;
  vtkInternals* Internals;
};

#endif
