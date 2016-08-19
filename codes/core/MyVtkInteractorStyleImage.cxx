#include "MyVtkInteractorStyleImage.h"
#include "vtkRenderer.h"
#include <vtkImageProperty.h>
#include <vtkImageProperty.h>
#include <qdebug.h>


vtkStandardNewMacro(MyVtkInteractorStyleImage);

void MyVtkInteractorStyleImage::SetImageViewer(MyImageViewer* imageViewer)
{
	//Contour Widget
	contourWidget = NULL;
	contourRep = NULL;
	//Paint Brush
	m_drawValueComboBox = NULL;
	m_drawBrushShapeComboBox = NULL;
	m_drawBrushSizeSpinBox = NULL;
	m_drawVolumetricCheckBox = NULL;
	m_drawIsotropicCheckBox = NULL;
	m_borderWidget = NULL;
	m_retangleRep = NULL;
	m_brush = NULL;
	BrushActor = NULL;
	//Default color: Red
	color_r = 255;
	color_g = 0;
	color_b = 0;
	opacity = 255;
	//Image Viewer
	this->imageViewer = imageViewer;
	m_minSlice = imageViewer->GetSliceMin();
	m_maxSlice = imageViewer->GetSliceMax();
	m_slice = imageViewer->GetSlice();
	orientation = imageViewer->GetSliceOrientation();
	//Default setting
	m_mode = NavaigationMode;
	leftClicked = false;
	rightClicked = false;
	middleClicked = false;

	//the property which paintbrush needed
	imageViewer->GetInput()->GetExtent(extent);
	imageViewer->GetInput()->GetSpacing(spacing);
	imageViewer->GetInput()->GetOrigin(origin);
}

void MyVtkInteractorStyleImage::SetSliceSpinBox(QSpinBox* x, QSpinBox* y, QSpinBox* z)
{
	m_sliceSplinBox[0] = x;
	m_sliceSplinBox[1] = y;
	m_sliceSplinBox[2] = z;
}


void MyVtkInteractorStyleImage::SetWindowLevelSpinBox(QDoubleSpinBox* w, QDoubleSpinBox* l)
{
	m_wlDoubleSpinBox[0] = w;
	m_wlDoubleSpinBox[1] = l;
}

void MyVtkInteractorStyleImage::SetDrawBrushSizeSpinBox(QSpinBox* s)
{
	m_drawBrushSizeSpinBox = s;
}

void MyVtkInteractorStyleImage::SetDrawBrushShapeComBox(QComboBox* comboBox)
{
	m_drawBrushShapeComboBox = comboBox;
}

void MyVtkInteractorStyleImage::SetDrawVolumetricCheckBox(QCheckBox* checkBox)
{
	m_drawVolumetricCheckBox = checkBox;
}

void MyVtkInteractorStyleImage::SetDrawIsotropicCheckBox(QCheckBox* checkBox)
{
	m_drawIsotropicCheckBox = checkBox;
}
void MyVtkInteractorStyleImage::SetDrawColor(const int* rgb)
{
	SetDrawColor(rgb[0], rgb[1], rgb[2]);
}
void MyVtkInteractorStyleImage::SetDrawColor(int r, int g, int b)
{
	color_r = r;
	color_g = g;
	color_b = b;
}
void MyVtkInteractorStyleImage::SetDrawOpacity(int opacity)
{
	this->opacity = opacity;
}
void MyVtkInteractorStyleImage::SetMode(int m)
{
	if (m == 1)
		m_mode = NavaigationMode;

	if (m == 2)
		m_mode = WindowLevelMode;

	if (m == 3)
	{
		m_mode = PaintBrushMode;
		SetPaintBrushModeEnabled(true);
		SetPolygonModeEnabled(false);
	}

	if (m == 4)
	{
		m_mode = PolygonMode;
		SetPolygonModeEnabled(true);
		SetPaintBrushModeEnabled(true);
	}

}

