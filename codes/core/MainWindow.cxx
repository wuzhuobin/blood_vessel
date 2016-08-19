#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "RegistrationWizard.h"
#include "MyProgressDialog.h"
#include "ImageRegistration.h"
#include "SurfaceCreator.h"
#include "VesselSegmentation.h"
#include "ExtractCenterline.h"
#include "ModuleWidget.h"
#include "GPUVolumeRenderingFilter.h"


#include <QUrl>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>
#include <QMimeData>
#include <QSignalMapper>

#include <vtkAxesActor.h>
#include <vtkProp.h>
#include <vtkPolyDataMapper.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkImageActor.h>
#include <vtkDistanceRepresentation3D.h>
#include <vtkDistanceRepresentation2D.h>
#include <vtkDistanceWidget.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkExtractVOI.h>
#include <vtkImageIterator.h>
#include <vtkCamera.h>
#include <vtkImageCast.h>

MainWindow::MainWindow() 
	:widgetGroup(this), viewerGroup(this), viewGroup(this)
{
	this->ui = new Ui::MainWindow;
	this->ui->setupUi(this);
	
	this->setWindowTitle("PlaqueQuant");
	
	// Set up action signals and slots
	connect(ui->actionOpenImage,				SIGNAL(triggered()), this, SLOT(slotOpenImage()));
	connect(ui->actionExit,						SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(ui->actionAbout,					SIGNAL(triggered()), this, SLOT(slotAbout()));
	connect(ui->actionHelp,						SIGNAL(triggered()), this, SLOT(slotHelp()));
	
	//widgets
	widgetGroup.addAction(ui->actionNavigation);
	widgetGroup.addAction(ui->actionWindowLevel);
	widgetGroup.addAction(ui->actionContour);
	widgetGroup.addAction(ui->actionBrush);
	widgetGroup.addAction(ui->actionRuler);
	widgetGroup.addAction(ui->actionROI);
	widgetGroup.setExclusive(true);
	connect(ui->actionNavigation, SIGNAL(triggered()), this, SLOT(slotNavigationMode()));
	connect(ui->actionWindowLevel, SIGNAL(triggered()), this, SLOT(slotWindowLevelMode()));
	connect(ui->actionContour, SIGNAL(triggered()), this, SLOT(slotContourMode()));
	connect(ui->actionBrush, SIGNAL(triggered()), this, SLOT(slotBrushMode()));
	connect(ui->actionRuler, SIGNAL(triggered(bool)), this, SLOT(slotRuler(bool)));
	connect(ui->actionROI, SIGNAL(triggered()), this, SLOT(slotROIMode()));


	//minimun, maximum
	viewerGroup.addAction(ui->actionImage1);
	viewerGroup.addAction(ui->actionImage2);
	viewerGroup.addAction(ui->actionImage3);
	viewerGroup.addAction(ui->actionImage4);
	viewerGroup.addAction(ui->actionFourViews);
	viewerGroup.setExclusive(true);
	QSignalMapper* viewerMapper = new QSignalMapper(this);
	viewerMapper->setMapping(ui->actionFourViews, 0);
	viewerMapper->setMapping(ui->actionImage1, 1);
	viewerMapper->setMapping(ui->actionImage2, 2);
	viewerMapper->setMapping(ui->actionImage3, 3);
	viewerMapper->setMapping(ui->actionImage4, 4);
	connect(ui->actionImage1, SIGNAL(triggered()), viewerMapper, SLOT(map()));
	connect(ui->actionImage2, SIGNAL(triggered()), viewerMapper, SLOT(map()));
	connect(ui->actionImage3, SIGNAL(triggered()), viewerMapper, SLOT(map()));
	connect(ui->actionImage4, SIGNAL(triggered()), viewerMapper, SLOT(map()));
	connect(ui->actionFourViews, SIGNAL(triggered()), viewerMapper, SLOT(map()));
	connect(viewerMapper, SIGNAL(mapped(int)), this, SLOT(slotImage(int)));
	connect(ui->ULBtn, SIGNAL(clicked()), ui->actionImage1, SLOT(trigger()));
	connect(ui->URBtn, SIGNAL(clicked()), ui->actionImage2, SLOT(trigger()));
	connect(ui->LLBtn, SIGNAL(clicked()), ui->actionImage3, SLOT(trigger()));
	connect(ui->LRBtn, SIGNAL(clicked()), ui->actionImage4, SLOT(trigger()));
	connect(ui->ULBtn2, SIGNAL(clicked()), ui->actionFourViews, SLOT(trigger()));
	connect(ui->URBtn2, SIGNAL(clicked()), ui->actionFourViews, SLOT(trigger()));
	connect(ui->LLBtn2, SIGNAL(clicked()), ui->actionFourViews, SLOT(trigger()));
	connect(ui->LRBtn2, SIGNAL(clicked()), ui->actionFourViews, SLOT(trigger()));

	connect(ui->ULSelectImgBtn, SIGNAL(clicked()), this, SLOT(slotChangeBaseImageUL()));
	connect(ui->URSelectImgBtn, SIGNAL(clicked()), this, SLOT(slotChangeBaseImageUR()));
	connect(ui->LLSelectImgBtn, SIGNAL(clicked()), this, SLOT(slotChangeBaseImageLL()));

	viewGroup.addAction(ui->actionMultiPlanarView);
	viewGroup.addAction(ui->actionAllAxialView);
	viewGroup.setExclusive(true);
	connect(ui->actionMultiPlanarView, SIGNAL(triggered()), this, SLOT(slotMultiPlanarView()));
	connect(ui->actionAllAxialView, SIGNAL(triggered()), this, SLOT(slotSegmentationView()));

	// CenterLineAlgorithm
	connect(ui->actionCenterlineAlgorithm, SIGNAL(triggered()), this, SLOT(slotCenterline()));

	//Tools
	connect(ui->actionOpenSegmentation, SIGNAL(triggered()), this, SLOT(slotAddExternalOverlay()));

	//3D view
	connect(ui->updateBtn, SIGNAL(clicked()), this, SLOT(slot3DUpdate()));


	//Initial Segmentation window
	segmentationView = false;

	//UI Setting
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
	visibleImageNum = 0;
    
	//Initialize
	initializeModule();
	setActionsEnable(false);
    
    
	//Parameter
	this->INITIALIZED_FLAG = false;
	for (int i = 0; i < 3; i++)
	{
		m_2DimageViewer[i] = MyImageViewer::New();
		m_interactor[i] = vtkRenderWindowInteractor::New();
		m_style[i] = InteractorStyleSwitch::New();
	}
	
	ui->image1View->SetRenderWindow(m_2DimageViewer[0]->GetRenderWindow());
	ui->image2View->SetRenderWindow(m_2DimageViewer[1]->GetRenderWindow());
	ui->image3View->SetRenderWindow(m_2DimageViewer[2]->GetRenderWindow());

	/// Set up 3D renderer
	this->m_3Dinteractor = vtkRenderWindowInteractor::New();
	this->m_3DimageViewer = vtkRenderWindow::New();
	//this->m_3DimageViewer->SetNumberOfLayers(2);
	this->m_3DimageViewer->SetInteractor(this->m_3Dinteractor);
	this->m_3Dinteractor->SetInteractorStyle(vtkInteractorStyleTrackballCamera::New());
	//this->m_3DAnnotationRenderer = vtkRenderer::New();
	//this->m_3DAnnotationRenderer->SetLayer(1);
	this->m_3DDataRenderer = vtkRenderer::New();
	this->m_3DDataRenderer->SetLayer(0);
	this->m_3DimageViewer->AddRenderer(this->m_3DDataRenderer);
	//this->m_3DimageViewer->AddRenderer(this->m_3DAnnotationRenderer);

	//vtkAxesActor* a = vtkAxesActor::New();
	//this->m_3DDataRenderer->AddActor(a);

	this->m_3Dinteractor->Initialize();

	ui->image4View->SetRenderWindow(this->m_3DimageViewer);

	ImageAlignment(NULL) = NULL;
	
	for (int i = 0; i < 5; ++i) {
		itkImage[i] = ImageType::New();
		//vtkImage[i] = vtkImageData::New();
		vtkImage[i] = NULL;
		vtkImageOriginal[i] = vtkImage[i];
	}
	vtkImageOverlay = vtkImageData::New();

	m_InfoDialog = NULL;

	//Segmentation
	SegmentationOverlay = NULL;
	
	overlayColor[0] = lumen;
	overlayColor[1] = vesselWall;
	overlayColor[2] = calcification;
	overlayColor[3] = hemorrhage;
	overlayColor[4] = lrnc;
	overlayColor[5] = lm;


	//distanceWidget3D
	m_distance3DWidget = Distance3DWidget::New();
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
	ui->image1View->SetRenderWindow(NULL);

	ui->image2View->SetRenderWindow(NULL);

	ui->image3View->SetRenderWindow(NULL);

	for (int i = 0; i < 3; i++)
	{
		if (m_2DimageViewer[i]) {
			m_2DimageViewer[i]->Delete();
			m_2DimageViewer[i] = NULL;
		}
		if (m_interactor[i]) {
			m_interactor[i]->Delete();
			m_interactor[i] = NULL;
		}
		if (m_style[i]) {
			m_style[i]->Delete();
			m_style[i] = NULL;
		}
	}
	if (m_3Dinteractor) {
		m_3Dinteractor->Delete();
		m_3Dinteractor = NULL;
	}


	for (int i = 0; i < 5; ++i) {
		if (vtkImage[i] != NULL) {
			if (vtkImageOriginal[i] != vtkImage[i]) {
				vtkImage[i]->Delete();
				vtkImage[i] = NULL;
			}
			if (vtkImageOriginal[i] != NULL) {
				vtkImageOriginal[i]->Delete();
				vtkImageOriginal[i] = NULL;
			}
			vtkImage[i] = NULL;
		}


	}
	if (m_moduleWidget != NULL) {
		delete m_moduleWidget;
		m_moduleWidget = NULL;
	}
	if (m_InfoDialog != NULL) {
		delete m_InfoDialog;
		m_InfoDialog = NULL;
	}
	if (SegmentationOverlay != NULL) {
		delete SegmentationOverlay;
	}

	if (m_distance3DWidget != NULL) {
		//delete distance3DWidget;
		//distance3DWidget->Delete();
	}
}

void MainWindow::initializeModule()
{
	m_moduleWidget = new ModuleWidget(this);
	ui->dockWidget->setWidget(m_moduleWidget);
}


void MainWindow::initializeViewers()
{
	segmentationView = false;

	for (int i = 0 ; i < 3 ; i++)
	{
	    m_2DimageViewer[i]->SetInputData(vtkImage[0]);
		m_2DimageViewer[i]->SetSliceOrientation(i);
		m_2DimageViewer[i]->SetSlice(m_2DimageViewer[i]->GetSliceMax() / 2);
        m_2DimageViewer[i]->InitializeHeader(this->GetFileName(0));
		m_2DimageViewer[i]->SetupInteractor(m_interactor[i]);

		//Update style
		m_style[i]->SetViewers(m_2DimageViewer[i]);
		m_style[i]->initializeQWidget(ui->xSpinBox, ui->ySpinBox, ui->zSpinBox, 
			ui->windowDoubleSpinBoxUL, ui->levelDoubleSpinBoxUL,
			m_moduleWidget->GetBrushSizeSpinBox(), 
			m_moduleWidget->GetBrushShapeComBox(),
			NULL, NULL);

		m_interactor[i]->SetInteractorStyle(m_style[i]);
		
	}
	this->addOverlay2ImageViewer();
}

void MainWindow::addOverlay2ImageViewer()
{
	if (SegmentationOverlay == NULL) 
	{
		//Overlay
		SegmentationOverlay = new Overlay;
		SegmentationOverlay->Initialize(vtkImage[0]);
		//SegmentationOverlay->Initialize(itkImage[0], vtkImage[0]->GetDimensions(),
			//vtkImage[0]->GetSpacing(), vtkImage[0]->GetOrigin(), VTK_DOUBLE);
		//vtkImageOverlay = SegmentationOverlay->GetOutput();

		//Add Overlay
		for (int i = 0; i < 3; i++)
		{
			if (segmentationView && i >= visibleImageNum)
				break;
			m_2DimageViewer[i]->SetInputDataLayer(SegmentationOverlay->GetOutput());
			m_2DimageViewer[i]->GetannotationRenderer()->AddViewProp(this->m_2DimageViewer[i]->GetdrawActor());
		}

	}


	this->SegmentationOverlay->GetOutput()->Modified();
	this->slotOverlayVisibilty(true);

}
void MainWindow::setActionsEnable( bool b )
{	
	//switch after open the image
	//ui->actionOpenImage->setEnabled(!b);
	//ui->menuRecentImage->setEnabled(!b);
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
    visibleImageNum=wizard.getTotalFileNo();
	QStringList* wizardFileNames[5] = {
		wizard.getFileNames1(), wizard.getFileNames2(), wizard.getFileNames3(),
		wizard.getFileNames4(), wizard.getFileNames5()	};
	for (int i = 0; i < 5; ++i) {
		if (wizardFileNames[i] != NULL) {
			FileNameList[i] = *wizardFileNames[i];
			this->loadImage(i, wizardFileNames[i]);
		}
	}
	//copy for segmentationROI to recover the original image

	adjustForCurrentFile(wizard.getDirectory());
	visualizeImage();

	// initialize image select function 
	for (int i = 0; i < visibleImageNum; i++)
	{
		QAction* act1 = ChangeBaseImageULMenu.addAction(this->GetFileName(i));
		act1->setData(i);

		QAction* act2 = ChangeBaseImageURMenu.addAction(this->GetFileName(i));
		act2->setData(i + 10);

		QAction* act3 = ChangeBaseImageLLMenu.addAction(this->GetFileName(i));
		act3->setData(i + 20);
	}
	ui->ULSelectImgBtn->setMenu(&ChangeBaseImageULMenu);
	connect(&ChangeBaseImageULMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));

	ui->URSelectImgBtn->setMenu(&ChangeBaseImageURMenu);
	connect(&ChangeBaseImageURMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));

	ui->LLSelectImgBtn->setMenu(&ChangeBaseImageLLMenu);
	connect(&ChangeBaseImageLLMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotSelectImageSeq(QAction*)));


	this->addOverlay2ImageViewer();
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
		slotOpenImage(action->data().toString());
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
		//for (int i = 0; i < n; ++i) {
			this->itkImage[n]->Graft(orienter->GetOutput());
			this->itkImage[n]->Update();
			if (n > 0) {
				connectorAfter->SetInput(ImageAlignment(itkImage[n]));
				try
				{
					connectorAfter->Update();
				}
				catch (itk::ExceptionObject &err)
				{
					std::cerr << err << std::endl;
					return 1;
				}
			}
			this->vtkImage[n] = vtkImageData::New();
			this->vtkImage[n]->DeepCopy(connector->GetOutput());
		//}

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
		//for (int i = 0; i < n; ++i) {
			this->vtkImage[n]->DeepCopy(connector->GetOutput());
		//}

	}

	

	return 0;
}

