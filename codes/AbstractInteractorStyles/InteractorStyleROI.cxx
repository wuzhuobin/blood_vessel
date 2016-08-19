#include "InteractorStyleROI.h"
#include "MainWindow.h"

vtkStandardNewMacro(InteractorStyleROI);

void InteractorStyleROI::SetPlaneWidgetEnabled(bool flag)
{
	MainWindow *mainWnd = MainWindow::GetMainWindow();

	if (flag) {
		planeWidget->initializeCustomFunction();
		planeWidget->SetInputData(imageViewer->GetInput());
		planeWidget->SetImageViewer(imageViewer);
		planeWidget->SetDefaultBound(imageViewer->GetBound());
		planeWidget->SetInteractor(mainWnd->GetVtkRenderWindowInteractor(orientation));
		//qDebug() << "planeWidget:" << planeWidget;
		//qDebug() << "planeWidgetCallback" << planeWidgetCallback;
		planeWidget->AddObserver(vtkCommand::InteractionEvent, planeWidgetCallback);
		planeWidget->AddObserver(vtkCommand::EndInteractionEvent, planeWidgetCallback);

		switch (orientation)
		{
		case 0:
			planeWidgetCallback->SetPlaneWidget(
				planeWidget,
				mainWnd->GetInteractorStyleImageSwitch(1)->GetROI()->GetPlaneWidget(),
				mainWnd->GetInteractorStyleImageSwitch(2)->GetROI()->GetPlaneWidget());
			planeWidget->NormalToXAxisOn();
			break;
		case 1:
			planeWidgetCallback->SetPlaneWidget(
				mainWnd->GetInteractorStyleImageSwitch(0)->GetROI()->GetPlaneWidget(),
				planeWidget,
				mainWnd->GetInteractorStyleImageSwitch(2)->GetROI()->GetPlaneWidget());
			planeWidget->NormalToYAxisOn();
			break;
		case 2:
			planeWidgetCallback->SetPlaneWidget(
				mainWnd->GetInteractorStyleImageSwitch(0)->GetROI()->GetPlaneWidget(),
				mainWnd->GetInteractorStyleImageSwitch(1)->GetROI()->GetPlaneWidget(),
				planeWidget);
			planeWidget->NormalToZAxisOn();
			break;
		default:
			break;
		}
		double* bound = imageViewer->GetBound();
		//double displayBound[6];
		double	m_currentBound[6];
		double* m_focalPoint = planeWidget->GetImageViewer()->GetFocalPoint();

		//Set Current Bound
		for (int j = 0; j < 3; j++)
		{
			double roiHalfSize[3];
			roiHalfSize[j] = (bound[j * 2 + 1] - bound[j * 2])*0.25;

			m_currentBound[j * 2] = m_focalPoint[j] - roiHalfSize[j] > bound[j * 2] ? m_focalPoint[j] - roiHalfSize[j] : bound[j * 2];
			m_currentBound[j * 2 + 1] = m_focalPoint[j] + roiHalfSize[j] < bound[j * 2 + 1] ? m_focalPoint[j] + roiHalfSize[j] : bound[j * 2 + 1];

			displayBound[j * 2] = m_focalPoint[j] - roiHalfSize[j] > bound[j * 2] ? m_focalPoint[j] - roiHalfSize[j] : bound[j * 2];
			displayBound[j * 2 + 1] = m_focalPoint[j] + roiHalfSize[j] < bound[j * 2 + 1] ? m_focalPoint[j] + roiHalfSize[j] : bound[j * 2 + 1];

			//Restrict display bound on the plane
			if (orientation == j)
			{
				displayBound[j * 2] = m_focalPoint[j];
				displayBound[j * 2 + 1] = m_focalPoint[j];
			}
		}
		//qDebug() << "orientation: " << orientation;
		//qDebug() << m_currentBound[0] << '\t' << m_currentBound[1] << '\t' << m_currentBound[2];
		//qDebug() << m_currentBound[3] << '\t' << m_currentBound[4] << '\t' << m_currentBound[5];
		planeWidget->SetCurrentBound(m_currentBound);
		planeWidget->ReplaceWidget(displayBound);
		planeWidget->On();
		imageViewer->Render();
	}
	else {
		if (imageViewer) {
			planeWidget->Off();
			imageViewer->Render();
		}
	}
}