void MyVtkInteractorStyleImage::MoveSliceForward()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_slice < m_maxSlice)
	{
		m_slice += 1;
		switch (orientation)
		{
		case 0:
			mainWnd->slotChangeSlice(m_slice, m_sliceSplinBox[1]->value(), m_sliceSplinBox[2]->value());
			break;
		case 1:
			mainWnd->slotChangeSlice(m_sliceSplinBox[0]->value(), m_slice, m_sliceSplinBox[2]->value());
			break;
		case 2:
			mainWnd->slotChangeSlice(m_sliceSplinBox[0]->value(), m_sliceSplinBox[1]->value(), m_slice);
			break;
		}

	}
}

void MyVtkInteractorStyleImage::MoveSliceBackward()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_slice > m_minSlice)
	{
		m_slice -= 1;
		switch (orientation)
		{
		case 0:
			mainWnd->slotChangeSlice(m_slice, m_sliceSplinBox[1]->value(), m_sliceSplinBox[2]->value());
			break;
		case 1:
			mainWnd->slotChangeSlice(m_sliceSplinBox[0]->value(), m_slice, m_sliceSplinBox[2]->value());
			break;
		case 2:
			mainWnd->slotChangeSlice(m_sliceSplinBox[0]->value(), m_sliceSplinBox[1]->value(), m_slice);
			break;
		}

	}
}

void MyVtkInteractorStyleImage::OnMouseWheelForward()
{
	//std::cout << "Scrolled mouse wheel forward." << std::endl;
	if (m_slice < m_maxSlice) {
		MoveSliceForward();
	}
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelForward();
}


void MyVtkInteractorStyleImage::OnMouseWheelBackward()
{
	//std::cout << "Scrolled mouse wheel backward." << std::endl;
	if (m_slice > m_minSlice) {
		MoveSliceBackward();
	}
	// don't forward events, otherwise the image will be zoomed 
	// in case another interactorstyle is used (e.g. trackballstyle, ...)
	// vtkInteractorStyleImage::OnMouseWheelBackward();
}

void MyVtkInteractorStyleImage::OnLeftButtonUp()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	leftClicked = false;
	if (m_mode == 3)
	{
		if (!isDraw)
		{
			this->Write2ImageData();
			mainWnd->slotOverlayVisibilty(true, orientation);
			//Clear Layer
			m_brush->SetDrawColor(0, 0, 0, 0);
			this->FillBox3D();
			this->Render();
			isDraw = true;
		}

	}

}

void MyVtkInteractorStyleImage::OnLeftButtonDown()
{
	leftClicked = true;
	isDraw = false;
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_mode == 1)
		CalculateIndex();

	if (m_mode == 2)
	{
		int x = this->GetInteractor()->GetEventPosition()[0];
		int y = this->GetInteractor()->GetEventPosition()[1];

		this->WindowLevelStartPosition[0] = x;
		this->WindowLevelStartPosition[1] = y;

		this->FindPokedRenderer(x, y);
	}
	if (m_mode == 3)
	{
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
			for (int i = 0; i<3; i++)
			{
				index[i] = (picked[i] - origin[i]) / spacing[i];
			}
		}
		draw_index_old[0] = (int)(index[0] + 0.5);
		draw_index_old[1] = (int)(index[1] + 0.5);
		draw_index_old[2] = (int)(index[2] + 0.5);
		if (rightClicked == true)
		{
			rightClicked = false;
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



}

void MyVtkInteractorStyleImage::OnRightButtonUp()
{
	vtkInteractorStyleTrackballCamera::OnRightButtonUp();
	rightClicked = false;
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_mode == 3)
	{
		if (!isDraw)
		{
			this->Write2ImageData();
			mainWnd->slotOverlayVisibilty(true, orientation);
			//Clear Layer
			m_brush->SetDrawColor(0, 0, 0, 0);
			this->FillBox3D();
			this->Render();
			isDraw = true;
		}
	}

}

