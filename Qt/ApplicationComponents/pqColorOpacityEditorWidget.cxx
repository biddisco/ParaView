/*=========================================================================

 Program: ParaView
 Module:    $RCSfile$

 Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
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

 ========================================================================*/
#include "pqColorOpacityEditorWidget.h"
#include "ui_pqColorOpacityEditorWidget.h"

#include "pqActiveObjects.h"
#include "pqColorPresetManager.h"
#include "pqDataRepresentation.h"
#include "pqPropertiesPanel.h"
#include "pqPropertyWidgetDecorator.h"
#include "pqRescaleRange.h"
#include "pqTransferFunctionWidget.h"
#include "pqUndoStack.h"
#include "vtkCommand.h"
#include "vtkDiscretizableColorTransferFunctionCollection.h"
#include "vtkEventQtSlotConnect.h"
#include "vtkNew.h"
#include "vtkPiecewiseFunction.h"
#include "vtkGaussianPiecewiseFunction.h"
#include "vtkPVXMLElement.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkVector.h"
#include "vtkPVImageAccumulateInformation.h"
#include "pqHistogramDialog.h"
//#include "QvisGaussianOpacityBar.h"

#include <QDoubleValidator>
#include <QMessageBox>
#include <QPointer>
#include <QtDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <iostream>
namespace
{
//-----------------------------------------------------------------------------
// Decorator used to hide the widget when using IndexedLookup.
  class pqColorOpacityEditorWidgetDecorator : public pqPropertyWidgetDecorator
  {
    typedef pqPropertyWidgetDecorator Superclass;
    bool Hidden;
  public:
    pqColorOpacityEditorWidgetDecorator(vtkPVXMLElement* xmlArg,
        pqPropertyWidget* parentArg) :
        Superclass(xmlArg, parentArg), Hidden(false)
    {
    }
    virtual
    ~pqColorOpacityEditorWidgetDecorator()
    {
    }

    void
    setHidden(bool val)
    {
      if (val != this->Hidden)
        {
          this->Hidden = val;
          emit this->visibilityChanged();
        }
    }
    virtual bool
    canShowWidget(bool show_advanced) const
    {
      Q_UNUSED(show_advanced);
      return !this->Hidden;
    }

  private:
    Q_DISABLE_COPY(pqColorOpacityEditorWidgetDecorator)
    ;
  };
}

//-----------------------------------------------------------------------------
class pqColorOpacityEditorWidget::pqInternals
{
public:
  Ui::ColorOpacityEditorWidget Ui;
  QPointer<pqColorOpacityEditorWidgetDecorator> Decorator;

  // We use this pqPropertyLinks instance to simply monitor smproperty changes.
  pqPropertyLinks LinksForMonitoringChanges;
  vtkNew<vtkEventQtSlotConnect> VTKConnector;

  pqInternals(pqColorOpacityEditorWidget* self)
  {
    this->Ui.setupUi(self);
    this->Ui.CurrentDataValue->setValidator(new QDoubleValidator(self));
    this->Ui.mainLayout->setMargin(pqPropertiesPanel::suggestedMargin());
    //this->Ui.mainLayout->setSpacing(
    //  pqPropertiesPanel::suggestedVerticalSpacing());

    this->Decorator = new pqColorOpacityEditorWidgetDecorator(NULL, self);
  }
};

