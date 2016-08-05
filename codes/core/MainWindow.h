#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QMainWindow>
#include <QDragEnterEvent> 
#include <QDropEvent>
#include <QAction>
#include <QMenu>

#include "ui_MainWindow.h"

#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>

//Own Class
#include "Define.h"
#include "MyImageViewer.h"
#include "InteractorStyleSwitch.h"
#include "InfoDialog.h"
#include "MyPlaneWidget.h"
#include "Overlay.h"

class ModuleWidget;
class MainWindow: public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(); 	

	static MainWindow* GetMainWindow();
    MyImageViewer* GetMyImageViewer(int);
    InteractorStyleSwitch* GetInteractorStyleImageSwitch(int);
	QString GetFileName(int);

	int GetVisibleImageNo();
	void RenderAllViewer();
	void Set3DRulerEnabled(bool b);
	void SetImageLayerNo(int);
	int GetImageLayerNo();
	Overlay* GetOverlay();
	~MainWindow();
	
	enum SLICE_ORIENTATION
  {
    SLICE_ORIENTATION_YZ = 0,
    SLICE_ORIENTATION_XZ = 1,
    SLICE_ORIENTATION_XY = 2
  };

public slots:
	//GUI
	virtual void resizeEvent(QResizeEvent * event);
	virtual void dragEnterEvent(QDragEnterEvent *ev);
	virtual void dropEvent( QDropEvent *ev );
	
	virtual void slotChangeSlice();
	virtual void slotChangeSlice(int, int, int);
	virtual void slotChangeWindowLevel();
	virtual void slotChangeWindowLevelUL();
    virtual void slotChangeWindowLevelUR();
    virtual void slotChangeWindowLevelLL();
    virtual void slotChangeWindowLevelLR();

	//Action
	virtual void slotExit();
	virtual void slotOpenImage(QString dir);
	virtual void slotOpenImage();
	virtual void slotOpenRecentImage();
	virtual void slotAbout();
	virtual void slotHelp();
	virtual void slot3DUpdate();
	virtual void slotNavigationMode();
	virtual void slotWindowLevelMode();
	virtual void slotBrushMode();
	virtual void slotContourMode();
	virtual void slotImage(int image);
	virtual void slotMultiPlanarView();



	//Overlay
    
	virtual void slotAddExternalOverlay();
	//virtual void slotRemoveOverlay();
	virtual void slotOverlayVisibilty(bool b);
	virtual void slotOverlayVisibilty(bool b, int);
    virtual void slotOverlayOpacity(double);
  
	//Centerline
	virtual void slotCenterline();
    //Segmentation
	virtual void slotSegmentationView();
	virtual void slotChangeBaseImageUL();
	virtual void slotChangeBaseImageUR();
	virtual void slotChangeBaseImageLL();
	virtual void slotChangeImageSeq(int image_no, int window_no);
	virtual void slotSelectImageSeq(QAction*);
	//Intensity
	virtual void slotChangeIntensity();

	//Widget
	virtual void slotRuler(bool b);
	virtual void slotSetROIWidgetEnabled(bool b);
	virtual void slotSelectROI();

	//Widget running 
	virtual void slotShowProgressDialog(int value, QString text);

	//Visualization
	bool loadImage(int, QStringList*list);
	bool visualizeImage();

private:
	//UI
	Ui_MainWindow ui;
	void setActionsEnable(bool b);

	//menu for changing images
	QMenu ChangeBaseImageULMenu;
	QMenu ChangeBaseImageURMenu;
	QMenu ChangeBaseImageLLMenu;

    
	//2D/3D view
	MyImageViewer*				m_2DimageViewer[3];
	vtkRenderWindowInteractor*  m_3Dinteractor;
	vtkRenderWindowInteractor*  m_interactor[3];
	InteractorStyleSwitch*		m_style[3];

    //ROI
	MyPlaneWidget*			m_planeWidget[3];
	MyPlaneWidgetCallback*	m_planeWidgetCallback[3];
	double	m_currentBound[6];
	bool	m_settingROI;

    //File Name
    QStringList FileNameList1;
    QStringList FileNameList2;
    QStringList FileNameList3;
    QStringList FileNameList4;
    QStringList FileNameList5;
    
	//3D ruler
	vtkDistanceWidget* DistanceWidget3D;
	//Recent File
	int m_maxRecentImage;
	QList<QAction*> recentFileActionList;
	void createRecentImageActions();
	void adjustForCurrentFile(const QString& filePath);
	void updateRecentActionList();

	//Initialization
	void initializeViewers();
	void initializeModule();
	void addOverlay2ImageViewer();

	
	//Data
	ImageType::Pointer  ImageAlignment(ImageType::Pointer);
	ImageType::Pointer	m_itkT1;
	ImageType::Pointer	m_itkT2;
	ImageType::Pointer	m_itkT1C;
	ImageType::Pointer	m_itk2DDIR;
	ImageType::Pointer	m_itkMPRAGE;
	vtkImageData*		m_vtkT1;
	vtkImageData*		m_vtkT2;
	vtkImageData*		m_vtkT1C;
    vtkImageData*		m_vtk2DDIR;
	vtkImageData*		m_vtkMPRAGE;
	//vtkPolyData*		m_vtkOutputPolyData;

    //Orientation
	int m_orientation;
    
	//Dock Widget
	ModuleWidget* m_moduleWidget;

	//Navigation
	double m_focalPoint[3];
    
    int visibleImageNum;

	//Information
	Dialog* m_InfoDialog;

	//Segmentation view flag
	bool segmentationView;


	//Overlay
	Overlay* SegmentationOverlay;
	int m_layer_no;		//Main Image

	const int* overlayColor[6];

	const int lumen[3] = { 255, 0, 0 };
	const int vesselWall[3] = { 0, 0, 255 };
	const int calcification[3] = { 0,255,0 };
	const int hemorrhage[3] = { 255, 255, 0 };
	const int lrnc[3] = { 0, 255, 255 };
	const int lm[3] = { 255, 0, 255 };

};
 
#endif //MAINWINDOW_H
