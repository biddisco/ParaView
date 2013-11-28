/*****************************************************************************
 *
 * Copyright (c) 2000 - 2007, The Regents of the University of California
 * Produced at the Lawrence Livermore National Laboratory
 * All rights reserved.
 *
 * This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
 * full copyright notice is contained in the file COPYRIGHT located at the root
 * of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
 *
 * Redistribution  and  use  in  source  and  binary  forms,  with  or  without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of  source code must  retain the above  copyright notice,
 *    this list of conditions and the disclaimer below.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
 *    documentation and/or materials provided with the distribution.
 *  - Neither the name of the UC/LLNL nor  the names of its contributors may be
 *    used to  endorse or  promote products derived from  this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
 * ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
 * CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
 * ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
 * CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
 * LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
 * OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 *****************************************************************************/

#ifndef QVIS_GAUSSIAN_OPACITY_BAR_H
#define QVIS_GAUSSIAN_OPACITY_BAR_H

#include "pqApplicationComponentsModule.h"
#include "QvisAbstractOpacityBar.h"
#include "vtkType.h"
#include "vtkSmartPointer.h"
#include "vtkNew.h"
#include <vector>

class QPixmap;
class vtkGaussianPiecewiseFunction;
class vtkScalarsToColors;
class vtkColorTransferFunction;
class vtkEventQtSlotConnect;
class QImage;

// ****************************************************************************
//  Class:  QvisGaussianOpacityBar
//
//  Purpose:
//    Gaussian-max implementation of QvisAbstractOpacityBar
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 30, 2001
//
//  Modifications:
//
// ****************************************************************************

class PQAPPLICATIONCOMPONENTS_EXPORT QvisGaussianOpacityBar: public QvisAbstractOpacityBar {
Q_OBJECT
public:
	QvisGaussianOpacityBar(QWidget *parent = NULL, const char *name = NULL);
	~QvisGaussianOpacityBar();
	void initialize(vtkGaussianPiecewiseFunction* gpwf, vtkScalarsToColors* stc);
	void getRawOpacities(int, float*);
	///returns the number of gaussians in the connected gaussianfunction
	int getNumberOfGaussians();
	/// Description:
	/// adds as an array of gaussians to the function.
	/// The gaussian is scaled and shifted into the space used by the function
	void setAllGaussians(int, float*);
	/// Description:
	/// Set the maximum number of gaussians allowed to be used in the widget
	/// Will conflict with the functions used by the widget if it has more than that number
	void setMaximumNumberOfGaussians(int);
	/// Description:
	/// Set the minimum number of gaussians allowed in the widget
	/// If there are less gaussians, then none can be deleted until the number is higher than the minimum
	void setMinimumNumberOfGaussians(int);
	/// Description:
	/// sets which gaussian is currently selected. If index is out of range, it is set to -1
	void setCurrentGaussian(int);
	/// Description:
	/// generate the histogram used in the background. The currently stored histogram values are used.
	void generateBackgroundHistogram(bool useLogScale);

	/// Description:
	///Sets the range of the function used by the widget. WARNING, THIS WILL ONLY HAPPEN ON THE CLIENT.
	void setFunctionRange(double range[2]);

	/// Description:
	/// size of the currently stored histogram.
	int currentHistogramSize;
	/// Description:
	/// size of the currently stored histogram.
	std::vector<bool> histogramEnabled;
	/// Description:
	/// The current histogram. array value = bin
	std::vector<int> histogramValues;
	/// Description:
	/// range of the current histogram
	double histogramRange[2];
	/// Description:
	/// copies a histogram. If the histogram to be copied has a larger range than the function of
	/// the widget, than a subsection is copied. If the histogramrange isn't as large, then the
	/// areas above or below will be empty.
	void updateHistogram(double rangeMin, double rangeMax, int histogramSize, int* histogram);
	/// Description:
	/// Gets the currently selected Gaussian
	int getCurrentGaussian();

	void removeHistogram();

protected:
	/// Description:
	/// Used for Histogram only.
	bool UseLogScale;
	void mouseMoveEvent(QMouseEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void paintToPixmap(int, int);
	/// Description:
	/// Draw control points of the gaussians.
	void drawControlPoints(QPainter &painter);
	/// Description:
	/// get the highest pixel in the background histogram for a histogram bin. Because origin is in top left, the lower the value, the higher the pixel.
	int getTopBinPixel(int bin, float scale, int currentMax, int currentUnEnabledMax, bool logScale, float enabledBarsHeight);
	/// Description:
	/// Controls whether or not the color background is painted
	bool paintScalarColorBackground;
	/// Description:
	/// Generates the background colors based on the colortransferfunction
	void createScalarColorBackground(float *values, int width, int height);
	/// Description:
	/// Colortransferfunction used to generate scalarcolorbackground
	vtkColorTransferFunction* colortransferfunction;
	/// Description:
	/// What it says on the tin. Store images you want to use as background here
	QImage* backgroundImage;
	/// Description:
	/// Connection used to update when the colortransferfunctions is changed
	vtkNew<vtkEventQtSlotConnect> VTKConnect;

	double currentFunctionRange[2];

signals:
	void mouseReleased();
	void mouseMoved();
	/// signal fired when the \c current selected control point changes.
	void currentPointChanged(int index);

	/// signal fired to indicate that the control points changed i.e. either they
	/// were moved, orone was added/deleted, or edited to change color, etc.
	void controlPointsModified();

	protected slots:
	void updateImage();

private:
	enum Mode {
		modeNone, modeX, modeH, modeW, modeWR, modeWL, modeB
	};
	// encapsulation of gaussian parameters
	class Gaussian {
	public:
		float x;
		float h;
		float w;
		float bx;
		float by;
	public:
		Gaussian(float x_, float h_, float w_, float bx_, float by_) :
				x(x_), h(h_), w(w_), bx(bx_), by(by_) {
		}
		;
		Gaussian() {
		}
		;
		~Gaussian() {
		}
		;
	};

	void convertGaussianToImageSpace(int index, Gaussian &gauss);
	void convertGaussianToDataSpace(int index, Gaussian &gauss);
	void preparePointsForDrawing(std::vector<Gaussian> &gaussians);

	enum gaussvalue {
		gaussX, gaussH, gaussW, gaussBx, gaussBy
	};
	/// Description:
	/// The function the widget uses and displays.
	vtkSmartPointer<vtkGaussianPiecewiseFunction> gaussianFunctionGroup;
	/// Description:
	/// Gets node from the gaussianpiecewisefunction
	Gaussian getNode(int index);
	/// Description:
	/// Sets node on in the function
	void setNode(int index, Gaussian &gauss);
	/// Description:
	/// remove a gaussian from the function
	void removeNode(int index);
	/// Description:
	/// Set one value of a gaussian
	void setGaussValue(int index, double value, gaussvalue v);
	/// Description:
	// Get one Value of a gaussian
	double getGaussValue(int index, gaussvalue v);

	// the list of gaussians
	int ngaussian;

	// the current interaction mode and the current gaussian
	Mode currentMode;
	int currentGaussian;

	// GUI interaction variables
	bool mousedown;
	int lastx;
	int lasty;

	// User specified constraints
	int maximumNumberOfGaussians;
	int minimumNumberOfGaussians;

	// helper functions
	bool findGaussianControlPoint(int, int, int*, Mode*);
	void removeGaussian(int);
	int addGaussian(float _x, float h, float w, float bx, float by);
};

#endif
