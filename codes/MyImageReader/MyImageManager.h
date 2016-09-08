#ifndef __MYIMAGEMANAGER_H__
#define __MYIMAGEMANAGER_H__

#include <QObject>
#include <QList>
#include <QStringList>
#include <QMap>

#include <vtkImageData.h>
#include <vtkSmartPointer.h>

#include <itkImage.h>

#include "Overlay.h"


class MyImageManager : public QObject
{
	Q_OBJECT
public:
	MyImageManager(QObject* parent = NULL);
	~MyImageManager();

	/**
	 * load Images whose path are listOfFileNames
	 * @return	true load images successfully
	 *			false load images fail
	 */
	int getNumberOfImages();
	QList<vtkSmartPointer<vtkImageData>> getListOfVtkImages();
	const QList<vtkSmartPointer<vtkImageData>> getListOfVtkOriginalImages();
	const QList<itk::Image<float, 3>::Pointer> getListOfItkImages();
	const QList<QMap<QString, QString>*> getListOfDICOMHeader();
	

	const QMap<QString, itk::Image<float, 3>::Pointer> getMapOfItkImages();
	const QMap<QString, vtkSmartPointer<vtkImageData>> getMapOfVtkImages();
	const QMap<QString, QString>* getDICOMHeader(itk::Image<float, 3>::Pointer itkImage);
	Overlay& getOverlay();

private:
	friend class IOManager;
	
	Overlay overlay;
	
	// QList
	QList<itk::Image<float, 3>::Pointer> listOfItkImages;
	QList<vtkSmartPointer<vtkImageData>> listOfVtkImages;
	QList<vtkSmartPointer<vtkImageData>> listOfVtkOriginalImages;
	QList<QMap<QString, QString>*> listOfDICOMHeader;

	// QMap
	QMap<QString, itk::Image<float, 3>::Pointer> mapOfItkImages;
	QMap<QString, vtkSmartPointer<vtkImageData>> mapOfVtkImages;
	QMap<itk::Image<float, 3>::Pointer, QMap<QString, QString>*> mapOfDICOMHeader;

};

#endif // !__MYIMAGEMANAGER_H__