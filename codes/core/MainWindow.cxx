#include "MainWindow.h"
#include "ui_MainWindow.h"



MainWindow::MainWindow() 
{
	this->ui = new Ui_MainWindow;
	this->ui->setupUi(this);
	
	this->setWindowTitle("PlaqueQuant");
	
	// Set up action signals and slots
	connect(ui->actionOpenImage,				SIGNAL(triggered()), this, SLOT(slotOpenImage()));
	connect(ui->actionExit,						SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(ui->actionAbout,					SIGNAL(triggered()), this, SLOT(slotAbout()));
	connect(ui->actionHelp,						SIGNAL(triggered()), this, SLOT(slotHelp()));
	connect(ui->actionNavigation,				SIGNAL(triggered()), this, SLOT(slotNavigationMode()));
	connect(ui->actionWindowLevel,				SIGNAL(triggered()), this, SLOT(slotWindowLevelMode()));
	connect(ui->actionContour,					SIGNAL(triggered()), this, SLOT(slotContourMode()));
	connect(ui->actionBrush,					SIGNAL(triggered()), this, SLOT(slotBrushMode()));

	//Viewer
    connect(ui->actionImage1,					SIGNAL(triggered()), this, SLOT(slotimage1()));
	connect(ui->actionImage2,					SIGNAL(triggered()), this, SLOT(slotimage2()));
	connect(ui->actionImage3,					SIGNAL(triggered()), this, SLOT(slotimage3()));
	connect(ui->actionImage4,					SIGNAL(triggered()), this, SLOT(slotimage4()));
	connect(ui->actionFourViews,				SIGNAL(triggered()), this, SLOT(slotFourViews()));
	connect(ui->ULBtn,							SIGNAL(clicked()), this, SLOT(slotimage1()));
	connect(ui->URBtn,							SIGNAL(clicked()), this, SLOT(slotimage2()));
	connect(ui->LLBtn,							SIGNAL(clicked()), this, SLOT(slotimage3()));
	connect(ui->LRBtn,							SIGNAL(clicked()), this, SLOT(slotimage4()));
	connect(ui->ULBtn2,							SIGNAL(clicked()), this, SLOT(slotFourViews()));
	connect(ui->URBtn2,							SIGNAL(clicked()), this, SLOT(slotFourViews()));
	connect(ui->LLBtn2,							SIGNAL(clicked()), this, SLOT(slotFourViews()));
	connect(ui->LRBtn2,							SIGNAL(clicked()), this, SLOT(slotFourViews()));

	connect(ui->ULSelectImgBtn,					SIGNAL(clicked()), this, SLOT(slotChangeBaseImageUL()));
	connect(ui->URSelectImgBtn,					SIGNAL(clicked()), this, SLOT(slotChangeBaseImageUR()));
	connect(ui->LLSelectImgBtn,					SIGNAL(clicked()), this, SLOT(slotChangeBaseImageLL()));

	connect(ui->actionMultiPlanarView, SIGNAL(triggered()), this, SLOT(slotMultiPlanarView()));
	connect(ui->actionAllAxialView, SIGNAL(triggered()), this, SLOT(slotSegmentationView()));
	connect(ui->actionCenterlineAlgorithm, SIGNAL(triggered()), this, SLOT(slotCenterline()));

	//Tools
	connect(ui->actionRuler, SIGNAL(triggered(bool)), this, SLOT(slotRuler(bool)));
	connect(ui->actionROI, SIGNAL(triggered(bool)), this, SLOT(slotSetROIWidgetEnabled(bool)));
	connect(ui->actionOpenSegmentation, SIGNAL(triggered()), this, SLOT(slotAddExternalOverlay()));

	//3D view
	connect(ui->updateBtn, SIGNAL(clicked()), this, SLOT(slot3DUpdate()));

	//Initial Segmentation window
	segmentation = false;

	//UI Setting
	ui->actionSagittal->setChecked(true);
	ui->ULBtn2->setHidden(true);
	ui->URBtn2->setHidden(true);
	ui->LLBtn2->setHidden(true);
	ui->LRBtn2->setHidden(true);
	ui->viewGridLayout->removeWidget(ui->windowlevelwidget);
	ui->windowlevelwidget->setHidden(true);

	//Recent Image
	m_maxRecentImage = 10;
	createRecentImageActions();

	//Overlay
	m_layer_no = 1;	

	// Initial Orientation
	m_orientation= SLICE_ORIENTATION_YZ;

	//Initial Visible image number
	m_visible_image_no = 0;

	//renderer
	for (int i=0; i<4; i++)
	{
		m_renderer[i]		= vtkRenderer::New();
		m_renderer[i]->GetActiveCamera()->SetParallelProjection(true);
	}
    
    ui->image1View->GetRenderWindow()->AddRenderer(m_renderer[0]);
	ui->image2View->GetRenderWindow()->AddRenderer(m_renderer[1]);
	ui->image3View->GetRenderWindow()->AddRenderer(m_renderer[2]);
	ui->image4View->GetRenderWindow()->AddRenderer(m_renderer[3]);
	
	//Initialize
	initializeModule();
	setActionsEnable(false);
    
    
	//Parameter
	for (int i=0;i<3;i++)
	{
		m_2DimageViewer[i]	= NULL;
		m_interactor[i]		= NULL;
		m_style[i]			= NULL;
		m_planeWidget[i] = NULL;
		m_planeWidgetCallback[i] = NULL;
		m_currentBound[i] = NULL;
		m_currentBound[i+3] = NULL;
		m_focalPoint[i] = NULL;
	}
	DistanceWidget3D = NULL;
	m_3DimageViewer = NULL;
	m_3Dinteractor = NULL;
	m_settingROI = false;
	ImageAlignment(NULL) = NULL;
	FileNameList1 = NULL;
	FileNameList2 = NULL;
	FileNameList3 = NULL;
	FileNameList4 = NULL;
	FileNameList5 = NULL;
	m_itkT1 = NULL;
	m_itkT2 = NULL;
	m_itkT1C = NULL;
	m_itk2DDIR = NULL;
	m_itkMPRAGE = NULL;
	m_vtkT1 = NULL;
	m_vtkT2 = NULL;
	m_vtkT1C = NULL;
	m_vtk2DDIR = NULL;
	m_vtkMPRAGE = NULL;
	m_InfoDialog = NULL;

	//Segmentation
	SegmentationOverlay = NULL;
	*overlayColor = NULL;

}

MainWindow* MainWindow::GetMainWindow()
{
	foreach ( QWidget* w, QApplication::topLevelWidgets() )
	{
		if ( w->objectName() == "MainWindow" )
		{
			return (MainWindow*)w;
		}
	}
	return NULL;
}


MainWindow::~MainWindow()
{
	for (int i=0;i<4;i++)
	{
		m_renderer[i]->Delete();
	}

	for (int i=0;i<3;i++)
	{
		if (m_2DimageViewer[i])
			m_2DimageViewer[i]->Delete();

		if(m_interactor[i])
			m_interactor[i]->Delete();

		if (m_style[i])
			m_style[i]->Delete();
	}

	if (m_3DimageViewer)
		m_3DimageViewer->Delete();

	if (m_3Dinteractor)
		m_3Dinteractor->Delete();

	delete m_moduleWidget;

}

void MainWindow::initializeModule()
{
	m_moduleWidget = new ModuleWidget();
	ui->dockWidget->setWidget(m_moduleWidget);
}


void MainWindow::initializeViewers()
{
    
	for (int i = 0; i < 3; i++)
	{
		//m_interactor[i]		= vtkRenderWindowInteractor::New();
		//m_style[i]			= MyVtkInteractorStyleImage::New();
		//m_2DimageViewer[i] = MyImageViewer::New();
		m_2DimageViewer[i] = controller.getViewers(i);

		//m_2DimageViewer[i]->SetInputData(m_vtkT1);
		//double* range = m_vtkT1->GetScalarRange();
		//m_2DimageViewer[i]->SetColorWindow(range[1] - range[0]);
		//m_2DimageViewer[i]->SetColorLevel((range[1] + range[0]) * 0.5);
		//double defaultWindowLevel[2] = { range[1] - range[0] ,(range[1] + range[0]) * 0.5 };
		//m_2DimageViewer[i]->SetDefaultWindowLevel(defaultWindowLevel);


	//	
	}
	
	this->ui->image1View->SetRenderWindow(m_2DimageViewer[0]->GetRenderWindow());
	//this->ui->image1View->GetRenderWindow()->SetInteractor(m_interactor[0]);
	//this->ui->image1View->GetRenderWindow()->GetInteractor()->Initialize();

	this->ui->image2View->SetRenderWindow(m_2DimageViewer[1]->GetRenderWindow());
	//this->ui->image2View->GetRenderWindow()->SetInteractor(m_interactor[1]);
	//this->ui->image2View->GetRenderWindow()->GetInteractor()->Initialize();

	this->ui->image3View->SetRenderWindow(m_2DimageViewer[2]->GetRenderWindow());
	//this->ui->image3View->GetRenderWindow()->SetInteractor(m_interactor[2]);
	//this->ui->image3View->GetRenderWindow()->GetInteractor()->Initialize();
	controller.initialize(m_vtkT1);


	for (int i = 0 ; i < 3 ; i++)
	{
		//m_2DimageViewer[i]->SetSliceOrientation(i);
		//m_2DimageViewer[i]->SetSlice(m_2DimageViewer[i]->GetSliceMax() / 2);
		//m_2DimageViewer[i]->SetupInteractor(m_interactor[i]);
		//m_interactor[i]->SetInteractorStyle(m_style[i]);
		////
		//////Update style
		//m_style[i]->SetImageViewer(m_2DimageViewer[i]);
		//m_style[i]->SetMode(MyVtkInteractorStyleImage::NavaigationMode);
		//m_style[i]->SetAutoAdjustCameraClippingRange(true);
		//m_style[i]->SetSliceSpinBox(ui->xSpinBox, ui->ySpinBox, ui->zSpinBox);
		//m_style[i]->SetWindowLevelSpinBox(ui->windowDoubleSpinBoxUL, ui->levelDoubleSpinBoxUL);
		//m_style[i]->SetDrawBrushSizeSpinBox(m_moduleWidget->GetBrushSizeSpinBox());
		//m_style[i]->SetDrawBrushShapeComBox(m_moduleWidget->GetBrushShapeComBox());

		m_2DimageViewer[i]->InitializeHeader(this->GetFileName(0));
		m_2DimageViewer[i]->Render();
		m_2DimageViewer[i]->GetRenderer()->ResetCamera();
		m_2DimageViewer[i]->Render();

		cout << "max:" << m_2DimageViewer[i]->GetSliceMax() << endl;
		cout << "min:" << m_2DimageViewer[i]->GetSliceMin() << endl;
		
	}
	this->initializeOverlay();
}

void MainWindow::initializeOverlay()
{
	for (int i = 0; i < 3; i++)
	{
		overlayColor[i] = new int[3];
	}
	//Red
	overlayColor[0][0] = 255;
	overlayColor[0][1] = 0;
	overlayColor[0][2] = 0;
	//Blue
	overlayColor[1][0] = 0;
	overlayColor[1][1] = 0;
	overlayColor[1][2] = 255;
	//Green
	overlayColor[2][0] = 0;
	overlayColor[2][1] = 255;
	overlayColor[2][2] = 0;
	
	//Overlay
	SegmentationOverlay = new Overlay;
	SegmentationOverlay->Initialize(m_itkT1, m_vtkT1->GetDimensions(), m_vtkT1->GetSpacing(), m_vtkT1->GetOrigin(), VTK_DOUBLE);

	//Add Overlay
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			m_2DimageViewer[i]->SetInputDataLayer(SegmentationOverlay->GetOutput());

		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			m_2DimageViewer[i]->SetInputDataLayer(SegmentationOverlay->GetOutput());
		}
	}

	this->slotOverlayVisibilty(true);

}
void MainWindow::setActionsEnable( bool b )
{	
	//switch after open the image
	ui->actionOpenImage->setEnabled(!b);
	ui->menuRecentImage->setEnabled(!b);
	ui->actionSave->setEnabled(b);
	ui->actionNavigation->setEnabled(b);
	ui->actionWindowLevel->setEnabled(b);
	ui->actionContour->setEnabled(b);
	ui->actionBrush->setEnabled(b);
	ui->actionRemoveContour->setEnabled(b);
	ui->actionRuler->setEnabled(b);
	ui->actionROI->setEnabled(b);
	ui->actionInformation->setEnabled(b);
	ui->actionImage1->setEnabled(b);
	ui->actionImage2->setEnabled(b);
	ui->actionImage3->setEnabled(b);
	ui->actionImage4->setEnabled(b);
	ui->actionFourViews->setEnabled(b);
	ui->dockWidget->setEnabled(b); 
	ui->actionMultiPlanarView->setEnabled(b);
	ui->actionAllAxialView->setEnabled(b);
	ui->ULBtn->setEnabled(b);
	ui->URBtn->setEnabled(b);
	ui->LLBtn->setEnabled(b);
	ui->LRBtn->setEnabled(b);
	ui->ULSelectImgBtn->setEnabled(b);
	ui->URSelectImgBtn->setEnabled(b);
	ui->LLSelectImgBtn->setEnabled(b);
}

