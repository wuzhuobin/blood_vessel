#include "Core.h"

#include "IADEOverlay.h"
#include "ui_MainWindow.h"
#include "ui_Switch2DWidget.h"
#include "ui_Switch3DWidget.h"
#include "ui_ViewerWidget.h"
#include "ui_Yolk3DSeries.h"
#include "ViewerWidget.h"
#include "Switch2DWidget.h"
#include "Switch3DWidget.h"
#include "ui_QAbstractNavigation.h"
#include "MeasurementWidget.h"
#include "IADEMeasurementWidget.h"
#include "ui_IADEMeasurementWidget.h"
#include "LabelWidget.h"


#include <qdebug.h>
#include <QVTKInteractor.h>
#include <qsignalmapper.h>
#include <qstringlist.h>

#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include "vtkROIWidget.h"
#include <vtkPolyDataMapper.h>
#include <vtkTransform.h>
#include <vtkProperty.h>

Core::Core(QObject * parent)
	:
	imageManager(NUM_OF_IMAGES, parent),
	ioManager(parent),
	dataProcessor(parent),
	measurement(parent),
	QObject(parent)
{
	ioManager.enableRegistration(true);
	//ioManager.enableRegistration(false);

	imageManager.setModalityName(0, "CUBE T1");
	imageManager.setModalityName(1, "CUBE T2");
	imageManager.setModalityName(2, "CUBE T1 + C");
	imageManager.setModalityName(3, "2D DIR/QIR");
	imageManager.setModalityName(4, "MPRAGE");
	mainWindow.addModalityNames("CUBE T1");
	mainWindow.addModalityNames("CUBE T2");
	mainWindow.addModalityNames("CUBE T1 + C");
	mainWindow.addModalityNames("2D DIR/QIR");
	mainWindow.addModalityNames("MPRAGE");


	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {

		imageViewers[i] = ImageViewer::New();
		imageViewers[i]->SetRenderWindow(mainWindow.getViewerWidget(i)->getUi()->qvtkWidget2->GetRenderWindow());
		imageViewers[i]->SetupInteractor(mainWindow.getViewerWidget(i)->getUi()->qvtkWidget2->GetInteractor());

		// Never use below method to set the interactorsyle
		//imageInteractorStyle[i]->SetInteractor(imageInteractor[i]);
		imageInteractorStyle[i] = StyleSwitch::New();
		imageInteractorStyle[i]->SetImageViewer(imageViewers[i]);
		mainWindow.getViewerWidget(i)->getUi()->qvtkWidget2->GetInteractor()->SetInteractorStyle(imageInteractorStyle[i]);

	}
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; ++i) {
		surfaceViewer[i] = CenterlineSurfaceViewer::New();
		surfaceViewer[i]->SetRenderWindow(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->qvtkWidget2->GetRenderWindow());
		surfaceViewer[i]->SetupInteractor(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->qvtkWidget2->GetInteractor());
		surfaceViewer[i]->EnableDepthPeelingOn();

		// Never use below method to set the interactorsyle
		//surfaceInteractorStyle[i]->SetInteractor(imageInteractor[i]);
		surfaceInteractorStyle[i] = StyleSwitch3D::New();
		surfaceInteractorStyle[i]->SetSurfaceViewer(surfaceViewer[i]);

		mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->qvtkWidget2->GetInteractor()->SetInteractorStyle(surfaceInteractorStyle[i]);
	}

	// find loading images and overlay
	connect(&mainWindow, SIGNAL(signalImageImportLoad(QStringList)),
		&ioManager, SLOT(slotAddToListOfFileNamesAndOpen(QStringList)));
	connect(&ioManager, SIGNAL(signalFinishOpenMultiImages()),
		this, SLOT(slotIOManagerToImageManager()));
	connect(&mainWindow, SIGNAL(signalCurvedImageExport(QString)),
		this, SLOT(slotSaveCurvedImage(QString)));
	connect(&ioManager, SIGNAL(signalFinishOpenOverlay()),
		this, SLOT(slotOverlayToImageManager()));


	// import and export overlay
	connect(&mainWindow, SIGNAL(signalOverlayImportLoad(QString)),
		&ioManager, SLOT(slotOpenOverlay(QString)));
	connect(&mainWindow, SIGNAL(signalOverlayExportSave(QString)),
		&ioManager, SLOT(slotSaveOverlay(QString)));

	// new overlay
	connect(mainWindow.getUi()->actionNew_segmentation, SIGNAL(triggered()),
		&ioManager, SLOT(slotInitializeOverlay()));