void MyVtkInteractorStyleImage::OnRightButtonDown()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	isDraw = false;
	rightClicked = true;
	if (m_mode == 3)
	{
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
			for (int i = 0; i<3; i++)
			{
				index[i] = (picked[i] - origin[i]) / spacing[i];
			}
		}
		draw_index_old[0] = (int)(index[0] + 0.5);
		draw_index_old[1] = (int)(index[1] + 0.5);
		draw_index_old[2] = (int)(index[2] + 0.5);
		if (leftClicked == true)
		{
			leftClicked = false;
		}
		else
		{
			this->ReadfromImageData();
			this->Render();
		}
		mainWnd->slotOverlayVisibilty(false, orientation);
		this->Draw(false);
		this->Render();
	}

	if (m_mode == 4) {
		if (contourWidget) {
			contourWidget->CloseLoop();
			FillPolygon();
		}
	}
	vtkInteractorStyleTrackballCamera::OnRightButtonDown();


}

void MyVtkInteractorStyleImage::OnMiddleButtonUp()
{
	middleClicked = false;
	// Forward events
	vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
}

void MyVtkInteractorStyleImage::OnMiddleButtonDown()
{
	middleClicked = true;
	vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
}
void MyVtkInteractorStyleImage::OnMouseMove()
{

	if (m_mode == 1 && leftClicked == true)
		CalculateIndex();
	else if (m_mode == 2 && leftClicked == true)
		WindowLevel();
	else if (m_mode == 3 && leftClicked == true)
	{
		Draw(true);
		this->UpdateBorderWidgetPosition();
		this->Render();
	}
	else if (m_mode == 3 && rightClicked == true)
	{
		Draw(false);
		this->UpdateBorderWidgetPosition();
		this->Render();
	}
	else if (m_mode == 3)
	{
		this->UpdateBorderWidgetPosition();
		this->Render();
		vtkInteractorStyleImage::OnMouseMove();
	}
	else
	{
		vtkInteractorStyleImage::OnMouseMove();
	}
	/*
	if ( rightClicked == true || middleClicked == true){
	Synchronize();
	}
	*/
}




void MyVtkInteractorStyleImage::OnChar()
{
	vtkRenderWindowInteractor *rwi = this->Interactor;
	switch (rwi->GetKeyCode())
	{
	case 'r':
	case 'R':
		if (imageViewer->GetInput() != NULL) {
			m_wlDoubleSpinBox[0]->setValue(imageViewer->GetDefaultWindowLevel()[0]);
			m_wlDoubleSpinBox[1]->setValue(imageViewer->GetDefaultWindowLevel()[1]);
		}
		break;
	}
}

void MyVtkInteractorStyleImage::OnKeyPress()
{
	// Get the keypress
	vtkRenderWindowInteractor *rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	// Handle an arrow key
	if (key == "Up")
		MoveSliceForward();
	else if (key == "Down")
		MoveSliceBackward();

	// Forward events
	vtkInteractorStyleImage::OnKeyPress();
}

void MyVtkInteractorStyleImage::CalculateIndex()
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
		for (int i = 0; i<3; i++)
		{
			index[i] = (picked[i] - origin[i]) / spacing[i];
		}
		mainWnd->slotChangeSlice((int)(index[0] + 0.5), (int)(index[1] + 0.5), (int)(index[2] + 0.5));
	}


}

void MyVtkInteractorStyleImage::RemoveContourWidgetAll() {
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	for (int i = 0; i < mainwnd->GetVisibleImageNo(); i++) {
		//mainwnd->GetMyVtkInteractorStyleImage(i)->RemoveContourWidget();
		mainwnd->GetMyImageViewer(i)->GetRenderWindow()->Render();
	}


}

