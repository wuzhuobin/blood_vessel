#pragma once
#include "InteractorStyleNavigation.h"

#include "MyPlaneWidget.h"

class InteractorStyleROI :
	public InteractorStyleNavigation
{
public:
	vtkTypeMacro(InteractorStyleROI, InteractorStyleNavigation);
	static InteractorStyleROI* New();
	void SetPlaneWidgetEnabled(bool flag);
	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void SelectROI(int* newExtent);
	void UpdateAllWidget();
	void UpdateAllWidget(double* bound);

	MyPlaneWidget* GetPlaneWidget();

protected:
	InteractorStyleROI();
	~InteractorStyleROI();


private:
	MyPlaneWidget* planeWidget;
	MyPlaneWidgetCallback* planeWidgetCallback;

	double displayBound[6];

};

