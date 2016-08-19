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

This class allows interactive segmentation on images.

Wong Matthew Lun, Lok Ka Hei
Copyright (C) 2016
*/

#include "InteractorStylePaintBrush.h"
#include "MainWindow.h"

#include <vtkTransformPolyDataFilter.h>
#include <vtkImageProperty.h>
#include <vtkProperty2D.h>
#include <vtkAbstractPicker.h>


vtkStandardNewMacro(InteractorStylePaintBrush);

#define SEGMENTATION_CIRCLE 1
#define SEGMENTATION_SQUARE 0

InteractorStylePaintBrush::InteractorStylePaintBrush()
	:AbstractInteractorStyleImage()
{


	m_borderWidget = NULL;
	m_retangleRep = NULL;
	m_brush = NULL;
	m_brushActor = NULL;
	//Default color: Red
	color_r = 255;
	color_g = 0;
	color_b = 0;
	opacity = 255;
	
	this->PAINT_BRUSH_MODE_ENABLED = false;
}

InteractorStylePaintBrush::~InteractorStylePaintBrush()
{
	//this->SetPaintBrushModeEnabled(false);

	// Clean again if the upper dosen't clean
	if (m_brushActor != NULL && imageViewer != NULL) {
		//this->imageViewer->GetannotationRenderer()->RemoveActor(m_brushActor);
		this->m_brushActor->Delete();
		this->m_brushActor = NULL;
		this->m_brush->Delete();
		this->m_brush = NULL;
	}

	if (m_borderWidget != NULL) {
		this->m_borderWidget->Delete();
		this->m_retangleRep->Delete();
	}
}


void InteractorStylePaintBrush::SetDrawColor(int r, int g, int b)
{
	color_r = r;
	color_g = g;
	color_b = b;
}
void InteractorStylePaintBrush::SetDrawColor(const int* rgb)
{
	SetDrawColor(rgb[0], rgb[1], rgb[2]);
}
void InteractorStylePaintBrush::SetDrawOpacity(int opacity)
{
	this->opacity = opacity;
}

void InteractorStylePaintBrush::SetDrawBrushSizeSpinBox(QSpinBox * s)
{
	m_drawBrushSizeSpinBox = s;
}

void InteractorStylePaintBrush::SetDrawBrushShapeComBox(QComboBox * comboBox)
{
	m_drawBrushShapeComboBox = comboBox;
}

void InteractorStylePaintBrush::SetDrawVolumetricCheckBox(QCheckBox * checkBox)
{
	m_drawVolumetricCheckBox = checkBox;
}

void InteractorStylePaintBrush::SetDrawIsotropicCheckBox(QCheckBox * checkBox)
{
	m_drawIsotropicCheckBox = checkBox;
}

void InteractorStylePaintBrush::OnLeftButtonUp()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (!m_isDraw)
	{
		this->Write2ImageData();
		mainWnd->slotOverlayVisibilty(true, orientation);
		//Clear Layer
		m_brush->SetDrawColor(0, 0, 0, 0);
		this->FillBox3D();
		this->Render();
		m_isDraw = true;
	}
	m_leftFunctioning = false;

}

void InteractorStylePaintBrush::OnLeftButtonDown()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	m_leftFunctioning = true;
	m_isDraw = false;
	this->GetInteractor()->GetPicker()->Pick(
		this->GetInteractor()->GetEventPosition()[0],
		this->GetInteractor()->GetEventPosition()[1],
		0,  // always zero.
		imageViewer->GetRenderer());

	double* picked = this->GetInteractor()->GetPicker()->GetPickPosition();

	//Check if valid pick
	qDebug() << "picked";
	qDebug() << picked[0] << picked[1];
	if (picked[0] == 0.0&&picked[1] == 0.0)
		return;
	// Check if valid pick
	//if (!this->CheckValidPick(picked))
	//	return;

	// Project picked world point to plane and obtain ijk index
	double index[3];
	if (imageViewer->GetInput() != NULL) {
		picked[orientation] = origin[orientation] + m_slice * spacing[orientation];
		for (int i = 0; i < 3; i++)
		{
			index[i] = (picked[i] - origin[i]) / spacing[i];
		}
	}
	qDebug() << "index";
	qDebug() << index[0] << index[1] << index[2];
	draw_index_old[0] = (int)(index[0] + 0.5);
	draw_index_old[1] = (int)(index[1] + 0.5);
	draw_index_old[2] = (int)(index[2] + 0.5);
	// 
	if (m_rightFunctioning == true)
	{
		m_rightFunctioning = false;
	}
	else
	{
		this->ReadfromImageData();
		this->Render();
	}
	mainWnd->slotOverlayVisibilty(false, orientation);
	Draw(true);
	this->Render();
}

