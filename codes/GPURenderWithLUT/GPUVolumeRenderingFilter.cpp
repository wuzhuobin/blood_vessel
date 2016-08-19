#include <vtkSmartPointer.h>
#include <vtkImageCast.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include "GPUVolumeRenderingFilter.h"

vtkStandardNewMacro(GPUVolumeRenderingFilter);

GPUVolumeRenderingFilter::GPUVolumeRenderingFilter()
{
	this->GPUVolumeMapper		= vtkGPUVolumeRayCastMapper::New();
	this->Volume				= vtkVolume::New();
	this->VolumeProperty		= vtkVolumeProperty::New();
	this->LookUpTable = NULL;
}

GPUVolumeRenderingFilter::~GPUVolumeRenderingFilter()
{
	
	if (this->Volume) {
		this->Volume->Delete();
		this->Volume = NULL;
	}
	if (this->GPUVolumeMapper) {
		this->GPUVolumeMapper->Delete();
		this->GPUVolumeMapper = NULL;
	}

	if (this->VolumeProperty) {
		this->VolumeProperty->Delete();
		this->VolumeProperty = NULL;
	}


}

void GPUVolumeRenderingFilter::SetInputData(vtkImageData *dat)
{
	this->InputData = dat;
	if (dat->GetScalarType() != VTK_UNSIGNED_INT) {
		vtkSmartPointer<vtkImageCast> castFilter = vtkSmartPointer<vtkImageCast>::New();
		castFilter->SetInputData(dat);
		castFilter->SetOutputScalarTypeToUnsignedInt();
		castFilter->Update();
		this->GPUVolumeMapper->SetInputData(castFilter->GetOutput());
	}
	else {
		this->GPUVolumeMapper->SetInputData(dat);
	}
}

void GPUVolumeRenderingFilter::Update()
{
	if (!this->LookUpTable) {
		vtkErrorMacro("No Input Lookup Table!");
		return;
	}

	if (!this->InputData) {
		vtkErrorMacro("No Input Data!");
		return;
	}
	//double *range = this->LookUpTable->GetRange();
	double range[2] = { 0.0, 6.0 };
	int numberOfValues = this->LookUpTable->GetNumberOfTableValues();

	cout << "Range: " << range[0] << " " << range[1] << endl;

	vtkSmartPointer<vtkColorTransferFunction>  color = vtkSmartPointer<vtkColorTransferFunction>::New();
	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	for (int i = 0; i < numberOfValues; i++)
	{
		cout << "Val: " << i / double(numberOfValues - 1) * (range[1] - range[0]) + range[0] << endl;
		double *rgba = this->LookUpTable->GetTableValue(i);
		compositeOpacity->AddPoint(double(i/double(numberOfValues - 1) * (range[1] - range[0]) + range[0]), rgba[3]);
		color->AddRGBPoint(double(i / double(numberOfValues - 1) * (range[1] - range[0]) + range[0]), rgba[0], rgba[1], rgba[2]);
	}

	compositeOpacity->AddPoint(double(range[1]) + 0.1, 0);
	
	this->VolumeProperty->SetScalarOpacity(compositeOpacity);
	this->VolumeProperty->SetColor(color);

	this->Volume->SetMapper(this->GPUVolumeMapper);
	this->Volume->SetProperty(this->VolumeProperty);
}
