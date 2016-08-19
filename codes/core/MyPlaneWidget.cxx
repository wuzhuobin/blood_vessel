#include "MyPlaneWidget.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(MyPlaneWidget);

void MyPlaneWidget::initializeCustomFunction()
{

	this->ConeActor->SetVisibility(false);
	this->ConeActor2->SetVisibility(false);
	this->LineActor->SetVisibility(false);
	this->LineActor2->SetVisibility(false);		

	this->SetPlaceFactor(1);
	this->SetRepresentationToWireframe();
	this->SetResolution(1);
	this->SetHandleSize(0.04);

	this->HandleProperty->SetColor(0,1,1);
	this->HandleProperty->SetAmbient(1);
	this->HandleProperty->SetDiffuse(0);
	this->HandleProperty->SetSpecular(0);

	this->SelectedHandleProperty->SetColor(1,0,0);
	this->SelectedHandleProperty->SetAmbient(1);
	this->SelectedHandleProperty->SetDiffuse(0);
	this->SelectedHandleProperty->SetSpecular(0);

	this->PlaneProperty->SetLineStipplePattern(0xf0f0);
	this->PlaneProperty->SetLineStippleRepeatFactor(1);

	this->SelectedPlaneProperty->SetLineStipplePattern(0xf0f0);
	this->SelectedPlaneProperty->SetLineStippleRepeatFactor(1);

}

void MyPlaneWidget::SetDefaultBound(double* bound)
{
	memcpy(m_deafultBound, bound, sizeof(m_deafultBound));

}

double* MyPlaneWidget::GetDefaultBound(){
	return m_deafultBound;
}

void MyPlaneWidget::SetImageViewer(MyImageViewer* viewer){
	m_2DImageViewer = viewer;
}

MyImageViewer* MyPlaneWidget::GetImageViewer(){
	return m_2DImageViewer;
}

void MyPlaneWidget::SetCurrentBound(double* bound)
{
	memcpy(m_currentBound, bound, sizeof(m_currentBound));
}

double* MyPlaneWidget::GetCurrentBound()
{
	return this->m_currentBound;
}

void MyPlaneWidget::SetVisibility(bool b)
{
	for (int i=0;i<4;i++)
	{
		this->Handle[i]->SetVisibility(b);
	}
	this->PlaneActor->SetVisibility(b);
}

void MyPlaneWidget::ReplaceWidget(double bds[6])
{
	int i;
	double bounds[6], center[3];

	for (int i=0;i<6;i++)
	{
		bounds[i] = bds[i];
	}

	this->AdjustBounds(bds, bounds, center);

	if (this->GetInput() || this->Prop3D)
	{
		if ( this->NormalToYAxis )
		{
			this->PlaneSource->SetOrigin(bounds[0],center[1],bounds[4]);
			this->PlaneSource->SetPoint1(bounds[1],center[1],bounds[4]);
			this->PlaneSource->SetPoint2(bounds[0],center[1],bounds[5]);
		}
		else if ( this->NormalToZAxis )
		{
			this->PlaneSource->SetOrigin(bounds[0],bounds[2],center[2]);
			this->PlaneSource->SetPoint1(bounds[1],bounds[2],center[2]);
			this->PlaneSource->SetPoint2(bounds[0],bounds[3],center[2]);
		}
		else //default or x-normal
		{
			this->PlaneSource->SetOrigin(center[0],bounds[2],bounds[4]);
			this->PlaneSource->SetPoint1(center[0],bounds[3],bounds[4]);
			this->PlaneSource->SetPoint2(center[0],bounds[2],bounds[5]);
		}
	}

	this->PlaneSource->Update();

	// Position the handles at the end of the planes
	this->PositionHandles();

	for (i=0; i<6; i++)
	{
		this->InitialBounds[i] = bounds[i];
	}


	if (this->GetInput() || this->Prop3D)
	{
		this->InitialLength = sqrt((bounds[1]-bounds[0])*(bounds[1]-bounds[0]) +
			(bounds[3]-bounds[2])*(bounds[3]-bounds[2]) +
			(bounds[5]-bounds[4])*(bounds[5]-bounds[4]));
	}
	else
	{
		// this means we have to make use of the PolyDataSource, so
		// we just calculate the magnitude of the longest diagonal on
		// the plane and use that as InitialLength
		double origin[3], point1[3], point2[3];
		this->PlaneSource->GetOrigin(origin);
		this->PlaneSource->GetPoint1(point1);
		this->PlaneSource->GetPoint2(point2);
		double sqr1 = 0, sqr2 = 0;
		for (i = 0; i < 3; i++)
		{
			sqr1 += (point1[i] - origin[i]) * (point1[i] - origin[i]);
			sqr2 += (point2[i] - origin[i]) * (point2[i] - origin[i]);
		}

		this->InitialLength = sqrt(sqr1 + sqr2);
	}

	// Set the radius on the sphere handles
	this->SizeHandles();
}