#ifdef PLAQUEQUANT_VER

	// Measurement
	connect(surfaceInteractorStyle[0]->GetStenosis(), SIGNAL(calculatedStenosis(double)),
		&measurement, SLOT(updateStenosis(double)));
	connect(imageInteractorStyle[DEFAULT_IMAGE]->GetNavigation()->QAbstractNavigation::getUi()->sliceSpinBoxZ, SIGNAL(valueChanged(int)),
		&measurement, SLOT(updateMaximumWallThickness(int)));


	// MIP

	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		connect(imageInteractorStyle[i]->GetNavigation(), SIGNAL(signalImagePos(int, int, int, unsigned int)),
			&this->mipViewer, SLOT(setImageCoordinate(int, int, int, unsigned int)));
		connect(this->mipViewer.getUi()->spinBoxSlice, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, [=](int) {
			this->imageViewers[i]->Render();
			int orientation = this->imageViewers[i]->GetSliceOrientation();
			this->mipPolyDataMapper[i]->SetInputConnection(this->mipViewer.getLineOutput(orientation));

		});
		mipPolyDataMapper[i] =
			vtkSmartPointer<vtkPolyDataMapper>::New();
		this->mipPolyDataMapper[i]->SetInputConnection(this->mipViewer.getLineOutput(i));
		vtkSmartPointer<vtkActor> actor =
			vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(this->mipPolyDataMapper[i]);
		actor->SetUserMatrix(this->mipViewer.getInverseImageDirection());
		actor->GetProperty()->SetColor(1, 0, 0);
		this->mipRenderer[i] = vtkRenderer::New();
		this->mipRenderer[i]->AddActor(actor);
		this->mipRenderer[i]->SetLayer(2);
		this->mipRenderer[i]->SetActiveCamera(this->imageViewers[i]->GetRenderer()->GetActiveCamera());

	}

	connect(&measurement, SIGNAL(signalMeasurement2D(double*)),
		mainWindow.getMeasurementWidget(), SLOT(slotUpdate2DMeasurements(double*)));
	connect(&measurement, SIGNAL(signalMeasurement3D(double*)),
		mainWindow.getMeasurementWidget(), SLOT(slotUpdate3DMeasurements(double*)));
	connect(&measurement, SIGNAL(signalStenosis(double)),
		mainWindow.getMeasurementWidget(), SLOT(slotUpdateStenosis(double)));

	mainWindow.getMeasurementWidget()->wind1 = imageViewers[2]->GetRenderWindow();
	mainWindow.getMeasurementWidget()->wind2 = surfaceViewer[0]->GetRenderWindow();

	dataProcessor.imageInteractorStyle = imageInteractorStyle;
	dataProcessor.surfaceInteractorStyle = surfaceInteractorStyle[0];
	dataProcessor.imageManager = &imageManager;
#endif // PLAQUEQUANT_VER

