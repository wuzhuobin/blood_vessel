/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
Junior Research Assistant


This class is based on vtkInteractorStyleSwitch, written to allow easy 
switching between 2D interactors. 

Wong Matthew Lun
Copyright (C) 2016
*/


#include <vtkImageInterpolator.h>
#include <vtkInteractorStyle.h>
#include "InteractorStyleSwitch.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

vtkStandardNewMacro(InteractorStyleSwitch);

InteractorStyleSwitch::InteractorStyleSwitch()
{
	WindowLevel = InteractorStyleWindowLevel::New();
	Navigation = InteractorStyleNavigation::New();
	PolygonDraw = InteractorStylePolygonDraw::New();
	PaintBrush = InteractorStylePaintBrush::New();
	this->CurrentStyle = 0;
}

InteractorStyleSwitch::~InteractorStyleSwitch()
{
	WindowLevel->Delete();
	WindowLevel = 0;

	Navigation->Delete();
	Navigation = 0;

	PolygonDraw->Delete();
	PolygonDraw = 0;

	PaintBrush->Delete();
	PaintBrush = 0;
}

void InteractorStyleSwitch::InternalUpdate()
{
	MainWindow* mainwnd = MainWindow::GetMainWindow();

	if (this->CurrentStyle != this->PolygonDraw) {
		this->PolygonDraw->SetPolygonModeEnabled(false);
	}

	//if (this->imageViewer->GetRenderWindow()) {
	//	this->imageViewer->Render();
	//}

	if (this->CurrentStyle != this->PaintBrush)
		this->PaintBrush->SetPaintBrushModeEnabled(false);
}

void InteractorStyleSwitch::SetAutoAdjustCameraClippingRange(int value)
{
	if (value == this->AutoAdjustCameraClippingRange)
	{
		return;
	}

	if (value < 0 || value > 1)
	{
		vtkErrorMacro("Value must be between 0 and 1 for" <<
			" SetAutoAdjustCameraClippingRange");
		return;
	}

	this->AutoAdjustCameraClippingRange = value;
	this->Navigation->SetAutoAdjustCameraClippingRange(value);
	this->WindowLevel->SetAutoAdjustCameraClippingRange(value);
	this->PolygonDraw->SetAutoAdjustCameraClippingRange(value);
	this->PaintBrush->SetAutoAdjustCameraClippingRange(value);
	this->Modified();
}

void InteractorStyleSwitch::SetDefaultRenderer(vtkRenderer* renderer)
{
	this->vtkInteractorStyle::SetDefaultRenderer(renderer);
	this->Navigation->SetDefaultRenderer(renderer);
	this->WindowLevel->SetDefaultRenderer(renderer);
	this->PolygonDraw->SetDefaultRenderer(renderer);
	this->PaintBrush->SetDefaultRenderer(renderer);
}

void InteractorStyleSwitch::SetCurrentRenderer(vtkRenderer* renderer)
{
	this->vtkInteractorStyle::SetCurrentRenderer(renderer);
	this->Navigation->SetCurrentRenderer(renderer);
	this->WindowLevel->SetCurrentRenderer(renderer);
	this->PolygonDraw->SetCurrentRenderer(renderer);
	this->PaintBrush->SetCurrentRenderer(renderer);
}

void InteractorStyleSwitch::SetViewers(MyImageViewer* imageViewer)
{
	//this->imageViewer = imageViewer;
	this->Navigation->SetImageViewer(imageViewer);
	this->WindowLevel->SetImageViewer(imageViewer);
	this->PolygonDraw->SetImageViewer(imageViewer);
	this->PaintBrush->SetImageViewer(imageViewer);
}

void InteractorStyleSwitch::initializeQWidget(QSpinBox * sliceX, QSpinBox * sliceY, QSpinBox * sliceZ, 
	QDoubleSpinBox * window, QDoubleSpinBox * level, 
	QSpinBox * drawBrushSize, 
	QComboBox * drawShape, 
	QCheckBox * drawVolumetric, 
	QCheckBox * drawIsotropic)
{
	this->Navigation->SetSliceSpinBox(sliceX, sliceY, sliceZ);
	this->WindowLevel->SetSliceSpinBox(sliceX, sliceY, sliceZ);
	this->PolygonDraw->SetSliceSpinBox(sliceX, sliceY, sliceZ);
	this->PaintBrush->SetSliceSpinBox(sliceX, sliceY, sliceZ);
	this->WindowLevel->SetWindowLevelSpinBox(window, level);
	this->PaintBrush->SetDrawBrushSizeSpinBox(drawBrushSize);
	this->PaintBrush->SetDrawBrushShapeComBox(drawShape);
	this->PaintBrush->SetDrawVolumetricCheckBox(drawVolumetric);
	this->PaintBrush->SetDrawIsotropicCheckBox(drawIsotropic);
}

void InteractorStyleSwitch::SetCurrentSlice(int slice)
{
	this->Navigation->SetCurrentSlice(slice);
	this->WindowLevel->SetCurrentSlice(slice);
	this->PaintBrush->SetCurrentSlice(slice);
	//this->po
}


void InteractorStyleSwitch::SetInteractor(vtkRenderWindowInteractor *iren)
{
	if (iren == this->Interactor)
	{
		return;
	}
	// if we already have an Interactor then stop observing it
	if (this->Interactor)
	{
		this->Interactor->RemoveObserver(this->EventCallbackCommand);
	}
	this->Interactor = iren;
	// add observers for each of the events handled in ProcessEvents
	if (iren)
	{
		iren->AddObserver(vtkCommand::CharEvent,
			this->EventCallbackCommand,
			this->Priority);

		iren->AddObserver(vtkCommand::DeleteEvent,
			this->EventCallbackCommand,
			this->Priority);
	}
}