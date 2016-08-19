#ifndef MYVTKINTERACTORYSTYLEIMAGE_H
#define MYVTKINTERACTORYSTYLEIMAGE_H

#include "Define.h"
#include "MyImageViewer.h"
#include "MainWindow.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include <QTime>
#include <QCoreApplication>


#include <vector>
#include <vtkSmartPointer.h>
#include "vtkBorderWidget.h"
#include "vtkBorderRepresentation.h"
#include <vtkProperty.h>
#include "vtkProperty2D.h"
#include <vtkContourWidget.h>
#include <vtkContourRepresentation.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkTransformPolyDataFilter.h>
#include "vtkInteractorStyleImage.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAbstractPicker.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkOrientedGlyphContourRepresentation.h>
#include <vtkCommand.h>
#include <vtkDebugLeaks.h>
#include <vtkCamera.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkMath.h>
#include <vtkTransform.h>
#include <vtkWidgetEvent.h>
#include <vtkWidgetEventTranslator.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkObjectFactory.h>

#include "MyImageViewer.h"
class MyImageViewer;
class MainWindow;
class ModuleWidget;

// Define own interaction style
class MyVtkInteractorStyleImage : public vtkInteractorStyleImage
{
public:
	static MyVtkInteractorStyleImage* New();
	vtkTypeMacro(MyVtkInteractorStyleImage, vtkInteractorStyleImage);

protected:
	MyImageViewer* imageViewer;
    int no_of_points;
    int max_point;
	int m_slice;
	int m_minSlice;
	int m_maxSlice;
	int orientation;
	enum Mode{NavaigationMode = 1, WindowLevelMode = 2, PaintBrushMode = 3, PolygonMode = 4} m_mode;
	QSpinBox* m_sliceSplinBox[3];
	QDoubleSpinBox* m_wlDoubleSpinBox[2];
	double* m_wlDouble[2];
	bool leftClicked;
    bool rightClicked;
    bool middleClicked;


	//PaintBrush
	QComboBox*	m_drawValueComboBox;
	QComboBox*  m_drawBrushShapeComboBox;
	QSpinBox*	m_drawBrushSizeSpinBox;
	QCheckBox*	m_drawVolumetricCheckBox;
	QCheckBox*	m_drawIsotropicCheckBox;
	vtkBorderWidget* m_borderWidget;
	vtkBorderRepresentation*		m_retangleRep;
	vtkImageCanvasSource2D* m_brush;
	vtkImageActor* BrushActor;
	double origin[3];
	double spacing[3];
	int extent[6];
	int draw_index_old[3];
	int color_r, color_g, color_b;
	int opacity;
private:
	bool isDraw;

public:
	void SetImageViewer(MyImageViewer* imageViewer);
	void SetSliceSpinBox(QSpinBox* x, QSpinBox* y, QSpinBox* z);
	void SetWindowLevelSpinBox(QDoubleSpinBox* w,QDoubleSpinBox* l);
	// Paint Brush
	void SetDrawBrushSizeSpinBox(QSpinBox* s);
	void SetDrawBrushShapeComBox(QComboBox* comboBox);
	void SetDrawVolumetricCheckBox(QCheckBox* checkBox);
	void SetDrawIsotropicCheckBox(QCheckBox* checkBox);
	void SetPaintBrushModeEnabled(bool b);
	void SetDrawColor(int r, int g, int b);
	void SetDrawColor(const int* rgb);
	void SetDrawOpacity(int opacity);
	// Contour Widget
	void SetPolygonModeEnabled(bool b);
	void SetMode(int m);
	void SetSliceRange(int min, int max);
	void SetCurrentSlice(int slice);
    void RemoveContourWidget();
    vtkPoints* GetPointsInsideContour();
    vtkOrientedGlyphContourRepresentation* GetContourRep();
    bool FillPolygon();
	void SetContourVisibility(bool b);
	void RemoveContourWidgetAll();
    
protected:
	virtual void OnMouseWheelForward();
	virtual void OnMouseWheelBackward();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();	
	virtual void OnMouseMove();
	virtual void OnChar();
	virtual void OnKeyPress();
    virtual void OnRightButtonUp();
    virtual void OnRightButtonDown();
    virtual void OnMiddleButtonUp();
    virtual void OnMiddleButtonDown();
    
	void MoveSliceForward();
	void MoveSliceBackward();
    void Synchronize();
	void CalculateIndex();
	void WindowLevel();

	void DrawCircle(int x0, int y0, int x1, int y1, int radius);
	void DrawLine3D(int x0, int y0, int x1, int y1);
	void FillBox3D();
	void Draw(bool b);
	void UpdateBorderWidgetPosition();
	void Write2ImageData();
	void ReadfromImageData();
	void Render();

private:
    // Drawing points and lines tool
    vtkOrientedGlyphContourRepresentation* contourRep;
    vtkContourWidget* contourWidget;
    vtkPoints* pointsInsideContour;
    
};

#endif // MYVTKINTERACTORYSTYLEIMAGE_H