#ifdef IADE_VER
	// VBD-Ubogu
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedLengthOfBasilarArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLengthBA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedLengthOfIntracranialSegmentOfLeftVertebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLengthLVA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedLengthOfIntracranialSegmentOfRightVertebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLengthRVA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedDistanceBetweenCenterlineAndTheLead(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxDeviationBA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedDistanceBetweenCenterlineAndConnectionLeadOf2TerminalsLeft(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxDeviationLVA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedDistanceBetweenCenterlineAndConnectionLeadOf2TerminalsRight(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxDeviationRVA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedMaximumDiameterOfBasilarArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxMaxDiameterBA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedMaximumDiameterOfLeftVertebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxMaxDiameterLVA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetUboguMeasure(), SIGNAL(valueChangedMaximumDiameterOfRightVertebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxMaxDiameterRVA, SLOT(setValue(double)));
	//VBD-Smoker
	connect(imageInteractorStyle[0]->GetVBDSmoker(), SIGNAL(elongationChanged(int)),
		mainWindow.getMeasurementWidget()->getUi()->comboBoxElongationClass, SLOT(setCurrentIndex(int)));
	connect(imageInteractorStyle[0]->GetVBDSmoker(), SIGNAL(detourChanged(int)),
		mainWindow.getMeasurementWidget()->getUi()->comboBoxTortuosityClass, SLOT(setCurrentIndex(int)));
	connect(surfaceInteractorStyle[0]->GetSmokerBADiameter(), SIGNAL(diameterChanged(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxSmokerMaxDiameterBA, SLOT(setValue(double)));
	//ICDA
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedLeftCavernousSegmentOfInternalCarotidArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLICA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedRightCavernousSegmentOfInternalCarotidArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxRICA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedLeftMiddleCerebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLMCA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedRightMiddleCerebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxRMCA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedLeftAnteriorCerebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxLACA, SLOT(setValue(double)));
	connect(surfaceInteractorStyle[0]->GetICDADiameter(), SIGNAL(diameterChangedRightAnteriorCerebralArtery(double)),
		mainWindow.getMeasurementWidget()->getUi()->doubleSpinBoxRACA, SLOT(setValue(double)));


#endif // IADE_VER


	// ImageViewer
	// connect changing mode
	mainWindow.getUi()->sliceScrollArea->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetNavigation());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetWindowLevel());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetWindowLevelThreshold());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetThreshold());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetPaintBrush());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetLumenSeedsPlacer());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVOI());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVBDSmoker());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetTubularVOI());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetRuler());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetMaximumWallThickness());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetPolygonDrawSeries());
	mainWindow.getSwitch2DWidget()->addWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVesselSegmentation2());

	connect(mainWindow.getUi()->actionTesting, SIGNAL(triggered()),
		this, SLOT(slotTesting()));
	connect(mainWindow.getUi()->actionNavigation, SIGNAL(triggered()),
		this, SLOT(slotNavigation()));
	connect(mainWindow.getUi()->actionWindow_level, SIGNAL(triggered()),
		this, SLOT(slotWindowLevel()));
	connect(mainWindow.getUi()->actionThreshold, SIGNAL(triggered()),
		this, SLOT(slotThreshold()));
	connect(mainWindow.getUi()->actionWindow_level_threshold, SIGNAL(triggered()),
		this, SLOT(slotWindowlevelThreshold()));
	connect(mainWindow.getUi()->actionPaint_brush, SIGNAL(triggered()),
		this, SLOT(slotPaintBrush()));
	connect(mainWindow.getUi()->actionSeeds_placer, SIGNAL(triggered()),
		this, SLOT(slotSeedsPlacer()));
	connect(mainWindow.getUi()->acitonVOI_selection, SIGNAL(triggered()),
		this, SLOT(slotVOI()));
	connect(mainWindow.getUi()->actionTubular_VOI, SIGNAL(triggered()),
		this, SLOT(slotTubularVOI()));
	connect(mainWindow.getUi()->actionDistance_measure, SIGNAL(triggered()),
		this, SLOT(slotRuler()));
	connect(mainWindow.getUi()->actionMaximum_wall_thickness, SIGNAL(triggered()),
		this, SLOT(slotMaximumWallThickness()));
	connect(mainWindow.getUi()->actionPolygon_draw, SIGNAL(triggered()),
		this, SLOT(slotPolygonDraw()));
	connect(mainWindow.getUi()->actionPolygon_draw_series, SIGNAL(triggered()),
		this, SLOT(slotPolygonDrawSeries()));
	connect(mainWindow.getUi()->actionVessel_segmentation, SIGNAL(triggered()),
		this, SLOT(slotVesselSegmentation()));
	connect(mainWindow.getUi()->actionVBD_Smoker_seed, SIGNAL(triggered()),
		this, SLOT(slotVBDSmokerSeed()));


	// change view mode
	connect(mainWindow.getUi()->actionCurved_view, SIGNAL(toggled(bool)),
		this, SLOT(slotCurvedView(bool)));
	connect(mainWindow.getUi()->actionMIP, SIGNAL(toggled(bool)),
		this, SLOT(slotMIP(bool)));
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		QPushButton* buttons[3] = {
			mainWindow.getViewerWidget(i)->getUi()->pushButtonSigitalView,
			mainWindow.getViewerWidget(i)->getUi()->pushButtonCoronalView,
			mainWindow.getViewerWidget(i)->getUi()->pushButtonAxialView };
		connect(mainWindow.getUi()->actionMulti_planar_view, SIGNAL(triggered()),
			buttons[i%3], SLOT(click()));
		connect(mainWindow.getUi()->actionAll_axial_view, SIGNAL(triggered()),
			buttons[2], SLOT(click()));
	}

	// change image
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		connect(mainWindow.getSelectImgMenu(i), SIGNAL(triggered(QAction*)),
			this, SLOT(slotChangeImage(QAction*)));
		connect(mainWindow.getSelectImgMenu(i), SIGNAL(triggered(QAction*)),
			this, SLOT(slotChangeImage(QAction*)));
	}
	// change slice orientation
	QSignalMapper* sliceOrientationMapperA = new QSignalMapper(this);
	QSignalMapper* sliceOrientationMapperS = new QSignalMapper(this);
	QSignalMapper* sliceOrientationMapperC = new QSignalMapper(this);
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		sliceOrientationMapperA->setMapping(mainWindow.getViewerWidget(i)->getUi()->pushButtonAxialView, i);
		sliceOrientationMapperS->setMapping(mainWindow.getViewerWidget(i)->getUi()->pushButtonSigitalView, i);
		sliceOrientationMapperC->setMapping(mainWindow.getViewerWidget(i)->getUi()->pushButtonCoronalView, i);
		connect(mainWindow.getViewerWidget(i)->getUi()->pushButtonAxialView, SIGNAL(clicked()),
			sliceOrientationMapperA, SLOT(map()));
		connect(mainWindow.getViewerWidget(i)->getUi()->pushButtonSigitalView, SIGNAL(clicked()),
			sliceOrientationMapperS, SLOT(map()));
		connect(mainWindow.getViewerWidget(i)->getUi()->pushButtonCoronalView, SIGNAL(clicked()),
			sliceOrientationMapperC, SLOT(map()));
	}
	connect(sliceOrientationMapperA, SIGNAL(mapped(int)),
		this, SLOT(slotChangeSliceOrientationToXY(int)));
	connect(sliceOrientationMapperS, SIGNAL(mapped(int)),
		this, SLOT(slotChangeSliceOrientationToYZ(int)));
	connect(sliceOrientationMapperC, SIGNAL(mapped(int)),
		this, SLOT(slotChangeSliceOrientationToXZ(int)));

	// SurfaceViewer
	// change orientation
	QSignalMapper* orientationMapperA = new QSignalMapper(this);
	QSignalMapper* orientationMapperS = new QSignalMapper(this);
	QSignalMapper* orientationMapperC = new QSignalMapper(this);
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		orientationMapperA->setMapping(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonAxialView, i);
		orientationMapperS->setMapping(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonSigitalView, i);
		orientationMapperC->setMapping(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonCoronalView, i);
		connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonAxialView, SIGNAL(clicked()),
			orientationMapperA, SLOT(map()));
		connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonSigitalView, SIGNAL(clicked()),
			orientationMapperS, SLOT(map()));
		connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonCoronalView, SIGNAL(clicked()),
			orientationMapperC, SLOT(map()));
		//connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonAxialView, SIGNAL(clicked()),
		//	this, SLOT(slotChangeOrientationToXY()));
		//connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonSigitalView, SIGNAL(clicked()),
		//	this, SLOT(slotChangeOrientationToYZ()));
		//connect(mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->pushButtonCoronalView, SIGNAL(clicked()),
		//	this, SLOT(slotChangeOrientationToXZ()));
	}
	connect(orientationMapperA, SIGNAL(mapped(int)),
		this, SLOT(slotChangeOrientationToXY(int)));
	connect(orientationMapperS, SIGNAL(mapped(int)),
		this, SLOT(slotChangeOrientationToYZ(int)));
	connect(orientationMapperC, SIGNAL(mapped(int)),
		this, SLOT(slotChangeOrientationToXZ(int)));
	// surface action

	mainWindow.getSwitch3DWidget()->addWidget(surfaceInteractorStyle[0]->GetICDADiameter());
	mainWindow.getSwitch3DWidget()->addWidget(surfaceInteractorStyle[0]->GetSmokerBADiameter());
	mainWindow.getSwitch3DWidget()->addWidget(surfaceInteractorStyle[0]->GetUboguMeasure());

	connect(mainWindow.getUi()->actionTraceball_camera, SIGNAL(triggered()),
		this, SLOT(slotTrackballCamera()));
	connect(mainWindow.getUi()->actionCenter_line, SIGNAL(triggered()),
		this, SLOT(slotCenterLine()));
	connect(mainWindow.getUi()->actionFind_maximum_radius, SIGNAL(triggered()),
		this, SLOT(slotFindMaximumRadius()));
	connect(mainWindow.getUi()->actionPerpendicular_measurement, SIGNAL(triggered()),
		this, SLOT(slotPerpendicularMeasurement()));
	connect(mainWindow.getUi()->actionCurved_navigation, SIGNAL(triggered()),
		this, SLOT(slotCurvedNavigation()));
	connect(mainWindow.getUi()->actionWay_point_centerline, SIGNAL(triggered()),
		this, SLOT(slotWaypoint()));
	connect(mainWindow.getUi()->actionStenosis, SIGNAL(triggered()),
		this, SLOT(slotStenosis()));
	connect(mainWindow.getUi()->actionICDA_diameter, SIGNAL(triggered()),
		this, SLOT(slotICDADiameter()));
	connect(mainWindow.getUi()->actionVBD_Smoker_BA_diameter, SIGNAL(triggered()),
		this, SLOT(slotVBDBADiameter()));
	connect(mainWindow.getUi()->actionVBD_ubogu_measure, SIGNAL(triggered()),
		this, SLOT(slotVBDUboguMeasure()));


	// update btn
	connect(mainWindow.getUi()->updateBtn, SIGNAL(clicked()),
		this, SLOT(slotUpdateSurfaceView()));
	connect(mainWindow.getUi()->updateBtn, SIGNAL(clicked()),
		mainWindow.getUi()->actionTraceball_camera, SLOT(trigger()));
	connect(mainWindow.getUi()->actionUpdate_curved_images, SIGNAL(triggered()),
		this, SLOT(slotInitializeCurvedImage()));

	// Close event handling
	connect(&mainWindow, SIGNAL(signalThisClosing()),
		&mipViewer, SLOT(slotClose()));

	mainWindow.show();
}

