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

#ifndef INTERACTOR_STYLE_PAINTBRUSH_H
#define INTERACTOR_STYLE_PAINTBRUSH_H

#include "AbstractInteractorStyleImage.h"


//Extra
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>

#include <vtkRenderWindowInteractor.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkBorderWidget.h>
#include <vtkBorderRepresentation.h>

// Our codes


class InteractorStylePaintBrush : public AbstractInteractorStyleImage
{
public:
	vtkTypeMacro(InteractorStylePaintBrush, AbstractInteractorStyleImage);
	static InteractorStylePaintBrush* New();
	void SetPaintBrushModeEnabled(bool b);
	void SetDrawColor(int r, int g, int b);
	void SetDrawColor(const int* rgb);
	void SetDrawOpacity(int opacity);
	
	void SetDrawBrushSizeSpinBox(QSpinBox* s);
	void SetDrawBrushShapeComBox(QComboBox* comboBox);
	void SetDrawVolumetricCheckBox(QCheckBox* checkBox);
	void SetDrawIsotropicCheckBox(QCheckBox* checkBox);

protected:
	InteractorStylePaintBrush();
	~InteractorStylePaintBrush();

	virtual void OnLeave();
	virtual void OnMouseMove();
	virtual void OnLeftButtonUp();
	virtual void OnLeftButtonDown();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();

	virtual void Render();

private:
	void DrawCircle(int x0, int y0, int x1, int y1, double radius);
	void DrawLine3D(int x0, int y0, int x1, int y1);
	void FillBox3D();
	void Draw(bool b);
	void UpdateBorderWidgetPosition();
	void Write2ImageData();
	void ReadfromImageData();

	bool CheckValidPick(double*);

	//ImageType::Pointer m_overlay;
	vtkImageData* m_imageData;
	vtkImageActor* m_imageActor;

	vtkBorderWidget*				m_borderWidget;
	vtkBorderRepresentation*		m_retangleRep;
	vtkImageCanvasSource2D* m_brush;
	vtkImageActor* m_brushActor;

	int draw_index_old[3];
	int color_r, color_g, color_b;
	int opacity;
	bool m_isDraw;
	bool PAINT_BRUSH_MODE_ENABLED;

	//PaintBrush
	QComboBox*  m_drawBrushShapeComboBox;
	QSpinBox*	m_drawBrushSizeSpinBox;
	QCheckBox*	m_drawVolumetricCheckBox;
	QCheckBox*	m_drawIsotropicCheckBox;
};


#endif // INTERACTOR_STYLE_PAINTBRUSH_H