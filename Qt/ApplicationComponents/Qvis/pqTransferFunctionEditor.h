/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: pqTransferFunctionEditor.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME pqTransferFunctionEditor
// .SECTION Thanks
// <verbatim>
//
//  This file is part of the PointSprites plugin developed and contributed by
//
//  Copyright (c) CSCS - Swiss National Supercomputing Centre
//
//  John Biddiscombe
//
// </verbatim>

#ifndef __pqTransferFunctionEditor_h
#define __pqTransferFunctionEditor_h

#include <QWidget>
#include <QDialog>
#include <qframe.h>

class pqPipelineRepresentation;
class vtkTwoDTransferFunction;
class vtkScalarsToColors;

class pqTransferFunctionEditor: public QFrame
{
  Q_OBJECT
  typedef QWidget Superclass;
public:
   pqTransferFunctionEditor(QWidget *parent = 0, Qt::WindowFlags f=0);
  ~pqTransferFunctionEditor();

  void setRepresentation(pqPipelineRepresentation* repr);
  //

  void initialize(vtkTwoDTransferFunction* function, vtkScalarsToColors* stc);

  void setCurrentRegion(int index);

  void generateHistogramBackground(int width, int height, std::vector<int> &array, std::vector<bool> &enabledBins, bool logScale);

  void removeHistogram();



  std::vector<bool> histogramEnabled;


protected slots :

  void onTFModeChanged(int index);
  void onOpacityLevelChanged(int value);
  void onActiveRegionChanged(int region);


protected:
  QList<QVariant> regionControlPoints();

  void setRegionControlPoints(const QList<QVariant>&);

  QList<QVariant> GetProxyValueList(const char *name);
  void SetProxyValue(const char *name, QList<QVariant> val, bool update = true);



  signals:
  void      activeRegionChanged(int);

      /// signal fired to indicate that the control points changed i.e. either they
      /// were moved, orone was added/deleted, or edited to change color, etc.
      void      controlPointsModified();

  //void  initialize();

private:
  class pqInternals;
  pqInternals* Internals;

  pqTransferFunctionEditor(const pqTransferFunctionEditor&); // Not implemented.
  void operator=(const pqTransferFunctionEditor&); // Not implemented.
};

#endif

