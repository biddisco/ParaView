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
#include "vtkWeakPointer.h"
#include "vtkPVSession.h"
#include "vtkPVImageAccumulateInformation.h"
#include "vtkPVTwoDHistogramInformation.h"
#include "pqHistogramDialog.h"
#include "pqTwoDHistogramDialog.h"
#include "vtkTwoDTransferFunction.h"
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
  class pqColorOpacityEditorWidgetDecorator: public pqPropertyWidgetDecorator
    {
    typedef pqPropertyWidgetDecorator Superclass;
    bool Hidden;
  public:
    pqColorOpacityEditorWidgetDecorator(vtkPVXMLElement* xmlArg,
        pqPropertyWidget* parentArg) :
        Superclass(xmlArg, parentArg), Hidden(false)
      {
      }
    virtual ~pqColorOpacityEditorWidgetDecorator()
      {
      }

    void setHidden(bool val)
      {
      if (val != this->Hidden)
        {
        this->Hidden = val;
        emit this->visibilityChanged();
        }
      }
    virtual bool canShowWidget(bool show_advanced) const
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
  vtkWeakPointer<vtkSMPropertyGroup> PropertyGroup;

  // We use this pqPropertyLinks instance to simply monitor smproperty changes.
  pqPropertyLinks LinksForMonitoringChanges;
  vtkNew<vtkEventQtSlotConnect> VTKConnector;

  pqInternals(pqColorOpacityEditorWidget* self, vtkSMPropertyGroup* group) :
      PropertyGroup(group)
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
    Superclass(smproxy, parentObject), Internals(new pqInternals(this, smgroup))
  {

  this->scalarOpacityAvailable = true;
  this->scalarGaussianAvailable = true;
  this->gradientLinearAvailable = true;
  this->gradientGaussianAvailable = true;
  this->twoDTransferFunctionAvailable = true;

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
    this->scalarOpacityAvailable = false;
    }
  if (stc)
    {
    ui.ColorEditor->initialize(stc, true, NULL, false);
    }

  pwf = stc ? stc->GetGradientLinearOpacityFunction() : NULL;
  size = pwf->GetSize();
  if (pwf)
    {

    ui.GradientLinearOpacityEditor->initialize(NULL, false, pwf, true);
    }
  else
    {
    ui.GradientLinearOpacityEditor->hide();
    this->gradientLinearAvailable = false;
    }

  vtkGaussianPiecewiseFunction* gpwf =
      stc ? stc->GetGradientGaussianOpacityFunction() : NULL;
  size = gpwf->GetSize();
  if (gpwf)
    {
    //TBD initialize stuff for gaussian
    ui.GradientGaussianOpacityEditor->initialize(gpwf, NULL);
    }
  else
    {
    ui.GradientGaussianOpacityEditor->hide();
    this->gradientGaussianAvailable = false;
    }

  gpwf = stc ? stc->GetScalarGaussianOpacityFunction() : NULL;
  size = gpwf->GetSize();
  if (gpwf)
    {
    //TBD initialize stuff for gaussian
    ui.ScalarGaussianOpacityEditor->initialize(gpwf, stc);
    }
  else
    {
    ui.ScalarGaussianOpacityEditor->hide();
    this->scalarGaussianAvailable = false;
    }

  vtkTwoDTransferFunction* tdtf = stc ? stc->GetTwoDTransferFunction() : NULL;
  if (tdtf)
    {
    //TBD initialize stuff for gaussian
    ui.TwoDTransferFunctionEditor->initialize(tdtf, stc);
    }
  else
    {
    ui.TwoDTransferFunctionEditor->hide();
    this->twoDTransferFunctionAvailable = false;
    }

  //change this to gaussian part---
  /*  Ui:QvisGaussianOpacityBar &ui = this->Internals->Ui;
   QvisGaussianOpacityBar* gaussianEditor =
   vtkDiscretizableColorTransferFunction::SafeDownCast(
   this->proxy()->GetClientSideObject());
   */
  //----
  this->disableGradientOpac = false;
  this->disableTwoDTransferFunc = false;

//  ui.DisableOpacityGradient->setCheckable(true);
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (repr->getProxy()->GetProperty("DisableGradientOpacity"))
    {
    QObject::connect(ui.DisableOpacityGradient, SIGNAL(clicked()), this,
        SLOT(disableGradientOpacity()));
    ui.DisableOpacityGradient->setChecked(false);
    /*  this->addPropertyLink(ui.DisableOpacityGradient, "checked",
     SIGNAL(clicked()), repr->getProxy(),
     repr->getProxy()->GetProperty("DisableGradientLinearOpacity"));*/
    }
  else
    {
    ui.DisableOpacityGradient->hide();
    }
  if (repr->getProxy()->GetProperty("TwoDTransferFunction"))
    {
    QObject::connect(ui.DisableTwoDTransferFunction, SIGNAL(clicked()), this,
        SLOT(disableTwoDTransferFunction()));
    ui.DisableTwoDTransferFunction->setChecked(false);
    /*  this->addPropertyLink(ui.DisableOpacityGradient, "checked",
     SIGNAL(clicked()), repr->getProxy(),
     repr->getProxy()->GetProperty("DisableGradientLinearOpacity"));*/
    }
  else
    {
    ui.DisableTwoDTransferFunction->hide();
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

  if (repr->getProxy()->GetProperty("SwitchScalarOpacity"))
    {
    QObject::connect(ui.SwitchScalarLinGauss, SIGNAL(clicked()), this,
        SLOT(switchScalarOpacity()));
    this->addPropertyLink(ui.SwitchScalarLinGauss, "checked", SIGNAL(clicked()),
        repr->getProxy(), repr->getProxy()->GetProperty("SwitchScalarOpacity"));
    }
  else
    {
    ui.SwitchScalarLinGauss->hide();
    }

  // vtkImageVolumeRepresentation* volumerep = vtkImageVolumeRepresentation::SafeDownCast(
  //      vtkPVCompositeRepresentation::SafeDownCast(obj)->GetActiveRepresentation());
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
  QObject::connect(ui.GradientLinearOpacityEditor,
      SIGNAL(currentPointChanged(vtkIdType)), this,
      SLOT(gradientLinearCurrentChanged(vtkIdType)));
  QObject::connect(ui.ScalarGaussianOpacityEditor,
      SIGNAL(currentPointChanged(int)), this,
      SLOT(scalarGaussianCurrentChanged(int)));
  QObject::connect(ui.GradientGaussianOpacityEditor,
      SIGNAL(currentPointChanged(int)), this,
      SLOT(gradientGaussianCurrentChanged(int)));
  QObject::connect(ui.TwoDTransferFunctionEditor,
      SIGNAL(activeRegionChanged(int)), this,
      SLOT(TwoDTransferCurrentChanged(int)));

  QObject::connect(ui.ColorEditor, SIGNAL(controlPointsModified()), this,
      SIGNAL(xrgbPointsChanged()));
  QObject::connect(ui.OpacityEditor, SIGNAL(controlPointsModified()), this,
      SIGNAL(xvmsPointsChanged()));
  QObject::connect(ui.GradientLinearOpacityEditor,
      SIGNAL(controlPointsModified()), this, SIGNAL(gvmsPointsChanged()));
  QObject::connect(ui.ScalarGaussianOpacityEditor,
      SIGNAL(controlPointsModified()), this,
      SIGNAL(ScalarxhwbbPointsChanged()));
  QObject::connect(ui.GradientGaussianOpacityEditor,
      SIGNAL(controlPointsModified()), this, SIGNAL(xhwbbPointsChanged()));
  QObject::connect(ui.TwoDTransferFunctionEditor,
      SIGNAL(controlPointsModified()), this,
      SIGNAL(twoDTransferPointsChanged()));

  QObject::connect(ui.ColorEditor, SIGNAL(controlPointsModified()), this,
      SLOT(updateCurrentData()));
  QObject::connect(ui.OpacityEditor, SIGNAL(controlPointsModified()), this,
      SLOT(updateCurrentData()));
  QObject::connect(ui.GradientLinearOpacityEditor,
      SIGNAL(controlPointsModified()), this, SLOT(updateCurrentData()));
  QObject::connect(ui.GradientGaussianOpacityEditor,
      SIGNAL(controlPointsModified()), this, SLOT(updateCurrentData()));
  QObject::connect(ui.TwoDTransferFunctionEditor,
      SIGNAL(controlPointsModified()), this, SLOT(updateCurrentData()));

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
  QObject::connect(ui.CurrentDataValue, SIGNAL(textChangedAndEditingFinished()),
      this, SLOT(currentDataEdited()));

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

  smproperty = smgroup->GetProperty("GradientLinearOpacityFunction");
  if (smproperty)
    {

    // TODO: T
    vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

    if (pwfProxy) //&& pwfProxy->GetProperty("Points")
      {
      this->addPropertyLink(this, "gvmsPoints", SIGNAL(gvmsPointsChanged()),
          pwfProxy, pwfProxy->GetProperty("Points"));
      }
    else
      {
      ui.GradientLinearOpacityEditor->hide();
      this->gradientLinearAvailable = false;
      }
    }
  else
    {
    ui.GradientLinearOpacityEditor->hide();
    this->gradientLinearAvailable = false;
    }

  smproperty = smgroup->GetProperty("ScalarOpacityFunction");
  if (smproperty)
    {
    // TODO: T

    vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

    if (pwfProxy && pwfProxy->GetProperty("Points"))
      {
      this->addPropertyLink(this, "xvmsPoints", SIGNAL(xvmsPointsChanged()),
          pwfProxy, pwfProxy->GetProperty("Points"));
      }
    else
      {
      ui.OpacityEditor->hide();
      this->scalarOpacityAvailable = false;
      }
    }
  else
    {
    ui.OpacityEditor->hide();
    this->scalarOpacityAvailable = false;
    }

  smproperty = smgroup->GetProperty("ScalarGaussianOpacityFunction");
  if (smproperty)
    {
    // TODO: T

    vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

    if (pwfProxy && pwfProxy->GetProperty("Points"))
      {
      this->addPropertyLink(this, "ScalarxhwbbPoints",
          SIGNAL(ScalarxhwbbPointsChanged()), pwfProxy,
          pwfProxy->GetProperty("Points"));
      }
    else
      {
      ui.ScalarGaussianOpacityEditor->hide();
      this->scalarGaussianAvailable = false;
      }
    }
  else
    {
    ui.ScalarGaussianOpacityEditor->hide();
    this->scalarGaussianAvailable = false;
    }

  smproperty = smgroup->GetProperty("GradientGaussianOpacityFunction");
  if (smproperty)
    {
    // TODO: T

    vtkSMProxy* pwfProxy = vtkSMPropertyHelper(smproperty).GetAsProxy();

    if (pwfProxy && pwfProxy->GetProperty("Points"))
      {
      this->addPropertyLink(this, "xhwbbPoints", SIGNAL(xhwbbPointsChanged()),
          pwfProxy, pwfProxy->GetProperty("Points"));
      }
    else
      {
      ui.GradientGaussianOpacityEditor->hide();
      this->gradientGaussianAvailable = false;
      }
    }
  else
    {
    ui.GradientGaussianOpacityEditor->hide();
    this->gradientGaussianAvailable = false;
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
      ui.TwoDTransferFunctionEditor->hide();
      this->twoDTransferFunctionAvailable = false;
      }
    }
  else
    {
    ui.TwoDTransferFunctionEditor->hide();
    this->twoDTransferFunctionAvailable = false;
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
        smproxy->GetProperty("IndexedLookup"), vtkCommand::ModifiedEvent, this,
        SLOT(updateIndexedLookupState()));
    this->updateIndexedLookupState();

    // Add decorator so the widget can be hidden when IndexedLookup is ON.
    this->addDecorator(this->Internals->Decorator);
    }

  // switchGradientOpacity();
  // switchScalarOpacity();
  // ui.ScalarGaussianOpacityEditor->hide();

  // hideGradientFunctions();

  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
