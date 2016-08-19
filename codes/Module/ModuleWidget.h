#ifndef MODULEWIDGET_H
#define MODULEWIDGET_H

#include <QProgressDialog>
#include <QWidget>
#include <QSettings>
#include <QProgressBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QDir>
#include <QSpinBox>
#include <QComboBox>
#include "MyThread.h"
#include "Define.h"
#include "MainWindow.h"
#include "ReportGenerator.h"
#include <QDebug>
#include <QDesktopServices>
#include <QApplication>
#include <QUrl>

namespace Ui {
class ModuleWidget;
}
class MainWindow;
class ModuleWidget: public QWidget
{
    Q_OBJECT

public:
    explicit ModuleWidget(QWidget *parent = 0);
    ~ModuleWidget();

	void SetVisibleImageNo(int);
	void SetPage(int);
	//QString GetLumenPath();
	//QString GetVesselWallPath();

	QSpinBox* GetBrushSizeSpinBox();
	QComboBox* GetBrushShapeComBox();
	void GenerateReport();

public slots:
    void slotSegmentationView();
	void slotChangeLayerNo();
	void slotSelectROI();
	void slotResetROI();
	void slotChangeOpacity();
	void slotChangeROI(double* bound);
	//void slotAddLumen();
	//void slotAddVesselWall();
	void NextBtnChangeText();
	void NextPage();
	void BackPage();
	void SetBrushSize();
	void slotReportGetInput();

signals:
	void finished();
	void busy(int, QString);

private:
    Ui::ModuleWidget *ui;
    
    //Recent Parameters
    int m_maxParameters;
	QStringList NameList, ParameterList;
    //bool First_Load;
    int m_visible_image_no;

	//QString lumen_Path, vesselWall_Path;
};

#endif // MODULEWIDGET_H