void MainWindow::slotExit() 
{
	qApp->exit();
}

void MainWindow::slotOpenImage(QString dir)
{
	RegistrationWizard wizard(this,dir);
	int isFinished = wizard.exec();

	if (isFinished == QWizard::Rejected )
		return;

	//Update Recent Image
    m_visible_image_no=wizard.getTotalFileNo();

    FileNameList1=wizard.getFileNames1();
    FileNameList2=wizard.getFileNames2();
    FileNameList3=wizard.getFileNames3();
    FileNameList4=wizard.getFileNames4();
	FileNameList5=wizard.getFileNames5();
    if (FileNameList1!=NULL) loadImage(1,FileNameList1);
    if (FileNameList2!=NULL) loadImage(2,FileNameList2);
    if (FileNameList3!=NULL) loadImage(3,FileNameList3);
    if (FileNameList4!=NULL) loadImage(4,FileNameList4);
	if (FileNameList5!=NULL) loadImage(5,FileNameList5);
	
	adjustForCurrentFile(wizard.getDirectory());
	visualizeImage();

}

void MainWindow::slotOpenImage()
{
	slotOpenImage("");
}


void MainWindow::slotOpenRecentImage()
{
	QCoreApplication::processEvents();

	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		RegistrationWizard wizard(this,action->data().toString());
		int isFinished = wizard.exec();
        
		if (isFinished == QWizard::Rejected )
			return;
        //Update Recent Image
        
        FileNameList1=wizard.getFileNames1();
        FileNameList2=wizard.getFileNames2();
        FileNameList3=wizard.getFileNames3();
        FileNameList4=wizard.getFileNames4();
        FileNameList5=wizard.getFileNames5();
        m_visible_image_no=wizard.getTotalFileNo();
        
        if (FileNameList1!=NULL) loadImage(1,FileNameList1);
        if (FileNameList2!=NULL) loadImage(2,FileNameList2);
        if (FileNameList3!=NULL) loadImage(3,FileNameList3);
        if (FileNameList4!=NULL) loadImage(4,FileNameList4);
        if (FileNameList5!=NULL) loadImage(5,FileNameList5);
		
		adjustForCurrentFile(wizard.getDirectory());
		visualizeImage();
	}
}


void MainWindow::createRecentImageActions()
{
	QAction* recentFileAction = 0;
	for(int i = 0; i < m_maxRecentImage; i++){
		recentFileAction = new QAction(this);
		recentFileAction->setVisible(false);
		connect(recentFileAction, SIGNAL(triggered()),this, SLOT(slotOpenRecentImage()));

		recentFileActionList.append(recentFileAction);
		ui->menuRecentImage->addAction(recentFileAction);
	}

	updateRecentActionList();
}