Core::~Core()
{

	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {

		imageInteractorStyle[i]->Delete();
		imageInteractorStyle[i] = nullptr;


		mainWindow.getViewerWidget(i)->getUi()->qvtkWidget2->GetInteractor()->SetInteractorStyle(nullptr);
		//imageInteractor[i]->Delete();
		//imageInteractor[i] = nullptr;

		imageViewers[i]->Delete();
		imageViewers[i] = nullptr;

		//mainWindow.setRenderWindow(i, nullptr);
	}

	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; ++i) {

		surfaceInteractorStyle[i]->Delete();
		surfaceInteractorStyle[i] = nullptr;


		mainWindow.getViewerWidget(MainWindow::NUM_OF_2D_VIEWERS + i)->getUi()->qvtkWidget2->GetInteractor()->SetInteractorStyle(nullptr);
		//imageInteractor[i]->Delete();
		//imageInteractor[i] = nullptr;

		surfaceViewer[i]->Delete();
		surfaceViewer[i] = nullptr;

		//mainWindow.setRenderWindow(i, nullptr);
	}
}

void Core::slotIOManagerToImageManager()
{
	for (int i = 0; i < NUM_OF_IMAGES; ++i) {
			imageManager.setImage(i, ioManager.getListOfImage()[i]);
			imageManager.setDicomIO(i, ioManager.getListOfDicomIOs()[i]);
	}

	// set input to image viewer
	ioManager.slotInitializeOverlay();


	// update selectImgMenus
	for (int i = 0; i < NUM_OF_IMAGES; ++i) {
		mainWindow.setSelectImgMenuVisible(i, imageManager.getImage(i));
	}
	// reset display extent first, otherwise the slice spin box cannot update right
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageViewers[i]->ResetDisplayExtent();
	}
	// initialization, and trigger the navigation interactorstyle

	mainWindow.getMeasurementWidget()->info = imageManager.getDicomIO(0);
	mainWindow.initialization();
	const int* extent = imageViewers[DEFAULT_IMAGE]->GetDisplayExtent();
	imageInteractorStyle[DEFAULT_IMAGE]->GetNavigation()->SetCurrentFocalPointWithImageCoordinate(
		(extent[1] - extent[0])*0.5,
		(extent[3] - extent[2])*0.5,
		(extent[5] - extent[4])*0.5
	);
	IVtkImageData::itkImageType::PointType origin =
		this->imageManager.getImage(0)->GetItkImage()->GetOrigin();

	IVtkImageData::itkImageType::DirectionType direction =
		this->imageManager.getImage(0)->GetItkImage()->GetDirection();

	vtkSmartPointer<vtkMatrix4x4> directionMatrix =
		vtkSmartPointer<vtkMatrix4x4>::New();
	//directionMatrix->Identity();
	for (int row = 0; row < 3; ++row) {
		for (int col = 0; col < 4; ++col) {
			if (col < 3) {
				directionMatrix->SetElement(row, col, direction[row][col]);
			}
			else {
				directionMatrix->SetElement(row, 3, origin[row]);

			}

		}
	}
	this->mipViewer.setImageDirection(directionMatrix);


	// clear the memory later, sometimes it will clear too early
	// make no different, if it has not been clear
	//ioManager.clearListOfItkImages();
	//ioManager.clearListOfDicoms();
}