void InteractorStylePaintBrush::OnRightButtonUp()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	m_rightFunctioning = false;
	if (!m_isDraw)
	{
		this->Write2ImageData();
		mainWnd->slotOverlayVisibilty(true, orientation);
		//Clear Layer
		m_brush->SetDrawColor(0, 0, 0, 0);
		this->FillBox3D();
		this->Render();
		m_isDraw = true;
	}
	AbstractInteractorStyleImage::OnRightButtonUp();
}

void InteractorStylePaintBrush::OnRightButtonDown()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	m_isDraw = false;

	// return if label image is not found

	this->GetInteractor()->GetPicker()->Pick(
		this->GetInteractor()->GetEventPosition()[0],
		this->GetInteractor()->GetEventPosition()[1],
		0,  // always zero.
		imageViewer->GetRenderer());

	double* picked = this->GetInteractor()->GetPicker()->GetPickPosition();


	//// Check if valid pick
	//if (!this->CheckValidPick(picked))
	//	return;
	//Check if valid pick
	if (picked[0] == 0.0&&picked[1] == 0.0)
		return;

	// Project picked world point to plane and obtain ijk index
	double index[3];
	if (imageViewer->GetInput() != NULL) {
		picked[orientation] = origin[orientation] + m_slice * spacing[orientation];
		for (int i = 0; i < 3; i++)
		{
			index[i] = (picked[i] - origin[i]) / spacing[i];
		}
	}
	draw_index_old[0] = (int)(index[0] + 0.5);
	draw_index_old[1] = (int)(index[1] + 0.5);
	draw_index_old[2] = (int)(index[2] + 0.5);

	if (m_leftFunctioning == true)
	{
		m_leftFunctioning = false;
	}
	else
	{
		this->ReadfromImageData();
		this->Render();
	}


	mainWnd->slotOverlayVisibilty(false, orientation);
	this->Draw(false);
	this->Render();
	m_rightFunctioning = true;
	AbstractInteractorStyleImage::OnRightButtonDown();
}

void InteractorStylePaintBrush::OnLeave()
{
	if (this->PAINT_BRUSH_MODE_ENABLED) {
		this->m_borderWidget->Off();
	}

	AbstractInteractorStyleImage::OnLeave();
}

void InteractorStylePaintBrush::OnMouseMove()
{
	if (!this->PAINT_BRUSH_MODE_ENABLED)
		this->SetPaintBrushModeEnabled(true);

	if (m_leftFunctioning == true)
		Draw(true);
	if (m_rightFunctioning == true)
		Draw(false);

	this->UpdateBorderWidgetPosition();
	this->Render();

	if (!m_rightFunctioning && !m_leftFunctioning)
		AbstractInteractorStyleImage::OnMouseMove();
}