bool MainWindow::loadImage(int n, QStringList* list )
{
	//Load Nifti Data
	if (list->size()==1&&list->at(0).contains(".nii"))
	{
		NiftiReaderType::Pointer reader= NiftiReaderType::New();
		reader->SetFileName( list->at(0).toStdString());
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
		catch(itk::ExceptionObject &err)
		{
			std::cerr<<err<<std::endl;
			return 1;
		}	

		//Get Data
		if (n==1)
		{
			m_itkT1 = ImageType::New();
			m_itkT1->Graft(orienter->GetOutput());
			m_itkT1->Update();

			m_vtkT1 = vtkImageData::New();
			m_vtkT1->DeepCopy(connector->GetOutput());

		}
		else if (n==2)
		{
			m_itkT2 = ImageType::New();
			m_itkT2->Graft(orienter->GetOutput());
			m_itkT2->Update();

			
			connectorAfter->SetInput(ImageAlignment(m_itkT2));
			try
			{
				connectorAfter->Update();
			}
			catch(itk::ExceptionObject &err)
			{
				std::cerr<<err<<std::endl;
				return 1;
			}	
			
			m_vtkT2 = vtkImageData::New();
			m_vtkT2->DeepCopy(connectorAfter->GetOutput());
          
		}
		else if (n==3)
		{
			m_itkT1C = ImageType::New();
			m_itkT1C->Graft(orienter->GetOutput());
			m_itkT1C->Update();
			connectorAfter->SetInput(ImageAlignment(m_itkT1C));
			try
			{
				connectorAfter->Update();
			}
			catch(itk::ExceptionObject &err)
			{
				std::cerr<<err<<std::endl;
				return 1;
			}	

			m_vtkT1C = vtkImageData::New();
			m_vtkT1C->DeepCopy(connectorAfter->GetOutput());
		}
        else if (n==4)
        {
			m_itk2DDIR = ImageType::New();
			m_itk2DDIR->Graft(orienter->GetOutput());
			m_itk2DDIR->Update();

			connectorAfter->SetInput(ImageAlignment(m_itk2DDIR));
			try
			{
				connectorAfter->Update();
			}
			catch(itk::ExceptionObject &err)
			{
				std::cerr<<err<<std::endl;
				return 1;
			}	

			m_vtk2DDIR = vtkImageData::New();
			m_vtk2DDIR->DeepCopy(connectorAfter->GetOutput());
        }
		else if (n==5)
        {
			m_itkMPRAGE = ImageType::New();
			m_itkMPRAGE->Graft(orienter->GetOutput());
			m_itkMPRAGE->Update();

			connectorAfter->SetInput(ImageAlignment(m_itkMPRAGE));
			try
			{
				connectorAfter->Update();
			}
			catch(itk::ExceptionObject &err)
			{
				std::cerr<<err<<std::endl;
				return 1;
			}	

			m_vtkMPRAGE = vtkImageData::New();
			m_vtkMPRAGE->DeepCopy(connectorAfter->GetOutput());
        }

	}
	else
	{
		//Load DICOM data
		ReaderType::Pointer reader = ReaderType::New();

		typedef std::vector<std::string>    FileNamesContainer;
		FileNamesContainer fileNames;

		for (int i=0;i<list->size();i++)
		{
			fileNames.push_back(list->at(i).toStdString());
		}

		reader->SetFileNames( fileNames );
		try
		{
			reader->Update();
		}
		catch (itk::ExceptionObject & e)
		{
			std::cerr << "exception in file reader " << std::endl;
			std::cerr << e << std::endl;
			return 1;
		}

		//re-orient
		OrienterType::Pointer orienter = OrienterType::New();
		orienter->UseImageDirectionOn();
		orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
		orienter->SetInput(reader->GetOutput());
		orienter->Update();

		ConnectorType::Pointer connector = ConnectorType::New();
		connector->SetInput(orienter->GetOutput());

		try
		{
			connector->Update();
		}
		catch(itk::ExceptionObject &err)
		{
			std::cerr<<err<<std::endl;
			return 1;
		}	

		//Get Data
		if (n==1)
		{
			m_vtkT1 = vtkImageData::New();
			m_vtkT1->DeepCopy(connector->GetOutput());
		}
		else if (n==2)
		{

			m_vtkT2 = vtkImageData::New();
			m_vtkT2->DeepCopy(connector->GetOutput());
		}
		else if (n==3)
		{

			m_vtkT1C = vtkImageData::New();
			m_vtkT1C->DeepCopy(connector->GetOutput());
		}
        else if (n==4)
        {  
			m_vtk2DDIR = vtkImageData::New();
			m_vtk2DDIR->DeepCopy(connector->GetOutput());
        }
		else if (n==5)
        {  
			m_vtkMPRAGE = vtkImageData::New();
			m_vtkMPRAGE->DeepCopy(connector->GetOutput());
        }
	}

	

	return 0;
}

bool MainWindow::visualizeImage()
{	

	initializeViewers();
	//Enable Actions 
	setActionsEnable(true);
	ui->actionMultiPlanarView->setChecked(true);
	//Update UI stuff
    //Assume the four images have equal number of slices
	ui->xSpinBox->setMaximum(m_2DimageViewer[0]->GetInput()->GetDimensions()[0]);
	ui->ySpinBox->setMaximum(m_2DimageViewer[1]->GetInput()->GetDimensions()[1]);
	ui->zSpinBox->setMaximum(m_2DimageViewer[2]->GetInput()->GetDimensions()[2]);
	ui->xSpinBox->setValue(m_2DimageViewer[0]->GetSlice());
	ui->ySpinBox->setValue(m_2DimageViewer[1]->GetSlice());
	ui->zSpinBox->setValue(m_2DimageViewer[2]->GetSlice());
	ui->windowDoubleSpinBoxUL->setValue(m_2DimageViewer[0]->GetColorWindow());
	ui->levelDoubleSpinBoxUL->setValue(m_2DimageViewer[0]->GetColorLevel());
	connect(ui->windowDoubleSpinBoxUL,	SIGNAL(valueChanged(double)), this, SLOT(slotChangeWindowLevel()), Qt::QueuedConnection);
	connect(ui->levelDoubleSpinBoxUL,	SIGNAL(valueChanged(double)), this, SLOT(slotChangeWindowLevel()), Qt::QueuedConnection);

	connect(this->ui->xSpinBox,				SIGNAL(valueChanged(int))	, this, SLOT(slotChangeSlice())		 ,Qt::QueuedConnection);
	connect(this->ui->ySpinBox,				SIGNAL(valueChanged(int))	, this, SLOT(slotChangeSlice())		 ,Qt::QueuedConnection);
	connect(this->ui->zSpinBox,				SIGNAL(valueChanged(int))	, this, SLOT(slotChangeSlice())		 ,Qt::QueuedConnection);
	
	//Update Cursor
	this->slotChangeSlice();
    
	//ROI
	for (int i=0;i<3;i++)
	{		
		m_planeWidgetCallback[i] = MyPlaneWidgetCallback::New();
		m_planeWidget[i] = MyPlaneWidget::New();		
		m_planeWidget[i]->initializeCustomFunction();			
		m_planeWidget[i]->SetInputData(m_2DimageViewer[i]->GetInput());
		m_planeWidget[i]->SetImageViewer(m_2DimageViewer[i]);
		m_planeWidget[i]->SetDefaultBound(m_2DimageViewer[i]->GetBound());
		m_planeWidget[i]->SetInteractor(m_interactor[i]);		
		m_planeWidget[i]->AddObserver(vtkCommand::InteractionEvent,m_planeWidgetCallback[i]);
		m_planeWidget[i]->AddObserver(vtkCommand::EndInteractionEvent,m_planeWidgetCallback[i]);
	
	}
		m_planeWidgetCallback[0]->SetPlaneWidget(m_planeWidget[0],m_planeWidget[1],m_planeWidget[2]);
		m_planeWidgetCallback[1]->SetPlaneWidget(m_planeWidget[0],m_planeWidget[1],m_planeWidget[2]);
		m_planeWidgetCallback[2]->SetPlaneWidget(m_planeWidget[0],m_planeWidget[1],m_planeWidget[2]);
			
	return 0;
}

void MainWindow::adjustForCurrentFile(const QString &filePath)
{
	QSettings settings("DIIR","Project");
	QStringList recentFilePaths = settings.value("recentFiles").toStringList();
	
	recentFilePaths.removeAll(filePath);
	recentFilePaths.prepend(filePath);
	while (recentFilePaths.size() > m_maxRecentImage)
		recentFilePaths.removeLast();
	settings.setValue("recentFiles", recentFilePaths);

	// see note
	updateRecentActionList();
}

void MainWindow::updateRecentActionList()
{
	QSettings settings("DIIR","Project");
	QStringList recentFilePaths =
		settings.value("recentFiles").toStringList();

	int itEnd = 0;
	if(recentFilePaths.size() <= m_maxRecentImage)
		itEnd = recentFilePaths.size();
	else
		itEnd = m_maxRecentImage;
    
	for (int i = 0; i < itEnd; i++) 
	{
		recentFileActionList.at(i)->setText(recentFilePaths.at(i));
		recentFileActionList.at(i)->setData(recentFilePaths.at(i));
		recentFileActionList.at(i)->setVisible(true);
	}

	for (int i = itEnd; i < m_maxRecentImage; i++)
		recentFileActionList.at(i)->setVisible(false);
}

void MainWindow::slotAbout()
{
	QMessageBox::about(	NULL, 
						"About", 
                       QString("It is a program of displaying multi-modality images made by Andy Li (2016)")
					   );
}

void MainWindow::slotHelp()
{
	//Info Dialog
	m_InfoDialog = new Dialog(this);
	m_InfoDialog->show();
	m_InfoDialog->setModal(true);

}