//-----------------------------------------------------------------------------
pqColorOpacityEditorWidget::pqColorOpacityEditorWidget(vtkSMProxy* smproxy,
    vtkSMPropertyGroup* smgroup, QWidget* parentObject) :
    Superclass(smproxy, parentObject), Internals(new pqInternals(this))
{
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());

  vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;
  int size = pwf->GetSize();
  if (pwf)
    {
      ui.OpacityEditor->initialize(stc, false, pwf, true);

    }
  else
    {
      ui.OpacityEditor->hide();
    }

  pwf = stc ? stc->GetGradientOpacityFunction() : NULL;
  size = pwf->GetSize();
  if (pwf)
    {

      ui.GradientOpacityEditor->initialize(NULL, false, pwf, true);
    }
  else
    {
      ui.GradientOpacityEditor->hide();
    }

  if (stc)
    {
      ui.ColorEditor->initialize(stc, true, NULL, false);
    }

  vtkGaussianPiecewiseFunction* gpwf =
      stc ? stc->GetGaussianOpacityFunction() : NULL;
  size = gpwf->GetSize();
  if (gpwf)
    {
      //TBD initialize stuff for gaussian
      ui.GaussianOpacityEditor->initialize(gpwf);
    }
  else
    {
      ui.GaussianOpacityEditor->hide();
    }

  vtkTwoDTransferFunction* tdtf = stc ? stc->GetTwoDTransferFunction() : NULL;
  if (tdtf)
    {
      //TBD initialize stuff for gaussian
      ui.TwoDTransferFunction->initialize(tdtf);
    }
  else
    {
      ui.TwoDTransferFunction->hide();
    }

  //change this to gaussian part---
  /*  Ui:QvisGaussianOpacityBar &ui = this->Internals->Ui;
   QvisGaussianOpacityBar* gaussianEditor =
   vtkDiscretizableColorTransferFunction::SafeDownCast(
   this->proxy()->GetClientSideObject());
   */
  //----
//--------------------------------------------------------------
  /*
   Experimental addition of colour data to Qvis widgets
   */
//--------------------------------------------------------------
  if (stc)
    {
      int N = 0;
      const unsigned char *colors = NULL;
      double minmax[2] =
        { 0.0, 1.0 };
      vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;
      if (pwf)
        {
//      ui.OpacityEditor->initialize(stc, false, pwf, true);
        }
      if (stc)
        {
          N = stc->GetNumberOfValues();
          stc->GetRange(minmax);
          colors = stc->GetTable(minmax[0], minmax[1], N);
        }

      if (N > 0 && colors)
        {
          //TBD uncomment the gaussian line after the guassian stuff is finished. Currently
          //I'm worried that it somehow might cause problems (though I think it doesn't).
          // ui.GaussianOpacityEditor->setBackgroundColourData(N, 3, colors);
          ui.TwoDTransferFunction->setUnderlayColourData(N, 3, colors);
//      this->Internals->XMin->setValue(minmax[0]);
//      this->Internals->XMax->setValue(minmax[1]);
        }
    }

//--------------------------------------------------------------
  /*
   End : Experimental addition of colour data to Qvis widgets
   */
//--------------------------------------------------------------
  disableGradientOpacity = false;

