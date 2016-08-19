Project(GPURenderWithLUT)
#VTK
find_package(VTK REQUIRED)
if (NOT VTK_FOUND)
	message(FATAL_ERROR "error: VTK is not found")
endif()
include(${VTK_USE_FILE})

SET(
	LIBRARY_OUTPUT_PATH
	${CMAKE_SOURCE_DIR}/lib
	${EXECUTABLE_OUTPUT_PATH}
)
SET(
	GPURENDER_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/GPUVolumeRenderingFilter.cpp
	)
SET(
	GPURENDER_INC
	${CMAKE_CURRENT_SOURCE_DIR}/GPUVolumeRenderingFilter.h
	)

add_library(
	GPURenderWithLUT
	${GPURENDER_INC}
	${GPURENDER_SRC}
	)

target_link_libraries(
	GPURenderWithLUT
	${VTK_LIBRARIES}
)

source_group(
	"Header Files" 
	FILES 
	${GPURENDER_INC}
	)
source_group(
	"Source Files"
	FILES
	${GPURENDER_SRC}
	)