void MainWindow::slotNavigationMode()
{	
	ui->actionWindowLevel->setChecked(false);
	ui->actionContour->setChecked(false);
	ui->actionNavigation->setChecked(true);
	ui->actionROI->setChecked(false);
	ui->actionBrush->setChecked(false);
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			//m_style[i]->SetMode(1);
		}
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			//m_style[i]->SetMode(1);
		}
	}
}

void MainWindow::slotWindowLevelMode()
{	
	ui->actionNavigation->setChecked(false);
	ui->actionContour->setChecked(false);
	ui->actionWindowLevel->setChecked(true);
	ui->actionROI->setChecked(false);
	ui->actionBrush->setChecked(false);
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			//m_style[i]->SetMode(2);
		}
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			//m_style[i]->SetMode(2);
		}
	}
}	

void MainWindow::slotBrushMode()
{

	//Update ui
	ui->actionNavigation->setChecked(false);
	ui->actionContour->setChecked(false);
	ui->actionWindowLevel->setChecked(false);
	ui->actionROI->setChecked(false);
	ui->actionBrush->setChecked(true);
	m_moduleWidget->SetPage(2);

	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			//m_style[i]->SetMode(3);
		}
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			//m_style[i]->SetMode(3);
		}
	}
}

void MainWindow::slotContourMode()
{
	ui->actionNavigation->setChecked(false);
	ui->actionWindowLevel->setChecked(false);
	ui->actionContour->setChecked(true);
	ui->actionROI->setChecked(false);
	ui->actionBrush->setChecked(false);
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			//m_style[i]->SetMode(4);
		}
	}
	else {
		for (int i = 0; i < 3; i++)
		{
			//m_style[i]->SetMode(4);
		}
	}
	m_moduleWidget->SetPage(1);
}	

void MainWindow::slotimage1()
{
    
    ui->viewGridLayout->removeWidget(ui->image1frame);
    ui->viewGridLayout->removeWidget(ui->image2frame);
    ui->viewGridLayout->removeWidget(ui->image3frame);
    ui->viewGridLayout->removeWidget(ui->image4frame);
	ui->viewGridLayout->removeWidget(ui->xyzwidget);
    
    ui->image1frame->setHidden(true);
    ui->image2frame->setHidden(true);
    ui->image3frame->setHidden(true);
    ui->image4frame->setHidden(true);
    ui->viewGridLayout->addWidget(ui->image1frame, 0, 0);
	ui->viewGridLayout->addWidget(ui->xyzwidget, 1, 0);
    ui->image1frame->setHidden(false);
	ui->ULBtn->setHidden(true);
	ui->ULBtn2->setHidden(false);

    if (m_2DimageViewer[0] != NULL)
	{
    m_2DimageViewer[0]->ResizeOrientationText();
    m_2DimageViewer[0]->Render();
	}
    //Update GUI
    ui->actionImage1->setChecked(true);
    ui->actionImage2->setChecked(false);
    ui->actionImage3->setChecked(false);
    ui->actionImage4->setChecked(false);
    ui->actionFourViews->setChecked(false);
}

void MainWindow::slotimage2()
{
    ui->viewGridLayout->removeWidget(ui->image1frame);
    ui->viewGridLayout->removeWidget(ui->image2frame);
    ui->viewGridLayout->removeWidget(ui->image3frame);
    ui->viewGridLayout->removeWidget(ui->image4frame);
	ui->viewGridLayout->removeWidget(ui->xyzwidget);

    ui->image1frame->setHidden(true);
    ui->image2frame->setHidden(true);
    ui->image3frame->setHidden(true);
    ui->image4frame->setHidden(true);

	ui->viewGridLayout->addWidget(ui->image2frame, 0, 0);
	ui->viewGridLayout->addWidget(ui->xyzwidget, 1, 0);
	ui->image2frame->setHidden(false);
	ui->URBtn->setHidden(true);
	ui->URBtn2->setHidden(false);

	if (m_2DimageViewer[1] != NULL)
	{
	m_2DimageViewer[1]->ResizeOrientationText();
	m_2DimageViewer[1]->Render();
	}
	//Update GUI
    ui->actionImage1->setChecked(false);
	ui->actionImage2->setChecked(true);
	ui->actionImage3->setChecked(false);
	ui->actionImage4->setChecked(false);
	ui->actionFourViews->setChecked(false);


}

void MainWindow::slotimage3()
{
    ui->viewGridLayout->removeWidget(ui->image1frame);
    ui->viewGridLayout->removeWidget(ui->image2frame);
    ui->viewGridLayout->removeWidget(ui->image3frame);
    ui->viewGridLayout->removeWidget(ui->image4frame);
	ui->viewGridLayout->removeWidget(ui->xyzwidget);

    ui->image1frame->setHidden(true);
    ui->image2frame->setHidden(true);
    ui->image3frame->setHidden(true);
    ui->image4frame->setHidden(true);

	ui->viewGridLayout->addWidget(ui->image3frame, 0, 0);
	ui->viewGridLayout->addWidget(ui->xyzwidget, 1, 0);
	ui->image3frame->setHidden(false);
	ui->LLBtn->setHidden(true);
	ui->LLBtn2->setHidden(false);

	if (m_2DimageViewer[2] != NULL)
	{
	m_2DimageViewer[2]->ResizeOrientationText();
	m_2DimageViewer[2]->Render();
	}
	//Update GUI
	ui->actionImage1->setChecked(false);
	ui->actionImage2->setChecked(false);
	ui->actionImage3->setChecked(true);
	ui->actionImage4->setChecked(false);
	ui->actionFourViews->setChecked(false);

}

void MainWindow::slotimage4()
{
    ui->viewGridLayout->removeWidget(ui->image1frame);
    ui->viewGridLayout->removeWidget(ui->image2frame);
    ui->viewGridLayout->removeWidget(ui->image3frame);
    ui->viewGridLayout->removeWidget(ui->image4frame);
	ui->viewGridLayout->removeWidget(ui->xyzwidget);

    ui->image1frame->setHidden(true);
    ui->image2frame->setHidden(true);
    ui->image3frame->setHidden(true);
	ui->image4frame->setHidden(true);

	ui->viewGridLayout->addWidget(ui->image4frame, 0, 0);
	ui->viewGridLayout->addWidget(ui->xyzwidget, 1, 0);
	ui->image4frame->setHidden(false);
	ui->LRBtn->setHidden(true);
	ui->LRBtn2->setHidden(false);

	//Update GUI
	ui->actionImage1->setChecked(false);
	ui->actionImage2->setChecked(false);
	ui->actionImage3->setChecked(false);
	ui->actionImage4->setChecked(true);
	ui->actionFourViews->setChecked(false);
	
}

void MainWindow::slotFourViews()
{
    ui->viewGridLayout->removeWidget(ui->image1frame);
    ui->viewGridLayout->removeWidget(ui->image2frame);
    ui->viewGridLayout->removeWidget(ui->image3frame);
    ui->viewGridLayout->removeWidget(ui->image4frame);
    ui->viewGridLayout->removeWidget(ui->xyzwidget);

    ui->image1frame->setHidden(true);
    ui->image2frame->setHidden(true);
    ui->image3frame->setHidden(true);
    ui->image4frame->setHidden(true);

    ui->viewGridLayout->addWidget(ui->image1frame, 0, 0);
    ui->viewGridLayout->addWidget(ui->image2frame, 0, 1);
	ui->viewGridLayout->addWidget(ui->image3frame, 1, 0);
	ui->viewGridLayout->addWidget(ui->image4frame, 1, 1);
	ui->viewGridLayout->addWidget(ui->xyzwidget, 2, 0 , 1 , 2);

	ui->image1frame->setHidden(false);
	ui->image2frame->setHidden(false);
	ui->image3frame->setHidden(false);
	ui->image4frame->setHidden(false);
	ui->ULBtn->setHidden(false);
	ui->ULBtn2->setHidden(true);
	ui->URBtn->setHidden(false);
	ui->URBtn2->setHidden(true);
	ui->LLBtn->setHidden(false);
	ui->LLBtn2->setHidden(true);
	ui->LRBtn->setHidden(false);
	ui->LRBtn2->setHidden(true);
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			if (m_2DimageViewer[i] != NULL)
			{
				m_2DimageViewer[i]->ResizeOrientationText();
				m_2DimageViewer[i]->Render();
			}
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_2DimageViewer[i] != NULL)
			{
				m_2DimageViewer[i]->ResizeOrientationText();
				m_2DimageViewer[i]->Render();
			}
		}
	}

	//Update GUI
	ui->actionImage1->setChecked(false);
	ui->actionImage2->setChecked(false);
	ui->actionImage3->setChecked(false);
	ui->actionImage4->setChecked(false);
	ui->actionFourViews->setChecked(true);
}

