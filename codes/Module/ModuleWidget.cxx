#include "ModuleWidget.h"
#include "ui_ModuleWidget.h"


ModuleWidget::ModuleWidget(QWidget *parent) : 
    QWidget(parent),
    ui(new Ui::ModuleWidget)
{
    ui->setupUi(this);
	ui->stackedWidget->setCurrentIndex(0);
    //Recent Parameters
    m_maxParameters = 10;

    //Initialize the widget
   // First_Load = true;
    //slotLoadParameters();
	this->ui->BrushSizeSlider->setMaximum(40);
	this->ui->BrushSizeSlider->setMinimum(1);
	this->ui->BrushSizeSpinBox->setMaximum(40);
	this->ui->BrushSizeSpinBox->setMinimum(1);

	ui->BackBtn->setVisible(false);
	ui->NextBtn->setVisible(false);
	//connect
	connect(ui->NextBtn, SIGNAL(clicked()), this, SLOT(NextPage()));
	connect(ui->BackBtn, SIGNAL(clicked()), this, SLOT(BackPage()));
	connect(ui->BrushSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(SetBrushSize()),Qt::UniqueConnection);
	connect(ui->labelComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeLayerNo()), Qt::UniqueConnection);
	connect(ui->segmentationPushButton, SIGNAL(clicked()), this, SLOT(slotSelectROI()));
	connect(ui->resetROIPushButton, SIGNAL(clicked()), this, SLOT(slotResetROI()));
	connect(ui->opacitySlider, SIGNAL(valueChanged(int)), ui->opacitySpinBox, SLOT(setValue(int)));
	connect(ui->opacitySpinBox, SIGNAL(valueChanged(int)), ui->opacitySlider, SLOT(setValue(int)));
	connect(ui->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotChangeOpacity()));
	connect(ui->opacitySpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotChangeOpacity()));

}

ModuleWidget::~ModuleWidget()
{
    delete ui;
}

void ModuleWidget::NextBtnChangeText()
{
	int index = ui->stackedWidget->currentIndex();
	if (index == 0 || index == 1 || index == 2)
	{
		ui->NextBtn->setText("Next");
	}
	if (index == 3)
	{
		ui->NextBtn->setText("Generate Report");
		disconnect(ui->NextBtn, SIGNAL(clicked()), this, SLOT(NextPage()));
		connect(ui->NextBtn, SIGNAL(clicked()), this, SLOT(slotReportGetInput()));
	}
}
void ModuleWidget::NextPage()
{
	int index = ui->stackedWidget->currentIndex();
	
	ui->stackedWidget->setCurrentIndex(index + 1);
	this->NextBtnChangeText();
}

void ModuleWidget::BackPage()
{
	int index = ui->stackedWidget->currentIndex();
	ui->stackedWidget->setCurrentIndex(index - 1);
	this->NextBtnChangeText();

}

void ModuleWidget::SetPage(int index)
{
	ui->stackedWidget->setCurrentIndex(index);
}
void ModuleWidget::SetBrushSize()
{
	ui->BrushSizeSpinBox->setValue((ui->BrushSizeSlider->value()));
}
QSpinBox* ModuleWidget::GetBrushSizeSpinBox()
{
	return this->ui->BrushSizeSpinBox;
}

QComboBox* ModuleWidget::GetBrushShapeComBox()
{
	return this->ui->BrushComBox;
}
void ModuleWidget::slotSegmentationView()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	mainWnd->slotSegmentationView();
}

void ModuleWidget::slotChangeLayerNo()
{
	MainWindow* mainwnd = MainWindow::GetMainWindow();
	int layer = ui->labelComboBox->currentIndex() + 1;
	mainwnd->SetImageLayerNo(layer);

}

void ModuleWidget::slotChangeOpacity()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	mainWnd->slotOverlayOpacity(ui->opacitySpinBox->value()/100.0);

}
void ModuleWidget::slotChangeROI(double * bound)
{
	ui->centerSpinBox1->setValue((bound[0] + bound[1])*0.5);
	ui->centerSpinBox2->setValue((bound[2] + bound[3])*0.5);
	ui->centerSpinBox3->setValue((bound[4] + bound[5])*0.5);
	ui->sizeSpinBox1->setValue(bound[1] - bound[0]);
	ui->sizeSpinBox2->setValue(bound[3] - bound[2]);
	ui->sizeSpinBox3->setValue(bound[5] - bound[4]);

}
void ModuleWidget::slotSelectROI()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	mainWnd->slotSelectROI();
}

void ModuleWidget::slotResetROI()
{
	MainWindow* mainWnd = MainWindow::GetMainWindow();
	mainWnd->slotResetROI();
}



void ModuleWidget::slotReportGetInput()
{
	this->GenerateReport();
}

