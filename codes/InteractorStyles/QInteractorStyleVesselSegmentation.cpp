﻿#include "QInteractorStyleVesselSegmentation.hpp"
#include "ui_qinteractorstylevesselsegmentation.h"

vtkStandardNewMacro(QInteractorStyleVesselSegmentation);
QSETUP_UI_SRC(QInteractorStyleVesselSegmentation);
void QInteractorStyleVesselSegmentation::Initialization()
{
	QAbstractNavigation::Initialization();
	this->setEnabled(true);
}
void QInteractorStyleVesselSegmentation::SetPolygonModeEnabled(bool flag)
{
	InteractorStyleVesselSegmentation::SetPolygonModeEnabled(flag);
	Initialization();
}
void QInteractorStyleVesselSegmentation::SetCurrentFocalPointWithImageCoordinate(int i, int j, int k)
{
	InteractorStyleVesselSegmentation::SetCurrentFocalPointWithImageCoordinate(i, j, k);
	QAbstractNavigation::SetCurrentFocalPointWithImageCoordinate(i, j, k);
}

void QInteractorStyleVesselSegmentation::FillPolygon()
{
	InteractorStyleVesselSegmentation::FillPolygon();
}

void QInteractorStyleVesselSegmentation::SetSmoothCurveEnable(bool flag)
{
	if (flag) {
		InteractorStyleVesselSegmentation::SetSmoothCurveEnable();
	}
}

void QInteractorStyleVesselSegmentation::SetPolygonEnable(bool flag)
{
	if (flag) {
		InteractorStyleVesselSegmentation::SetPolygonEnable();
	}
}

void QInteractorStyleVesselSegmentation::EnableNoSegmentation(bool flag)
{
	if (flag)
		InteractorStyleVesselSegmentation::EnableNoSegmentation();
}

void QInteractorStyleVesselSegmentation::EnableLumenSegmentation(bool flag)
{
	if(flag)
		InteractorStyleVesselSegmentation::EnableLumenSegmentation();
}

void QInteractorStyleVesselSegmentation::EnableVesselWallSegmentation(bool flag)
{
	if (flag)
		InteractorStyleVesselSegmentation::EnableVesselWallSegmentation();
}

void QInteractorStyleVesselSegmentation::SetContourLabel(int label)
{
	InteractorStyleVesselSegmentation::SetContourLabel(label + 1);
}

void QInteractorStyleVesselSegmentation::SetGenerateValue(int value)
{
	InteractorStyleVesselSegmentation::SetGenerateValue(value);
}

void QInteractorStyleVesselSegmentation::GenerateLumenWallContourWidget()
{
	InteractorStyleVesselSegmentation::GenerateLumenWallContourWidget();
}

QInteractorStyleVesselSegmentation::QInteractorStyleVesselSegmentation(int uiType, QWidget * parent)
{
	QNEW_UI();
	if (numOfMyself == 1) {
		ui->vesselFrame->setHidden(true);
	}
	connect(ui->polygonRadionButton, SIGNAL(toggled(bool)), this, SLOT(SetPolygonEnable(bool)));
	connect(ui->smoothCurveRadioButton, SIGNAL(toggled(bool)), this, SLOT(SetSmoothCurveEnable(bool)));
	connect(ui->fillContourPushButton, SIGNAL(clicked()), this, SLOT(FillPolygon()));
	connect(ui->lumenSegmentationRadioButton, SIGNAL(toggled(bool)),
		this, SLOT(EnableLumenSegmentation(bool)));
	connect(ui->vesselWallSegmentationRadioButton, SIGNAL(toggled(bool)),
		this, SLOT(EnableVesselWallSegmentation(bool)));
	connect(ui->noSegmentationRadioButton, SIGNAL(toggled(bool)),
		this, SLOT(EnableNoSegmentation(bool)));
	connect(ui->labelComboBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(SetContourLabel(int)));
	connect(ui->generatePushButton, SIGNAL(clicked()),
		this, SLOT(GenerateLumenWallContourWidget()));
	connect(ui->autoLumenSegmentationSpinBox, SIGNAL(valueChanged(int)), 
		this, SLOT(SetGenerateValue(int)));
	connect(ui->autoLumenSegmentationSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(GenerateLumenWallContourWidget()));
}

QInteractorStyleVesselSegmentation::~QInteractorStyleVesselSegmentation() 
{
	QDELETE_UI();
}