pqColorOpacityEditorWidget::~pqColorOpacityEditorWidget()
  {
  delete this->Internals;
  this->Internals = NULL;
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::updateIndexedLookupState()
  {
  if (this->proxy()->GetProperty("IndexedLookup"))
    {
    bool val = vtkSMPropertyHelper(this->proxy(), "IndexedLookup").GetAsInt()
        != 0;
    this->Internals->Decorator->setHidden(val);
    }
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::opacityCurrentChanged(vtkIdType index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.ColorEditor->setCurrentPoint(-1);
    ui.GradientLinearOpacityEditor->setCurrentPoint(-1);
    ui.GradientGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.ScalarGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.TwoDTransferFunctionEditor->setCurrentRegion(-1);
    }
  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::gradientLinearCurrentChanged(vtkIdType index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.OpacityEditor->setCurrentPoint(-1);
    ui.ColorEditor->setCurrentPoint(-1);
    ui.GradientGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.ScalarGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.TwoDTransferFunctionEditor->setCurrentRegion(-1);
    }
  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::colorCurrentChanged(vtkIdType index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.OpacityEditor->setCurrentPoint(-1);
    ui.GradientLinearOpacityEditor->setCurrentPoint(-1);
    ui.GradientGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.ScalarGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.TwoDTransferFunctionEditor->setCurrentRegion(-1);
    }
  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::scalarGaussianCurrentChanged(int index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.OpacityEditor->setCurrentPoint(-1);
    ui.GradientLinearOpacityEditor->setCurrentPoint(-1);
    ui.ColorEditor->setCurrentPoint(-1);
    ui.GradientGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.TwoDTransferFunctionEditor->setCurrentRegion(-1);
    }
  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::gradientGaussianCurrentChanged(int index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.OpacityEditor->setCurrentPoint(-1);
    ui.GradientLinearOpacityEditor->setCurrentPoint(-1);
    ui.ColorEditor->setCurrentPoint(-1);
    ui.ScalarGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.TwoDTransferFunctionEditor->setCurrentRegion(-1);
    }
  this->updateCurrentData();
  }
//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::TwoDTransferCurrentChanged(int index)
  {
  if (index != -1)
    {
    Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
    ui.OpacityEditor->setCurrentPoint(-1);
    ui.GradientLinearOpacityEditor->setCurrentPoint(-1);
    ui.ColorEditor->setCurrentPoint(-1);
    ui.GradientGaussianOpacityEditor->setCurrentGaussian(-1);
    ui.ScalarGaussianOpacityEditor->setCurrentGaussian(-1);
    }
  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::showTwoDHistogram()
  {

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
    std::cout << "Did not find SupportHistogramWidget in showhistogramtest"
        << std::endl;
    return;
    }

  repr->getProxy()->InvokeCommand("UpdateTwoDHistogram");

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  vtkSmartPointer<vtkPVTwoDHistogramInformation> infotwod = vtkSmartPointer<
      vtkPVTwoDHistogramInformation>::New();
  repr->getProxy()->GatherInformation(infotwod.GetPointer(),
      vtkPVSession::RENDER_SERVER);

  bool logscale = false;
  int siz[2];
  siz[0] = infotwod->dimensions[0];
  siz[1] = infotwod->dimensions[1];
  if (ui.TwoDTransferFunctionEditor->histogramEnabled.size() != siz[0] * siz[1])
    {
    ui.TwoDTransferFunctionEditor->histogramEnabled.clear();
    ui.TwoDTransferFunctionEditor->histogramEnabled.resize(siz[0] * siz[1],
        true);
    }

  pqTwoDHistogramDialog dialog(this, &(infotwod->values), siz,
      &(ui.TwoDTransferFunctionEditor->histogramEnabled), &logscale);

  dialog.exec();

  ui.TwoDTransferFunctionEditor->generateHistogramBackground(
      infotwod->getDimensionAtIndex(0), infotwod->getDimensionAtIndex(1),
      infotwod->values, ui.TwoDTransferFunctionEditor->histogramEnabled,
      logscale);
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::showOneDHistogram()
  {
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
    std::cout << "Did not find SupportHistogramWidget in showhistogramtest"
        << std::endl;
    return;
    }

  repr->getProxy()->InvokeCommand("UpdateHistogram");

  vtkSmartPointer<vtkPVImageAccumulateInformation> info = vtkSmartPointer<
      vtkPVImageAccumulateInformation>::New();
  repr->getProxy()->GatherInformation(info.GetPointer(),
      vtkPVSession::RENDER_SERVER);

  double gradientrange[2];
  info->GetCurrentGradientRange(gradientrange);
  //
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.GradientGaussianOpacityEditor->updateHistogram(gradientrange[0],
      gradientrange[1], info->GetSizeOfHistogramX(),
      info->GetHistogramValues());

  float enabledBarsHeight;
  bool logscale = false;
  pqHistogramDialog dialog(this,
      &(ui.GradientGaussianOpacityEditor->histogramValues),
      ui.GradientGaussianOpacityEditor->currentHistogramSize,
      &(ui.GradientGaussianOpacityEditor->histogramEnabled), &logscale,
      &enabledBarsHeight);
  //dialog.setData();
  dialog.exec();

  ui.GradientGaussianOpacityEditor->generateBackgroundHistogram(logscale);

  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::showHistogramWidget()
  {
  if (this->Internals->Ui.TwoDTransferFunctionEditor->isHidden())
    {
    showOneDHistogram();
    }
  else
    {
    showTwoDHistogram();
    }

  this->update();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::updateCurrentData()
  {
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf = stc ? stc->GetScalarOpacityFunction() : NULL;
  vtkPiecewiseFunction* gof =
      stc ? stc->GetGradientLinearOpacityFunction() : NULL;
  vtkGaussianPiecewiseFunction* ggwf =
      stc ? stc->GetGradientGaussianOpacityFunction() : NULL;
  vtkGaussianPiecewiseFunction* sgwf =
      stc ? stc->GetScalarGaussianOpacityFunction() : NULL;

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
  else if (ui.GradientLinearOpacityEditor->currentPoint() >= 0 && gof)
    {
    double xvms[4];
    gof->GetNodeValue(ui.GradientLinearOpacityEditor->currentPoint(), xvms);
    ui.CurrentDataValue->setEnabled(true);
    ui.CurrentDataValue->setText(QString::number(xvms[0]));

    ui.CurrentDataValue->setEnabled(
        ui.GradientLinearOpacityEditor->currentPoint() != 0
            && ui.GradientLinearOpacityEditor->currentPoint()
                != (ui.GradientLinearOpacityEditor->numberOfControlPoints() - 1));

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
  else if (ui.GradientGaussianOpacityEditor->getCurrentGaussian() >= 0 && pwf)
    {
    double xvms[5];
    ggwf->GetNodeValue(ui.GradientGaussianOpacityEditor->getCurrentGaussian(),
        xvms);
    ui.CurrentDataValue->setText(QString::number(xvms[0]));
    ui.CurrentDataValue->setEnabled(
        ui.GradientGaussianOpacityEditor->getCurrentGaussian() != -1
            && ui.GradientGaussianOpacityEditor->getCurrentGaussian() >= 0
                != (ui.GradientGaussianOpacityEditor->getNumberOfGaussians() - 1));

    }
  else if (ui.ScalarGaussianOpacityEditor->getCurrentGaussian() >= 0 && pwf)
    {
    double xvms[5];
    sgwf->GetNodeValue(ui.ScalarGaussianOpacityEditor->getCurrentGaussian(),
        xvms);
    ui.CurrentDataValue->setText(QString::number(xvms[0]));

    ui.CurrentDataValue->setEnabled(
        ui.ScalarGaussianOpacityEditor->getCurrentGaussian() != -1
            && ui.ScalarGaussianOpacityEditor->getCurrentGaussian() >= 0
                != (ui.ScalarGaussianOpacityEditor->getNumberOfGaussians() - 1));

    }
  else
    {
    ui.CurrentDataValue->setEnabled(false);
    }
  }

//-----------------------------------------------------------------------------
QList<QVariant> pqColorOpacityEditorWidget::xrgbPoints() const
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

QList<QVariant> pqColorOpacityEditorWidget::ScalarxhwbbPoints() const
  {
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkGaussianPiecewiseFunction* pwf =
      stc ? stc->GetScalarGaussianOpacityFunction() : NULL;

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

//----------------------------------------------------------------------------------------

QList<QVariant> pqColorOpacityEditorWidget::xhwbbPoints() const
  {
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkGaussianPiecewiseFunction* pwf =
      stc ? stc->GetGradientGaussianOpacityFunction() : NULL;

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
QList<QVariant> pqColorOpacityEditorWidget::xvmsPoints() const
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
QList<QVariant> pqColorOpacityEditorWidget::gvmsPoints() const
  {
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  vtkPiecewiseFunction* pwf =
      stc ? stc->GetGradientLinearOpacityFunction() : NULL;

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
QList<QVariant> pqColorOpacityEditorWidget::twoDTransferPoints() const
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
bool pqColorOpacityEditorWidget::useLogScale() const
  {
  return this->Internals->Ui.UseLogScale->isChecked();
  }
//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::disableGradientOpacity()
  {
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (!repr->getProxy()->GetProperty("InfoDisableGradientOpacity"))
    return;

  repr->getProxy()->UpdatePropertyInformation(
      repr->getProxy()->GetProperty("InfoDisableGradientOpacity"));

  int disabled;
  vtkSMPropertyHelper(repr->getProxy(), "InfoDisableGradientOpacity").Get(
      &disabled, 1);

  vtkSMProperty* property = repr->getProxy()->GetProperty(
      "DisableGradientOpacity");
  vtkSMPropertyHelper prpty(property);
  disabled = !disabled;
  prpty.Set(disabled);

  repr->getProxy()->UpdateVTKObjects();

  if (!disabled)
    {
    vtkDiscretizableColorTransferFunctionCollection* stc =
        vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
            this->proxy()->GetClientSideObject());
    if (!stc->everythingInitialized)
      {
      //rescale gradient ranges
      repr->getProxy()->InvokeCommand("UpdateGradientRange");
      vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionToDataRange(
          repr->getProxy());
      repr->getProxy()->InvokeCommand(
          "EnableUseAdjustMapperGradientRangeFactor");
      stc->everythingInitialized = true;
      }

    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("InfoDisableTwoDTransferFunction"));

    vtkSMPropertyHelper(repr->getProxy(), "InfoDisableTwoDTransferFunction").Get(
        &disabled, 1);
    if (!disabled)
      {
      disableTwoDTransferFunction();
      }

    showGradientFunctions();
    }
  else
    {
    hideGradientFunctions();
    }

  }
//-----------------------------------------------------------------------------

void pqColorOpacityEditorWidget::disableTwoDTransferFunction()
  {
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (!repr->getProxy()->GetProperty("InfoDisableTwoDTransferFunction"))
    return;

  repr->getProxy()->UpdatePropertyInformation(
      repr->getProxy()->GetProperty("InfoDisableTwoDTransferFunction"));

  int disabled;
  vtkSMPropertyHelper(repr->getProxy(), "InfoDisableTwoDTransferFunction").Get(
      &disabled, 1);

  vtkSMProperty* property = repr->getProxy()->GetProperty(
      "DisableTwoDTransferFunction");
  vtkSMPropertyHelper prpty(property);
  disabled = !disabled;
  prpty.Set(disabled);

  repr->getProxy()->UpdateVTKObjects();

  if (!disabled)
    {
    vtkDiscretizableColorTransferFunctionCollection* stc =
        vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
            this->proxy()->GetClientSideObject());
    if (!stc->everythingInitialized)
      {
      //rescale gradient ranges
      repr->getProxy()->InvokeCommand("UpdateGradientRange");
      vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionToDataRange(
          repr->getProxy());
      repr->getProxy()->InvokeCommand(
          "EnableUseAdjustMapperGradientRangeFactor");
      stc->everythingInitialized = true;
      }

    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("InfoDisableGradientOpacity"));

    vtkSMPropertyHelper(repr->getProxy(), "InfoDisableGradientOpacity").Get(
        &disabled, 1);
    if (!disabled)
      {
      disableGradientOpacity();
      }
    showTwoDTransferFunction();
    }
  else
    {
    hideTwoDTransferFunction();
    }
  }
//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::switchScalarOpacity()
  {

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("IsScalarGaussianFunction"))
    return;
  repr->getProxy()->UpdatePropertyInformation(
      repr->getProxy()->GetProperty("IsScalarGaussianFunction"));

  int isGaussian;
  vtkSMPropertyHelper(repr->getProxy(), "IsScalarGaussianFunction").Get(
      &isGaussian, 1);

  if (!repr->getProxy()->GetProperty("SwitchScalarOpacity"))
    return;
  vtkSMProperty* property = repr->getProxy()->GetProperty(
      "SwitchScalarOpacity");
  vtkSMPropertyHelper prpty(property);
  isGaussian = !isGaussian;
  prpty.Set(isGaussian);

  repr->getProxy()->UpdateVTKObjects();

  if (isGaussian)
    {
    this->Internals->Ui.ScalarGaussianOpacityEditor->show();
    this->Internals->Ui.OpacityEditor->hide();
    }
  else
    {
    this->Internals->Ui.ScalarGaussianOpacityEditor->hide();
    this->Internals->Ui.OpacityEditor->show();
    }

  }

void pqColorOpacityEditorWidget::switchGradientOpacity()
  {
  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("IsGradientGaussianFunction"))
    return;

  repr->getProxy()->UpdatePropertyInformation(
      repr->getProxy()->GetProperty("IsGradientGaussianFunction"));
  // repr->getProxy()->UpdatePropertyInformation();//("IsGradientGaussianFunction");

  int isGaussian;
  vtkSMPropertyHelper(repr->getProxy(), "IsGradientGaussianFunction").Get(
      &isGaussian, 1);

  if (!repr->getProxy()->GetProperty("SwitchGradientOpacity"))
    return;
  vtkSMProperty* property = repr->getProxy()->GetProperty(
      "SwitchGradientOpacity");
  vtkSMPropertyHelper prpty(property);
  isGaussian = !isGaussian;
  prpty.Set(isGaussian);

  repr->getProxy()->UpdateVTKObjects();

  if (isGaussian)
    {
    this->Internals->Ui.GradientGaussianOpacityEditor->show();
    this->Internals->Ui.GradientLinearOpacityEditor->hide();
    }
  else
    {
    this->Internals->Ui.GradientGaussianOpacityEditor->hide();
    this->Internals->Ui.GradientLinearOpacityEditor->show();
    }

  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setUseLogScale(bool val)
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.UseLogScale->setChecked(val);
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::useLogScaleClicked(bool log_space)
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
bool pqColorOpacityEditorWidget::lockScalarRange() const
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  return !ui.AutoRescaleRange->isChecked();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setLockScalarRange(bool val)
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.AutoRescaleRange->setChecked(!val);
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setXvmsPoints(const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkPiecewiseFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setGvmsPoints(const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkPiecewiseFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setXrgbPoints(const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setScalarXhwbbPoints(
    const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::setXhwbbPoints(const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::settwoDTransferPoints(
    const QList<QVariant>& values)
  {
  Q_UNUSED(values);
  // Since the vtkColorTransferFunction connected to the widget is directly obtained
  // from the proxy, we don't need to do anything here. The widget will be
  // updated when the proxy updates.
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::currentDataEdited()
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
  else if (ui.GradientLinearOpacityEditor->currentPoint() >= 0 && pwf)
    {
    ui.GradientLinearOpacityEditor->setCurrentPointPosition(
        ui.CurrentDataValue->text().toDouble());
    }

  this->updateCurrentData();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::resetRangeToData()
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

  if (repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
    repr->getProxy()->InvokeCommand("SetTwoDHistogramOutOfDate");
    repr->getProxy()->InvokeCommand("SetHistogramOutOfDate");
    }

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;

  ui.GradientGaussianOpacityEditor->removeHistogram();
  ui.TwoDTransferFunctionEditor->removeHistogram();

  emit this->changeFinished();

  END_UNDO_SET();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::resetRangeToDataOverTime()
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
    BEGIN_UNDO_SET("Reset transfer function ranges using temporal data range");
    if (repr->getProxy()->GetProperty("UpdateGradientRange"))
      repr->getProxy()->InvokeCommand("UpdateGradientRange");
    vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRangeOverTime(
        repr->getProxy());

    // disable auto-rescale of transfer function since the user has set on
    // explicitly (BUG #14371).
    this->setLockScalarRange(true);
    emit this->changeFinished();
    END_UNDO_SET();
    }

  if (repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
    repr->getProxy()->InvokeCommand("SetTwoDHistogramOutOfDate");
    repr->getProxy()->InvokeCommand("SetHistogramOutOfDate");
    }

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;

  ui.GradientGaussianOpacityEditor->removeHistogram();
  ui.TwoDTransferFunctionEditor->removeHistogram();

  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::resetRangeToCustom()
  {
  vtkDiscretizableColorTransferFunctionCollection* stc =
      vtkDiscretizableColorTransferFunctionCollection::SafeDownCast(
          this->proxy()->GetClientSideObject());
  double range[2];
  stc->GetRange(range);
  double grange[2];
  stc->GetGradientGaussianOpacityFunction()->GetRange(grange);

  pqRescaleRange dialog(this);
  dialog.setRange(range[0], range[1]);
  dialog.setGradientRange(grange[0], grange[1]);
  if (dialog.exec() == QDialog::Accepted)
    {
    this->resetRangeToCustom(dialog.getMinimum(), dialog.getMaximum(),
        dialog.getMinimumGradient(), dialog.getMaximumGradient());
    }

  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::resetRangeToCustom(double min, double max,
    double gmin, double gmax)
  {
  BEGIN_UNDO_SET("Reset transfer function ranges");
  vtkSMTransferFunctionProxy::RescaleTransferFunction(this->proxy(), min, max);
  vtkSMTransferFunctionProxy::RescaleTransferFunction(
      vtkSMPropertyHelper(this->proxy(), "ScalarOpacityFunction").GetAsProxy(),
      min, max);

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  // disable auto-rescale of transfer function since the user has set on
  // explicitly (BUG #14371).
  this->setLockScalarRange(true);

  vtkSMPVRepresentationProxy::RescaleGradientTransferFunctionsToCustomRange(
      repr->getProxy(), min, max, gmin, gmax);

  if (repr->getProxy()->GetProperty("SupportHistogramWidget"))
    {
    repr->getProxy()->InvokeCommand("SetTwoDHistogramOutOfDate");
    repr->getProxy()->InvokeCommand("SetHistogramOutOfDate");
    }

  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;

  ui.GradientGaussianOpacityEditor->removeHistogram();
  ui.TwoDTransferFunctionEditor->removeHistogram();

  emit this->changeFinished();
  END_UNDO_SET();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::invertTransferFunctions()
  {
  BEGIN_UNDO_SET("Invert transfer function");
  vtkSMTransferFunctionProxy::InvertTransferFunction(this->proxy());

  emit this->changeFinished();
  // We don't invert the opacity function, for now.
  END_UNDO_SET();
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::choosePreset(
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
void pqColorOpacityEditorWidget::applyPreset(const pqColorMapModel* preset)
  {
  vtkNew<vtkPVXMLElement> xml;
  xml->SetName("ColorMap");
  if (pqColorPresetManager::saveColorMapToXML(preset, xml.GetPointer()))
    {
    BEGIN_UNDO_SET("Apply color preset");
    vtkSMTransferFunctionProxy::ApplyColorMap(this->proxy(), xml.GetPointer());
    emit this->changeFinished();
    END_UNDO_SET();
    }
  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::saveAsPreset()
  {
  vtkNew<vtkPVXMLElement> xml;
  if (vtkSMTransferFunctionProxy::SaveColorMap(this->proxy(), xml.GetPointer()))
    {
    pqColorMapModel colorMap = pqColorPresetManager::createColorMapFromXML(
        xml.GetPointer());
    this->choosePreset(&colorMap);
    }
  }
//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::showGradientFunctions()
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  //ui.TwoDTransferFunctionEditor->hide();
  ui.gaussorgrad->show();

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("IsGradientGaussianFunction"))
    {
    ui.GradientGaussianOpacityEditor->hide();
    return;
    }

//  repr->getProxy()->InvokeCommand("UpdateGradientRange");

  repr->getProxy()->UpdatePropertyInformation(
      repr->getProxy()->GetProperty("IsGradientGaussianFunction"));

  int isGaussian;
  vtkSMPropertyHelper(repr->getProxy(), "IsGradientGaussianFunction").Get(
      &isGaussian, 1);

  if (isGaussian)
    {
    ui.GradientGaussianOpacityEditor->show();
    ui.GradientLinearOpacityEditor->hide();
    }
  else
    {
    ui.GradientLinearOpacityEditor->show();
    ui.GradientGaussianOpacityEditor->hide();
    }

  ui.HistogramDialog->show();

  }
//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::hideGradientFunctions()
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.GradientGaussianOpacityEditor->hide();
  ui.GradientLinearOpacityEditor->hide();
  ui.gaussorgrad->hide();
  ui.HistogramDialog->hide();
  ui.DisableOpacityGradient->setChecked(false);
  }

//-----------------------------------------------------------------------------

void pqColorOpacityEditorWidget::hideTwoDTransferFunction()
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;
  ui.TwoDTransferFunctionEditor->hide();
  ui.DisableTwoDTransferFunction->setChecked(false);
  ui.HistogramDialog->hide();
  ui.OpacityEditor->hide();

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (repr->getProxy()->GetProperty("IsScalarGaussianFunction"))
    {
    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("IsScalarGaussianFunction"));
    int isGaussian;
    vtkSMPropertyHelper(repr->getProxy(), "IsScalarGaussianFunction").Get(
        &isGaussian, 1);
    if (isGaussian)
      {
      ui.ScalarGaussianOpacityEditor->show();
      }
    else
      {
      ui.OpacityEditor->show();
      }
    }
  }

void pqColorOpacityEditorWidget::showTwoDTransferFunction()
  {
  Ui::ColorOpacityEditorWidget &ui = this->Internals->Ui;

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();
  if (!repr)
    {
    qDebug("No active representation.");
    return;
    }

  if (!repr->getProxy()->GetProperty("TwoDTransferFunction"))
    {
    return;
    }
  ui.TwoDTransferFunctionEditor->show();
  ui.HistogramDialog->show();
  ui.OpacityEditor->hide();
  ui.ScalarGaussianOpacityEditor->hide();

  }

//-----------------------------------------------------------------------------
void pqColorOpacityEditorWidget::showEvent(QShowEvent * event)
  {
  //make sure all the buttons are correct

  //std::cout << "pqcoloropacityeditorwidget paint event"<< std::endl;

  pqDataRepresentation* repr =
      pqActiveObjects::instance().activeRepresentation();

  if (!repr)
    {
    Superclass::showEvent(event);
    return;
    }

  if (!repr->getProxy()->GetProperty("InfoDisableGradientOpacity"))
    {
    hideGradientFunctions();
    }

  else
    {
    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("InfoDisableGradientOpacity"));
    // repr->getProxy()->UpdatePropertyInformation();//("IsGradientGaussianFunction");

    int disabled;
    vtkSMPropertyHelper(repr->getProxy(), "InfoDisableGradientOpacity").Get(
        &disabled, 1);

    if (disabled)
      {
      hideGradientFunctions();
      this->Internals->Ui.DisableOpacityGradient->setChecked(false);
      }
    else
      {
      showGradientFunctions();
      this->Internals->Ui.DisableOpacityGradient->setChecked(true);
      }
    }

  //decide on scalar stuff
  if (repr->getProxy()->GetProperty("IsScalarGaussianFunction"))
    {
    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("IsScalarGaussianFunction"));
    int isGaussian;
    vtkSMPropertyHelper(repr->getProxy(), "IsScalarGaussianFunction").Get(
        &isGaussian, 1);
    if (isGaussian)
      {
      this->Internals->Ui.ScalarGaussianOpacityEditor->show();
      this->Internals->Ui.OpacityEditor->hide();
      }
    else
      {
      this->Internals->Ui.ScalarGaussianOpacityEditor->hide();
      this->Internals->Ui.OpacityEditor->show();
      }
    }
  else
    {
    this->Internals->Ui.ScalarGaussianOpacityEditor->hide();
    if (scalarOpacityAvailable && this->Internals->Ui.OpacityEditor->isHidden())
      this->Internals->Ui.OpacityEditor->show();
    }

  //decide on twodstuff
  if (!repr->getProxy()->GetProperty("InfoTwoDTransferFunction"))
    {
    hideTwoDTransferFunction();
    }
  else
    {
    repr->getProxy()->UpdatePropertyInformation(
        repr->getProxy()->GetProperty("InfoTwoDTransferFunction"));
    int disabled;
    vtkSMPropertyHelper(repr->getProxy(), "InfoTwoDTransferFunction").Get(
        &disabled, 1);
    if (disabled)
      {
      hideTwoDTransferFunction();
      this->Internals->Ui.DisableTwoDTransferFunction->setChecked(false);
      }
    else
      {
      showTwoDTransferFunction();
      this->Internals->Ui.DisableTwoDTransferFunction->setChecked(true);
      }
    }

  Superclass::showEvent(event);

  }