void InteractorStylePaintBrush::SetPaintBrushModeEnabled(bool b)
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	/// Delete stuff if the mode is disabled to save memory
	if (m_borderWidget != NULL)
	{
		m_retangleRep->Delete();
		m_borderWidget->Delete();
		m_borderWidget = NULL;
	}


	if (m_brush != NULL)
	{
		if (m_brushActor != NULL && imageViewer != NULL) {
			this->imageViewer->GetannotationRenderer()->RemoveActor(m_brushActor);
		}

		m_brush->Delete();
		m_brush = NULL;

		m_brushActor->Delete();
		m_brushActor = NULL;

	}

	if (b)
	{
		m_retangleRep = vtkBorderRepresentation::New();
		m_borderWidget = vtkBorderWidget::New();
		m_borderWidget->SetInteractor(this->GetInteractor());
		m_borderWidget->SetCurrentRenderer(imageViewer->GetannotationRenderer());
		m_borderWidget->SetRepresentation(m_retangleRep);
		m_borderWidget->SetManagesCursor(false);
		m_borderWidget->GetBorderRepresentation()->SetMoving(false);
		m_borderWidget->GetBorderRepresentation()->SetProportionalResize(true);
		m_borderWidget->SelectableOff();
		m_borderWidget->ProcessEventsOff();
		m_borderWidget->Off();

		// Delete m_overlay image


		if (this->Interactor)
			this->Interactor->Render();

		/// Set up brush boarder widget
		m_brush = vtkImageCanvasSource2D::New();
		m_brush->SetScalarTypeToUnsignedChar();
		m_brush->SetNumberOfScalarComponents(4);
		switch (orientation)
		{
		case 0:
		{
			m_brush->SetExtent(0, 0, extent[2], extent[3], extent[4], extent[5]);
			break;
		}
		case 1:
		{
			m_brush->SetExtent(extent[0], extent[1], 0, 0, extent[4], extent[5]);
			break;
		}
		case 2:
		{
			m_brush->SetExtent(extent[0], extent[1], extent[2], extent[3], 0, 0);
			break;
		}
		}
		m_brush->SetDrawColor(0, 0, 0, 0);
		this->FillBox3D();
		m_brush->Update();
		m_brush->GetOutput()->SetOrigin(origin);
		m_brush->GetOutput()->SetSpacing(spacing);

		m_brushActor = vtkImageActor::New();
		m_brushActor->SetInputData(m_brush->GetOutput());
		m_brushActor->GetProperty()->SetInterpolationTypeToNearest();
		m_brushActor->SetPickable(false);
		// Set the image actor

		switch (this->orientation)
		{
		case 0:
			m_brushActor->SetDisplayExtent(
				0, 0, extent[2], extent[3], extent[4], extent[5]);
			break;

		case 1:
			m_brushActor->SetDisplayExtent(
				extent[0], extent[1], 0, 0, extent[4], extent[5]);
			break;

		case 2:
			m_brushActor->SetDisplayExtent(
				extent[0], extent[1], extent[2], extent[3], 0, 0);
			break;
		}
		imageViewer->GetannotationRenderer()->AddActor(m_brushActor);
		imageViewer->GetannotationRenderer()->Render();
	}

	this->PAINT_BRUSH_MODE_ENABLED = b;
}