void MyVtkInteractorStyleImage::SetPaintBrushModeEnabled(bool b)
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();

	if (m_borderWidget != NULL)
	{
		m_retangleRep->Delete();
		m_borderWidget->Delete();
		m_borderWidget = NULL;
	}
	if (m_brush != NULL)
	{
		imageViewer->GetannotationRenderer()->RemoveActor(BrushActor);

		m_brush->Delete();
		m_brush = NULL;
		BrushActor->Delete();
		BrushActor = NULL;

	}
	if (b)
	{
		m_retangleRep = vtkBorderRepresentation::New();
		m_borderWidget = vtkBorderWidget::New();
		m_borderWidget->SetInteractor(this->GetInteractor());
		m_borderWidget->SetCurrentRenderer(imageViewer->GetRenderer());
		m_borderWidget->SetRepresentation(m_retangleRep);
		m_borderWidget->SetManagesCursor(true);
		m_borderWidget->GetBorderRepresentation()->SetMoving(false);
		m_borderWidget->GetBorderRepresentation()->SetProportionalResize(true);
		m_borderWidget->SelectableOff();
		m_borderWidget->ProcessEventsOff();
		m_borderWidget->Off();
	}

	this->GetInteractor()->Render();



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

	BrushActor = vtkImageActor::New();
	BrushActor->SetInputData(m_brush->GetOutput());
	BrushActor->GetProperty()->SetInterpolationTypeToNearest();
	BrushActor->Update();

	// Set the image actor

	switch (this->orientation)
	{
	case 0:
		this->BrushActor->SetDisplayExtent(
			0, 0, extent[2], extent[3], extent[4], extent[5]);
		break;
	case 1:
		this->BrushActor->SetDisplayExtent(
			extent[0], extent[1], 0, 0, extent[4], extent[5]);
		break;
	case 2:
		this->BrushActor->SetDisplayExtent(
			extent[0], extent[1], extent[2], extent[3], 0, 0);
		break;
	}
	imageViewer->GetannotationRenderer()->AddActor(BrushActor);
	imageViewer->GetRenderWindow()->Render();

}

void MyVtkInteractorStyleImage::Draw(bool b)
{
	if (b)
		m_brush->SetDrawColor(color_r, color_g, color_b, opacity);
	else
		m_brush->SetDrawColor(0, 0, 0, 0);

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
		for (int i = 0; i<3; i++)
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
		if (m_drawBrushSizeSpinBox->value() % 2 == 1)
			DrawCircle(draw_index_old[x2Dindex], draw_index_old[y2Dindex], draw_index[x2Dindex], draw_index[y2Dindex], m_drawBrushSizeSpinBox->value() / 2);
		else
		{
			DrawCircle(draw_index_old[x2Dindex] - 1, draw_index_old[y2Dindex], draw_index[x2Dindex] - 1, draw_index[y2Dindex], m_drawBrushSizeSpinBox->value() / 2 - 1);
			DrawCircle(draw_index_old[x2Dindex] - 1, draw_index_old[y2Dindex] - 1, draw_index[x2Dindex] - 1, draw_index[y2Dindex] - 1, m_drawBrushSizeSpinBox->value() / 2 - 1);
			DrawCircle(draw_index_old[x2Dindex], draw_index_old[y2Dindex] - 1, draw_index[x2Dindex], draw_index[y2Dindex] - 1, m_drawBrushSizeSpinBox->value() / 2 - 1);
			DrawCircle(draw_index_old[x2Dindex], draw_index_old[y2Dindex], draw_index[x2Dindex], draw_index[y2Dindex], m_drawBrushSizeSpinBox->value() / 2 - 1);
		}
	}
	//Default:Square
	else
	{
		if (m_drawBrushSizeSpinBox->value() % 2 == 1)
		{// Odd Number Brush Size
			for (int x = -(m_drawBrushSizeSpinBox->value() - 1) / 2; x < (m_drawBrushSizeSpinBox->value() - 1) / 2 + 1; x++)
			{
				for (int y = -(m_drawBrushSizeSpinBox->value() - 1) / 2; y < (m_drawBrushSizeSpinBox->value() - 1) / 2 + 1; y++)
				{
					this->DrawLine3D(draw_index_old[x2Dindex] + x, draw_index_old[y2Dindex] + y, draw_index[x2Dindex] + x, draw_index[y2Dindex] + y);
				}
			}
		}
		else
		{ // Even Number Brush Size
			for (int x = -(m_drawBrushSizeSpinBox->value()) / 2; x < (m_drawBrushSizeSpinBox->value()) / 2; x++)
			{
				for (int y = -(m_drawBrushSizeSpinBox->value()) / 2; y < (m_drawBrushSizeSpinBox->value()) / 2; y++)
				{
					this->DrawLine3D(draw_index_old[x2Dindex] + x, draw_index_old[y2Dindex] + y, draw_index[x2Dindex] + x, draw_index[y2Dindex] + y);
				}
			}

		}
	}

	// Reset the old draw index
	for (int i = 0; i < 3; i++)
	{
		draw_index_old[i] = draw_index[i];
	}


}

