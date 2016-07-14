#include "MultiplanarViewController.h"



MultiplanarViewController::MultiplanarViewController()
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		viewers[i] = (vtkSmartPointer<MyImageViewer>::New());
		interactor[i] = (vtkSmartPointer<vtkRenderWindowInteractor>::New());
		style[i] = (vtkSmartPointer<MyVtkInteractorStyleImage>::New());
		//Cursor3D.push_back(vtkSmartPointer<vtkCursor3D>::New());
		//Cursormapper.push_back(vtkSmartPointer<vtkPolyDataMapper>::New());
		//CursorActor.push_back(vtkSmartPointer<vtkActor>::New());

	}
	img = vtkSmartPointer<vtkImageData>::New();
}

MultiplanarViewController::~MultiplanarViewController()
{
}

void MultiplanarViewController::setSlices(int * slices)
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		this->slices[i] = slices[i];
		viewers[i]->SetSlice(slices[i]);
		vtkInteractorStyleImage* style1 = style[i];
		MyVtkInteractorStyleImage* style2 = dynamic_cast<MyVtkInteractorStyleImage*>(style1);
		if (style2 != NULL) {
			style2->setSlice(slices[i]);
		}
		cout << "slices:" << slices[i];
	}
	cout << endl;
}

void MultiplanarViewController::setColorWindow(double colorWindow)
{
	this->colorWindow = colorWindow;
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		viewers[i]->SetColorWindow(colorWindow);
	}
}

void MultiplanarViewController::setColorLevel(double colorLevel)
{
	this->colorLevel = colorLevel;
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		viewers[i]->SetColorLevel(colorLevel);
	}
}

void MultiplanarViewController::render()
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {

		viewers[i]->ResizeOrientationText();
		viewers[i]->Render();

	}
}

void MultiplanarViewController::setMode(MyVtkInteractorStyleImage::Mode mode)
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		vtkInteractorStyleImage* style1 = style[i];
		MyVtkInteractorStyleImage* style2 = dynamic_cast<MyVtkInteractorStyleImage*>(style1);
		if (style2 != NULL) {
			style2->SetMode(mode);
		}
	}
}

void MultiplanarViewController::setMode(int mode)
{
	if (mode < 0 || mode > 3)
		return;
	setMode((MyVtkInteractorStyleImage::Mode)mode);
}

void MultiplanarViewController::setIntensityText(int* index)
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {

		double intensity = viewers[i]->GetInput()->GetScalarComponentAsDouble
		((int)(index[0] + 0.5), (int)(index[1] + 0.5), (int)(index[2] + 0.5), 0);
		stringstream ss;
		ss << intensity;
		viewers[i]->InitializeIntensityText(ss.str());
	}
}

void MultiplanarViewController::setInteractorStyle()
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {
		vtkSmartPointer<InteractorStylePaintBrush> styleNew =
			vtkSmartPointer<InteractorStylePaintBrush>::New();
		styleNew->SetImageViewer(viewers[i]);
		interactor[i]->SetInteractorStyle(styleNew);
		styleNew->SetPaintBrushModeEnabled(true);
	}
}

MyImageViewer* MultiplanarViewController::getViewers(int i)
{
	if (i > -1 && i < NUMOFVIEWERS) {
		return viewers[i];
	}
	else
		return NULL;
}

int MultiplanarViewController::loadImage(list<string> list)
{
	//Load Nifti Data
	if (list.size() == 1 && (list.cbegin()->find(".nii") != string::npos))
	{
		NiftiReaderType::Pointer reader = NiftiReaderType::New();
		reader->SetFileName(*list.cbegin());
		reader->Update();

		//re-orient
		OrienterType::Pointer orienter = OrienterType::New();
		orienter->UseImageDirectionOn();
		orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
		orienter->SetInput(reader->GetOutput());
		orienter->Update();

		ConnectorType::Pointer connector = ConnectorType::New();
		ConnectorType::Pointer connectorAfter = ConnectorType::New();
		connector->SetInput(orienter->GetOutput());
		try
		{
			connector->Update();
		}
		catch (itk::ExceptionObject &err)
		{
			std::cerr << err << std::endl;
			return 1;
		}

		//Get Data
		img->DeepCopy(connector->GetOutput());
	}
}

void MultiplanarViewController::setImageName(string name)
{
	imgName = name;
}

string MultiplanarViewController::getImageName()
{
	return imgName;
}