void ModuleWidget::GenerateReport()
{
	//General 
	//Basic Information to fill
	QString FilePath = "C:/Users/user/Desktop/Andy/ReportGenerator/ReportGenerator/result.pdf";
	QString ReportName = "Plaque Quantification Report";
	QString PatientName = "Chan Tai Man";
	QString PatientID = "11223344";
	QString PatientDOB = "01/01/1956";
	QString PatientGender = "M";
	QString ScanDate = "2016/5/27";
	QString ReportDate = "2016/5/30";
	QString MRISide = "R";
	QString DoctorName = "Dr. Lau";
	//Stenosis Measurement
	QString StenosisPercent = "30%";
	//2D Measurement
	QString LumenArea = "20";
	QString VesselWallArea = "40";
	QString WallThickness = "4.2";
	QString NWI = "0.67";
	//3D Measurement
	QString LumenVolume = "600";
	QString WallVolume = "800";
	QString PlaqueVolume = "200";
	//Plaque Composition
	QString Calcification = "140";
	QString CalcificationPercent = "70%";
	QString Hemorrhage = "30";
	QString HemorrhagePercent = "15%";
	QString LRNC = "10";
	QString LRNCPercent = "5%";

	ReportGenerator* reportGenerator = new ReportGenerator;
	reportGenerator->SetDirectory(FilePath);
	reportGenerator->SetReportName(ReportName);

	//Table
	int headerColor[4] = { 155, 155, 155, 255 };
	reportGenerator->AddTable(0, "Information", QStringList(), QStringList(), 5, 4);
	reportGenerator->SetTableSize(0, 700, 185);
	reportGenerator->SetTableItem(0, 0, 0, "Patient Information");
	reportGenerator->SetTableItemBackgroundColor(0, 0, 0, headerColor);
	reportGenerator->SetTableItemColumnSpan(0, 0, 0, 2);
	reportGenerator->SetTableItemBold(0, 0, 0, true);
	reportGenerator->SetTableItem(0, 0, 2, "MRI Information");
	reportGenerator->SetTableItemBackgroundColor(0, 0, 2, headerColor);
	reportGenerator->SetTableItemColumnSpan(0, 0, 2, 2);
	reportGenerator->SetTableItemBold(0, 0, 2, true);
	reportGenerator->SetTableItem(0, 1, 0, "Name");
	reportGenerator->SetTableItem(0, 1, 1, PatientName);
	reportGenerator->SetTableItem(0, 1, 2, "Scan Date");
	reportGenerator->SetTableItem(0, 1, 3, ScanDate);
	reportGenerator->SetTableItem(0, 2, 0, "ID");
	reportGenerator->SetTableItem(0, 2, 1, PatientID);
	reportGenerator->SetTableItem(0, 2, 2, "Report Date");
	reportGenerator->SetTableItem(0, 2, 3, ReportDate);
	reportGenerator->SetTableItem(0, 3, 0, "DOB");
	reportGenerator->SetTableItem(0, 3, 1, PatientDOB);
	reportGenerator->SetTableItem(0, 3, 2, "L/R Side");
	reportGenerator->SetTableItem(0, 3, 3, MRISide);
	reportGenerator->SetTableItem(0, 4, 0, "Gender");
	reportGenerator->SetTableItem(0, 4, 1, PatientGender);
	reportGenerator->SetTableItem(0, 4, 2, "Doctor's Name");
	reportGenerator->SetTableItem(0, 4, 3, DoctorName);


	reportGenerator->AddTable(1, "Stenosis Measurement", QStringList(), QStringList(), 2, 4);
	reportGenerator->SetTableSize(1, 700, 74);
	reportGenerator->SetTableItem(1, 0, 0, "Stenosis Measurement");
	reportGenerator->SetTableItemBackgroundColor(1, 0, 0, headerColor);
	reportGenerator->SetTableItemColumnSpan(1, 0, 0, 4);
	reportGenerator->SetTableItemBold(1, 0, 0, true);
	reportGenerator->SetTableItem(1, 1, 0, "% stenosis");
	reportGenerator->SetTableItemColumnSpan(1, 1, 0, 2);
	reportGenerator->SetTableItem(1, 1, 2, StenosisPercent);
	reportGenerator->SetTableItemColumnSpan(1, 1, 2, 2);



	reportGenerator->AddTable(2, "2D Measurement", QStringList(), QStringList(), 4, 4);
	reportGenerator->SetTableSize(2, 700, 148);
	reportGenerator->SetTableItem(2, 0, 0, "2D Measurement");
	reportGenerator->SetTableItemBackgroundColor(2, 0, 0, headerColor);
	reportGenerator->SetTableItemColumnSpan(2, 0, 0, 4);
	reportGenerator->SetTableItemBold(2, 0, 0, true);
	reportGenerator->SetTableItem(2, 1, 0, "Lumen Area\n(mm^2)");
	reportGenerator->SetTableItemRowSpan(2, 1, 0, 2);
	reportGenerator->SetTableItem(2, 3, 0, LumenArea);
	reportGenerator->SetTableItemRowSpan(2, 2, 0, 2);
	reportGenerator->SetTableItem(2, 1, 1, "Vessel Wall Area\n(mm^2)");
	reportGenerator->SetTableItemRowSpan(2, 1, 1, 2);
	reportGenerator->SetTableItem(2, 3, 1, VesselWallArea);
	reportGenerator->SetTableItemRowSpan(2, 2, 1, 2);
	reportGenerator->SetTableItem(2, 1, 2, "Wall Thickness\n(mm)");
	reportGenerator->SetTableItemRowSpan(2, 1, 2, 2);
	reportGenerator->SetTableItem(2, 3, 2, WallThickness);
	reportGenerator->SetTableItemRowSpan(2, 2, 2, 2);
	reportGenerator->SetTableItem(2, 1, 3, "Normalized Wall Index\n(NWI)");
	reportGenerator->SetTableItemRowSpan(2, 1, 3, 2);
	reportGenerator->SetTableItem(2, 3, 3, NWI);
	reportGenerator->SetTableItemRowSpan(2, 2, 3, 2);

	reportGenerator->AddTable(3, "3D Measurement", QStringList(), QStringList(), 3, 4);
	reportGenerator->SetTableSize(3, 700, 111);
	reportGenerator->SetTableItem(3, 0, 0, "3D Measurement");
	reportGenerator->SetTableItemBackgroundColor(3, 0, 0, headerColor);
	reportGenerator->SetTableItemColumnSpan(3, 0, 0, 4);
	reportGenerator->SetTableItemBold(3, 0, 0, true);
	reportGenerator->SetTableItem(3, 2, 0, "volume");
	reportGenerator->SetTableItem(3, 1, 1, "Lumen(mm^3)");
	reportGenerator->SetTableItem(3, 2, 1, LumenVolume);

	reportGenerator->SetTableItem(3, 1, 2, "Wall(mm^3)");
	reportGenerator->SetTableItem(3, 2, 2, WallVolume);
	reportGenerator->SetTableItem(3, 1, 3, "Plaque(mm^3)");
	reportGenerator->SetTableItem(3, 2, 3, PlaqueVolume);

	reportGenerator->AddTable(4, "Plaque Composition", QStringList(), QStringList(), 4, 4);
	reportGenerator->SetTableSize(4, 700, 148);
	reportGenerator->SetTableItem(4, 0, 0, "Plaque Composition");
	reportGenerator->SetTableItemBackgroundColor(4, 0, 0, headerColor);
	reportGenerator->SetTableItemColumnSpan(4, 0, 0, 4);
	reportGenerator->SetTableItemBold(4, 0, 0, true);
	reportGenerator->SetTableItem(4, 2, 0, "volume");
	reportGenerator->SetTableItem(4, 2, 1, Calcification);
	reportGenerator->SetTableItem(4, 2, 2, Hemorrhage);
	reportGenerator->SetTableItem(4, 2, 3, LRNC);
	reportGenerator->SetTableItem(4, 3, 1, CalcificationPercent);
	reportGenerator->SetTableItem(4, 3, 2, HemorrhagePercent);
	reportGenerator->SetTableItem(4, 3, 3, LRNCPercent);
	reportGenerator->SetTableItem(4, 3, 0, "% volume");
	reportGenerator->SetTableItem(4, 1, 1, "calcification");
	reportGenerator->SetTableItem(4, 1, 2, "hemorrhage");
	reportGenerator->SetTableItem(4, 1, 3, "lipid-rich necrotic core");

	//Figure

	reportGenerator->AddFigure(0, "2D result", "C:/Users/user/Desktop/Andy/ReportGenerator/ReportGenerator/2dresult.png");
	reportGenerator->AddFigure(1, "3D result", "C:/Users/user/Desktop/Andy/ReportGenerator/ReportGenerator/3dresult.png");
	reportGenerator->AddTable(5, "Images", QStringList(), QStringList(), 10, 4);
	reportGenerator->SetTableSize(5, 700, 350);
	reportGenerator->SetTableItemRowSpan(5, 1, 0, 9);
	reportGenerator->SetTableItemRowSpan(5, 1, 2, 9);
	reportGenerator->SetTableItemColumnSpan(5, 0, 0, 2);
	reportGenerator->SetTableItemColumnSpan(5, 0, 2, 2);
	reportGenerator->SetTableItemColumnSpan(5, 1, 0, 2);
	reportGenerator->SetTableItemColumnSpan(5, 1, 2, 2);
	reportGenerator->SetTableItem(5, 0, 0, "2D result");
	reportGenerator->SetTableItem(5, 0, 2, "3D result");

	reportGenerator->SetTableImageItem(5, 1, 0, 0);
	reportGenerator->SetTableImageItem(5, 1, 2, 1);

	//Update
	reportGenerator->Update();

	QDesktopServices::openUrl(QUrl("C:/Users/user/Desktop/Andy/ReportGenerator/ReportGenerator/result.pdf"));

	delete reportGenerator;

}

