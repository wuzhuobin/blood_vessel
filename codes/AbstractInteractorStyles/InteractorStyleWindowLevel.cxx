/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
			Department of Imaging and Inteventional Radiology,
			Junior Research Assistant

Author:		Lok, Ka Hei Jason
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
			Department of Imaging and Inteventional Radiology,
			M.Phil Student

This class allows interactive modification to window and level.


Wong Matthew Lun, Lok Ka Hei
Copyright (C) 2016
*/

#include "InteractorStyleWindowLevel.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

vtkStandardNewMacro(InteractorStyleWindowLevel);

void InteractorStyleWindowLevel::SetWindowLevelSpinBox(QDoubleSpinBox * w, QDoubleSpinBox * l)
{
	m_wlDoubleSpinBox[0] = w;
	m_wlDoubleSpinBox[1] = l;
}

InteractorStyleWindowLevel::InteractorStyleWindowLevel()
	:AbstractInteractorStyleImage()
{

}

InteractorStyleWindowLevel::~InteractorStyleWindowLevel()
{

}

void InteractorStyleWindowLevel::OnMouseMove()
{
	if (m_leftFunctioning) {
		this->WindowLevel();
	}
	//AbstractInteractorStyleImage::OnMouseMove();
}
	



void InteractorStyleWindowLevel::WindowLevel()
{
	this->WindowLevelCurrentPosition[0] = this->GetInteractor()->GetEventPosition()[0];
	this->WindowLevelCurrentPosition[1] = this->GetInteractor()->GetEventPosition()[1];

	int *size = this->GetCurrentRenderer()->GetSize();

	double window = 0.0, level = 0.0;
	if (imageViewer->GetInput() != NULL) {
		window = imageViewer->GetDefaultWindowLevel()[0];
		level = imageViewer->GetDefaultWindowLevel()[1];
	}

	// Compute normalized delta

	double dx = (this->WindowLevelCurrentPosition[0] -
		this->WindowLevelStartPosition[0]) * 4.0 / size[0];
	double dy = (this->WindowLevelStartPosition[1] -
		this->WindowLevelCurrentPosition[1]) * 4.0 / size[1];

	// Scale by current values

	if (fabs(window) > 0.01)
	{
		dx = dx * window;
	}
	else
	{
		dx = dx * (window < 0 ? -0.01 : 0.01);
	}
	if (fabs(level) > 0.01)
	{
		dy = dy * level;
	}
	else
	{
		dy = dy * (level < 0 ? -0.01 : 0.01);
	}

	// Abs so that direction does not flip

	if (window < 0.0)
	{
		dx = -1 * dx;
	}
	if (level < 0.0)
	{
		dy = -1 * dy;
	}

	// Compute new window level

	double newWindow = dx + window;
	double newLevel = level - dy;

	if (newWindow < 0.01)
	{
		newWindow = 0.01;
	}

	m_wlDoubleSpinBox[0]->setValue(newWindow);
	m_wlDoubleSpinBox[1]->setValue(newLevel);
}