//  ui.DisableOpacityGradient->setCheckable(true);
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (repr->getProxy()->GetProperty("DisableGradientOpacity"))
    {
      QObject::connect(ui.DisableOpacityGradient, SIGNAL(clicked()), this,
      SLOT(disableGradientOpacty()));
      this->addPropertyLink(ui.DisableOpacityGradient, "checked",
      SIGNAL(clicked()), repr->getProxy(),
          repr->getProxy()->GetProperty("DisableGradientOpacity"));
    }
  else
    {
      ui.DisableOpacityGradient->hide();
    }

  if (repr->getProxy()->GetProperty("SwitchGradientOpacity"))
    {
      QObject::connect(ui.gaussorgrad, SIGNAL(clicked()), this,
      SLOT(switchGradientOpacity()));
      this->addPropertyLink(ui.gaussorgrad, "checked", SIGNAL(clicked()),
          repr->getProxy(),
          repr->getProxy()->GetProperty("SwitchGradientOpacity"));
    }
  else
    {
      ui.gaussorgrad->hide();
    }

  // vtkImageVolumeRepresentation* volumerep = vtkImageVolumeRepresentation::SafeDownCast(
  //			vtkPVCompositeRepresentation::SafeDownCast(obj)->GetActiveRepresentation());
  pqDataRepresentation* volumerepr =
      pqActiveObjects::instance().activeRepresentation();
  //vtkPVCompositeRepresentation* volumerep = vtkPVCompositeRepresentation::SafeDownCast(volumerepr);
  if (volumerepr->getProxy()->GetProperty("SupportHistogramWidget"))
    QObject::connect(ui.HistogramDialog, SIGNAL(clicked()), this,
    SLOT(showHistogramWidget()));
  else
    ui.HistogramDialog->hide();

  QObject::connect(ui.OpacityEditor, SIGNAL(currentPointChanged(vtkIdType)),
      this, SLOT(opacityCurrentChanged(vtkIdType)));
  QObject::connect(ui.ColorEditor, SIGNAL(currentPointChanged(vtkIdType)), this,
      SLOT(colorCurrentChanged(vtkIdType)));
  QObject::connect(ui.GradientOpacityEditor,
  SIGNAL(currentPointChanged(vtkIdType)), this,
  SLOT(gradientCurrentChanged(vtkIdType)));
  QObject::connect(ui.GaussianOpacityEditor, SIGNAL(currentPointChanged(int)),
      this, SLOT(gaussianCurrentChanged(int)));
  QObject::connect(ui.TwoDTransferFunction, SIGNAL(activeRegionChanged(int)),
       this, SLOT(TwoDTransferCurrentChanged(int)));

  QObject::connect(ui.ColorEditor, SIGNAL(controlPointsModified()), this,
  SIGNAL(xrgbPointsChanged()));
  QObject::connect(ui.OpacityEditor, SIGNAL(controlPointsModified()), this,
  SIGNAL(xvmsPointsChanged()));
  QObject::connect(ui.GradientOpacityEditor, SIGNAL(controlPointsModified()),
      this, SIGNAL(gvmsPointsChanged()));
  QObject::connect(ui.GaussianOpacityEditor, SIGNAL(controlPointsModified()),
      this, SIGNAL(xhwbbPointsChanged()));
  QObject::connect(ui.TwoDTransferFunction, SIGNAL(controlPointsModified()),
        this, SIGNAL(twoDTransferPointsChanged()));

  QObject::connect(ui.ColorEditor, SIGNAL(controlPointsModified()), this,
  SLOT(updateCurrentData()));
  QObject::connect(ui.OpacityEditor, SIGNAL(controlPointsModified()), this,
  SLOT(updateCurrentData()));
  QObject::connect(ui.GradientOpacityEditor, SIGNAL(controlPointsModified()),
      this, SLOT(updateCurrentData()));
  QObject::connect(ui.GaussianOpacityEditor, SIGNAL(controlPointsModified()),
      this, SLOT(updateCurrentData()));
  QObject::connect(ui.TwoDTransferFunction, SIGNAL(controlPointsModified()),
        this, SLOT(updateCurrentData()));

  QObject::connect(ui.ResetRangeToData, SIGNAL(clicked()), this,
  SLOT(resetRangeToData()));

  QObject::connect(ui.ResetRangeToCustom, SIGNAL(clicked()), this,
  SLOT(resetRangeToCustom()));

  QObject::connect(ui.ResetRangeToDataOverTime, SIGNAL(clicked()), this,
  SLOT(resetRangeToDataOverTime()));

  QObject::connect(ui.InvertTransferFunctions, SIGNAL(clicked()), this,
  SLOT(invertTransferFunctions()));

  QObject::connect(ui.ChoosePreset, SIGNAL(clicked()), this,
  SLOT(choosePreset()));
  QObject::connect(ui.SaveAsPreset, SIGNAL(clicked()), this,
  SLOT(saveAsPreset()));

  // TODO: at some point, I'd like to add a textual editor for users to simply
  // enter the text for the transfer function control points for finer control
  // of the LUT.
  ui.Edit->hide();

  // if the user edits the "DataValue", we need to update the transfer function.
  QObject::connect(ui.CurrentDataValue,
  SIGNAL(textChangedAndEditingFinished()), this,
  SLOT(currentDataEdited()));

  vtkSMProperty* smproperty = smgroup->GetProperty("XRGBPoints");
  if (smproperty)
    {
      this->addPropertyLink(this, "xrgbPoints", SIGNAL(xrgbPointsChanged()),
          smproperty);
    }
  else
    {
      qCritical(
          "Missing 'XRGBPoints' property. Widget may not function correctly.");
    }

  smproperty = smgroup->GetProperty("GradientOpacityFunction");
  if (smproperty)
    {

      // TODO: T
      vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

      if (pwfProxy) //&& pwfProxy->GetProperty("Points")
        {
          this->addPropertyLink(this, "gvmsPoints",
          SIGNAL(gvmsPointsChanged()), pwfProxy,
              pwfProxy->GetProperty("Points"));
        }
      else
        {
          ui.GradientOpacityEditor->hide();
        }
    }
  else
    {
      ui.GradientOpacityEditor->hide();
    }

  //change to gradient

  smproperty = smgroup->GetProperty("ScalarOpacityFunction");
  if (smproperty)
    {
      // TODO: T

      vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

      if (pwfProxy && pwfProxy->GetProperty("Points"))
        {
          this->addPropertyLink(this, "xvmsPoints",
          SIGNAL(xvmsPointsChanged()), pwfProxy,
              pwfProxy->GetProperty("Points"));
        }
      else
        {
          ui.OpacityEditor->hide();
        }
    }
  else
    {
      ui.OpacityEditor->hide();
    }

  smproperty = smgroup->GetProperty("GaussianOpacityFunction");
  if (smproperty)
    {
      // TODO: T

      vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

      if (pwfProxy && pwfProxy->GetProperty("Points"))
        {
          this->addPropertyLink(this, "xhwbbPoints",
          SIGNAL(xhwbbPointsChanged()), pwfProxy,
              pwfProxy->GetProperty("Points"));
        }
      else
        {
          ui.GaussianOpacityEditor->hide();
        }
    }
  else
    {
      ui.GaussianOpacityEditor->hide();
    }

  smproperty = smgroup->GetProperty("TwoDTransferFunction");
    if (smproperty)
      {
        // TODO: T

        vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

        if (pwfProxy && pwfProxy->GetProperty("Points"))
          {
            this->addPropertyLink(this, "twoDTransferPoints",
            SIGNAL(twoDTransferPointsChanged()), pwfProxy,
                pwfProxy->GetProperty("Points"));
          }
        else
          {
            ui.TwoDTransferFunction->hide();
          }
      }
    else
      {
        ui.TwoDTransferFunction->hide();
      }








  smproperty = smgroup->GetProperty("EnableOpacityMapping");
  if (smproperty)
    {
      this->addPropertyLink(ui.EnableOpacityMapping, "checked",
      SIGNAL(toggled(bool)), smproperty);
    }
  else
    {
      ui.EnableOpacityMapping->hide();
    }

  smproperty = smgroup->GetProperty("UseLogScale");
  if (smproperty)
    {
      this->addPropertyLink(this, "useLogScale", SIGNAL(useLogScaleChanged()),
          smproperty);
      QObject::connect(ui.UseLogScale, SIGNAL(clicked(bool)), this,
      SLOT(useLogScaleClicked(bool)));
      // QObject::connect(ui.UseLogScale, SIGNAL(toggled(bool)),
      //  this, SIGNAL(useLogScaleChanged()));
    }
  else
    {
      ui.UseLogScale->hide();
    }

  smproperty = smgroup->GetProperty("LockScalarRange");
  if (smproperty)
    {
      this->addPropertyLink(this, "lockScalarRange",
      SIGNAL(lockScalarRangeChanged()), smproperty);
      QObject::connect(ui.AutoRescaleRange, SIGNAL(toggled(bool)), this,
      SIGNAL(lockScalarRangeChanged()));
    }
  else
    {
      ui.AutoRescaleRange->hide();
    }

  // if proxy has a property named IndexedLookup, we hide this entire widget
  // when IndexedLookup is ON.
  if (smproxy->GetProperty("IndexedLookup"))
    {
      // we are not controlling the IndexedLookup property, we are merely
      // observing it to ensure the UI is updated correctly. Hence we don't fire
      // any signal to update the smproperty.
      this->Internals->VTKConnector->Connect(
          smproxy->GetProperty("IndexedLookup"), vtkCommand::ModifiedEvent,
          this,
          SLOT(updateIndexedLookupState()));
      this->updateIndexedLookupState();

      // Add decorator so the widget can be hidden when IndexedLookup is ON.
      this->addDecorator(this->Internals->Decorator);
    }

  this->updateCurrentData();
}