void Core::slotOverlayToImageManager()
{
	imageManager.setOverlay(ioManager.getOverlay());

	// Opacity

	connect(mainWindow.getLabelWidget(), SIGNAL(signalOpacityRequested(int)),
		imageManager.getOverlay(), SLOT(slotRequestOpacity(int)));
	connect(imageManager.getOverlay(), SIGNAL(signalGetRequestedOpacity(double)),
		mainWindow.getLabelWidget(), SLOT(slotSetOpacity(double)));
	connect(mainWindow.getLabelWidget(), SIGNAL(signalOpacityChanged(int, double)),
		imageManager.getOverlay(), SLOT(slotSetOpacity(int, double)));
	connect(mainWindow.getLabelWidget(), SIGNAL(signalOpacityChanged(int, double)),
		this, SLOT(slotRenderALlViewers()));



	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		slotUpdateImageViewersToCurrent(i);
	}




	// clear the memory later, sometimes it will clear too early
	// make no different, if it has not been clear
	//ioManager.clearOverlay();
}

void Core::slotSaveCurvedImage(QString fileName)
{
#ifdef PLAQUEQUANT_VER

	ioManager.slotSaveCurvedOverlay(fileName, imageManager.getCurvedPlaqueQuantOverlay()->getData());

	for (int i = 0; i < NUM_OF_IMAGES; ++i) {

		QStringList _f = fileName.split('.');
		_f[0] += '_' + imageManager.getModalityName(i);
		QString _newFileName = _f.join('.');

		ioManager.slotSaveCurvedImages(_newFileName, imageManager.getCurvedImage(i));
	}

#endif // PLAQUEQUANT_VER
}