void InteractorStylePaintBrush::Draw(bool b)
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (b) {
		m_brush->SetDrawColor(color_r, color_g, color_b, opacity);
	}
	else
		m_brush->SetDrawColor(0, 0, 0, 0);

	//Pick
	this->GetInteractor()->GetPicker()->Pick(
		this->GetInteractor()->GetEventPosition()[0],
		this->GetInteractor()->GetEventPosition()[1],
		0,  // always zero.
		imageViewer->GetRenderer());

	double* picked = this->GetInteractor()->GetPicker()->GetPickPosition();

	////Check if valid pick
	//if (!this->CheckValidPick(picked))
	//	return;
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
	}
	int draw_index[3];

	draw_index[0] = (int)(index[0] + 0.5);
	draw_index[1] = (int)(index[1] + 0.5);
	draw_index[2] = (int)(index[2] + 0.5);

	int x2Dindex, y2Dindex;
	switch (orientation)
	{
	case 0:
		x2Dindex = 1;
		y2Dindex = 2;
		break;
	case 1:
		x2Dindex = 0;
		y2Dindex = 2;
		break;
	case 2:
		x2Dindex = 0;
		y2Dindex = 1;
		break;
	}

	//Determine Brush Size and Position

	//Circle
	if (m_drawBrushShapeComboBox->currentText() == "Circle") {
		// if size is odd
		DrawCircle(draw_index_old[x2Dindex], draw_index_old[y2Dindex], draw_index[x2Dindex], draw_index[y2Dindex], m_drawBrushSizeSpinBox->value() / 2.);

	}
	//Default:Square
	else
	{
		int loopLowerBoundX = ceil(-m_drawBrushSizeSpinBox->value() / spacing[x2Dindex] / 2.);
		int loopUpperBoundX = ceil(m_drawBrushSizeSpinBox->value() / spacing[x2Dindex] / 2.);
		int loopLowerBoundY = ceil(-m_drawBrushSizeSpinBox->value() / spacing[y2Dindex] / 2.);
		int loopUpperBoundY = ceil(m_drawBrushSizeSpinBox->value() / spacing[y2Dindex] / 2.);

		for (int x = loopLowerBoundX; x < loopUpperBoundX; x++)
		{
			for (int y = loopLowerBoundY; y < loopUpperBoundY; y++)
			{
				this->DrawLine3D(draw_index_old[x2Dindex] + x, draw_index_old[y2Dindex] + y, draw_index[x2Dindex]+x, draw_index[y2Dindex]+y);
			}
		}

	}
	
	// Reset the old draw index
	for (int i = 0; i < 3; i++)
	{
		draw_index_old[i] = draw_index[i];
	}

}

void InteractorStylePaintBrush::FillBox3D()
{
	switch (this->orientation)
	{
	case 2:
		m_brush->FillBox(extent[0], extent[1], extent[2], extent[3]);
		break;
	case 1:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			m_brush->DrawSegment3D(x, 0, extent[4], x, 0, extent[5]);
		}
		break;
	case 0:
		for (int y = extent[2]; y < extent[3]; y++)
		{
			m_brush->DrawSegment3D(0,y, extent[4], 0, y, extent[5]);
		}
		break;	
	}
}

void InteractorStylePaintBrush::DrawLine3D(int x0, int y0, int x1, int y1)
{
	int index[6];
	switch (orientation)
	{
	case 0:
		if (x0 < extent[2] || x0 > extent[3] || y0 < extent[4] || y0 > extent[5] || x1 < extent[2] || x1 > extent[3] || y1 < extent[4] || y1 > extent[5])
			return;
		index[0] = 0;
		index[1] = x0;
		index[2] = y0;
		index[3] = 0;
		index[4] = x1;
		index[5] = y1;
		break;
	case 1:
		if (x0 < extent[0] || x0 > extent[1] || y0 < extent[4] || y0 > extent[5] || x1 < extent[0] || x1 > extent[1] || y1 < extent[4] || y1 > extent[5])
			return;
		index[0] = x0;
		index[1] = 0;
		index[2] = y0;
		index[3] = x1;
		index[4] = 0;
		index[5] = y1;
		break;
	case 2:
		if (x0 < extent[0] || x0 > extent[1] || y0 < extent[2] || y0 > extent[3] || x1 < extent[0] || x1 > extent[1] || y1 < extent[2] || y1 > extent[3])
			return;
		index[0] = x0;
		index[1] = y0;
		index[2] = 0;
		index[3] = x1;
		index[4] = y1;
		index[5] = 0;
		break;
	}
	m_brush->DrawSegment3D(index[0], index[1], index[2], index[3], index[4], index[5]);
}

