/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
Junior Research Assistant

Author:		Lok, Ka Hei Jason
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
M.Phil Student

This class allows interaction related to reslice position and orientation on images.

Wong Matthew Lun, Lok Ka Hei
Copyright (C) 2016
*/

#ifndef INTERACTOR_STYLE_NAVIGATION_H
#define INTERACTOR_STYLE_NAVIGATION_H

#include "AbstractInteractorStyleImage.h"


class InteractorStyleNavigation : public AbstractInteractorStyleImage
{
public:
	vtkTypeMacro(InteractorStyleNavigation, AbstractInteractorStyleImage);
	static InteractorStyleNavigation* New();

protected:
	InteractorStyleNavigation();
	~InteractorStyleNavigation();

	virtual void OnMouseMove();
	virtual void OnLeftButtonUp();

private:
	void MoveSliceForward();
	void MoveSliceBackward();


	void SynchronizedZooming();
	void CalculateIndex();
	
};


#endif // INTERACTOR_STYLE_NAVIGATION_H