void Core::slotMIP(bool flag)
{

	if (flag) {
		this->mipViewer.show();
		for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
			this->imageViewers[i]->GetRenderWindow()->SetNumberOfLayers(3);
			this->imageViewers[i]->GetRenderWindow()->AddRenderer(this->mipRenderer[i]);
		}
	}
	else {
		this->mipViewer.hide();
		for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
			//this->imageViewers[i]->GetRenderWindow()->SetNumberOfLayers(3);
			this->imageViewers[i]->GetRenderWindow()->RemoveRenderer(this->mipRenderer[i]);
		}
	}

}

void Core::slotNavigation()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToNavigation();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetNavigation());
}

void Core::slotWindowLevel()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToWindowLevel();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetWindowLevel());
}

void Core::slotThreshold()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToThreshold();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetThreshold());

}

void Core::slotWindowlevelThreshold()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToWindowLevelThreshold();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetWindowLevelThreshold());
}

void Core::slotPaintBrush()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToPaintBrush();
		//imageInteractorStyle[i]->GetPaintBrush()->SetOverlay(imageManager.getOverlay()->getData());
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetPaintBrush());

}

void Core::slotTesting()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToInteractorStyleTesting();
	}
	mainWindow.getSwitch2DWidget()->setWidget(nullptr);

}

void Core::slotSeedsPlacer()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToLumenSeedsPlacer();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetLumenSeedsPlacer());

}

void Core::slotVOI()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToVOI();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVOI());

}