void MultiplanarViewController::initialize()
{
	vtkSmartPointer<MultiplanarViewControllerCallback> callback =
		vtkSmartPointer<MultiplanarViewControllerCallback>::New();
	callback->controller = this;
	double* range = img->GetScalarRange();
	for (int i = 0; i < NUMOFVIEWERS; ++i) {

		viewers[i]->SetInputData(img);
		viewers[i]->SetColorLevel((range[1] + range[0]) * 0.5);
		viewers[i]->SetColorWindow(range[1] - range[0]);
		viewers[i]->initializeHeader(imgName);
		viewers[i]->InitializeOrientationText();
		viewers[i]->InitializeIntensityText("");

		viewers[i]->SetSliceOrientation(i);
		viewers[i]->SetSlice(viewers[i]->GetSliceMax() / 2);
		viewers[i]->SetupInteractor(interactor[i]);

		vtkInteractorStyleImage* style1 = style[i];
		MyVtkInteractorStyleImage* style2 = dynamic_cast<MyVtkInteractorStyleImage*>(style1);
		if (style2 != NULL) {
		
			style2->SetImageViewer(viewers[i]);
			style2->SetMode(MyVtkInteractorStyleImage::OtherMode); 
			style2->AddObserver(MyVtkCommand::WindowLevelEvent, callback);
			style2->AddObserver(MyVtkCommand::SliceChangeEvent, callback);
			style2->AddObserver(MyVtkCommand::ResetWindowLevelEvent, callback);
			style2->AddObserver(MyVtkCommand::NavigationEvent, callback);
		}
		interactor[i]->SetInteractorStyle(style[i]);
		//interactor[i]->SetInteractorStyle(vtkInteractorStyleImage::New());


		slices[i] = viewers[i]->GetSlice();



		viewers[i]->Render();
		viewers[i]->GetRenderer()->ResetCamera();
		viewers[i]->Render();
		interactor[i]->Initialize();

		//Cursor
		//Cursor3D[i]->AllOff();
		//Cursor3D[i]->AxesOn();
		//Cursor3D[i]->SetModelBounds(0, 0, 0, 0, 0, 0);
		//Cursor3D[i]->Update();

		//Cursormapper[i]->SetInputData(Cursor3D[i]->GetOutput());

		//CursorActor[i]->SetMapper(Cursormapper[i]);
		//CursorActor[i]->GetProperty()->SetColor(0, 0, 1);
		//CursorActor[i]->GetProperty()->SetLineStipplePattern(0xf0f0);
		//viewers[i]->GetRenderer()->AddViewProp(this->CursorActor[i]);

	}
}

void MultiplanarViewController::finalize()
{
	for (int i = 0; i < NUMOFVIEWERS; ++i) {

		viewers[i]->GetRenderWindow()->Finalize();
	}
}

void MultiplanarViewController::initialize(vtkImageData * in)
{
	vtkSmartPointer<MultiplanarViewControllerCallback> callback =
		vtkSmartPointer<MultiplanarViewControllerCallback>::New();
	callback->controller = this;
	double* range = in->GetScalarRange();
	setColorLevel((range[1] + range[0]) * 0.5);
	setColorWindow(range[1] - range[0]);
	for (int i = 0; i < NUMOFVIEWERS; ++i) {

		viewers[i]->SetInputData(in);

		//viewers[i]->SetColorWindow(range[1] - range[0]);
		//viewers[i]->SetColorLevel((range[1] + range[0]) * 0.5);

		double defaultWindowLevel[2] = { range[1] - range[0], (range[1] + range[0]) * 0.5 };

		viewers[i]->SetSliceOrientation(i);
		viewers[i]->SetSlice(viewers[i]->GetSliceMax() / 2);
		viewers[i]->SetupInteractor(interactor[i]);

		vtkInteractorStyleImage* style1 = style[i];
		MyVtkInteractorStyleImage* style2 = dynamic_cast<MyVtkInteractorStyleImage*>(style1);
		if (style2 != NULL) {

			style2->SetImageViewer(viewers[i]);
			style2->SetMode(MyVtkInteractorStyleImage::OtherMode);
			style2->AddObserver(MyVtkCommand::WindowLevelEvent, callback);
			style2->AddObserver(MyVtkCommand::SliceChangeEvent, callback);
			style2->AddObserver(MyVtkCommand::ResetWindowLevelEvent, callback);
			style2->AddObserver(MyVtkCommand::NavigationEvent, callback);
		}
		interactor[i]->SetInteractorStyle(style[i]);
		//interactor[i]->SetInteractorStyle(vtkInteractorStyleImage::New());

		slices[i] = viewers[i]->GetSlice();

		viewers[i]->Render();
		viewers[i]->GetRenderer()->ResetCamera();
		viewers[i]->Render();
		interactor[i]->Initialize();


	}
	//colorLevel = viewers[0]->GetColorLevel();
	//colorWindow = viewers[0]->GetColorWindow();

}

vtkImageData * MultiplanarViewController::getImage()
{
	return img;
}


MultiplanarViewControllerCallback* MultiplanarViewControllerCallback::New() {
	return new MultiplanarViewControllerCallback;
}

void MultiplanarViewControllerCallback::Execute(vtkObject* caller,
	unsigned long event, void* callData) {



	MyVtkInteractorStyleImage* style = static_cast<MyVtkInteractorStyleImage*>(caller);
	int orientation = style->getOrientation();
	double* range = controller->getViewers(orientation)->GetInput()->GetScalarRange();
	cout << "orientatino:" << orientation << endl;
	int index[3];

	switch (event) {
	case MyVtkCommand::SliceChangeEvent:
		cout << "slice change" << endl;
		break;
	case MyVtkCommand::ResetWindowLevelEvent:
		controller->setColorWindow(range[1] - range[0]);
		controller->setColorLevel((range[1] + range[0]) * 0.5);
		cout << "reset" << endl;
		break;
	case MyVtkCommand::NavigationEvent:
		for (int i = 0; i < MultiplanarViewController::NUMOFVIEWERS; ++i) {
			index[i] =(int)( style->getIndex()[i]+ 0.5);
		}
		controller->setIntensityText(index);
		controller->setSlices(index);
		break;
	case MyVtkCommand::WindowLevelEvent:
		controller->setColorLevel(controller->getViewers(orientation)->GetColorLevel());
		controller->setColorWindow(controller->getViewers(orientation)->GetColorWindow());
		cout << " window level " << endl;
		break;
	}
	controller->render();

}