// Draw lines from (x0, y0) -> (y0, y1) with base as circle
void InteractorStylePaintBrush::DrawCircle(int x0, int y0, int x1, int y1, double radius)
{
	int x2Dindex, y2Dindex;
	switch (orientation)
	{
	case 0:
		x2Dindex = 1;
		y2Dindex = 2;
		break;
	case 1:
		x2Dindex = 0;
		y2Dindex = 2;
		break;
	case 2:
		x2Dindex = 0;
		y2Dindex = 1;
		break;
	}

	int loopLowerBoundX = floor(-radius / (this->spacing[x2Dindex]));
	int loopUpperBoundX = ceil(radius / (this->spacing[x2Dindex]));
	int loopLowerBoundY = floor(-radius / (this->spacing[y2Dindex]));
	int loopUpperBoundY = ceil(radius / (this->spacing[y2Dindex]));


	// check bound
	if (loopLowerBoundX >= loopUpperBoundX || loopLowerBoundY >= loopUpperBoundY) {
		std::cerr << "Bound calculation failed.";
		return;
	}

	for (int i = loopLowerBoundX; i < loopUpperBoundX; i++)
	{
		for (int j = loopLowerBoundY; j < loopUpperBoundY; j++)
		{
			double d = pow(i * spacing[x2Dindex], 2) + pow(j * spacing[y2Dindex], 2.);
			if (sqrt(d) < radius)
				this->DrawLine3D(x0 + i, y0 + j, x1 + i, y1 + j);
		}
	}
}

void InteractorStylePaintBrush::UpdateBorderWidgetPosition()
{
	if (m_borderWidget == NULL)
		return;

	// Update Brush propertyies
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	double size = m_drawBrushSizeSpinBox->value();
	double color[3] = { (double)color_r / 255.0,(double)color_g / 255.0,(double)color_b / 255.0 };
	
	//Restrict the box applying on whole pixel 
	this->GetInteractor()->GetPicker()->Pick(
		this->GetInteractor()->GetEventPosition()[0],
		this->GetInteractor()->GetEventPosition()[1],
		0,  // always zero.
		imageViewer->GetRenderer());

	double* picked = this->GetInteractor()->GetPicker()->GetPickPosition();

	//Check if valid pick
	if (picked[0] == 0.0&&picked[1] == 0.0)
		return;
	////Check if valid pick
	//if (!this->CheckValidPick(picked)) {
	//	this->m_borderWidget->Off();
	//	return;
	//}

	int x = this->GetInteractor()->GetEventPosition()[0];
	int y = this->GetInteractor()->GetEventPosition()[1];
	//int x = (int)(index[0] + 0.5)*spacing[0] + origin[0];
	//int y = (int)(index[1] + 0.5)*spacing[1] + origin[1];


	double X1 = x;
	double Y1 = y;
	double display_origin[3];
	double display_size[3];

	imageViewer->GetRenderer()->SetWorldPoint(0, 0, 0, 1);
	imageViewer->GetRenderer()->WorldToDisplay();
	imageViewer->GetRenderer()->GetDisplayPoint(display_origin);
	//cout << "World->Display vtkRenderer 00: " << "(" << x << "," << y << ")" << " --> " << "(" << result2[0] << "," << result2[1] << ")\n";

	imageViewer->GetRenderer()->SetWorldPoint(size, size, size, 1);
	imageViewer->GetRenderer()->WorldToDisplay();
	imageViewer->GetRenderer()->GetDisplayPoint(display_size);
	//cout << "World->Display vtkRenderer: "	<< "(" << x << "," << y << ")" << " --> " << "(" << result[0] << "," << result[1] << ")\n";

	double X3 = display_size[0] - display_origin[0];
	double Y3 = display_size[1] - display_origin[1];

	imageViewer->GetRenderer()->DisplayToNormalizedDisplay(X1, Y1);
	imageViewer->GetRenderer()->NormalizedDisplayToViewport(X1, Y1);
	imageViewer->GetRenderer()->ViewportToNormalizedViewport(X1, Y1);

	imageViewer->GetRenderer()->DisplayToNormalizedDisplay(X3, Y3);
	imageViewer->GetRenderer()->NormalizedDisplayToViewport(X3, Y3);
	imageViewer->GetRenderer()->ViewportToNormalizedViewport(X3, Y3);

	//if (m_drawBrushShapeComboBox->currentText() == "Square") //Square
	//{
	m_borderWidget->SetRepresentation(m_retangleRep);
	X1 = X1 - X3 / 2.0;
	Y1 = Y1 - Y3 / 2.0;
	m_retangleRep->SetPosition(X1, Y1);
	m_retangleRep->SetPosition2(X3, Y3);
	//	}

	vtkCoordinate* pos1 = m_borderWidget->GetBorderRepresentation()->GetPositionCoordinate();
	pos1->SetValue(X1, Y1);

	vtkCoordinate* pos2 = m_borderWidget->GetBorderRepresentation()->GetPosition2Coordinate();
	pos2->SetValue(X3, Y3);
	m_borderWidget->GetBorderRepresentation()->GetBorderProperty()->SetColor(color);
	m_borderWidget->GetBorderRepresentation()->GetBorderProperty()->SetLineStipplePattern(0xAAAA);
	m_borderWidget->GetBorderRepresentation()->Modified();
	m_borderWidget->Modified();
	m_borderWidget->On();
}