void Core::slotTubularVOI()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToTubularVOI();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetTubularVOI());

}

void Core::slotRuler()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToRuler();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetRuler());

}

void Core::slotMaximumWallThickness()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToMaximumWallThickness();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetMaximumWallThickness());
}

void Core::slotPolygonDraw()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToPolygonDraw();
	}
	mainWindow.getSwitch2DWidget()->setWidget(nullptr);
}

void Core::slotPolygonDrawSeries()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToPolygonDrawSeries();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetPolygonDrawSeries());

}

void Core::slotVesselSegmentation()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToVesselSegmentation2();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVesselSegmentation2());
}

void Core::slotTrackballCamera()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DTrackballCamera();
	}
}

void Core::slotCenterLine()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DCenterLine();
	}

}

void Core::slotFindMaximumRadius()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DFindMaximumRadius();
	}
}

void Core::slotPerpendicularMeasurement()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DPerpendicularMeasurement();
	}
}

void Core::slotCurvedNavigation()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DCurvedNavigation();
	}
}

void Core::slotWaypoint()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DWaypoint();
	}
}

void Core::slotStenosis()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DStenosis();
	}
}

void Core::slotVBDUboguMeasure()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DUboguMeasure();
	}
	mainWindow.getSwitch3DWidget()->setWidget(surfaceInteractorStyle[0]->GetUboguMeasure());

}

void Core::slotICDADiameter()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DICDADiameter();
	}
	mainWindow.getSwitch3DWidget()->setWidget(surfaceInteractorStyle[0]->GetICDADiameter());
}

void Core::slotVBDBADiameter()
{
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceInteractorStyle[i]->SetInteractorStyleTo3DSmokerBADiameter();
	}
	mainWindow.getSwitch3DWidget()->setWidget(surfaceInteractorStyle[0]->GetSmokerBADiameter());

}

void Core::slotVBDSmokerSeed()
{
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		imageInteractorStyle[i]->SetInteractorStyleToVBDSmoker();
	}
	mainWindow.getSwitch2DWidget()->setWidget(imageInteractorStyle[DEFAULT_IMAGE]->GetVBDSmoker());
}
#include <qmessagebox.h>
#include <vtkPolyData.h>
bool Core::slotInitializeCurvedImage()
{
#ifdef PLAQUEQUANT_VER
	if (!surfaceViewer[0]->GetCenterline() || surfaceViewer[0]->GetCenterline()->GetNumberOfPoints() < 2) {
		QMessageBox::critical(&mainWindow, QString("No centerline"),
			QString("Please Generate centerline first !"));
		return false;
	}
	dataProcessor.initializeCurved();
#endif // PLAQUEQUANT_VER
	return true;
}

void Core::slotChangeImage(QAction * action)
{
	QMenu* selectImgMenu = static_cast<QMenu*>(sender());
	unsigned int viewer = 0;
	for (int i = 0; i < MainWindow::NUM_OF_VIEWERS; ++i) {
		if (selectImgMenu == mainWindow.getSelectImgMenu(i)) {
			viewer = i;
			break;
		}
	}
	int image = imageManager.getIndexOfModalityName(action->text());
	slotChangeImage(viewer, image);
}

void Core::slotChangeImage(int viewer, int image)
{
	currentImage[viewer] = image;
	slotUpdateImageViewersToCurrent(viewer);
}

void Core::slotChangeCurved(int viewer, bool curvedFlag)
{
	currentCurved[viewer] = curvedFlag;
	slotUpdateImageViewersToCurrent(viewer);
}

void Core::slotChangeSliceOrientation(int viewer, int sliceOrientation)
{
	currentSliceOrientation[viewer] = sliceOrientation;
	slotUpdateImageViewersToCurrent(viewer);
}

void Core::slotChangeSliceOrientationToYZ(int viewer)
{
	slotChangeSliceOrientation(viewer, ImageViewer::SLICE_ORIENTATION_YZ);
}

void Core::slotChangeSliceOrientationToXZ(int viewer)
{
	slotChangeSliceOrientation(viewer, ImageViewer::SLICE_ORIENTATION_XZ);

}

void Core::slotChangeSliceOrientationToXY(int viewer)
{
	slotChangeSliceOrientation(viewer, ImageViewer::SLICE_ORIENTATION_XY);
}