//-----------------------------------------------------------------------------
pqColorOpacityEditorWidget::~pqColorOpacityEditorWidget()
{
  delete this->Internals;
  this->Internals = NULL;
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::updateIndexedLookupState()
{
  if (this->proxy()->GetProperty("IndexedLookup"))
    {
      bool val = vtkSMPropertyHelper(this->proxy(), "IndexedLookup").GetAsInt()
          != 0;
      this->Internals->Decorator->setHidden(val);
    }
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::opacityCurrentChanged(vtkIdType index)
{
  if (index != -1)
    {
      Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
      ui.ColorEditor->setCurrentPoint(-1);
      ui.GradientOpacityEditor->setCurrentPoint(-1);
      ui.GaussianOpacityEditor->setCurrentGaussian(-1);
    }
  this->updateCurrentData();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::gradientCurrentChanged(vtkIdType index)
{
  if (index != -1)
    {
      Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
      ui.OpacityEditor->setCurrentPoint(-1);
      ui.ColorEditor->setCurrentPoint(-1);
      ui.GaussianOpacityEditor->setCurrentGaussian(-1);
      ui.TwoDTransferFunction->setCurrentRegion(-1);
    }
  this->updateCurrentData();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::colorCurrentChanged(vtkIdType index)
{
  if (index != -1)
    {
      Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
      ui.OpacityEditor->setCurrentPoint(-1);
      ui.GradientOpacityEditor->setCurrentPoint(-1);
      ui.GaussianOpacityEditor->setCurrentGaussian(-1);
      ui.TwoDTransferFunction->setCurrentRegion(-1);
    }
  this->updateCurrentData();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::gaussianCurrentChanged(int index)
{
  if (index != -1)
    {
      Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
      ui.OpacityEditor->setCurrentPoint(-1);
      ui.GradientOpacityEditor->setCurrentPoint(-1);
      ui.ColorEditor->setCurrentPoint(-1);
      ui.TwoDTransferFunction->setCurrentRegion(-1);
    }
  this->updateCurrentData();
}
void
pqColorOpacityEditorWidget::TwoDTransferCurrentChanged(int index)
{
  if (index != -1)
    {
      Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
      ui.OpacityEditor->setCurrentPoint(-1);
      ui.GradientOpacityEditor->setCurrentPoint(-1);
      ui.ColorEditor->setCurrentPoint(-1);
      ui.GaussianOpacityEditor->setCurrentGaussian(-1);
    }
  this->updateCurrentData();
}

void
pqColorOpacityEditorWidget::showHistogramWidget()
{
  vtkSmartPointer<vtkPVImageAccumulateInformation> info = vtkSmartPointer<
      vtkPVImageAccumulateInformation>::New();

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
      qDebug("No active representation.");
      return;
    }

  repr->getProxy()->InvokeCommand("UpdateHistogram");


  repr->getProxy()->GatherInformation(info.GetPointer());
 // repr->
  if (!repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
      std::cout<< "Did not find SupportHistogramWidget in showhistogramtest" << std::endl;
    }
/*
  if (repr->getProxy()->GetProperty("UpdateGradientRange"))
      repr->getProxy()->InvokeCommand("UpdateGradientRange");
    double gofrange[2] = {0, 1};
      this->UpdatePropertyInformation(this->GetProperty("GradientRange"));
      vtkSMPropertyHelper(this, "GradientRange").Get(gofrange,2);
      */

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
double gradientrange[2];
  if (repr->getProxy()->GetProperty("GradientRange")){
      repr->getProxy()->UpdatePropertyInformation(repr->getProxy()->GetProperty("GradientRange"));
      vtkSMPropertyHelper(repr->getProxy(), "GradientRange").Get(gradientrange,2);
  }
  ui.GaussianOpacityEditor->updateHistogram(gradientrange[0],gradientrange[1],info->sizeOfX,info->values);
  int enabledBarsHeight;

  bool logscale = false;
  pqHistogramDialog dialog(this, ui.GaussianOpacityEditor->histogramValues, ui.GaussianOpacityEditor->currentHistogramSize,ui.GaussianOpacityEditor->histogramEnabled, &logscale,
      &enabledBarsHeight);
  //dialog.setData();
  dialog.exec();
  //dialog.


  ui.GaussianOpacityEditor->generateBackgroundHistogram(logscale);
  this->update();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::updateCurrentData()
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;
  vtkPiecewiseFunction* gof = stc ? stc->GetGradientOpacityFunction() : NULL;

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  if (ui.ColorEditor->currentPoint() >= 0 && stc)
    {
      double xrgbms[6];
      stc->GetNodeValue(ui.ColorEditor->currentPoint(), xrgbms);
      ui.CurrentDataValue->setText(QString::number(xrgbms[0]));

      // Don't enable widget for first/last control point. For those, users must
      // rescale the transfer function manually
      ui.CurrentDataValue->setEnabled(
          ui.ColorEditor->currentPoint() != 0
              && ui.ColorEditor->currentPoint()
                  != (ui.ColorEditor->numberOfControlPoints() - 1));
    }
  else if (ui.GradientOpacityEditor->currentPoint() >= 0 && gof)
    {
      double xvms[4];
      gof->GetNodeValue(ui.GradientOpacityEditor->currentPoint(), xvms);
      ui.CurrentDataValue->setEnabled(true);
      ui.CurrentDataValue->setText(QString::number(xvms[0]));
    }
  else if (ui.OpacityEditor->currentPoint() >= 0 && pwf)
    {
      double xvms[4];
      pwf->GetNodeValue(ui.OpacityEditor->currentPoint(), xvms);
      ui.CurrentDataValue->setText(QString::number(xvms[0]));

      // Don't enable widget for first/last control point. For those, users must
      // rescale the transfer function manually
      ui.CurrentDataValue->setEnabled(
          ui.OpacityEditor->currentPoint() != 0
              && ui.OpacityEditor->currentPoint()
                  != (ui.OpacityEditor->numberOfControlPoints() - 1));
    }
  else
    {
      ui.CurrentDataValue->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
QList<QVariant>
pqColorOpacityEditorWidget::xrgbPoints() const
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  QList<QVariant> values;
  for (int cc = 0; stc != NULL && cc < stc->GetSize(); cc++)
    {
      double xrgbms[6];
      stc->GetNodeValue(cc, xrgbms);
      vtkVector<double, 4> value;
      values.push_back(xrgbms[0]);
      values.push_back(xrgbms[1]);
      values.push_back(xrgbms[2]);
      values.push_back(xrgbms[3]);
    }

  return values;
}

//----------------------------------------------------------------------------------------

QList<QVariant>
pqColorOpacityEditorWidget::xhwbbPoints() const
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkGaussianPiecewiseFunction* pwf =
      stc ? stc->GetGaussianOpacityFunction() : NULL;

  QList<QVariant> values;
  for (int cc = 0; pwf != NULL && cc < pwf->GetSize(); cc++)
    {
      double xhwbb[5];
      pwf->GetNodeValue(cc, xhwbb);
      values.push_back(xhwbb[0]);
      values.push_back(xhwbb[1]);
      values.push_back(xhwbb[2]);
      values.push_back(xhwbb[3]);
      values.push_back(xhwbb[4]);
    }
  return values;
}

//-----------------------------------------------------------------------------
QList<QVariant>
pqColorOpacityEditorWidget::xvmsPoints() const
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;

  QList<QVariant> values;
  for (int cc = 0; pwf != NULL && cc < pwf->GetSize(); cc++)
    {
      double xvms[4];
      pwf->GetNodeValue(cc, xvms);
      values.push_back(xvms[0]);
      values.push_back(xvms[1]);
      values.push_back(xvms[2]);
      values.push_back(xvms[3]);
    }
  return values;
}

//-----------------------------------------------------------------------------
QList<QVariant>
pqColorOpacityEditorWidget::gvmsPoints() const
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf = stc ? stc->GetGradientOpacityFunction() : NULL;

  QList<QVariant> values;
  for (int cc = 0; pwf != NULL && cc < pwf->GetSize(); cc++)
    {
      double xvms[4];
      pwf->GetNodeValue(cc, xvms);
      values.push_back(xvms[0]);
      values.push_back(xvms[1]);
      values.push_back(xvms[2]);
      values.push_back(xvms[3]);
    }
  return values;
}

QList<QVariant>
pqColorOpacityEditorWidget::twoDTransferPoints() const
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkTwoDTransferFunction* pwf = stc ? stc->GetTwoDTransferFunction() : NULL;

  QList<QVariant> values;
  for (int cc = 0; pwf != NULL && cc < pwf->GetSize(); cc++)
    {
      double tdps[6];
      pwf->GetRegionValues(cc, tdps);
      values.push_back(tdps[0]);
      values.push_back(tdps[1]);
      values.push_back(tdps[2]);
      values.push_back(tdps[3]);
      values.push_back(tdps[4]);
      values.push_back(tdps[5]);
    }
  return values;
}




//-----------------------------------------------------------------------------
bool
pqColorOpacityEditorWidget::useLogScale() const
{
  return this->Internals->Ui.UseLogScale->isChecked();
}

void
pqColorOpacityEditorWidget::disableGradientOpacty()
{

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  // repr->getProxy()->UpdatePropertyInformation(repr->getProxy()->GetProperty("GradientRange"));
//	 disableGradientOpacity = !disableGradientOpacity;
//	   vtkSMPropertyHelper(repr->getProxy(), "DisableGradientOpacity").Set(disableGradientOpacity);
//	 repr->getProxy()->UpdateVTKObjects();
}

void
pqColorOpacityEditorWidget::switchGradientOpacity()
{
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setUseLogScale(bool val)
{
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.UseLogScale->setChecked(val);
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::useLogScaleClicked(bool log_space)
{
  if (log_space)
    {
      vtkSMTransferFunctionProxy::MapControlPointsToLogSpace(this->proxy());
    }
  else
    {
      vtkSMTransferFunctionProxy::MapControlPointsToLinearSpace(this->proxy());
    }

  // FIXME: ensure scalar range is valid.

  emit this->useLogScaleChanged();
}

//-----------------------------------------------------------------------------
bool
pqColorOpacityEditorWidget::lockScalarRange() const
{
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  return !ui.AutoRescaleRange->isChecked();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setLockScalarRange(bool val)
{
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.AutoRescaleRange->setChecked(!val);
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setXvmsPoints(const QList<QVariant>& values)
{
  Q_UNUSED(values);
  // Since the vtkPiecewiseFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setGvmsPoints(const QList<QVariant>& values)
{
  Q_UNUSED(values);
  // Since the vtkPiecewiseFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setXrgbPoints(const QList<QVariant>& values)
{
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::setXhwbbPoints(const QList<QVariant>& values)
{
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::settwoDTransferPoints(const QList<QVariant>& values)
{
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::currentDataEdited()
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;

  if (ui.ColorEditor->currentPoint() >= 0 && stc)
    {
      ui.ColorEditor->setCurrentPointPosition(
          ui.CurrentDataValue->text().toDouble());
    }
  else if (ui.OpacityEditor->currentPoint() >= 0 && pwf)
    {
      ui.OpacityEditor->setCurrentPointPosition(
          ui.CurrentDataValue->text().toDouble());
    }
  else if (ui.GradientOpacityEditor->currentPoint() >= 0 && pwf)
    {
      ui.GradientOpacityEditor->setCurrentPointPosition(
          ui.CurrentDataValue->text().toDouble());
    }

  this->updateCurrentData();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::resetRangeToData()
{
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
      qDebug("No active representation.");
      return;
    }

  if (repr->getProxy()->GetProperty("UpdateGradientRange"))
    repr->getProxy()->InvokeCommand("UpdateGradientRange");

  BEGIN_UNDO_SET("Reset transfer function ranges using data range");
  vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(
      repr->getProxy());
  emit this->changeFinished();
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  // std::cout << "width "<<ui.GaussianOpacityEditor->contentsRect().width() << std::endl;
  // repr->setProperty("HistogramBins",ui.GaussianOpacityEditor->contentsRect().width());
  /*
   * if (repr->getProxy()->GetProperty("UpdateGradientRange"))
    repr->getProxy()->InvokeCommand("UpdateGradientRange");
  double gofrange[2] = {0, 1};
    this->UpdatePropertyInformation(this->GetProperty("GradientRange"));
    vtkSMPropertyHelper(this, "GradientRange").Get(gofrange,2);
    */

  // vtkSMProperty* temp = repr->getProxy()->GetProperty("HistogramBins");
  // vtkSMPropertyHelper bins(temp);
  // bins.Set(ui.GaussianOpacityEditor->contentsRect().width(),1);
//  repr->getProxy()->UpdateVTKObjects();

  END_UNDO_SET();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::resetRangeToDataOverTime()
{
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
      qDebug("No active representation.");
      return;
    }



  if (QMessageBox::warning(this, "Potentially slow operation",
      "This can potentially take a long time to complete. \n"
          "Are you sure you want to continue?",
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
      BEGIN_UNDO_SET(
          "Reset transfer function ranges using temporal data range");
      if (repr->getProxy()->GetProperty("UpdateGradientRange"))
            repr->getProxy()->InvokeCommand("UpdateGradientRange");
      vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRangeOverTime(
          repr->getProxy());
      emit this->changeFinished();
      END_UNDO_SET();
    }
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::resetRangeToCustom()
{
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  double range[2];
  stc->GetRange(range);
  double grange[2];
  stc->GetGradientOpacityFunction()->GetRange(grange);

  pqRescaleRange dialog(this);
  dialog.setRange(range[0], range[1]);
  dialog.setGradientRange(grange[0], grange[1]);
  if (dialog.exec() == QDialog::Accepted)
    {
      this->resetRangeToCustom(dialog.getMinimum(), dialog.getMaximum(), dialog.getMinimumGradient(), dialog.getMaximumGradient());
    }

}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::resetRangeToCustom(double min, double max, double gmin, double gmax)
{
  BEGIN_UNDO_SET("Reset transfer function ranges");
  vtkSMTransferFunctionProxy::RescaleTransferFunction(this->proxy(), min, max);
  vtkSMTransferFunctionProxy::RescaleTransferFunction(
      vtkSMPropertyHelper(this->proxy(), "ScalarOpacityFunction").GetAsProxy(),
      min, max);

  pqDataRepresentation* repr =
        pqActiveObjects::instance().activeRepresentation();

  vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionsToCustomRange(repr->getProxy(), min, max, gmin, gmax);




  emit this->changeFinished();
  END_UNDO_SET();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::invertTransferFunctions()
{
  BEGIN_UNDO_SET("Invert transfer function");
  vtkSMTransferFunctionProxy::InvertTransferFunction(this->proxy());

  emit this->changeFinished();
  // We don't invert the opacity function, for now.
  END_UNDO_SET();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::choosePreset(
    const pqColorMapModel* add_new/*=NULL*/)
{
  pqColorPresetManager preset(this,
      pqColorPresetManager::SHOW_NON_INDEXED_COLORS_ONLY);
  preset.setUsingCloseButton(true);
  preset.loadBuiltinColorPresets();
  preset.restoreSettings();
  if (add_new)
    {
      preset.addColorMap(*add_new, "New Color Preset");
    }

  QObject::connect(&preset, SIGNAL(currentChanged(const pqColorMapModel*)),
      this, SLOT(applyPreset(const pqColorMapModel*)));
  preset.exec();
  preset.saveSettings();
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::applyPreset(const pqColorMapModel* preset)
{
  vtkNew<vtkPVXMLElement> xml;
  xml->SetName("ColorMap");
  if (pqColorPresetManager::saveColorMapToXML(preset, xml.GetPointer()))
    {
      BEGIN_UNDO_SET("Apply color preset");
      vtkSMTransferFunctionProxy::ApplyColorMap(this->proxy(),
          xml.GetPointer());
      emit this->changeFinished();
      END_UNDO_SET();
    }
}

//-----------------------------------------------------------------------------
void
pqColorOpacityEditorWidget::saveAsPreset()
{
  vtkNew<vtkPVXMLElement> xml;
  if (vtkSMTransferFunctionProxy::SaveColorMap(this->proxy(), xml.GetPointer()))
    {
      pqColorMapModel colorMap = pqColorPresetManager::createColorMapFromXML(
          xml.GetPointer());
      this->choosePreset(&colorMap);
    }
}