void MainWindow::slotRuler(bool b)
{

	for (int i=0;i<3;i++)
	{
		m_2DimageViewer[i]->SetRulerEnabled(b);
	}

	this->Set3DRulerEnabled(b);
}

void MainWindow::slotSetROIWidgetEnabled( bool b )
{
	ui->actionNavigation->setChecked(false);
	ui->actionWindowLevel->setChecked(false);
	ui->actionContour->setChecked(false);
	//ui->actionSegmentation->setChecked(true);
	this->slotMultiPlanarView();	//Change to multiplanar view if we want to do segmentation
	if (b)
	{
		m_settingROI = true;

		for (int i=0;i<3;i++)
		{			
			if (i==0)
				m_planeWidget[i]->SetNormalToXAxis(true);
			else if (i==1)
				m_planeWidget[i]->SetNormalToYAxis(true);
			else if (i==2)
				m_planeWidget[i]->SetNormalToZAxis(true);

			double* bound		= m_2DimageViewer[i]->GetBound();
			double displayBound[6];

			//Set Current Bound
			for (int j=0; j<3; j++)
			{			
				double roiHalfSize[3];
				roiHalfSize[j] = (bound[j*2+1] - bound[j*2])*0.25;

				m_currentBound[j*2]   = m_focalPoint[j]-roiHalfSize[j]>bound[j*2]?m_focalPoint[j]-roiHalfSize[j]:bound[j*2];
				m_currentBound[j*2+1] = m_focalPoint[j]+roiHalfSize[j]<bound[j*2+1]?m_focalPoint[j]+roiHalfSize[j]:bound[j*2+1];
				
				displayBound[j*2]   = m_focalPoint[j]-roiHalfSize[j]>bound[j*2]?m_focalPoint[j]-roiHalfSize[j]:bound[j*2];
				displayBound[j*2+1] = m_focalPoint[j]+roiHalfSize[j]<bound[j*2+1]?m_focalPoint[j]+roiHalfSize[j]:bound[j*2+1];

				//Restrict display bound on the plane
				if (i==j)
				{
					displayBound[j*2] = m_focalPoint[j];
					displayBound[j*2+1] = m_focalPoint[j];
				}
			}

			m_planeWidget[i]->SetCurrentBound(m_currentBound);
			m_planeWidget[i]->PlaceWidget(displayBound);
			
			m_planeWidget[i]->On();
			m_2DimageViewer[i]->Render();
		}
	}
	else
	{
		m_settingROI = false;

		for (int i=0;i<3;i++)
		{
			m_planeWidget[i]->Off();
			m_2DimageViewer[i]->Render();
		}
	}

}

void MainWindow::slotSelectROI()
{
	for (int i = 0; i < 3; i++)
	{
		m_2DimageViewer[i]->SetBound(m_currentBound);
		
	}
}
void MainWindow::slot3DUpdate()
{

	if (SegmentationOverlay == NULL)
		return;
	m_renderer[3]->RemoveAllViewProps();
	
	//Layer 1
	SurfaceCreator* SegmentationOverlayCreator = new SurfaceCreator();
	SegmentationOverlayCreator->SetInput(SegmentationOverlay->GetOutput());
	SegmentationOverlayCreator->SetResamplingFactor(1.0);
	SegmentationOverlayCreator->SetValue(1);
	SegmentationOverlayCreator->Update();

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(SegmentationOverlayCreator->GetOutput());
	mapper->SetScalarRange(0, 1);
	vtkSmartPointer<vtkActor> Actor = vtkSmartPointer<vtkActor>::New();
	Actor->GetProperty()->SetColor(overlayColor[0][0]/255.0, overlayColor[0][1] / 255.0, overlayColor[0][2] / 255.0);
	Actor->SetMapper(mapper);
	m_renderer[3]->AddActor(Actor);
	m_renderer[3]->ResetCameraClippingRange();
	m_renderer[3]->ResetCamera();
	this->ui->image4View->GetRenderWindow()->Render();
}

void MainWindow::slotChangeIntensity()
{    
	double* I_xyz = new double[3];
	if(segmentation == true)
	{
	
    for (int i = 0 ; i < m_visible_image_no ; i++)
    {
        switch (i)
        {
            case 0:
                I_xyz[0]= m_vtkT1->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(),0);
                break;
            case 1:
                I_xyz[1]= m_vtkT2->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(),0);
                break;
            case 2:
                I_xyz[2]= m_vtkT1C->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(),0);
                break;
            case 3:
                I_xyz[3]= m_vtk2DDIR->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(),0);
                break;
			case 4:
				I_xyz[4] = m_vtkMPRAGE->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(), 0);
				break;
        }
        
		m_2DimageViewer[i]->InitializeIntensityText(QString::number(I_xyz[i]));
		m_2DimageViewer[i]->Render();
    }
	}
	else
	{
		I_xyz[0] = m_vtkT1->GetScalarComponentAsDouble(ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(), 0);
		for (int i = 0; i < 3; i++)
		{
			m_2DimageViewer[i]->InitializeIntensityText(QString::number(I_xyz[0]));
			m_2DimageViewer[i]->Render();
		}
	}
}


void MainWindow::slotChangeSlice(int x, int y, int z)
{
	disconnect(this->ui->xSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()));
	disconnect(this->ui->ySpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()));
	disconnect(this->ui->zSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()));
	ui->xSpinBox->setValue(x);
	ui->ySpinBox->setValue(y);
	ui->zSpinBox->setValue(z);

	this->slotChangeSlice();
}

void MainWindow::slotChangeSlice()
{
	//Force update UI (QSpinBox) 
	//Cause problem if the cursor move too fast
	//QCoreApplication::processEvents();

	if(segmentation == true)
	{

    for ( int i = 0 ; i < m_visible_image_no ; i++){
        if (m_orientation == 0){
            m_2DimageViewer[i]->SetSlice(ui->xSpinBox->value());
            //m_style[i]->SetCurrentSlice(ui->xSpinBox->value());
        }
        if (m_orientation == 1){
            m_2DimageViewer[i]->SetSlice(ui->ySpinBox->value());
            //m_style[i]->SetCurrentSlice(ui->ySpinBox->value());
        }
        if (m_orientation == 2){
            m_2DimageViewer[i]->SetSlice(ui->zSpinBox->value());
            //m_style[i]->SetCurrentSlice(ui->zSpinBox->value());
        }
    }

	//Calculate the cursor focal point
    
    m_focalPoint[0] = m_2DimageViewer[0]->GetInput()->GetOrigin()[0] + ui->xSpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[0];
    m_focalPoint[1] = m_2DimageViewer[0]->GetInput()->GetOrigin()[1] + ui->ySpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[1];
    m_focalPoint[2] = m_2DimageViewer[0]->GetInput()->GetOrigin()[2] + ui->zSpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[2];
   
	
	for (int i=0; i<m_visible_image_no; i++)
	{
		//Update the Cursor Position
        m_2DimageViewer[i]->SetFocalPoint(m_focalPoint[0],m_focalPoint[1],m_focalPoint[2]);
		m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->Render();
	}
    slotChangeIntensity();
	}
	else
	{
            m_2DimageViewer[0]->SetSlice(ui->xSpinBox->value());
            //m_style[0]->SetCurrentSlice(ui->xSpinBox->value());
            m_2DimageViewer[1]->SetSlice(ui->ySpinBox->value());
            //m_style[1]->SetCurrentSlice(ui->ySpinBox->value());
            m_2DimageViewer[2]->SetSlice(ui->zSpinBox->value());
            //m_style[2]->SetCurrentSlice(ui->zSpinBox->value());
	
	for (int i=0;i<3;i++)
	{
		m_focalPoint[i] = m_2DimageViewer[i]->GetInput()->GetOrigin()[i] + m_2DimageViewer[i]->GetSlice() * m_2DimageViewer[i]->GetInput()->GetSpacing()[i];
	}

	//ROI
	//Update ROI extent
	if (m_settingROI)
	{
		for (int i=0;i<3;i++)
		{				
			double* currentBound = m_planeWidget[i]->GetCurrentBound();
			double displayBound[6]; 
			
			for (int j=0;j<3;j++)
			{	

				displayBound[j*2]   = currentBound[j*2];
				displayBound[j*2+1] = currentBound[j*2+1];
				
				//Restrict display bound on the plane
				if (i==j)
				{
					displayBound[j*2]	= m_focalPoint[j];
					displayBound[j*2+1] = m_focalPoint[j];
				}

			}

			if (m_focalPoint[i]>=currentBound[i*2]&&m_focalPoint[i]<=currentBound[i*2+1])
				m_planeWidget[i]->SetVisibility(true);
			else
				m_planeWidget[i]->SetVisibility(false);

			m_planeWidget[i]->ReplaceWidget(displayBound);
		}
	}
	slotChangeIntensity();
	for (int i=0; i<3; i++)
	{
		//Update the Cursor Position
        m_2DimageViewer[i]->SetFocalPoint(m_focalPoint[0],m_focalPoint[1],m_focalPoint[2]);
		m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->Render();
	}
	}

	//Connect back the spinbox
	connect(this->ui->xSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()), Qt::QueuedConnection);
	connect(this->ui->ySpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()), Qt::QueuedConnection);
	connect(this->ui->zSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeSlice()), Qt::QueuedConnection);
}