bool MainWindow::visualizeImage()
{	

	//Enable Actions 
	setActionsEnable(true);
	//connected to slotMultiPlanarView
	ui->actionMultiPlanarView->trigger();


	//for (int i = 0; i < 3; ++i) {
	//	m_2DimageViewer[i]->SetSlice(m_2DimageViewer[i]->GetSliceMax() / 2);
	//}
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
	//connected to slotNavigationMode()
	ui->actionNavigation->trigger();
				
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

	for (int i = 0; i < 3; i++)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		m_style[i]->SetInteractorStyleToNavigation();
	}
}

void MainWindow::slotWindowLevelMode()
{	

	for (int i = 0; i < 3; i++)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		m_style[i]->SetInteractorStyleToWindowLevel();
	}
}	

void MainWindow::slotBrushMode()
{
	for (int i = 0; i < 3; i++)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		m_style[i]->SetInteractorStyleToPaintBrush();
	}
	//Update ui
	m_moduleWidget->SetPage(2);
}

void MainWindow::slotContourMode()
{
	for (int i = 0; i < 3; i++)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		m_style[i]->SetInteractorStyleToPolygonDraw();
		//m_style[i]->SetMode(4);
	}
	m_moduleWidget->SetPage(1);
}

void MainWindow::slotImage(int image)
{
	ui->image1frame->setHidden(true);
	ui->image2frame->setHidden(true);
	ui->image3frame->setHidden(true);
	ui->image4frame->setHidden(true);
	switch (image)
	{
	case 0:
		ui->ULBtn->setHidden(false);
		ui->URBtn->setHidden(false);
		ui->LLBtn->setHidden(false);
		ui->LRBtn->setHidden(false);
		ui->ULBtn2->setHidden(true);
		ui->URBtn2->setHidden(true);
		ui->LLBtn2->setHidden(true);
		ui->LRBtn2->setHidden(true);
		ui->image1frame->setHidden(false);
		ui->image2frame->setHidden(false);
		ui->image3frame->setHidden(false);
		ui->image4frame->setHidden(false);
		break;
	case 1:
		ui->ULBtn->setHidden(true);
		ui->ULBtn2->setHidden(false);
		ui->image1frame->setHidden(false);
		ui->image2frame->setHidden(true);
		ui->image3frame->setHidden(true);
		ui->image4frame->setHidden(true);
		break;
	case 2:
		ui->URBtn->setHidden(true);
		ui->URBtn2->setHidden(false);
		ui->image1frame->setHidden(true);
		ui->image2frame->setHidden(false);
		ui->image3frame->setHidden(true);
		ui->image4frame->setHidden(true);
		break;
	case 3:
		ui->LLBtn->setHidden(true);
		ui->LLBtn2->setHidden(false);
		ui->image1frame->setHidden(true);
		ui->image2frame->setHidden(true);
		ui->image3frame->setHidden(false);
		ui->image4frame->setHidden(true);
		break;
	case 4:
		ui->LRBtn->setHidden(true);
		ui->LRBtn2->setHidden(false);
		ui->image1frame->setHidden(true);
		ui->image2frame->setHidden(true);
		ui->image3frame->setHidden(true);
		ui->image4frame->setHidden(false);
	default:
		break;
	}
	for (int i = 0; i < 3; i++)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		if (m_2DimageViewer[i] != NULL)
		{
			m_2DimageViewer[i]->ResizeOrientationText();
			m_2DimageViewer[i]->Render();
		}
	}
}

