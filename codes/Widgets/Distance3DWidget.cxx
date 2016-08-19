#include <vtkPointHandleRepresentation3D.h>
#include <vtkDistanceRepresentation3D.h>
#include <vtkAbstractPicker.h>
#include <vtkRendererCollection.h>

#include "Distance3DWidget.h"
#include "MainWindow.h"

vtkStandardNewMacro(Distance3DWidget);




Distance3DWidget::Distance3DWidget()
{
	//this->m_distanceWidget = vtkSmartPointer<vtkDistanceWidget>::New();
}

Distance3DWidget::~Distance3DWidget()
{

}

void Distance3DWidget::SetRepresentation(MyVtkDistanceRepresentation3D *widrep)
{
	this->WidgetRep = widrep;
}

void Distance3DWidget::MoveAction(vtkAbstractWidget * w)
{
	cout << "move";
	vtkDistanceWidget::MoveAction(w);
}


//////////////////////////////////////////////////////////////////////////////
vtkStandardNewMacro(MyVtkDistanceRepresentation3D);

void MyVtkDistanceRepresentation3D::SetDistanceWidget(vtkDistanceWidget * distanceWidget)
{
	this->distanceWidget = distanceWidget;
}

void MyVtkDistanceRepresentation3D::SetPoint2DisplayPosition(double e[3])
{
	this->Point2Representation->SetDisplayPosition(e);
	MainWindow* mainWnd = MainWindow::GetMainWindow();

	this->distanceWidget->GetInteractor()->GetPicker()->Pick(
		e[0],
		e[1],
		0,  // always zero.
		mainWnd->GetRenderWindow(4)->GetRenderers()->GetFirstRenderer());
	double* pos = this->distanceWidget->GetInteractor()->GetPicker()->GetPickPosition();
	reinterpret_cast<MyPointHandleRepresentation*>(this->Point2Representation)->SetWorldPosition(pos);
	this->BuildRepresentation();
}

void MyVtkDistanceRepresentation3D::SetPoint1DisplayPosition(double e[3])
{
	this->Point1Representation->SetDisplayPosition(e);
	MainWindow* mainWnd = MainWindow::GetMainWindow();

	this->distanceWidget->GetInteractor()->GetPicker()->Pick(
		e[0],
		e[1],
		0,  // always zero.
		mainWnd->GetRenderWindow(4)->GetRenderers()->GetFirstRenderer());
	double* pos = this->distanceWidget->GetInteractor()->GetPicker()->GetPickPosition();
	reinterpret_cast<MyPointHandleRepresentation*>(this->Point1Representation)->SetWorldPosition(pos);
	this->BuildRepresentation();
}

void MyVtkDistanceRepresentation3D::WidgetInteraction(double e[2])
{
	vtkDistanceRepresentation3D::WidgetInteraction(e);

}

////////////////////////////////////////////////////////////////////////////////
MyPointHandleRepresentation::MyPointHandleRepresentation()
{
}

MyPointHandleRepresentation::~MyPointHandleRepresentation()
{
}

void MyPointHandleRepresentation::SetWorldPosition(double p[3])
{
	cout << "fuck u ";
	vtkPointHandleRepresentation3D::SetWorldPosition(p);
}