void MainWindow::slotChangeWindowLevel()
{
	for (int i=0; i<3; i++)
	{
		m_2DimageViewer[i]->SetColorWindow(ui->windowDoubleSpinBoxUL->value());
		m_2DimageViewer[i]->SetColorLevel(ui->levelDoubleSpinBoxUL->value());
		m_2DimageViewer[i]->Render();
	}
}


void MainWindow::slotChangeWindowLevelUL()
{
		m_2DimageViewer[0]->SetColorWindow(ui->windowDoubleSpinBoxUL->value());
		m_2DimageViewer[0]->SetColorLevel(ui->levelDoubleSpinBoxUL->value());
		m_2DimageViewer[0]->Render();
}
void MainWindow::slotChangeWindowLevelUR()
{
    m_2DimageViewer[1]->SetColorWindow(ui->windowDoubleSpinBoxUR->value());
    m_2DimageViewer[1]->SetColorLevel(ui->levelDoubleSpinBoxUR->value());
    m_2DimageViewer[1]->Render();

}
void MainWindow::slotChangeWindowLevelLL()
{
    m_2DimageViewer[2]->SetColorWindow(ui->windowDoubleSpinBoxLL->value());
    m_2DimageViewer[2]->SetColorLevel(ui->levelDoubleSpinBoxLL->value());
    m_2DimageViewer[2]->Render();
    
}
void MainWindow::slotChangeWindowLevelLR()
{
    m_2DimageViewer[3]->SetColorWindow(ui->windowDoubleSpinBoxLR->value());
    m_2DimageViewer[3]->SetColorLevel(ui->levelDoubleSpinBoxLR->value());
    m_2DimageViewer[3]->Render();
    
}

void MainWindow::resizeEvent( QResizeEvent * event )
{
	QMainWindow::resizeEvent(event);
	if (segmentation)
	{
		for (int i = 0; i < m_visible_image_no; i++)
		{
			if (m_2DimageViewer[i] != NULL)
				m_2DimageViewer[i]->ResizeOrientationText();
		}
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			if (m_2DimageViewer[i] != NULL)
				m_2DimageViewer[i]->ResizeOrientationText();
		}
	}

}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
	ev->accept();
}

void MainWindow::dropEvent( QDropEvent *ev )
{
    const QMimeData *mimeData = ev->mimeData();
	QList<QUrl> urls = mimeData->urls();
	foreach(QUrl url, urls)
	{
		QString folder = url.toLocalFile();
		folder.replace("\\","/");
		
		RegistrationWizard wizard(this,folder);
		int isFinished = wizard.exec();

		if (isFinished == QWizard::Rejected )
			return;

		if (wizard.getFileNames1()!=NULL) loadImage(1,wizard.getFileNames1());
		if (wizard.getFileNames2()!=NULL) loadImage(2,wizard.getFileNames2());
		if (wizard.getFileNames3()!=NULL) loadImage(3,wizard.getFileNames3());
        if (wizard.getFileNames4()!=NULL) loadImage(4,wizard.getFileNames4());
		if (wizard.getFileNames5()!=NULL) loadImage(5,wizard.getFileNames5());
		visualizeImage();
		adjustForCurrentFile(wizard.getDirectory());
	}
}


void MainWindow::slotShowProgressDialog( int value, QString text )
{
	//m_progressDialog->setValue(value);
	//m_progressDialog->setLabelText(text);
	//m_progressDialog->show();
}




QString MainWindow::GetFileName(int i)
{
    
    QString FileName;
    int j = 0, start, end;
    
    switch(i) {
        case 0:
        {
            QString Path1 =FileNameList1->at(0);
            if (Path1 != NULL)
            {
                start = Path1.lastIndexOf("/");
                end = Path1.length();
                for ( int k = start+1 ; k< end ; k ++){
                    FileName[j]=Path1[k];
                    j++;
                }
            }
            break;
        }
        case 1:
        {
             QString Path2 =FileNameList2->at(0);
            if (Path2 != NULL)
            {
                start = Path2.lastIndexOf("/");
                end = Path2.length();
                for ( int k = start+1 ; k< end ; k ++){
                    FileName[j]=Path2[k];
                    j++;
                }
            }
            break;
        }
        case 2:
        {
            QString Path3 =FileNameList3->at(0);
            if (Path3 != NULL)
            {
                start = Path3.lastIndexOf("/");
                end = Path3.length();
                for ( int k = start+1 ; k< end ; k ++){
                    FileName[j]=Path3[k];
                    j++;
                }
            }
            break;
        }
        case 3:
        {
            QString Path4 =FileNameList4->at(0);
            if (Path4 != NULL)
            {
                start = Path4.lastIndexOf("/");
                end = Path4.length();
                for ( int k = start+1 ; k< end ; k ++){
                    FileName[j]=Path4[k];
                    j++;
                }
            }
            break;
        }
        default:
        {
            return NULL;
            break;
        }
    }
	FileName.remove(".nii");
    return FileName;
    
}

MyImageViewer* MainWindow::GetMyImageViewer(int i){
    if(m_2DimageViewer[i])
        return m_2DimageViewer[i];
    else
        return NULL;
}
MyVtkInteractorStyleImage* MainWindow::GetMyVtkInteractorStyleImage(int i){
    //if(m_style[i])
       // return m_style[i];
    //else
        return NULL;
}

ImageType::Pointer MainWindow::ImageAlignment(ImageType::Pointer inputImage)
{

	 ImageRegistration* imageReg = new ImageRegistration(); 
	 ImageType::Pointer outputImage;
	 imageReg->SetFixedImage(m_itkT1);		//Use image 1 as the reference image
	 imageReg->SetMovingImage(inputImage);
	 imageReg->Update();

	 outputImage = imageReg->GetOutput();
	 return outputImage;
}

int MainWindow::GetVisibleImageNo()
{
	return m_visible_image_no;
}


//Segmentation Window
void MainWindow::slotChangeImageSeq(int image_no, int window_no)
{
	if (segmentation)
	{
        switch (image_no)
        {         
            case 0:
                m_2DimageViewer[window_no]->SetInputData(m_vtkT1);
                break;
            case 1:
                m_2DimageViewer[window_no]->SetInputData(m_vtkT2);
                break;
            case 2:
                m_2DimageViewer[window_no]->SetInputData(m_vtkT1C);
                break;
			case 3:
                m_2DimageViewer[window_no]->SetInputData(m_vtk2DDIR);
                break;
			case 4:
                m_2DimageViewer[window_no]->SetInputData(m_vtkMPRAGE);
                break;
                
        }
        m_2DimageViewer[window_no]->InitializeHeader(this->GetFileName(image_no));
        m_2DimageViewer[window_no]->Render();
    }
	else
	{
		for (int i = 0 ; i < 3 ; i++)
		{
			switch (image_no)
			{
				case 0:
                m_2DimageViewer[i]->SetInputData(m_vtkT1);
                break;
            case 1:
                m_2DimageViewer[i]->SetInputData(m_vtkT2);
                break;
            case 2:
                m_2DimageViewer[i]->SetInputData(m_vtkT1C);
                break;
			case 3:
                m_2DimageViewer[i]->SetInputData(m_vtk2DDIR);
                break;
			case 4:
                m_2DimageViewer[i]->SetInputData(m_vtkMPRAGE);
                break;
			}
			m_2DimageViewer[i]->InitializeHeader(this->GetFileName(image_no));
			m_2DimageViewer[i]->Render();
		}
		
	}
				
}


