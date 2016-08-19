#ifndef DISTANCE_3DWIDGET_H
#define DISTANCE_3DWIDGET_H

#include <vtkSmartPointer.h>
#include <vtkDistanceWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkDistanceRepresentation3D.h>
#include <vtkObjectFactory.h>
#include <vtkPointHandleRepresentation3D.h>

class MyDistanceWidget;
class MyVtkDistanceRepresentation3D;
class MyPointHandleRepresentation;

class Distance3DWidget : public vtkDistanceWidget
{
public:
	static Distance3DWidget* New();
	vtkTypeMacro(Distance3DWidget, vtkDistanceWidget);

	Distance3DWidget();
	~Distance3DWidget();

	virtual void SetRepresentation(MyVtkDistanceRepresentation3D*);
	virtual void MoveAction(vtkAbstractWidget *w);
protected:
};

//////////////////////////////////////////////////////////////////////////////////////////
class MyVtkDistanceRepresentation3D: public vtkDistanceRepresentation3D {
	
public:
	static MyVtkDistanceRepresentation3D* New();
	vtkTypeMacro(MyVtkDistanceRepresentation3D, vtkDistanceRepresentation3D);
	void SetDistanceWidget(vtkDistanceWidget* distanceWidget);

	
	virtual void SetPoint2DisplayPosition(double e[3]);
	virtual void SetPoint1DisplayPosition(double e[3]);
	virtual void WidgetInteraction(double e[2]);

protected:
	MyVtkDistanceRepresentation3D() {};
	~MyVtkDistanceRepresentation3D() {};

private:
	MyVtkDistanceRepresentation3D(const MyVtkDistanceRepresentation3D&);
	void operator=(const MyVtkDistanceRepresentation3D&);
	vtkDistanceWidget* distanceWidget;

};
//////////////////////////////////////////////////////////////////////////////////////////
class MyPointHandleRepresentation : public vtkPointHandleRepresentation3D
{
public:
	MyPointHandleRepresentation();
	~MyPointHandleRepresentation();

	virtual void SetWorldPosition(double p[3]);

private:

};



#endif