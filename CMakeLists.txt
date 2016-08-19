cmake_minimum_required(VERSION 2.8)
 
PROJECT(PlaqueQuant)

IF(COMMAND CMAKE_POLICY)
	   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)


SET(
  CMAKE_PREFIX_PATH
  "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64"
  )

#ITK
find_package(ITK REQUIRED)
if (NOT ITK_FOUND)
	message(FATAL_ERROR "error: ITK is not found")
endif()
include(${ITK_USE_FILE})

#VTK
find_package(VTK REQUIRED)
if (NOT VTK_FOUND)
	message(FATAL_ERROR "error: VTK is not found")
endif()
include(${VTK_USE_FILE})

#VMTK
find_package(VMTK REQUIRED)
if (NOT VMTK_FOUND)
	message(FATAL_ERROR "error: VMTK is not found")
endif()
include(${VMTK_USE_FILE})

#QT
if(${VTK_VERSION} VERSION_GREATER "6" AND VTK_QT_VERSION VERSION_GREATER "4")
  # Instruct CMake to run moc automatically when needed.
  # set(CMAKE_AUTOMOC ON)
  find_package(Qt5PrintSupport REQUIRED)
  find_package(Qt5Widgets REQUIRED QUIET)
  # add_definitions(-DQT5_IS_LOADED)
else()
  find_package(Qt4 REQUIRED)
  include(${QT_USE_FILE})
endif()


SET(
	MAIN_INCLUDE_DIRS
	${PROJECT_BINARY_DIR}
	${PROJECT_BINARY_DIR}/include
	${PROJECT_BINARY_DIR}/codes/Module
	${PROJECT_BINARY_DIR}/codes/Core
	# AbstractInteractorStyles
	${PROJECT_BINARY_DIR}/codes/AbstractInteractorStyles
	# Source for Widgets
	${PROJECT_BINARY_DIR}/codes/Widgets
	# Source for GPURender
	${PROJECT_BINARY_DIR}/codes/GPURenderWithLUT

	### Directories for headers ###
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/codes/Module
	${PROJECT_SOURCE_DIR}/codes/Core
	# AbstractInteractorStyles
	${PROJECT_SOURCE_DIR}/codes/AbstractInteractorStyles
	# Source for Widgets
	${PROJECT_SOURCE_DIR}/codes/Widgets
	# Source for GPURender
	${PROJECT_SOURCE_DIR}/codes/GPURenderWithLUT
)

SET(
	LIBRARY_OUTPUT_PATH
	${CMAKE_SOURCE_DIR}/lib
	${EXECUTABLE_OUTPUT_PATH}
)

include_directories(
	${MAIN_INCLUDE_DIRS}
)


Add_subdirectory(codes)



