/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
Junior Research Assistant


The abstract interactor class used in medical viewers.
This class is the ultimate parent of all interactor classes.


Wong Matthew Lun
Copyright (C) 2016
*/

#include "AbstractInteractorStyle.h"

std::list<AbstractInteractorStyle*> AbstractInteractorStyle::m_abstractInteractorStyles;

AbstractInteractorStyle::AbstractInteractorStyle() {
	//m_rightFunctioning = false;
	//m_leftFunctioning = false;
	//m_middleFunctioning = false;
	m_abstractInteractorStyles.push_back(this);
}

AbstractInteractorStyle::~AbstractInteractorStyle() {
	m_abstractInteractorStyles.remove(this);
}


//int AbstractInteractorStyle::GetOrientation()
//{
//	return m_orientation;
//}
//
//void AbstractInteractorStyle::SetOrientation(int i)
//{
//	m_orientation = i;
//}

void AbstractInteractorStyle::OnLeftButtonDown()
{
	m_leftFunctioning = true;
}

void AbstractInteractorStyle::OnRightButtonDown()
{
	m_rightFunctioning = true;
}

void AbstractInteractorStyle::OnMiddleButtonDown()
{
	m_middleFunctioning = true;
}

void AbstractInteractorStyle::OnLeftButtonUp()
{
	m_leftFunctioning = false;
	m_leftDoubleClick = false;
}

void AbstractInteractorStyle::OnRightButtonUp()
{
	m_rightFunctioning = false;
	m_rightDoubleClick = false;
}

void AbstractInteractorStyle::OnMiddleButtonUp()
{
	m_middleFunctioning = false;
	m_middleDoubleClick = false;
}


//vtkActor * AbstractInteractorStyle::PickActor(int x, int y)
//{
//	return NULL;
//}