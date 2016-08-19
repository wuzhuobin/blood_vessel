#ifndef MYPLANEWIDGET_H
#define MYPLANEWIDGET_H

#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkPlaneSource.h"
#include "vtkPlaneWidget.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkImageData.h"
#include "vtkActor.h"
#include "MyImageViewer.h"

 class MyPlaneWidget : public vtkPlaneWidget
{
public:
	static MyPlaneWidget *New();
	vtkTypeMacro(MyPlaneWidget, vtkPlaneWidget);

	virtual void initializeCustomFunction();
	void ReplaceWidget(double bds[6]);
	
	void SetDefaultBound(double* bound);
	void SetImageViewer(MyImageViewer* viewer);
	void SetCurrentBound(double* bound);
	void SetVisibility(bool b);	
	
	double* GetDefaultBound();	
	double* GetCurrentBound();
	MyImageViewer* GetImageViewer();

private:
	double m_deafultBound[6];
	double m_currentBound[6];
	MyImageViewer* m_2DImageViewer;
};

class MyPlaneWidgetCallback : public vtkCommand
{
public:
	MyPlaneWidgetCallback(){}

	static MyPlaneWidgetCallback *New(){return new MyPlaneWidgetCallback;}

	virtual void Execute(vtkObject *caller, unsigned long ev, void* );
	void SetPlaneWidget(MyPlaneWidget* s, MyPlaneWidget* c, MyPlaneWidget* a);

private:
	MyPlaneWidget* m_planeWidget[3];

};


#endif //MYPLANEWIDGET_H