void MyVtkInteractorStyleImage::FillBox3D()
{

	switch (orientation)
	{
	case 0:
		for (int y = extent[2]; y < extent[3]; y++)
		{
			m_brush->DrawSegment3D(0, y, extent[4], 0, y, extent[5]);
		}
		break;
	case 1:
		for (int x = extent[0]; x < extent[1]; x++)
		{
			m_brush->DrawSegment3D(x, 0, extent[4], x, 0, extent[5]);
		}
		break;
	case 2:
		m_brush->FillBox(extent[0], extent[1], extent[2], extent[3]);
		break;
	}

}

void MyVtkInteractorStyleImage::DrawLine3D(int x0, int y0, int x1, int y1)
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

void MyVtkInteractorStyleImage::DrawCircle(int x0, int y0, int x1, int y1, int radius)
{
	int x = 0, y = radius;
	int d = 1 - radius;
	while (x <= y) {
		for (int i = -x; i <= x; i++)
		{
			this->DrawLine3D(x0 + i, y0 + y, x1 + i, y1 + y);
			this->DrawLine3D(x0 + i, y0 - y, x1 + i, y1 - y);
		}

		for (int j = -y; j <= y; j++)
		{
			this->DrawLine3D(x0 + j, y0 + x, x1 + j, y1 + x);
			this->DrawLine3D(x0 + j, y0 - x, x1 + j, y1 - x);
		}

		if (d<0)
			d += (2 * x) + 3;
		else {
			d += (2 * (x - y)) + 5;
			y -= 1;
		}
		x++;
	}

	/*			//MidPoint Circle algorithm //Th shape of the circle is a little bit wierd
	int x = radius;
	int y = 0;
	int err = 0;

	while (x >= y)
	{
	for (int i = -x; i <= x; i++)
	{
	this->DrawLine3D(x0 + i, y0 + y, x1 + i, y1 + y);
	this->DrawLine3D(x0 + i, y0 - y, x1 + i, y1 - y);
	}

	for (int j = -y; j <= y; j++)
	{
	this->DrawLine3D(x0 + j, y0 + x, x1 + j, y1 + x);
	this->DrawLine3D(x0 + j, y0 - x, x1 + j, y1 - x);
	}

	y += 1;
	err += 1 + 2 * y;
	if (2 * (err - x) + 1 > 0)
	{
	x -= 1;
	err += 1 - 2 * x;
	}
	}
	*/
}
void MyVtkInteractorStyleImage::UpdateBorderWidgetPosition()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	if (m_borderWidget == NULL)
		return;

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
	double index[3];
	if (imageViewer->GetInput() != NULL) {
		picked[orientation] = origin[orientation] + m_slice * spacing[orientation];
		for (int i = 0; i<3; i++)
		{
			index[i] = (picked[i] - origin[i]) / spacing[i];
		}
	}

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

	imageViewer->GetRenderer()->SetWorldPoint(size*spacing[0], size*spacing[1], size*spacing[2], 1);
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
	//	}

	vtkCoordinate* pos1 = m_borderWidget->GetBorderRepresentation()->GetPositionCoordinate();
	pos1->SetValue(X1, Y1);

	vtkCoordinate* pos2 = m_borderWidget->GetBorderRepresentation()->GetPosition2Coordinate();
	pos2->SetValue(X3, Y3);
	m_borderWidget->GetBorderRepresentation()->GetBorderProperty()->SetColor(color);
	m_borderWidget->GetBorderRepresentation()->GetBorderProperty()->SetLineStipplePattern(0xf0f0);
	m_borderWidget->GetBorderRepresentation()->Modified();
	m_borderWidget->Modified();
	m_borderWidget->On();


}