void MainWindow::slotRuler(bool b)
{

	for (int i=0;i<3;i++)
	{
		//m_2DimageViewer[i]->SetRulerEnabled(b);
		m_style[i]->SetInteractorStyleToInteractorStyleTesting();
	}

	this->Set3DRulerEnabled(b);
}

void MainWindow::slotROIMode()
{
	if (segmentationView) {
		//connected to slotMultiPlanarView()
		ui->actionMultiPlanarView->trigger();
	}
	m_moduleWidget->SetPage(0);
	for (int i = 0; i < 3; ++i) {
		m_style[i]->SetInteractorStyleToROI();
	}
}

void MainWindow::slotChangeROI(double * bound)
{
	//m_moduleWidget->slotChangeROI(bound);
	//for (int i = 0; i < 6; ++i)
}

void MainWindow::slotSelectROI()
{

	int newExtent[6];
	m_style[0]->GetROI()->SelectROI(newExtent);
	// Extract VOI of the overlay Image data

	// Extract VOI of the vtkImage data
	for (int i = 0; i < 5; ++i) {
		if (vtkImage[i] != NULL) {
			vtkSmartPointer<vtkExtractVOI> extractVOIFilter =
				vtkSmartPointer<vtkExtractVOI>::New();
			extractVOIFilter->SetInputData(vtkImage[i]);
			extractVOIFilter->SetVOI(newExtent);
			extractVOIFilter->Update();
			// make sure the vtkImageOriginal won't be deleted 
			// delete multiple segmentations of vtkImage data 
			if (vtkImage[i] != vtkImageOriginal[i]) {
				vtkImage[i]->Delete();
			}
			vtkImage[i] = vtkImageData::New();
			vtkImage[i]->DeepCopy(extractVOIFilter->GetOutput());
		}
	}
	ui->actionMultiPlanarView->trigger();
	ui->actionNavigation->trigger();
}
void MainWindow::slotResetROI()
{
	for (int i = 0; i < 5; ++i) {
		if (vtkImage[i] != vtkImageOriginal[i] && vtkImage[i] != NULL) {
			vtkImage[i]->Delete();
			vtkImage[i] = vtkImageOriginal[i];
		}
	}

	ui->actionMultiPlanarView->trigger();
	ui->actionNavigation->trigger();
}

