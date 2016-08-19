/*
Author:		Wong, Matthew Lun
Date:		16th, June 2016
Occupation:	Chinese University of Hong Kong,
Department of Imaging and Inteventional Radiology,
Junior Research Assistant

This file is a marcro of implementation of usual functions used in 
style switch.

Wong Matthew Lun
Copyright (C) 2016
*/

#define SetInteractorStyleMacro(style) \
void SetInteractorStyleTo##style () { \
	if (!this->CurrentStyleIs##style ()) { \
		if (this->CurrentStyle) { \
			this->CurrentStyle->SetInteractor(0); \
	} \
		this->CurrentStyle =(vtkInteractorStyle*) this->##style;\
	} \
	if (this->CurrentStyle) { \
		this->CurrentStyle->SetInteractor(this->Interactor);\
		this->CurrentStyle->SetTDxStyle(this->TDxStyle);\
	} \
	this->InternalUpdate();\
}

#define CurrentStyleMacro(style) \
bool CurrentStyleIs##style () {\
	return CurrentStyle == ##style; \
}

#define SetInteractorStyle3DMacro(style) \
void SetInteractorStyleTo3D##style () { \
	if (!this->CurrentStyleIs3D##style ()) { \
		if (this->CurrentStyle) { \
			this->CurrentStyle->SetInteractor(0); \
	} \
		this->CurrentStyle = this->##style;\
	} \
	if (this->CurrentStyle) { \
		this->CurrentStyle->SetInteractor(this->Interactor);\
		this->CurrentStyle->SetTDxStyle(this->TDxStyle);\
	} \
	this->InternalUpdate();\
}

#define CurrentStyle3DMacro(style) \
bool CurrentStyleIs3D##style () {\
	return CurrentStyle == ##style; \
}
