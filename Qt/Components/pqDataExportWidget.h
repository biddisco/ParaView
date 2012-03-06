/*=========================================================================

   Program:   ParaView
   Module:    pqDataExportWidget.h

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
#ifndef __pqDataExportWidget_h
#define __pqDataExportWidget_h

#include <QWidget>
#include "pqComponentsModule.h"
#include "pqVariableType.h"

class vtkSMProxy;

class PQCOMPONENTS_EXPORT pqDataExportWidget : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString value READ value WRITE setValue USER true)

public:
   /// constructor requires the proxy, property
   pqDataExportWidget(QWidget* parent = NULL);
  ~pqDataExportWidget();

  /// get the value
  QString value() const;
    
  /// Create a string to represent the GUI state
  QString GenerateValue();
  /// Turn a string into the GUI state
  void ShowValue();

  vtkSMProxy *getControlledProxy();
  void updateVariables();
  void addVariable(pqVariableType type,
    const QString& arg_name,
    bool is_partial);

  void    setCommandProperty(const char *cp);
  QString getCommandProperty();
signals:
  /// signal the value changed
  void valueChanged(QString);

  /// signal the value was edited
  /// this means the user is done changing text
  /// or the slider was moved
  void valueEdited(QString);

public slots:
  /// set the value
  void setValue(QString);

private slots:
  void fullChanged(int);
  void geometryChanged(int);
  void topologyChanged(int);
  void fieldChanged(int);
  void chooseSource(bool);

private:
  class pqImplementation;
  pqImplementation* Implementation;

  QString   Value;
  QString   CommandProperty;
  bool      BlockUpdate;
  bool      FullPressed;
};
#endif
