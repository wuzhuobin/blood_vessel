#ifndef GPU_VOLUME_RENDERING_FILTER_H
#define GPU_VOLUME_RENDERING_FILTER_H

#include <vtkImageData.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkImageData.h>

class GPUVolumeRenderingFilter : vtkObject
{
public:
	static GPUVolumeRenderingFilter* New();
	vtkTypeMacro(GPUVolumeRenderingFilter, vtkObject);

	GPUVolumeRenderingFilter();
	~GPUVolumeRenderingFilter();

	vtkSetMacro(LookUpTable, vtkLookupTable*);
	vtkGetMacro(LookUpTable, vtkLookupTable*);

	vtkSetMacro(Volume, vtkVolume*);
	vtkGetMacro(Volume, vtkVolume*);

	vtkSetMacro(VolumeProperty, vtkVolumeProperty*);
	vtkGetMacro(VolumeProperty, vtkVolumeProperty*);

	void SetInputData(vtkImageData*);
	vtkGetMacro(InputData, vtkImageData*);

	void Update();


private:
	vtkGPUVolumeRayCastMapper*			GPUVolumeMapper;
	vtkLookupTable*						LookUpTable;
	vtkVolume*							Volume;
	vtkVolumeProperty*					VolumeProperty;
	vtkImageData*						InputData;


};



#endif