void InteractorStylePaintBrush::ReadfromImageData()
{

	MainWindow* mainWnd = MainWindow::GetMainWindow();
	//Clear Layer
	m_brush->SetDrawColor(0, 0, 0, 0);
	this->FillBox3D();

	int pos[3];
	switch (orientation)
	{
	case 0:
		for (int y = extent[2]; y < extent[3]; y++)
		{
			for (int z = extent[4]; z < extent[5]; z++)
			{
				pos[0] = m_sliceSplinBox[orientation]->value();
				pos[1] = y;
				pos[2] = z;
				double* val = static_cast<double *>(mainWnd->GetOverlay()->GetOutput()->GetScalarPointer(pos));
				if (*val > 0) {
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); ++i) {
						if (*val == i) {
							double rgba[4];
							uchar rgbaUCHAR[4];
							imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
							imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar
							m_brush->SetDrawColor(rgbaUCHAR[0], rgbaUCHAR[1], rgbaUCHAR[2], rgbaUCHAR[3]);
							m_brush->DrawSegment3D(0, y, z, 0, y, z);
							break;
						}
					}
				}
			}
		}
		break;
	case 1:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			for (int z = extent[4]; z < extent[5]; z++)
			{
				pos[0] = x;
				pos[1] = m_sliceSplinBox[orientation]->value();
				pos[2] = z;
				double* val = static_cast<double *>(mainWnd->GetOverlay()->GetOutput()->GetScalarPointer(pos));
				if (*val > 0) {
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); ++i) {
						if (*val == i) {
							double rgba[4];
							uchar rgbaUCHAR[4];
							imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
							imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar
							m_brush->SetDrawColor(rgbaUCHAR[0], rgbaUCHAR[1], rgbaUCHAR[2], rgbaUCHAR[3]);
							m_brush->DrawSegment3D(x, 0, z, x, 0, z);
							break;
						}
					}
				}
			}
		}
		break;
	case 2:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			for (int y = extent[2]; y < extent[3]; y++)
			{
				pos[0] = x;
				pos[1] = y;
				pos[2] = m_sliceSplinBox[orientation]->value();
				double * val = static_cast<double *>(mainWnd->GetOverlay()->GetOutput()->GetScalarPointer(pos));
				if (val == nullptr)
					continue;
				if (*val > 0) {
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); ++i) {
						if (*val == i) {
							double rgba[4];
							uchar rgbaUCHAR[4];
							imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
							imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar
							m_brush->SetDrawColor(rgbaUCHAR[0], rgbaUCHAR[1], rgbaUCHAR[2], rgbaUCHAR[3]);
							m_brush->DrawPoint(x, y);
							break;
						}
					}
				}
			}
		}
		break;
	}

}

