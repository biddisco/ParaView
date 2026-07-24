// SPDX-FileCopyrightText: Copyright (c) Kitware Inc.
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class   vtkPVInteractorStyle
 * @brief   interactive manipulation of the camera
 *
 * vtkPVInteractorStyle allows the user to interactively
 * manipulate the camera, the viewpoint of the scene.
 * The left button is for rotation; shift + left button is for rolling;
 * the right button is for panning; and shift + right button is for zooming.
 * This class fires vtkCommand::StartInteractionEvent and
 * vtkCommand::EndInteractionEvent to signal start and end of interaction.
 */

#ifndef vtkPVInteractorStyle_h
#define vtkPVInteractorStyle_h

#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkPVVTKExtensionsInteractionStyleModule.h" // needed for export macro

class vtkPVCameraManipulator;
class vtkCollection;

class VTKPVVTKEXTENSIONSINTERACTIONSTYLE_EXPORT vtkPVInteractorStyle
  : public vtkInteractorStyleTrackballCamera
{
public:
  static vtkPVInteractorStyle* New();
  vtkTypeMacro(vtkPVInteractorStyle, vtkInteractorStyleTrackballCamera);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  ///@{
  /**
   * Event bindings controlling the effects of pressing mouse buttons
   * or moving the mouse.
   */
  void OnMouseMove() override;
  void OnLeftButtonDown() override;
  void OnLeftButtonUp() override;
  void OnMiddleButtonDown() override;
  void OnMiddleButtonUp() override;
  void OnRightButtonDown() override;
  void OnRightButtonUp() override;
  ///@}

  ///@{
  /**
   * Unlike mouse events, these are forwarded to all camera manipulators
   * since we don't have a mechanism to activate a manipulator by key presses
   * currently.
   */
  void OnKeyDown() override;
  void OnKeyUp() override;
  ///@}

  /**
   * Overrides superclass behaviors to only support the key codes that make
   * sense in a ParaView application.
   */
  void OnChar() override;

  ///@{
  /**
   * Override mouse wheel zoom to fire StartInteractionEvent once,
   * then InteractionEvent + Render on every wheel event, and
   * EndInteractionEvent after a short delay (WheelInteractionTimeout)
   * once scrolling stops.  This matches button-based interaction
   * behaviour where many InteractiveRender (LOD) calls happen during
   * the interaction and StillRender only fires when it ends.
   */
  void OnMouseWheelForward() override;
  void OnMouseWheelBackward() override;
  void OnTimer() override;
  ///@}

  ///@{
  /**
   * Set/get the timeout in milliseconds after the last wheel event
   * before EndInteractionEvent fires and a full-resolution StillRender
   * is performed.  Defaults to 500 ms; vtkPVRenderView sets this from
   * the NonInteractiveRenderDelay property.
   */
  vtkSetClampMacro(WheelInteractionTimeout, int, 100, 10000);
  vtkGetMacro(WheelInteractionTimeout, int);
  ///@}

  /**
   * Access to adding or removing manipulators.
   */
  void AddManipulator(vtkPVCameraManipulator* m);

  /**
   * Removes all manipulators.
   */
  void RemoveAllManipulators();

  ///@{
  /**
   * Accessor for the collection of camera manipulators.
   */
  vtkGetObjectMacro(CameraManipulators, vtkCollection);
  ///@}

  ///@{
  /**
   * When enabled, mouse wheel  will zoom to the projected point under the cursor position.
   * There is no need to hold down Ctrl key to achieve this.
   */
  vtkSetMacro(MouseWheelZoomsToCursor, bool);
  vtkGetMacro(MouseWheelZoomsToCursor, bool);
  ///@}

  ///@{
  /**
   * Propagates the center to the manipulators.
   * This simply sets an internal ivar.
   * It is propagated to a manipulator before the event
   * is sent to it.
   * Also changing the CenterOfRotation during interaction
   * i.e. after a button press but before a button up
   * has no effect until the next button press.
   */
  vtkSetVector3Macro(CenterOfRotation, double);
  vtkGetVector3Macro(CenterOfRotation, double);
  ///@}

  ///@{
  /**
   * Propagates the rotation factor to the manipulators.
   * This simply sets an internal ivar.
   * It is propagated to a manipulator before the event
   * is sent to it.
   * Also changing the RotationFactor during interaction
   * i.e. after a button press but before a button up
   * has no effect until the next button press.
   */
  vtkSetMacro(RotationFactor, double);
  vtkGetMacro(RotationFactor, double);
  ///@}

  /**
   * Returns the chosen manipulator based on the modifiers.
   */
  virtual vtkPVCameraManipulator* FindManipulator(int button, int shift, int control);

  /**
   * Dolly the renderer's camera to a specific point
   */
  static void DollyToPosition(double fact, int* position, vtkRenderer* renderer);

  /**
   * Translate the renderer's camera
   */
  static void TranslateCamera(vtkRenderer* renderer, int toX, int toY, int fromX, int fromY);

  using vtkInteractorStyleTrackballCamera::Dolly;

protected:
  vtkPVInteractorStyle();
  ~vtkPVInteractorStyle() override;

  void Dolly(double factor) override;

  vtkPVCameraManipulator* CurrentManipulator;
  bool MouseWheelZoomsToCursor = false;
  double CenterOfRotation[3];
  double RotationFactor;

  // Wheel-zoom interaction: instead of firing Start/EndInteractionEvent
  // on every wheel event (which causes an immediate StillRender after
  // each LOD render), we keep the interaction "alive" with a timer.
  // StartInteractionEvent fires once, then each wheel event does an
  // InteractiveRender (LOD).  When the user stops scrolling for
  // WheelInteractionTimeout ms, the timer fires EndInteractionEvent
  // -> StillRender (full quality).
  bool WheelInteracting = false;
  int WheelTimerId = -1;
  int WheelInteractionTimeout = 500; // ms

  // The CameraInteractors also store there button and modifier.
  vtkCollection* CameraManipulators;

  void OnButtonDown(int button, int shift, int control);
  void OnButtonUp(int button);
  void ResetLights();
  void WheelZoomCommon(double factor);

  vtkPVInteractorStyle(const vtkPVInteractorStyle&) = delete;
  void operator=(const vtkPVInteractorStyle&) = delete;
};

#endif