void MainWindow::slotSelectImageSeq(QAction* act)
{
	//0-5 is for UL, 10-15 is for UR, 20-25 is for LL
	if (act->data().toInt() < 10 && act->data().toInt() >= 0)
		slotChangeImageSeq(act->data().toInt(),0);
	else if (act->data().toInt() < 20 && act->data().toInt() >= 10)
		slotChangeImageSeq(act->data().toInt()-10,1);
	else if (act->data().toInt() < 30 && act->data().toInt() >= 20)
		slotChangeImageSeq(act->data().toInt()-20,2);

}
void MainWindow::slotChangeBaseImageUL()
{
	QMenu *menu = new QMenu(this);
	for (int i = 0 ; i < m_visible_image_no ; i++)
	{
	QAction* act = menu->addAction(this->GetFileName(i)); 
	act->setData(i);
	}
	ui->ULSelectImgBtn->setMenu(menu);
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));
	menu->show();
}

void MainWindow::slotChangeBaseImageUR()
{
	QMenu *menu = new QMenu(this);
	for (int i = 0 ; i < m_visible_image_no ; i++)
	{
	QAction* act = menu->addAction(this->GetFileName(i)); 
	act->setData(i+10);
	}
	ui->URSelectImgBtn->setMenu(menu);
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));
	menu->show();
}

void MainWindow::slotChangeBaseImageLL()
{
	QMenu *menu = new QMenu(this);
	for (int i = 0 ; i < m_visible_image_no ; i++)
	{
	QAction* act = menu->addAction(this->GetFileName(i)); 
	act->setData(i+20);
	}
	ui->LLSelectImgBtn->setMenu(menu);
	connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));
	menu->show();
}


void MainWindow::slotMultiPlanarView()
{
	ui->actionAllAxialView->setChecked(false);
	ui->actionMultiPlanarView->setChecked(true);
	segmentation = false;

	for (int i = 0; i<3; i++)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkT1);
		m_2DimageViewer[i]->SetSliceOrientation(i);
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->InitializeHeader(this->GetFileName(0));
		m_2DimageViewer[i]->Render();

		//Update style
		//m_style[i]->SetOrientation(m_2DimageViewer[i]);
		//m_style[i]->SetWindowLevelSpinBox(ui->windowDoubleSpinBoxUL, ui->levelDoubleSpinBoxUL);
		m_interactor[i]->Initialize();
	}

	this->slotChangeSlice();


}

void MainWindow::slotSegmentationView()
{
	ui->actionAllAxialView->setChecked(true);
	ui->actionMultiPlanarView->setChecked(false);
	m_orientation = SLICE_ORIENTATION_XY;
	segmentation = true;
	bool image[5] = {false};

	for (int i = 0; i < 3; i++)
	{
		m_2DimageViewer[i]->Delete();
		m_2DimageViewer[i] = NULL;
	}

	for (int i = 0; i < 3; i++)
	{
		m_2DimageViewer[i] = MyImageViewer::New();
	}

	m_2DimageViewer[0]->SetRenderWindow(this->ui->image1View->GetRenderWindow());
	m_2DimageViewer[1]->SetRenderWindow(this->ui->image2View->GetRenderWindow());
	m_2DimageViewer[2]->SetRenderWindow(this->ui->image3View->GetRenderWindow());

	this->ui->image1View->GetRenderWindow()->SetInteractor(m_interactor[0]);
	this->ui->image1View->GetRenderWindow()->GetInteractor()->Initialize();
	m_2DimageViewer[1]->SetRenderWindow(this->ui->image2View->GetRenderWindow());
	this->ui->image2View->GetRenderWindow()->SetInteractor(m_interactor[1]);
	this->ui->image2View->GetRenderWindow()->GetInteractor()->Initialize();
	m_2DimageViewer[2]->SetRenderWindow(this->ui->image3View->GetRenderWindow());
	this->ui->image3View->GetRenderWindow()->SetInteractor(m_interactor[2]);
	this->ui->image3View->GetRenderWindow()->GetInteractor()->Initialize();


	if (m_vtkT1)
		m_2DimageViewer[0]->SetInputData(m_vtkT1);
	if (m_vtkT2)
		m_2DimageViewer[1]->SetInputData(m_vtkT2);
	if (m_vtkT1C)
		m_2DimageViewer[2]->SetInputData(m_vtkT1C);
	/*


	//SetInput to three seperate windows


	if (m_vtkInputImageData1 && image[0] == false)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkInputImageData1);
		image[0] = true;
	}
	else if(m_vtkInputImageData2 && image[1] == false)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkInputImageData2);
		image[1] = true;
	}
	else if (m_vtkInputImageData3 && image[2] == false)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkInputImageData3);
		image[2] = true;
	}
	else if (m_vtkInputImageData4 && image[3] == false)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkInputImageData4);
		image[3] = true;
	}
	else if (m_vtkInputImageData5 && image[4] == false)
	{
		m_2DimageViewer[i]->SetInputData(m_vtkInputImageData5);
		image[4] = true;
	}

			*/

	for (int i = 0; i<m_visible_image_no; i++)
	{

		m_2DimageViewer[i]->SetSliceOrientation(m_orientation);
		//int max = m_2DimageViewer[i]->GetSliceMax();
		//m_2DimageViewer[i]->SetSlice(max/2);
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCameraClippingRange();
        m_2DimageViewer[i]->InitializeHeader(this->GetFileName(i));
		m_2DimageViewer[i]->Render();

		//Update style
		//m_style[i]->SetOrientation(m_2DimageViewer[i]);
		//m_style[i]->SetImageViewer(m_2DimageViewer[i]);
		switch(i){
		case 0:
			//m_style[i]->SetWindowLevelSpinBox(ui->windowDoubleSpinBoxUL,ui->levelDoubleSpinBoxUL);
			break;
		case 1:
			//m_style[i]->SetWindowLevelSpinBox(ui->windowDoubleSpinBoxUR,ui->levelDoubleSpinBoxUR);
			break;
		case 2:
			//m_style[i]->SetWindowLevelSpinBox(ui->windowDoubleSpinBoxLL,ui->levelDoubleSpinBoxLL);
			break;
        
		}
		m_interactor[i]->Initialize();
	}
	
	this->slotChangeSlice();
	
}

void MainWindow::slotAddExternalOverlay()
{
	SegmentationOverlay = new Overlay;
	QDir dir;
	QString Path = QFileDialog::getOpenFileName(this, QString(tr("DICOM")), dir.absolutePath());

	if (Path == "")
		return;
	SegmentationOverlay->SetInputImageData(Path);

}

void MainWindow::ChangeOrientation(int orientation){
    
     m_orientation=orientation;

	for (int i=0; i<m_visible_image_no; i++)
	{
		m_2DimageViewer[i]->SetSliceOrientation(m_orientation);
		/*
        if (m_orientation == 0)
            m_2DimageViewer[i]->SetSlice(ui->xSpinBox->value());
        else if (m_orientation == 1)
            m_2DimageViewer[i]->SetSlice(ui->ySpinBox->value());
        else if (m_orientation == 2)
            m_2DimageViewer[i]->SetSlice(ui->zSpinBox->value());
        */
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
		m_2DimageViewer[i]->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCameraClippingRange();
        
		m_2DimageViewer[i]->Render();

		//Update style
		//m_style[i]->SetOrientation(m_2DimageViewer[i]);
		//m_style[i]->SetAutoAdjustCameraClippingRange(true);
	}

    
}


void MainWindow::slotOverlayVisibilty(bool b, int orientation)
{
	m_2DimageViewer[orientation]->GetImageActorLayer()->SetVisibility(b);
	m_2DimageViewer[orientation]->GetRenderer()->ResetCameraClippingRange();
    m_2DimageViewer[orientation]->Render();
}

void MainWindow::slotOverlayVisibilty(bool b)
{
	for (int i = 0; i<3; i++)
	{
		m_2DimageViewer[i]->GetImageActorLayer()->SetVisibility(b);
		m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->Render();
	}
}

void MainWindow::slotOverlayOpacity(double op)
{

    for (int i=0;i<3;i++)
    {
		//if (m_style[i] != NULL)
		//	m_style[i]->SetDrawOpacity(op*255);
        m_2DimageViewer[i]->GetImageActorLayer()->SetOpacity(op);
        m_2DimageViewer[i]->Render();
    }
}


void MainWindow::RenderAllViewer()
{
	this->ui->image4View->GetRenderWindow()->Render();
	//QCoreApplication::processEvents();
	if (segmentation)
	{
	for (int i = 0 ; i < m_visible_image_no ; i++)
	{
		m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->Render();
	}
	}
	else
	{
		for (int i = 0 ; i < 3 ; i++)
	{
		m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->Render();
	}
	}
}

