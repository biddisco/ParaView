/*=========================================================================

   Program: ParaView
   Module:    pqPythonDebugLeaksView.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2. 

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "pqPythonDebugLeaksView.h"

#include "pqPythonManager.h"
#include "pqPythonDialog.h"
#include "pqPythonShell.h"
#include "pqApplicationCore.h"

#include "vtkPython.h"
#include "vtkPythonUtil.h"
#include "vtkQtDebugLeaksModel.h"

//-----------------------------------------------------------------------------
class pqPythonShellContext {
public:
  pqPythonShellContext()
  {
    this->shell()->makeCurrent();
  }

  ~pqPythonShellContext()
  {
    this->shell()->releaseControl();
  }

  pqPythonShell* shell()
  {
    return qobject_cast<pqPythonManager*>(
             pqApplicationCore::instance()->manager("PYTHON_MANAGER"))->pythonShellDialog()->shell();
  }
};

//-----------------------------------------------------------------------------
pqPythonDebugLeaksView::pqPythonDebugLeaksView(QWidget* p) : vtkQtDebugLeaksView(p)
{

}

//-----------------------------------------------------------------------------
pqPythonDebugLeaksView::~pqPythonDebugLeaksView()
{

}

//-----------------------------------------------------------------------------
void pqPythonDebugLeaksView::onObjectDoubleClicked(vtkObjectBase* object)
{
  pqPythonShellContext context;
  this->addObjectToPython(object);
}
 
//-----------------------------------------------------------------------------
void pqPythonDebugLeaksView::onClassNameDoubleClicked(const QString& className)
{
  pqPythonShellContext context;
  this->addObjectsToPython(this->model()->getObjects(className));
}

//-----------------------------------------------------------------------------
void pqPythonDebugLeaksView::addObjectToPython(vtkObjectBase* object)
{
  PyObject* mainModule = PyImport_AddModule((char*)"__main__");
  PyObject* globalDict = PyModule_GetDict(mainModule);
  PyObject* pyObj = vtkPythonUtil::GetObjectFromPointer(object);
  PyDict_SetItemString(globalDict, "obj", pyObj);
  Py_DECREF(pyObj);
}

//-----------------------------------------------------------------------------
void pqPythonDebugLeaksView::addObjectsToPython(const QList<vtkObjectBase*>& objects)
{
  PyObject* pyListObj = PyList_New(objects.size());
  for (int i = 0; i < objects.size(); ++i)
    {
    PyObject* pyObj = vtkPythonUtil::GetObjectFromPointer(objects[i]);
    PyList_SET_ITEM(pyListObj, i, pyObj);
    }

  PyObject* mainModule = PyImport_AddModule((char*)"__main__");
  PyObject* globalDict = PyModule_GetDict(mainModule);
  PyDict_SetItemString(globalDict, "objs", pyListObj);
  Py_DECREF(pyListObj);
}