void MyPlaneWidgetCallback::Execute(vtkObject *caller, unsigned long ev, void* )
{
	MyPlaneWidget *planeWidget = 
		dynamic_cast<MyPlaneWidget*>(caller);
	//double* currentBound = planeWidget->GetCurrentBound();
	double currentBound[6];
	double* defaultBound = planeWidget->GetDefaultBound(); 		
	double* p1	  = planeWidget->GetPoint1();
	double* p2	  = planeWidget->GetPoint2();
	double* spacing = planeWidget->GetImageViewer()->GetInput()->GetSpacing();
	double* orgin	= planeWidget->GetImageViewer()->GetInput()->GetOrigin();		
	int* pos = planeWidget->GetInteractor()->GetEventPosition();

	//updated Boundary should retain in the image extent
	for (int i=0; i<3; i++)
	{
		//Boundary Restriction
		p1[i] = p1[i]<defaultBound[i*2]?defaultBound[i*2]:p1[i];
		p2[i] = p2[i]<defaultBound[i*2]?defaultBound[i*2]:p2[i];
		p1[i] = p1[i]>defaultBound[i*2+1]?defaultBound[i*2+1]:p1[i];
		p2[i] = p2[i]>defaultBound[i*2+1]?defaultBound[i*2+1]:p2[i];

		//Fine tune the ROI corner to the nearest point
		if (ev == EndInteractionEvent)
		{
			int index1, index2;
			index1 = ((p1[i] - orgin[i])/spacing[i])+0.5;
			index2 = ((p2[i] - orgin[i])/spacing[i])+0.5;

			p1[i] = orgin[i]+ index1*spacing[i];
			p2[i] = orgin[i]+ index2*spacing[i];
		}
	}

	if (planeWidget->GetNormalToXAxis())
	{
		currentBound[0] = planeWidget->GetCurrentBound()[0];
		currentBound[1] = planeWidget->GetCurrentBound()[1];
		currentBound[2] = p2[1];
		currentBound[3] = p1[1];
		currentBound[4] = p1[2];
		currentBound[5] = p2[2];
	}
	else if (planeWidget->GetNormalToYAxis())
	{
		currentBound[0] = p2[0];
		currentBound[1] = p1[0];
		currentBound[2] = planeWidget->GetCurrentBound()[2];
		currentBound[3] = planeWidget->GetCurrentBound()[3];
		currentBound[4] = p1[2];
		currentBound[5] = p2[2];
	}
	else if (planeWidget->GetNormalToZAxis())
	{
		currentBound[0] = p2[0];
		currentBound[1] = p1[0];
		currentBound[2] = p1[1];
		currentBound[3] = p2[1];
		currentBound[4] = planeWidget->GetCurrentBound()[4];
		currentBound[5] = planeWidget->GetCurrentBound()[5];
	}

	//Update all PlaneWidget
	for (int i=0 ; i<3; i++) 
	{
		//Current bound
		m_planeWidget[i]->SetCurrentBound(currentBound);

		//Display bound
		double displayBound[6];
		double* focalPoint = m_planeWidget[i]->GetImageViewer()->GetFocalPoint();

		for (int j=0;j<3;j++)
		{
			displayBound[j*2] = currentBound[j*2];
			displayBound[j*2+1] = currentBound[j*2+1];

			if (i==j)
			{
				displayBound[j*2] = focalPoint[j];
				displayBound[j*2+1] = focalPoint[j];
			}
		}
		m_planeWidget[i]->ReplaceWidget(displayBound);

		//Decide if it can be displayed
		if (focalPoint[i] >= currentBound[i * 2] && focalPoint[i] <= currentBound[i * 2 + 1])
			m_planeWidget[i]->SetVisibility(true);
		else
			m_planeWidget[i]->SetVisibility(false);
		m_planeWidget[i]->GetInteractor()->GetRenderWindow()->Render();
	}
}

void MyPlaneWidgetCallback::SetPlaneWidget(MyPlaneWidget* s, MyPlaneWidget* c, MyPlaneWidget* a)
{
	m_planeWidget[0] = s;
	m_planeWidget[1] = c;
	m_planeWidget[2] = a;
}