void InteractorStylePaintBrush::Write2ImageData()
{
	//Update Layer
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	int pos[3];
	double pixelval;
	switch (orientation)
	{
	case 0:
		;
		for (int y = extent[2]; y < extent[3]; y++)
		{
			for (int z = extent[4]; z < extent[5]; z++)
			{
				pos[0] = m_sliceSplinBox[orientation]->value();
				pos[1] = y;
				pos[2] = z;
				uchar* val = static_cast<uchar *>(m_brush->GetOutput()->GetScalarPointer(0, y, z));
				if (val[0] > 0 || val[1] > 0 || val[2] > 0 || val[3] > 0) {
					// Check if color of the lookup table matches the color drawn on canvas
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); i++)
					{
						double rgba[4];
						uchar rgbaUCHAR[4];
						imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
						imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar

						if (val[0] == rgbaUCHAR[0] && val[1] == rgbaUCHAR[1] && val[2] == rgbaUCHAR[2]) {
							pixelval = i;
							break;
						}
					}
				}
				else
					pixelval = 0;
				mainWnd->GetOverlay()->SetPixel(pos, pixelval);
			}
		}
		break;
	case 1:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			for (int z = extent[4]; z < extent[5]; z++)
			{
				pos[0] = x;
				pos[1] = m_sliceSplinBox[orientation]->value();
				pos[2] = z;
				uchar* val = static_cast<uchar *>(m_brush->GetOutput()->GetScalarPointer(x, 0, z));
				if (val[0] > 0 || val[1] > 0 || val[2] > 0 || val[3] > 0)
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); i++)
					{
						double rgba[4];
						uchar rgbaUCHAR[4];
						imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
						imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar

						if (val[0] == rgbaUCHAR[0] && val[1] == rgbaUCHAR[1] && val[2] == rgbaUCHAR[2]) {
							pixelval = i;
							break;
						}
					}
				else
					pixelval = 0;
				mainWnd->GetOverlay()->SetPixel(pos, pixelval);
			}
		}
		break;
	case 2:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			for (int y = extent[2]; y < extent[3]; y++)
			{
				pos[0] = x;
				pos[1] = y;
				pos[2] = m_sliceSplinBox[orientation]->value();
				uchar * val = static_cast<uchar *>(m_brush->GetOutput()->GetScalarPointer(x, y, 0));
				if (val[0] > 0 || val[1] > 0 || val[2] > 0 || val[3] > 0)
					for (int i = 0; i < imageViewer->getLookupTable()->GetNumberOfColors(); i++)
					{
						double rgba[4];
						uchar rgbaUCHAR[4];
						imageViewer->getLookupTable()->GetIndexedColor(i, rgba);
						imageViewer->getLookupTable()->GetColorAsUnsignedChars(rgba, rgbaUCHAR); // convert double to uchar

						if (val[0] == rgbaUCHAR[0] && val[1] == rgbaUCHAR[1] && val[2] == rgbaUCHAR[2]) {
							pixelval = i;
							break;
						}
					}
				else
					pixelval = 0;
				mainWnd->GetOverlay()->SetPixel(pos, pixelval);

			}
		}
		break;
	}
	mainWnd->GetOverlay()->GetOutput()->Modified();

}


void InteractorStylePaintBrush::Render()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_brush == NULL)
		return;

	mainWnd->RenderAll2DViewers();

	m_brush->Update();
	m_brush->GetOutput()->SetOrigin(origin);
	m_brush->GetOutput()->SetSpacing(spacing);
}


/* Return true if pick is valid, false ortherwise */
bool InteractorStylePaintBrush::CheckValidPick(double *picked)
{
	//MainWindow* mainWnd = MainWindow::GetMainWindow();
	//Image* labelMap = mainWnd->GetPlanner()->GetImageByUniqueName("Label");

	//if (!labelMap)
	//	return false;


	///// Check if valid pick on display
	//if (picked[0] == 0.0&&picked[1] == 0.0)
	//	return false;

	///// Check if pick is inside label map
	//int* extent = labelMap->GetImageData()->GetExtent();
	//
	//// obtain ijk index of the clicked point
	//int ijk[3];
	//labelMap->GetReslicer()->GetITKIndexFromVTKImageActorPoint(picked, ijk, this->orientation);

	//// index has to be within extent
	//for (int i = 0; i < 3; i++)
	//{
	//	int l_index = ijk[i];
	//	int l_lowerRange = extent[i * 2], l_upperRange = extent[i * 2 + 1];
	//
	//	if (l_index < l_lowerRange || l_index > l_upperRange) {
	//		return false;
	//	}
	//}

	return true;
}