void MyVtkInteractorStyleImage::ReadfromImageData()
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

void MyVtkInteractorStyleImage::Render()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	m_brush->Update();
	m_brush->GetOutput()->SetOrigin(origin);
	m_brush->GetOutput()->SetSpacing(spacing);
	mainWnd->RenderAllViewer();
}

void MyVtkInteractorStyleImage::Write2ImageData()
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
				if (val[0] > 0 || val[1] > 0 || val[2] > 0 || val[3] > 0){
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

void MyVtkInteractorStyleImage::SetPolygonModeEnabled(bool b)
{
	if (contourWidget) {
		contourWidget->Off();
		contourWidget->SetRepresentation(NULL);
		contourWidget->EnabledOff();
		contourWidget->Delete();
		contourWidget = NULL;
	}


	if (contourRep) {
		contourRep->Delete();
		contourRep = NULL;
	}
	if (b) {

		//this->Synchronize();


		contourWidget = vtkContourWidget::New();
		contourWidget->SetInteractor(imageViewer->GetRenderWindow()->GetInteractor());
		contourWidget->SetCurrentRenderer(imageViewer->GetannotationRenderer());

		contourRep = vtkOrientedGlyphContourRepresentation::New();
		contourRep->SetRenderer(imageViewer->GetannotationRenderer());
		contourRep->GetLinesProperty()->SetColor(255, 255, 0);

		vtkPolyData* cursorpolyData = contourRep->GetActiveCursorShape();
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
		contourWidget->SetRepresentation(contourRep);
		contourWidget->FollowCursorOn();
		contourWidget->On();
		contourWidget->EnabledOn();

		imageViewer->GetRenderWindow()->Render();
	}
}



bool MyVtkInteractorStyleImage::FillPolygon()
{
	//Check if region is selected by the user
	if (contourRep == NULL)
		return false;
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	// enable brush to fill the contour
	m_brush->SetDrawColor(color_r, color_g, color_b, opacity);

	vtkPolyData* polydata;
	polydata = contourRep->GetContourRepresentationAsPolyData();
	pointsInsideContour = vtkPoints::New();
	vtkPolygon* polygon = vtkPolygon::New();
	int numOfPoints = polydata->GetNumberOfPoints();
	cout << numOfPoints << endl;
	// Get the coordinates of the contour data points
	double origin[3];
	double spacing[3];
	imageViewer->GetInput()->GetOrigin(origin);
	imageViewer->GetInput()->GetSpacing(spacing);

	for (vtkIdType i = 0; i < numOfPoints; i++)
	{
		double p[3];
		double s[3];
		polydata->GetPoint(i, p);
		//std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2] << ")" << std::endl;
		//Take one image data 1 to be reference
		s[0] = (p[0] - origin[0]) / spacing[0];
		s[1] = (p[1] - origin[1]) / spacing[1];
		s[2] = (p[2] - origin[2]) / spacing[2];
		//cout << s[0] << " " << s[1] << " " << s[2] << endl;
		//Test whether the points are inside the polygon or not
		if (orientation == 0)
			polygon->GetPoints()->InsertNextPoint(0.0, s[1], s[2]);
		else if (orientation == 1)
			polygon->GetPoints()->InsertNextPoint(s[0], 0.0, s[2]);
		else if (orientation == 2)
			polygon->GetPoints()->InsertNextPoint(s[0], s[1], 0.0);
	}

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

	cout << "Bounds: " << bounds_int[0] << " " << bounds_int[1] << " " << bounds_int[2] << " " << bounds_int[3] << " " << bounds_int[4] << " " << bounds_int[5] << endl;

	if (orientation == 0)
	{
		for (int y = bounds_int[2]; y < bounds_int[3]; y++) {
			for (int z = bounds_int[4]; z < bounds_int[5]; z++) {
				double p[3];
				p[0] = 0.0;
				p[1] = (double)y;
				p[2] = (double)z;
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					pointsInsideContour->InsertNextPoint(p);
					DrawLine3D(y, z, y, z);
				}

			}
		}
	}
	else if (orientation == 1)
	{
		for (int x = bounds_int[0]; x < bounds_int[1]; x++) {
			for (int z = bounds_int[4]; z < bounds_int[5]; z++) {
				double p[3];
				p[0] = (double)x;
				p[1] = 0.0;
				p[2] = (double)z;
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					pointsInsideContour->InsertNextPoint(p);
					DrawLine3D(x, z, x, z);
				}
			}
		}
	}
	else if (orientation == 2)
	{
		for (int x = bounds_int[0]; x < bounds_int[1]; x++) {
			for (int y = bounds_int[2]; y < bounds_int[3]; y++) {
				double p[3];
				p[0] = (double)x;
				p[1] = (double)y;
				p[2] = 0.0;
				if (polygon->PointInPolygon(p, polygon->GetPoints()->GetNumberOfPoints(),
					static_cast<double*>(polygon->GetPoints()->GetData()->GetVoidPointer(0)), bounds, n)) {
					// fill the contour
					pointsInsideContour->InsertNextPoint(p);
					DrawLine3D(x, y, x, y);
				}

			}
		}
	}
	this->Write2ImageData();
	mainWnd->slotOverlayVisibilty(true, orientation);
	m_brush->SetDrawColor(0, 0, 0, 0);
	this->FillBox3D();
	this->Render();

	if (pointsInsideContour->GetNumberOfPoints() == 0)
	{
		return false;
	}
	else
		return true;
}