void Core::slotUpdateImageViewersToCurrent(int viewer)
{

	imageViewers[viewer]->InitializeHeader(imageManager.getModalityName(currentImage[viewer]).toStdString());
	imageViewers[viewer]->SetLookupTable(imageManager.getOverlay()->getLookupTable());
	if (currentCurved[viewer]) {
#ifdef PLAQUEQUANT_VER
		imageViewers[viewer]->SetOverlay(imageManager.getCurvedPlaqueQuantOverlay()->getData());
		imageViewers[viewer]->SetInputData(imageManager.getCurvedImage(currentImage[viewer]));
		// Measurement
		measurement.setOverlay(imageManager.getCurvedPlaqueQuantOverlay()->getData());
#endif // PLAQUEQUANT_VER
	}
	else
	{
		imageViewers[viewer]->SetOverlay(imageManager.getOverlay()->getData());
		imageViewers[viewer]->SetInputData(imageManager.getImage(currentImage[viewer]));
#ifdef PLAQUEQUANT_VER
		// Measurement
		measurement.setOverlay(imageManager.getOverlay()->getData());
#endif // PLAQUEQUANT_VER
	}
	imageViewers[viewer]->SetSliceOrientation(currentSliceOrientation[viewer]);
	imageViewers[viewer]->Render();

	//mainWindow.getViewerWidget(viewer)->setWindowTitle(imageManager.getModalityName(currentImage[viewer]));

}

void Core::slotCurvedView(bool flag)
{
#ifdef PLAQUEQUANT_VER
	bool _flag = true;
	for (int i = 0; i < MainWindow::NUM_OF_2D_VIEWERS; ++i) {
		if (!imageManager.getCurvedPlaqueQuantOverlay()) {
			_flag = slotInitializeCurvedImage();
		}
		if (!_flag) {
			return;
		}
		currentCurved[i] = flag;
		slotUpdateImageViewersToCurrent(i);

	}
#endif // PLAQUEQUANT_VER
}



void Core::slotUpdateSurfaceView()
{
	// temporary fix for real time updated.
	vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
	if (currentCurved[DEFAULT_IMAGE]) {
#ifdef PLAQUEQUANT_VER
		image->ShallowCopy(imageManager.getCurvedPlaqueQuantOverlay()->getData());
#endif // PLAQUEQUANT_VER
	}
	else {
		image->ShallowCopy(imageManager.getOverlay()->getData());
	}
	//surfaceViewer->SetInputData(imageManager.getOverlay()->getData());
	for (int i = 0; i < MainWindow::NUM_OF_3D_VIEWERS; i++)
	{
		surfaceViewer[i]->SetInputData(image);
		surfaceViewer[i]->SetLookupTable(imageManager.getOverlay()->getLookupTable());
		surfaceViewer[i]->SetCenterline(static_cast<OVERLAY*>(imageManager.getOverlay())->getCenterLine());

		surfaceViewer[i]->Render();
	}

	this->mainWindow.getUi()->menuSurface->setEnabled(true);
	this->mainWindow.getUi()->menuSegmentation->setEnabled(true);
}

void Core::slotChangeOrientationToYZ(int viewer)
{
	currentOrientation[viewer] = SurfaceViewer::ORIENTATION_YZ;
	slotUpdateSurfaceViewersToCurrent(viewer);
}

void Core::slotChangeOrientationToXZ(int viewer)
{
	currentOrientation[viewer] = SurfaceViewer::ORIENTATION_XZ;
	slotUpdateSurfaceViewersToCurrent(viewer);
}

void Core::slotChangeOrientationToXY(int viewer)
{
	currentOrientation[viewer] = SurfaceViewer::ORIENTATION_XY;
	slotUpdateSurfaceViewersToCurrent(viewer);
}

void Core::slotUpdateSurfaceViewersToCurrent(int viewer)
{
	surfaceViewer[viewer]->SetOrientation(currentOrientation[viewer]);
	surfaceViewer[viewer]->Render();
}

void Core::slotRenderALlViewers()
{
	imageViewers[0]->Render();
	imageViewers[1]->Render();
	imageViewers[2]->Render();
	surfaceViewer[0]->Render();
}

void Core::slotMainWndCloseEvent()
{
	mipViewer.close();
}

