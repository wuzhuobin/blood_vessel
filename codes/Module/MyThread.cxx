#include "mythread.h"
#include "MainWindow.h"
#include <QMutex>
#include <QDebug>

MyThread::MyThread(QObject *parent) : QThread(parent)
{
	//Default setting
	m_stop = false;
	m_inputImage = NULL;
	m_outputImage = NULL;

}
void MyThread::run()
{
	//Reset stop boolean
	m_stop = false;

    QMutex mutex;
    mutex.lock();

    if(this->m_stop)
		return;


	//Update the output image

    mutex.unlock();
	emit finished(true);
}

void MyThread::forceStop()
{
	m_stop = true;
}

void MyThread::SetInputImage(ImageType::Pointer image)
{
	m_inputImage = image;
}

void MyThread::SetROI( double* roi )
{
	m_roiBound = roi;
}

void MyThread::SetIndex( int * index )
{
	m_index = index;
}

ImageType::Pointer MyThread::GetOutputImage()
{
	return m_outputImage;
}