void MainWindow::SetImageLayerNo(int layer)
{
	m_layer_no = layer;
	for (int i = 0; i < 3; i++)
	{
		//if (m_style[i] != NULL)
		//m_style[i]->SetDrawColor(overlayColor[m_layer_no-1]);
	}
}

int MainWindow::GetImageLayerNo()
{
	return m_layer_no;
}

Overlay* MainWindow::GetOverlay()
{
	if (SegmentationOverlay)
		return SegmentationOverlay;
}


void MainWindow::Set3DRulerEnabled(bool b)
{
	if (b)
	{
		if (DistanceWidget3D)
			DistanceWidget3D->Delete();

		DistanceWidget3D = vtkDistanceWidget::New();
		DistanceWidget3D->SetInteractor(this->ui->image4View->GetRenderWindow()->GetInteractor());
		DistanceWidget3D->SetPriority(this->ui->image4View->GetRenderWindow()->GetInteractor()->GetInteractorStyle()->GetPriority() + 0.1);
		
		vtkSmartPointer< vtkPointHandleRepresentation3D > rulerHandleRep3D = vtkSmartPointer< vtkPointHandleRepresentation3D >::New();
	
		vtkSmartPointer< vtkDistanceRepresentation2D > distanceRep3D = vtkSmartPointer< vtkDistanceRepresentation2D >::New();
		distanceRep3D->SetHandleRepresentation(rulerHandleRep3D);
		DistanceWidget3D->SetRepresentation(distanceRep3D);
		distanceRep3D->InstantiateHandleRepresentation();
		distanceRep3D->SetLabelFormat("%-#11.2f mm");
		//distanceRep3D->GetAxis()->GetProperty()->SetColor(0, 1, 0);

		DistanceWidget3D->CreateDefaultRepresentation();

		DistanceWidget3D->On();
	}
	else
	{
		DistanceWidget3D->Off();

	}

	this->ui->image4View->GetRenderWindow()->Render();
}


void MainWindow::slotCenterline()
{
	// segmentation
	VesselSegmentation* vesselSegmentation = new VesselSegmentation();
	vesselSegmentation->SetT1(m_itkT1);
	vesselSegmentation->SetInputSegmentationImage(SegmentationOverlay->GetITKOutput());
	vesselSegmentation->SetMPRAGE(m_itkMPRAGE);
	vesselSegmentation->Update();

	vtkSmartPointer<vtkPolyData> lumen = vtkSmartPointer<vtkPolyData>::New();
	vesselSegmentation->GetVesselLumenSurface(lumen);

	vtkSmartPointer<vtkPolyData> plaque = vtkSmartPointer<vtkPolyData>::New();
	vesselSegmentation->GetPlaqueSurface(plaque);

	vtkSmartPointer<vtkPolyData> wall = vtkSmartPointer<vtkPolyData>::New();
	vesselSegmentation->GetVesselWallSurface(wall);

	// calculate centerlines
	ExtractCenterline* lumenExtractCenterline = new ExtractCenterline;
	lumenExtractCenterline->SetSurface(lumen);
	lumenExtractCenterline->Update();
	vtkSmartPointer<vtkPolyData> lumenCapped = vtkSmartPointer<vtkPolyData>::New();
	lumenExtractCenterline->GetCappedSurface(lumenCapped);
	vtkSmartPointer<vtkPolyData> lumenCenterline = vtkSmartPointer<vtkPolyData>::New();
	lumenExtractCenterline->GetCenterline(lumenCenterline);

	ExtractCenterline* wallExtractCenterline = new ExtractCenterline;
	wallExtractCenterline->SetSurface(wall);
	wallExtractCenterline->Update();
	vtkSmartPointer<vtkPolyData> wallCapped = vtkSmartPointer<vtkPolyData>::New();
	wallExtractCenterline->GetCappedSurface(wallCapped);
	vtkSmartPointer<vtkPolyData> wallCenterline = vtkSmartPointer<vtkPolyData>::New();
	wallExtractCenterline->GetCenterline(wallCenterline);

	vtkSmartPointer<vtkPolyDataMapper> lumenMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	lumenMapper->SetInputData(lumen);

	vtkSmartPointer<vtkPolyDataMapper> plaqueMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	plaqueMapper->SetInputData(plaque);

	vtkSmartPointer<vtkPolyDataMapper> wallMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	wallMapper->SetInputData(wall);

	// Create Lookup Table to match color
	vtkSmartPointer<vtkLookupTable> LUT = vtkSmartPointer<vtkLookupTable>::New();
	double LUTmin = min(lumenCenterline->GetScalarRange()[0], wallCenterline->GetScalarRange()[0]);
	double LUTmax = max(lumenCenterline->GetScalarRange()[1], wallCenterline->GetScalarRange()[1]);

	LUT->SetTableRange(LUTmin, LUTmax);
	LUT->Build();
	LUT->SetTableValue(0, 1.0, 0.0, 0.0, 1);  //Red
	LUT->SetTableValue(1, 0.0, 1.0, 0.0, 1); // Green
	LUT->SetTableValue(2, 0.0, 0.0, 1.0, 1); // Blue

											 // scalar bar
	vtkSmartPointer<vtkScalarBarActor> scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
	scalarBar->SetLookupTable(LUT);
	scalarBar->SetTitle("Vessel Radius");
	scalarBar->SetNumberOfLabels(4);

	vtkSmartPointer<vtkPolyDataMapper> lumenCenterlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	lumenCenterlineMapper->SetInputData(lumenCenterline);
	lumenCenterlineMapper->ScalarVisibilityOn();
	lumenCenterlineMapper->SetScalarModeToUsePointData();
	lumenCenterlineMapper->SetColorModeToMapScalars();
	lumenCenterlineMapper->SetLookupTable(LUT);
	lumenCenterlineMapper->SetUseLookupTableScalarRange(1);

	vtkSmartPointer<vtkPolyDataMapper> wallCenterlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	wallCenterlineMapper->SetInputData(wallCenterline);
	wallCenterlineMapper->ScalarVisibilityOn();
	wallCenterlineMapper->SetScalarModeToUsePointData();
	wallCenterlineMapper->SetColorModeToMapScalars();
	wallCenterlineMapper->SetLookupTable(LUT);
	wallCenterlineMapper->SetUseLookupTableScalarRange(1);

	vtkSmartPointer<vtkActor> lumenActor = vtkSmartPointer<vtkActor>::New();
	lumenActor->SetMapper(lumenMapper);
	lumenActor->GetProperty()->SetColor(1, 0, 0);
	lumenActor->GetProperty()->SetOpacity(0.5);

	vtkSmartPointer<vtkActor> plaqueActor = vtkSmartPointer<vtkActor>::New();
	plaqueActor->SetMapper(plaqueMapper);
	plaqueActor->GetProperty()->SetColor(0, 1, 0);
	plaqueActor->GetProperty()->SetOpacity(0.2);

	vtkSmartPointer<vtkActor> wallActor = vtkSmartPointer<vtkActor>::New();
	wallActor->SetMapper(wallMapper);
	wallActor->GetProperty()->SetColor(0, 0, 1);
	wallActor->GetProperty()->SetOpacity(0.3);

	vtkSmartPointer<vtkActor> lumenCenterlineActor = vtkSmartPointer<vtkActor>::New();
	lumenCenterlineActor->SetMapper(lumenCenterlineMapper);

	vtkSmartPointer<vtkActor> wallCenterlineActor = vtkSmartPointer<vtkActor>::New();
	wallCenterlineActor->SetMapper(wallCenterlineMapper);

	// pick point spheres
	vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
	sphere->SetRadius(0.5);
	sphere->SetCenter(lumenCenterline->GetPoint(0));

	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphere->GetOutputPort());

	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->SetMapper(sphereMapper);
	sphereActor->GetProperty()->SetColor(1, 0, 0);

	vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
	ren->AddActor(lumenActor);
	//ren->AddActor(plaqueActor);
	ren->AddActor(wallActor);
	//ren->AddActor(lumenCenterlineActor);
	ren->AddActor(wallCenterlineActor);
	ren->AddActor(sphereActor);
	ren->AddActor2D(scalarBar);

	vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
	renWin->AddRenderer(ren);

	vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	//vtkSmartPointer<MouseInteractorStylePP> style = vtkSmartPointer<MouseInteractorStylePP>::New();
	//style->SetSurface(wall);
	//style->SetCenterline(wallCenterline);
	//style->SetSphere(sphere);
	//iren->SetInteractorStyle(style);
	iren->SetRenderWindow(renWin);

	renWin->Render();

}