#include "Overlay.h"

#include <itkCastImageFilter.h>
#include <itkVTKImageToImageFilter.h>

#include <vtkImageMagnitude.h>
#include <vtkImageCast.h>
#include <vtkSimpleImageToImageFilter.h>

Overlay::Overlay(QObject* parent) : QObject(parent)
{
	m_itkOverlay = ImageType::New();
	m_vtkOverlay = NULL;
}

Overlay::~Overlay()
{
}

void Overlay::SetInputImageData(vtkImageData* vtkimagedata)
{
	m_vtkOverlay = vtkimagedata;
}

void Overlay::SetInputImageData(FloatImageType::Pointer imageData)
{
	//re-orient
	OrienterType::Pointer orienter = OrienterType::New();
	orienter->UseImageDirectionOn();
	orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
	orienter->SetInput(imageData);
	orienter->Update();

	ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(orienter->GetOutput());
	connector->Update();

	SetInputImageData(connector->GetOutput());
}

void Overlay::SetInputImageData(QString fileName)
{
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(fileName.toStdString().c_str());
	reader->Update();
	ImageType::Pointer image = reader->GetOutput();

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
		return;
	}
	vtkSmartPointer<vtkImageCast> castFilter =
		vtkSmartPointer<vtkImageCast>::New();
	castFilter->SetInputData(connector->GetOutput());
	castFilter->SetOutputScalarTypeToDouble();
	castFilter->Update();
	
	this->m_vtkOverlay->DeepCopy(castFilter->GetOutput());
}	

void Overlay::SetVisibleImageNo(int n)
{
	m_visible_image_no = n;
}

void Overlay::Initialize(ImageType::Pointer itkinputimage, int dim[3], double spacing[3], double origin[3], int type)
{
	DuplicatorType::Pointer duplicator = DuplicatorType::New();
	duplicator->SetInputImage(itkinputimage);
	try
	{
		duplicator->Update();
	}
	catch (itk::ExceptionObject & err)
	{
		std::cerr << "ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
		return; // Since the goal of the example is to catch the exception, we declare this a success.
	}
	ImageType::Pointer m_itkOverlay = duplicator->GetOutput();

	itk::ImageRegionIterator<ImageType> itr(m_itkOverlay, m_itkOverlay->GetLargestPossibleRegion());

	//Set all pixel to zero
	while (!itr.IsAtEnd())
	{
		itr.Set(0);
		++itr;
	}

	ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(m_itkOverlay);
	try
	{
		connector->Update();
	}
	catch (itk::ExceptionObject &err)
	{
		std::cerr << err << std::endl;
	}

	m_vtkOverlay = vtkImageData::New();
	m_vtkOverlay->DeepCopy(connector->GetOutput());
	//const int numofElements = dim[0] * dim[1] * dim[2];
	//m_vtkOverlay->SetDimensions(dim);
	//m_vtkOverlay->SetOrigin(origin);
	//m_vtkOverlay->SetSpacing(spacing);
	//m_vtkOverlay->AllocateScalars(VTK_DOUBLE, 1);

	//for (int i = 0; i < dim[0]; i++)
	//{
	//	for (int j = 0; j < dim[1]; j++)
	//	{
	//		for (int k = 0; k < dim[2]; k++)
	//		{
	//			float* pixel = static_cast<float*>(this->m_vtkOverlay->GetScalarPointer(i, j, k));
	//			*pixel = 0.;
	//		}
	//	}
	//}
}

void Overlay::SetDisplayExtent(int *extent)
{
	memcpy(this->DisplayExtent, extent, sizeof(int) * 6);
}

int * Overlay::GetDisplayExtent()
{
	return this->DisplayExtent;
}

void Overlay::GetDisplayExtent(int *extent)
{
	memcpy(extent, this->DisplayExtent, sizeof(int) * 6);
}

void Overlay::DisplayExtentOn()
{
	
}

void Overlay::DisplayExtentOff()
{
	this->m_vtkOverlay->GetExtent(this->DisplayExtent);
}