#include <vtkAxesActor.h>

void MainWindow::slot3DUpdate()
{

	//if (SegmentationOverlay == NULL)
	//	return;
	this->ui->image4View->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->RemoveAllViewProps();
	
	////Marching cubes
	//SurfaceCreator* surfaceCreator = new SurfaceCreator();
	//surfaceCreator->SetInput(SegmentationOverlay->GetOutput());
	//surfaceCreator->SetDiscrete(true);
	//surfaceCreator->SetResamplingFactor(1.0);
	//surfaceCreator->Update();

	//vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//mapper->SetInputData(surfaceCreator->GetOutput());
	//mapper->SetLookupTable(this->m_2DimageViewer[0]->getLookupTable());
	//mapper->SetScalarRange(0, 6); // Change this too if you change the look up table!
	//mapper->Update();
	//vtkSmartPointer<vtkActor> Actor = vtkSmartPointer<vtkActor>::New();
	////Actor->GetProperty()->SetColor(overlayColor[0][0]/255.0, overlayColor[0][1] / 255.0, overlayColor[0][2] / 255.0);
	//Actor->SetMapper(mapper);
	//this->ui->image4View->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->AddActor(Actor);
	//delete surfaceCreator;

	///Volume Render
	GPUVolumeRenderingFilter* volumeRenderingFilter =
		GPUVolumeRenderingFilter::New();
	
	volumeRenderingFilter->SetInputData(this->SegmentationOverlay->GetOutput());
	volumeRenderingFilter->SetLookUpTable(this->GetMyImageViewer(0)->getLookupTable());
	volumeRenderingFilter->Update();

	this->m_3DDataRenderer->AddVolume(volumeRenderingFilter->GetVolume());
	this->ui->image4View->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->SetBackground(0.3,0.3,0.3);
	this->ui->image4View->GetRenderWindow()->GetRenderers()->GetFirstRenderer()->ResetCamera();
	this->ui->image4View->GetRenderWindow()->Render();

	//vtkRenderWindow* rwin = vtkRenderWindow::New();
	//rwin->AddRenderer(vtkRenderer::New());
	//rwin->GetRenderers()->GetFirstRenderer()->AddVolume(volumeRenderingFilter->GetVolume());
	//vtkRenderWindowInteractor* interacotr = vtkRenderWindowInteractor::New();
	//rwin->SetInteractor(interacotr);
	//interacotr->Initialize();
	//rwin->Render();
	//interacotr->Start();

	//volumeRenderingFilter->Delete();

}

