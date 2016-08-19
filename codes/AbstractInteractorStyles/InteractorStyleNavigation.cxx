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

This class allows interaction related to reslice position and orientation on images.

Wong Matthew Lun, Lok Ka Hei
Copyright (C) 2016
*/

#include "InteractorStyleNavigation.h"
#include "MainWindow.h"

#include <vtkRenderWindowInteractor.h>
#include <vtkCamera.h>
#include <vtkAbstractPicker.h>


vtkStandardNewMacro(InteractorStyleNavigation);

InteractorStyleNavigation::InteractorStyleNavigation()
	:AbstractInteractorStyleImage()
{
}

InteractorStyleNavigation::~InteractorStyleNavigation()
{
}

void InteractorStyleNavigation::OnMouseMove()
{
	//if (m_rightFunctioning) {
	//	this->SynchronizedZooming();
	//}
	if (m_leftFunctioning) {
		this->CalculateIndex();
		return;
	}
	AbstractInteractorStyleImage::OnMouseMove();

	if (m_rightFunctioning) {
		SynchronizedZooming();
	}
}


void InteractorStyleNavigation::OnLeftButtonUp()
{
	this->CalculateIndex();
	AbstractInteractorStyleImage::OnLeftButtonUp();
}



void InteractorStyleNavigation::SynchronizedZooming()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	MyImageViewer* viewer = NULL;
	double scale = imageViewer->GetRenderer()->GetActiveCamera()->GetParallelScale();
	
	for (int i = 0; i < 3; ++i) {
		viewer = mainWnd->GetMyImageViewer(i);
		if (viewer != NULL) {
			viewer->GetRenderer()->GetActiveCamera()->SetParallelScale(scale);
			viewer->Render();
		}
	}


}

void InteractorStyleNavigation::CalculateIndex()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	//Pick
	this->GetInteractor()->GetPicker()->Pick(
		this->GetInteractor()->GetEventPosition()[0],
		this->GetInteractor()->GetEventPosition()[1],
		0,  // always zero.
		imageViewer->GetRenderer());

	double* picked = this->GetInteractor()->GetPicker()->GetPickPosition();

	//Check if valid pick
	if (picked[0] == 0.0&&picked[1] == 0.0)
		return;
	double index[3];
	if (imageViewer->GetInput() != NULL) {
		picked[orientation] = origin[orientation] + m_slice * spacing[orientation];
		for (int i = 0; i < 3; i++)
		{
			index[i] = (picked[i] - origin[i]) / spacing[i];
		}
		mainWnd->slotChangeSlice((int)(index[0] + 0.5), (int)(index[1] + 0.5), (int)(index[2] + 0.5));
	}

}

