#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include "Define.h"
class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = 0);
    void run();
	void forceStop();

	//Set Input
	void SetInputImage(ImageType::Pointer image);
	void SetROI(double* roi);
	void SetIndex(int* index);
	ImageType::Pointer GetOutputImage();

signals:
	void finished(bool);

private:
	//Parameter    
	bool m_stop;

	ImageType::Pointer m_inputImage;
	ImageType::Pointer m_outputImage;
	double* m_roiBound;
	int* m_index;
};

#endif // MYTHREAD_H