void MainWindow::slotChangeIntensity()
{
	double intensity[3];
	for (int i = 0; i < 3; ++i) {
		if (segmentationView && i >= visibleImageNum)
			break;
		intensity[i] = m_2DimageViewer[i]->GetInput()->GetScalarComponentAsDouble(
			ui->xSpinBox->value(), ui->ySpinBox->value(), ui->zSpinBox->value(), 0
		);
		m_2DimageViewer[i]->InitializeIntensityText(QString::number(intensity[i]));
		m_2DimageViewer[i]->Render();
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

	if(segmentationView == true)
	{

		for (int i = 0; i < (3 < visibleImageNum ? 3 : visibleImageNum); i++) {
			if (m_orientation == 0) {
				m_2DimageViewer[i]->SetSlice(ui->xSpinBox->value());
				m_style[i]->SetCurrentSlice(ui->xSpinBox->value());
			}
			if (m_orientation == 1) {
				m_2DimageViewer[i]->SetSlice(ui->ySpinBox->value());
				m_style[i]->SetCurrentSlice(ui->ySpinBox->value());
			}
			if (m_orientation == 2) {
				m_2DimageViewer[i]->SetSlice(ui->zSpinBox->value());
				m_style[i]->SetCurrentSlice(ui->zSpinBox->value());
			}
		}

	//Calculate the cursor focal point
    
    m_focalPoint[0] = m_2DimageViewer[0]->GetInput()->GetOrigin()[0] + ui->xSpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[0];
    m_focalPoint[1] = m_2DimageViewer[0]->GetInput()->GetOrigin()[1] + ui->ySpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[1];
    m_focalPoint[2] = m_2DimageViewer[0]->GetInput()->GetOrigin()[2] + ui->zSpinBox->value() * m_2DimageViewer[0]->GetInput()->GetSpacing()[2];
   
	
	for (int i=0; i<(3<visibleImageNum ? 3 : visibleImageNum); i++)
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
            m_style[0]->SetCurrentSlice(ui->xSpinBox->value());
            m_2DimageViewer[1]->SetSlice(ui->ySpinBox->value());
            m_style[1]->SetCurrentSlice(ui->ySpinBox->value());
            m_2DimageViewer[2]->SetSlice(ui->zSpinBox->value());
            m_style[2]->SetCurrentSlice(ui->zSpinBox->value());
	
	for (int i=0;i<3;i++)
	{
		m_focalPoint[i] = m_2DimageViewer[i]->GetInput()->GetOrigin()[i] + m_2DimageViewer[i]->GetSlice() * m_2DimageViewer[i]->GetInput()->GetSpacing()[i];
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
		for (int i = 0; i < 3; i++)
		{
			if (m_2DimageViewer[i] != NULL)
				m_2DimageViewer[i]->ResizeOrientationText();
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




vtkRenderWindow * MainWindow::GetRenderWindow(int i)
{
	switch (i)
	{
	case 1:
		return ui->image1View->GetRenderWindow();
	case 2:
		return ui->image2View->GetRenderWindow();
	case 3:
		return ui->image3View->GetRenderWindow();
	case 4:
		return ui->image4View->GetRenderWindow();
	default:
		break;
	}
	return nullptr;
}

QString MainWindow::GetFileName(int i)
{
    
    QString FileName;
	QString path;
    int j = 0, start, end;
	path = FileNameList[i].at(0);
	if (!path.isEmpty())
	{
		start = path.lastIndexOf("/");
		end = path.length();
		for (int k = start + 1; k < end; k++) {
			FileName[j] = path[k];
			j++;
		}
		FileName.remove(".nii");
	}
    return FileName;
    
}

ImageType::Pointer MainWindow::ImageAlignment(ImageType::Pointer inputImage)
{

	 ImageRegistration* imageReg = new ImageRegistration(); 
	 ImageType::Pointer outputImage;
	 imageReg->SetFixedImage(itkImage[0]);		//Use image 1 as the reference image
	 imageReg->SetMovingImage(inputImage);
	 imageReg->Update();

	 outputImage = imageReg->GetOutput();
	 return outputImage;
}

int MainWindow::GetVisibleImageNo()
{
	return visibleImageNum;
}


//Segmentation Window
void MainWindow::slotChangeImageSeq(int image_no, int window_no)
{
	if (segmentationView)
	{
		m_2DimageViewer[window_no]->SetInputData(vtkImage[image_no]);
        m_2DimageViewer[window_no]->InitializeHeader(this->GetFileName(image_no));
        m_2DimageViewer[window_no]->Render();
    }
	else
	{
		for (int i = 0 ; i < 3 ; i++)
		{
			m_2DimageViewer[i]->SetInputData(vtkImage[image_no]);
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
	ChangeBaseImageULMenu.show();
}

void MainWindow::slotChangeBaseImageUR()
{
	ChangeBaseImageURMenu.show();
}

void MainWindow::slotChangeBaseImageLL()
{
	ChangeBaseImageLLMenu.show();
}


void MainWindow::slotMultiPlanarView()
{
	segmentationView = false;

	std::vector<QVTKWidget*> imageViews({ ui->image1View, ui->image2View, ui->image3View });

	if (!this->INITIALIZED_FLAG) {
		// Create  viewers if it hasn't been created
		ui->image1View->SetRenderWindow(m_2DimageViewer[0]->GetRenderWindow());
		ui->image2View->SetRenderWindow(m_2DimageViewer[1]->GetRenderWindow());
		ui->image3View->SetRenderWindow(m_2DimageViewer[2]->GetRenderWindow());
	}
	

	for (int i = 0; i < 3; ++i) {
		if (!this->INITIALIZED_FLAG) {
			//imageViews[i]->SetRenderWindow(m_2DimageViewer[0]->GetRenderWindow());


			// Change input to same image, default 0
			m_2DimageViewer[i]->SetInputData(vtkImage[0]);

			// Change orientation to orthogonal views
			m_2DimageViewer[i]->SetSliceOrientation(i);

			// initialize stuff
			m_2DimageViewer[i]->InitializeHeader(this->GetFileName(0));

			// setup viewers
			m_2DimageViewer[i]->SetupInteractor(m_interactor[i]);
			m_style[i]->SetViewers(m_2DimageViewer[i]);
			m_style[i]->initializeQWidget(ui->xSpinBox, ui->ySpinBox, ui->zSpinBox,
				ui->windowDoubleSpinBoxUL, ui->levelDoubleSpinBoxUL,
				m_moduleWidget->GetBrushSizeSpinBox(),
				m_moduleWidget->GetBrushShapeComBox(),
				NULL, NULL);

			m_interactor[i]->SetInteractorStyle(m_style[i]);
			m_style[i]->SetOrientation(i);

			// Set Flag to true after everything is done
			if (i == 2)
				this->INITIALIZED_FLAG = true;
		}
		else {
			// else only change input and viewer orientation
			m_2DimageViewer[i]->SetInputData(vtkImage[0]);
			m_2DimageViewer[i]->SetSliceOrientation(i);
			this->m_2DimageViewer[i]->GetRenderWindow()->GetInteractor()->Enable();
			this->m_style[i]->SetOrientation(i);
			m_2DimageViewer[i]->InitializeOrientationText();


			vtkPropCollection* props = this->m_2DimageViewer[i]->GetRenderer()->GetViewProps();
			for (int j = 0; j < props->GetNumberOfItems(); j++)
			{
				reinterpret_cast<vtkProp*>(props->GetItemAsObject(j))->SetVisibility(true);
			}


			vtkPropCollection* annProps = m_2DimageViewer[i]->GetannotationRenderer()->GetViewProps();
			for (int j = 0; j < annProps->GetNumberOfItems(); j++)
			{
				reinterpret_cast<vtkProp*>(annProps->GetItemAsObject(j))->SetVisibility(true);
			}

			// Make sure view up is correct
			switch (i)
			{
			case 0:
				this->m_2DimageViewer[i]->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
				break;
			case 1:
				this->m_2DimageViewer[i]->GetRenderer()->GetActiveCamera()->SetViewUp(0, 0, 1);
				break;
			case 2:
				this->m_2DimageViewer[i]->GetRenderer()->GetActiveCamera()->SetViewUp(0, -1, 0);
				break;
			default:
				break;
			}

		}
	}

	this->slotChangeSlice();
	QAction* action = widgetGroup.checkedAction();
	if (action != NULL) {
		action->triggered();
	}
}

void MainWindow::slotSegmentationView()
{
	m_orientation = SLICE_ORIENTATION_XY;
	segmentationView = true;

	for (int i = 0; i < 3; i++)
	{

		if (this->vtkImage[i] != NULL) {
			this->m_2DimageViewer[i]->SetSliceOrientationToXY();
			this->m_style[i]->SetOrientation(2);
			this->m_2DimageViewer[i]->GetRenderer()->GetActiveCamera()->SetViewUp(0, -1, 0);
			this->m_2DimageViewer[i]->InitializeOrientationText();
			this->m_2DimageViewer[i]->SetInputData(vtkImage[i]);
		}
		else {
			// disable view props
			vtkPropCollection* props = this->m_2DimageViewer[i]->GetRenderer()->GetViewProps();
			props->InitTraversal();
			for (int j = 0; j < props->GetNumberOfItems(); j++)
			{
				//static_cast<vtkProp*>(props->GetItemAsObject(j))->SetVisibility(false);
				props->GetNextProp()->SetVisibility(false);
			}

			vtkPropCollection* annProps = m_2DimageViewer[i]->GetannotationRenderer()->GetViewProps();
			annProps->InitTraversal();
			for (int j = 0; j < annProps->GetNumberOfItems(); j++)
			{
				//static_cast<vtkProp*>(annProps->GetItemAsObject(j))->SetVisibility(false);
				annProps->GetNextProp()->SetVisibility(false);
			}
			



			this->m_2DimageViewer[i]->GetRenderWindow()->Modified();
			this->m_2DimageViewer[i]->GetRenderWindow()->Render();
			this->m_2DimageViewer[i]->GetRenderWindow()->GetInteractor()->Disable();
		}
	}

	this->RenderAllViewer();
	this->slotChangeSlice();

	QAction* action = widgetGroup.checkedAction();
	if (action != NULL) {
		action->triggered();
	}
}

void MainWindow::slotAddExternalOverlay()
{
	if (SegmentationOverlay== NULL) {
		SegmentationOverlay = new Overlay;
	}
	QDir dir;
	QString Path = QFileDialog::getOpenFileName(this, QString(tr("DICOM")), dir.absolutePath());

	if (Path == "")
		return;
	this->SegmentationOverlay->SetInputImageData(Path);
	//this->vtkImageOverlay = SegmentationOverlay->GetOutput();

	if(m_2DimageViewer[0]->GetInput()!=NULL)
		addOverlay2ImageViewer();

}


void MainWindow::slotOverlayVisibilty(bool b, int orientation)
{
	for (int i = 0; i < 3; i++)
	{
		// Render only if the interactor is not disabled
		if (this->m_2DimageViewer[i]->GetSliceOrientation() == orientation
			&& this->m_2DimageViewer[i]->GetRenderWindow()->GetInteractor()->GetEnabled()) 
		{
			this->m_2DimageViewer[i]->GetdrawActor()->SetVisibility(b);
			this->m_2DimageViewer[i]->Render();
		}
	}
}

void MainWindow::slotOverlayVisibilty(bool b)
{
	for (int i = 0; i < 3; ++i)
	{
		if (m_2DimageViewer[i] != NULL) {
			m_2DimageViewer[i]->GetdrawActor()->SetVisibility(b);
			m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
			m_2DimageViewer[i]->Render();
		}
	}
}

void MainWindow::slotOverlayOpacity(double op)
{
    for (int i=0;i<3;i++)
    {
		if (m_style[i] != NULL)
			m_style[i]->GetPaintBrush()->SetDrawOpacity(op * 255);
        m_2DimageViewer[i]->GetdrawActor()->SetOpacity(op);
        m_2DimageViewer[i]->Render();
    }
}


void MainWindow::RenderAllViewer()
{
	this->ui->image4View->GetRenderWindow()->Render();
	//QCoreApplication::processEvents();
	for (int i = 0; i < 3; ++i)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		//m_2DimageViewer[i]->GetRenderer()->ResetCameraClippingRange();
		//m_2DimageViewer[i]->GetannotationRenderer()->ResetCameraClippingRange();
		m_2DimageViewer[i]->GetRenderWindow()->Render();
	}
}

void MainWindow::RenderAll2DViewers()
{
	//QCoreApplication::processEvents();
	for (int i = 0; i < 3; ++i)
	{
		if (segmentationView && i >= visibleImageNum)
			break;
		m_2DimageViewer[i]->GetRenderWindow()->Render();
	}
}

void MainWindow::SetImageLayerNo(int layer)
{
	m_layer_no = layer;
	for (int i = 0; i < 3; i++)
	{
		if (m_style[i] != NULL) {
			m_style[i]->GetPaintBrush()->SetDrawColor(overlayColor[m_layer_no-1]);
			m_style[i]->GetPolygonDraw()->SetLabel(m_layer_no);
		}
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
	else
		return NULL;
}


void MainWindow::Set3DRulerEnabled(bool b)
{
	MyVtkDistanceRepresentation3D* rep = MyVtkDistanceRepresentation3D::New();
	rep->SetDistanceWidget(this->m_distance3DWidget);
	this->m_distance3DWidget->SetInteractor(ui->image4View->GetRenderWindow()->GetInteractor());
	this->m_distance3DWidget->SetRepresentation(rep);
	this->m_distance3DWidget->On();
}


void MainWindow::slotCenterline()
{
	// segmentation
	VesselSegmentation* vesselSegmentation = new VesselSegmentation();
	vesselSegmentation->SetT1(itkImage[0]);
	vesselSegmentation->SetInputSegmentationImage(SegmentationOverlay->GetITKOutput());
	vesselSegmentation->SetMPRAGE(itkImage[4]);
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