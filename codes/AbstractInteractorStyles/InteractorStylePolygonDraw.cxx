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

This class allows interactive segmentation on images with contour widget.

Wong Matthew Lun, Lok Ka Hei
Copyright (C) 2016
*/

#include <vtkContourWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPolygon.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMath.h>

#include "InteractorStylePolygonDraw.h"
#include "MainWindow.h"

vtkStandardNewMacro(InteractorStylePolygonDraw);

InteractorStylePolygonDraw::InteractorStylePolygonDraw()
	:AbstractInteractorStyleImage()
{
	DOUBLE_CLICKED_FLAG = false;
	CONTOUR_IS_ON_FLAG = false;
	m_timer.start();
	m_firstClickTimeStamp = m_timer.elapsed();
	m_contourWidget = NULL;
	m_contourRep = NULL;
	label = 1;
}

InteractorStylePolygonDraw::~InteractorStylePolygonDraw()
{
	if (m_contourRep) {
		m_contourRep->Delete();
		m_contourRep = NULL;
	}

	if (m_contourWidget) {
		m_contourWidget->Delete();
		m_contourWidget = NULL;
	}
}

void InteractorStylePolygonDraw::OnLeftButtonDown()
{
	if (this->CheckDoubleClicked() && CONTOUR_IS_ON_FLAG) {
		this->FillPolygon();
		this->SetPolygonModeEnabled(false);
		this->CONTOUR_IS_ON_FLAG = false;
	}
	else if (!CONTOUR_IS_ON_FLAG){
		this->SetPolygonModeEnabled(true);
		this->CONTOUR_IS_ON_FLAG = true;
	}

	AbstractInteractorStyleImage::OnLeftButtonDown();
}

void InteractorStylePolygonDraw::OnRightButtonDown()
{
	if (CONTOUR_IS_ON_FLAG) {
		this->FillPolygon();
		this->SetPolygonModeEnabled(false);
		this->CONTOUR_IS_ON_FLAG = false;
	}

	AbstractInteractorStyleImage::OnRightButtonDown();
}

void InteractorStylePolygonDraw::OnMouseMove()
{
	if (!this->CONTOUR_IS_ON_FLAG) {
		this->SetPolygonModeEnabled(true);
		this->CONTOUR_IS_ON_FLAG = true;
	}
}

void InteractorStylePolygonDraw::OnKeyPress()
{
	std::string key = this->Interactor->GetKeySym();

	std::cout << key << endl;

	if (key == "Escape") {
		this->SetPolygonModeEnabled(false);
	}
	if (key == "Return" && m_contourWidget) {
		if (CONTOUR_IS_ON_FLAG)
			this->FillPolygon();
	}
	AbstractInteractorStyleImage::OnKeyPress();
}

bool InteractorStylePolygonDraw::CheckDoubleClicked()
{
	int t = m_timer.elapsed() - m_firstClickTimeStamp;

	if (t < 200 && !DOUBLE_CLICKED_FLAG) {
		DOUBLE_CLICKED_FLAG = true;
		m_firstClickTimeStamp = m_timer.elapsed();
		return true;
	}
	else {
		DOUBLE_CLICKED_FLAG = false;
		m_firstClickTimeStamp = m_timer.elapsed();
		return false;
	}
}

void InteractorStylePolygonDraw::SetPolygonModeEnabled(bool b)
{
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	if (m_contourWidget) {
		m_contourWidget->Off();
		m_contourWidget->SetRepresentation(NULL);
		m_contourWidget->EnabledOff();
		m_contourWidget->Delete();
		m_contourWidget = NULL;
	}


	if (m_contourRep) {
		m_contourRep->Delete();
		m_contourRep = NULL;
	}
	if (b)
	{
		m_contourWidget = vtkContourWidget::New();
		m_contourWidget->SetInteractor(this->Interactor);
		m_contourWidget->SetDefaultRenderer(imageViewer->GetannotationRenderer());

		m_contourRep = vtkOrientedGlyphContourRepresentation::New();
		m_contourRep->SetRenderer(imageViewer->GetannotationRenderer());
		m_contourRep->SetNeedToRender(true);
		m_contourRep->GetLinesProperty()->SetColor(255, 255, 0);
		m_contourRep->SetLineInterpolator(NULL);
		m_contourRep->AlwaysOnTopOn();


		vtkPolyData* cursorpolyData = m_contourRep->GetActiveCursorShape();
		vtkSmartPointer<vtkTransform> translation = vtkSmartPointer<vtkTransform>::New();
		if (orientation == 0) {
			translation->RotateX(90);
			translation->RotateZ(90);
		}
		else if (orientation == 1) {
			translation->RotateX(90);
			translation->RotateY(90);
		}

		vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = 
			vtkSmartPointer<vtkTransformPolyDataFilter>::New();
		transformFilter->SetInputData(cursorpolyData);
		transformFilter->SetTransform(translation);
		transformFilter->Update();

		cursorpolyData->DeepCopy(transformFilter->GetOutput());
		m_contourRep->BuildRepresentation();
		m_contourWidget->SetRepresentation(m_contourRep);
		m_contourWidget->FollowCursorOn();
		m_contourWidget->ContinuousDrawOn();
		//m_contourWidget->CreateDefaultRepresentation();
		m_contourWidget->On();
		m_contourWidget->EnabledOn();

		mainwnd->RenderAllViewer();
		this->CONTOUR_IS_ON_FLAG = true;
	} else{
		this->CONTOUR_IS_ON_FLAG = false;
	}
}