void InteractorStyleROI::OnMouseMove()
{
	InteractorStyleNavigation::OnMouseMove();
	if (m_leftFunctioning) {
		UpdateAllWidget();
	}

}

void InteractorStyleROI::OnLeftButtonDown()
{
	InteractorStyleNavigation::OnLeftButtonDown();
	UpdateAllWidget();
}

void InteractorStyleROI::OnLeftButtonUp()
{
	InteractorStyleNavigation::OnLeftButtonUp();
	UpdateAllWidget();
}

void InteractorStyleROI::SelectROI(int* newExtent)
{
	const double* bound = planeWidget->GetCurrentBound();
	
	newExtent[0] = (bound[0] - origin[0]) / spacing[0];
	newExtent[1] = (bound[1] - origin[0]) / spacing[0];
	newExtent[2] = (bound[2] - origin[1]) / spacing[1];
	newExtent[3] = (bound[3] - origin[1]) / spacing[1];
	newExtent[4] = (bound[4] - origin[2]) / spacing[2];
	newExtent[5] = (bound[5] - origin[2]) / spacing[2];
	




	//for (int i = 0; i < 3; ++i) {
	//	qDebug() << i;
	//	qDebug() << "extent:" << extent[i * 2] << '\t' << extent[i * 2 + 1];
	//	qDebug() << "spacing:" << spacing[i];
	//	qDebug() << "bound:" << bound[i * 2] << '\t' << bound[i * 2 + 1];
	//	qDebug() << "origin:" << origin[i];
	//	qDebug() << "newExtent:" << newExtent[i * 2] << newExtent[2 * i + 1];
	//}
}

void InteractorStyleROI::UpdateAllWidget()
{
	double* bound = planeWidget->GetCurrentBound();
	UpdateAllWidget(bound);

}

void InteractorStyleROI::UpdateAllWidget(double * bound)
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	MyPlaneWidget* m_planeWidget[3] = {
		mainWnd->GetInteractorStyleImageSwitch(0)->GetROI()->GetPlaneWidget(),
		mainWnd->GetInteractorStyleImageSwitch(1)->GetROI()->GetPlaneWidget(),
		mainWnd->GetInteractorStyleImageSwitch(2)->GetROI()->GetPlaneWidget() };
	double* currentBound;
	//Update all PlaneWidget
	for (int i = 0; i < 3; i++)
	{
		//Current bound
		currentBound = m_planeWidget[i]->GetCurrentBound();
		double* focalPoint = m_planeWidget[i]->GetImageViewer()->GetFocalPoint();

		if (bound[0] != currentBound[0] || bound[1] != currentBound[1] ||
			bound[2] != currentBound[2] || bound[3] != currentBound[3] ||
			bound[4] != currentBound[4] || bound[5] != currentBound[5]) {
			//Display bound
			double displayBound[6];
			for (int j = 0; j < 3; j++)
			{
				displayBound[j * 2] = currentBound[j * 2];
				displayBound[j * 2 + 1] = currentBound[j * 2 + 1];

				if (i == j)
				{
					displayBound[j * 2] = focalPoint[j];
					displayBound[j * 2 + 1] = focalPoint[j];
				}
			}
			m_planeWidget[i]->ReplaceWidget(displayBound);

		}


		//Decide if it can be displayed
		if (focalPoint[i] >= currentBound[i * 2] && focalPoint[i] <= currentBound[i * 2 + 1])
			m_planeWidget[i]->SetVisibility(true);
		else
			m_planeWidget[i]->SetVisibility(false);
		m_planeWidget[i]->GetInteractor()->GetRenderWindow()->Render();
	}
}

MyPlaneWidget * InteractorStyleROI::GetPlaneWidget()
{
	return this->planeWidget;
}

InteractorStyleROI::InteractorStyleROI()
	:InteractorStyleNavigation()
{
	this->imageViewer = NULL;

	planeWidget = MyPlaneWidget::New();
	planeWidgetCallback = MyPlaneWidgetCallback::New();
}


InteractorStyleROI::~InteractorStyleROI()
{
	if (planeWidget != NULL) {
		planeWidget->Delete();
	}
	if (planeWidgetCallback != NULL) {
		planeWidgetCallback->Delete();
	}
}