void Overlay::SetPixel(int pos[3], double value)
{
		double * pixel = static_cast<double *>(m_vtkOverlay->GetScalarPointer(pos[0], pos[1], pos[2]));
		if (pixel == nullptr) {
			return;
		}
		pixel[0] = value;
		//cout << pixel[0] << endl;
}

void Overlay::SetPixel(int pos[3], unsigned char value)
{
	unsigned char * pixel = static_cast<uchar *>(m_vtkOverlay->GetScalarPointer(pos[0], pos[1], pos[2]));
	if (pixel == nullptr)
		return;
	pixel[0] = value;
	//cout << pixel[0] << endl;
}
bool Overlay::Update()
{
	if (!m_vtkOverlay)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

void Overlay::Initialize(vtkImageData * img)
{
	m_vtkOverlay = vtkImageData::New();
	m_vtkOverlay->DeepCopy(img);
	
	vtkSmartPointer<vtkImageCast> castfilter = vtkSmartPointer<vtkImageCast>::New();
	castfilter->SetInputData(m_vtkOverlay);
	castfilter->SetOutputScalarTypeToDouble();
	castfilter->Update();

	m_vtkOverlay->DeepCopy(castfilter->GetOutput());
	//m_vtkOverlay->SetDimensions(img->GetDimensions());
	//m_vtkOverlay->SetSpacing(img->GetSpacing());
	//m_vtkOverlay->SetOrigin(img->GetOrigin());
	////cout << img->GetScalarTypeAsString() << endl;
	//m_vtkOverlay->AllocateScalars(VTK_DOUBLE, img->GetNumberOfScalarComponents());
	int dim[3];
	m_vtkOverlay->GetDimensions(dim);


	for (int i = 0; i < dim[0]; i++)
	{
		for (int j = 0; j < dim[1]; j++)
		{
			for (int k = 0; k < dim[2]; k++)
			{
				double* pixel = static_cast<double*>(this->m_vtkOverlay->GetScalarPointer(i, j, k));
				*pixel = 0;
			}
		}
	}

	this->m_vtkOverlay->GetExtent(this->DisplayExtent);
}


vtkImageData* Overlay::GetOutput()
{
	if (!m_vtkOverlay)
		return NULL;
	else
		return this->m_vtkOverlay;
}

vtkImageData * Overlay::GetVTKImageData()
{
	if (!m_vtkOverlay)
		return NULL;
	else
		return this->m_vtkOverlay;
}

//FloatImageType::Pointer Overlay::GetITKOutput(ImageType format)
//{
//	return FloatImageType::Pointer();
//}

ImageType::Pointer Overlay::GetITKOutput(ImageType::Pointer format)
{

	//if (!m_vtkOverlay)
	//	return NULL;
	//else {

		itk::VTKImageToImageFilter<itk::Image<double, 3>>::Pointer vtkImageToImageFilter =
			itk::VTKImageToImageFilter<itk::Image<double, 3>>::New();
		vtkImageToImageFilter->SetInput(this->GetOutput());
		vtkImageToImageFilter->Update();

		itk::CastImageFilter< itk::Image<double, 3>, itk::Image<float, 3> >::Pointer castImageFilter =
			itk::CastImageFilter< itk::Image<double, 3>, itk::Image<float, 3> >::New();
		castImageFilter->SetInput(vtkImageToImageFilter->GetOutput());
		castImageFilter->Update();

		m_duplicator = DuplicatorType::New();
		m_duplicator->SetInputImage(castImageFilter->GetOutput());
		m_duplicator->Update();
		m_duplicator->GetOutput()->SetDirection(format->GetDirection());

		m_itkOverlay->Graft(m_duplicator->GetOutput());
		cout << __FUNCSIG__ << endl;
		cout << "m_vtkOverlay" << endl;
		m_vtkOverlay->Print(cerr);
		cout << "m_itkOverlay" << endl;
		cout << m_itkOverlay << endl;

	//}
	return m_itkOverlay;

}