void InteractorStylePolygonDraw::SetLabel(int label)
{
	this->label = label;
}

void InteractorStylePolygonDraw::DisplayPolygon(vtkObject* caller, long unsigned vtkNotUsed(eventId), void* vtkNotUsed(clientData))
{
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	imageViewer->Render();
}

void InteractorStylePolygonDraw::FillPolygon()
{
	//Get Main window
	MainWindow* mainWnd = MainWindow::GetMainWindow();

	if (m_contourRep == NULL) return;
	if (m_contourWidget == NULL) return;

	//Get most updated current value
	//mainWnd->GetCursorPosition(m_currentPos);

	m_contourWidget->CloseLoop();

	vtkPolyData* polydata = m_contourRep->GetContourRepresentationAsPolyData();

	// Check if contour is drawn
	if (polydata->GetNumberOfPoints() == 0)
		return;

	vtkSmartPointer<vtkPolygon> polygon = vtkSmartPointer<vtkPolygon>::New();
	int numOfPoints = polydata->GetNumberOfPoints();

	// Get the coordinates of the contour data points

	double lastPoint[3] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MAX, VTK_DOUBLE_MAX };
	for (vtkIdType i = 0; i < numOfPoints; i++)
	{
		double pictureCoordinate[3];
		double worldCoordinate[3];
		polydata->GetPoint(i, pictureCoordinate);

		//Take one image data 1 to be reference
		worldCoordinate[0] = (pictureCoordinate[0] - origin[0]) / spacing[0];
		worldCoordinate[1] = (pictureCoordinate[1] - origin[1]) / spacing[1];
		worldCoordinate[2] = (pictureCoordinate[2] - origin[2]) / spacing[2];
		//cout << s[0] << " " << s[1] << " " << s[2] << endl;
		//Test whether the points are inside the polygon or not
		// if the points is too close to the previous point, skip it to avoid error in PointInPolygon algorithm
		double d = vtkMath::Distance2BetweenPoints(lastPoint, worldCoordinate);
		if (d < 1E-5)
			continue;
		memcpy(lastPoint, worldCoordinate, sizeof(double) * 3);
		switch (orientation)
		{
		case 0:
			polygon->GetPoints()->InsertNextPoint(0.0, worldCoordinate[1], worldCoordinate[2]);
			break;
		case 1:
			polygon->GetPoints()->InsertNextPoint(worldCoordinate[0], 0.0, worldCoordinate[2]);
			break;
		case 2:
			polygon->GetPoints()->InsertNextPoint(worldCoordinate[0], worldCoordinate[1], 0.0);
			break;
		default:
			break;
		}
	}
	//Test whether the points are inside the polygon or not
	double n[3];
	polygon->ComputeNormal(polygon->GetPoints()->GetNumberOfPoints(),
		static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), n);
	double bounds[6];
	int bounds_int[6];

	polygon->GetPoints()->GetBounds(bounds);

	bounds_int[0] = ceil(bounds[0]);
	bounds_int[1] = floor(bounds[1]);
	bounds_int[2] = ceil(bounds[2]);
	bounds_int[3] = floor(bounds[3]);
	bounds_int[4] = ceil(bounds[4]);
	bounds_int[5] = floor(bounds[5]);


	double p[3];
	int p2[3];
	if (orientation == 0)
	{
		for (int y = bounds_int[2]; y < bounds_int[3]; y++) {
			for (int z = bounds_int[4]; z < bounds_int[5]; z++) {
				p[0] = 0.0;
				p[1] = (double)y;
				p[2] = (double)z;
				p2[0] = imageViewer->GetSlice();
				p2[1] = y;
				p2[2] = z;
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					mainWnd->GetOverlay()->SetPixel(p2, (double)label);
				}
			}
		}
	}
	else if (orientation == 1)
	{
		for (int x = bounds_int[0]; x < bounds_int[1]; x++) {
			for (int z = bounds_int[4]; z < bounds_int[5]; z++) {
				p[0] = (double)x;
				p[1] = 0.0;
				p[2] = (double)z;
				p2[0] = x;
				p2[1] = imageViewer->GetSlice();
				p2[2] = z;
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					mainWnd->GetOverlay()->SetPixel(p2, (double)label);
				}
			}
		}
	}
	else if (orientation == 2)
	{
		for (int x = bounds_int[0]; x < bounds_int[1]; x++) {
			for (int y = bounds_int[2]; y < bounds_int[3]; y++) {
				p[0] = (double)x;
				p[1] = (double)y;
				p[2] = 0.0;
				p2[0] = x;
				p2[1] = y;
				p2[2] = imageViewer->GetSlice();
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					mainWnd->GetOverlay()->SetPixel(p2, (double)label);
				}

			}
		}
	}

	SetPolygonModeEnabled(false);
	SetPolygonModeEnabled(true);
	mainWnd->GetOverlay()->GetOutput()->Modified();

}

