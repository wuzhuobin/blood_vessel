#include "Overlay.h"

#include <itkCastImageFilter.h>
#include <vtkImageCast.h>

Overlay::Overlay(QObject* parent) : QObject(parent)
{
	m_itkOverlay = NULL;
	m_vtkOverlay = NULL;
}

Overlay::~Overlay()
{
}

void Overlay::SetInputImageData(vtkImageData* vtkimagedata)
{
	m_vtkOverlay = vtkimagedata;
}


void Overlay::SetInputImageData(QString fileName)
{
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName(fileName.toStdString().c_str());
	reader->Update();

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
	ReaderType::Pointer reader = ReaderType::New();
	reader->SetFileName("C:/Users/lwong/Source/datacenter/segmentation_right.nii");
	reader->Update();

	//re-orient
	OrienterType::Pointer orienter = OrienterType::New();
	orienter->UseImageDirectionOn();
	orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);
	orienter->SetInput(reader->GetOutput());
	orienter->Update();

	ConnectorType::Pointer connector = ConnectorType::New();
	connector->SetInput(orienter->GetOutput());
	connector->Update();


	m_vtkOverlay = vtkImageData::New();
	m_vtkOverlay->DeepCopy(connector->GetOutput());
	
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

}


vtkImageData* Overlay::GetOutput()
{
	if (!m_vtkOverlay)
		return NULL;
	else
	return m_vtkOverlay;
}

ImageType::Pointer Overlay::GetITKOutput()
{
	if (!m_itkOverlay)
		return NULL;
	else
		return m_itkOverlay;

}