void MyVtkInteractorStyleImage::RemoveContourWidget() {
	if (contourWidget)
	{
		contourWidget->SetInteractor(NULL);
		contourWidget->Off();
		contourWidget->SetRepresentation(NULL);
		contourWidget->EnabledOff();
		contourWidget->Delete();
		contourWidget = NULL;
	}
	if (contourRep) {
		contourRep->Delete();
		contourRep = NULL;
	}
	pointsInsideContour = NULL;

}


void MyVtkInteractorStyleImage::WindowLevel()
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



void MyVtkInteractorStyleImage::SetSliceRange(int min, int max)
{
	m_minSlice = min;
	m_maxSlice = max;
}

void MyVtkInteractorStyleImage::SetCurrentSlice(int slice)
{
	m_slice = slice;
}

void MyVtkInteractorStyleImage::Synchronize()
{

	MainWindow* mainwnd = MainWindow::GetMainWindow();
	vtkCamera* camera = imageViewer->GetRenderer()->GetActiveCamera();
	for (int i = 0; i<mainwnd->GetVisibleImageNo(); i++)
	{
		mainwnd->GetMyImageViewer(i)->GetRenderer()->SetActiveCamera(camera);
		mainwnd->GetMyImageViewer(i)->Render();
	}
}

vtkPoints* MyVtkInteractorStyleImage::GetPointsInsideContour()
{
	if (pointsInsideContour != NULL)
		return pointsInsideContour;
	else
		return NULL;
}

vtkOrientedGlyphContourRepresentation* MyVtkInteractorStyleImage::GetContourRep()
{
	if (contourRep != NULL)
		return contourRep;
	else
		return NULL;
}

void MyVtkInteractorStyleImage::SetContourVisibility(bool b)
{
	if (b)
	{
		if (contourWidget)
			contourWidget->SetEnabled(1);
	}
	else
	{
		if (contourWidget)
			contourWidget->SetEnabled(0);
	}
}