cmake_minimum_required(VERSION 2.8)

project(Core)
include_directories(
	${PROJECT_BINARY_DIR}
	${MAIN_INCLUDE_DIRS})

message(${MAIN_INCLUDE_DIRS})
 # message("${PROJECT_BINARY_DIR}")
IF(COMMAND CMAKE_POLICY)
	   CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

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
	QT_SRC
	${PROJECT_SOURCE_DIR}/main.cxx
	${PROJECT_SOURCE_DIR}/MainWindow.cxx
	${PROJECT_SOURCE_DIR}/InfoDialog.cxx
	${PROJECT_SOURCE_DIR}/MyProgressDialog.cxx
	${PROJECT_SOURCE_DIR}/RegistrationWizard.cxx
	${PROJECT_SOURCE_DIR}/ImageRegistration.cxx
	${PROJECT_SOURCE_DIR}/Overlay.cxx
	${PROJECT_SOURCE_DIR}/SurfaceCreator.cxx
	${PROJECT_SOURCE_DIR}/AbstractFilter.cxx
	${PROJECT_SOURCE_DIR}/ReportGenerator.cxx
	${PROJECT_SOURCE_DIR}/ExtractCenterline.cpp
	${PROJECT_SOURCE_DIR}/VesselSegmentation.cpp

)

SET(
	QT_INC
	${PROJECT_SOURCE_DIR}/MainWindow.h
	${PROJECT_SOURCE_DIR}/InfoDialog.h
	${PROJECT_SOURCE_DIR}/MyProgressDialog.h
	${PROJECT_SOURCE_DIR}/RegistrationWizard.h
	${PROJECT_SOURCE_DIR}/ImageRegistration.h
	${PROJECT_SOURCE_DIR}/Overlay.h
	${PROJECT_SOURCE_DIR}/SurfaceCreator.h
	${PROJECT_SOURCE_DIR}/AbstractFilter.h
	${PROJECT_SOURCE_DIR}/ReportGenerator.h
	${PROJECT_SOURCE_DIR}/ExtractCenterline.h
	${PROJECT_SOURCE_DIR}/VesselSegmentation.h
)

SET(
	QT_UI
	${PROJECT_SOURCE_DIR}/MainWindow.ui
	${PROJECT_SOURCE_DIR}/InfoDialog.ui
)

SET(
	CORE_SRC
	${PROJECT_SOURCE_DIR}/MyImageViewer.cxx
	# ${CMAKE_SOURCE_DIR}/codes/core/MyVtkInteractorStyleImage.cxx
	${PROJECT_SOURCE_DIR}/MyPlaneWidget.cxx
)

SET(
	CORE_INC
	${PROJECT_SOURCE_DIR}/MyImageViewer.h
	  # ${CMAKE_SOURCE_DIR}/codes/core/MyVtkInteractorStyleImage.h
	${PROJECT_SOURCE_DIR}/MyPlaneWidget.h
	# ${CMAKE_SOURCE_DIR}/include/Define.h

)

Set(
	QT_RSRS 
	${CMAKE_SOURCE_DIR}/resources/Project.qrc
)

Set(
	RC_FILE
	${CMAKE_SOURCE_DIR}/resources/Project.rc
)
#Generate moc and ui file

QT5_WRAP_UI(UISrcs ${QT_UI})
QT5_WRAP_CPP(MOCSrcs ${QT_INC})
QT5_ADD_RESOURCES(RCC_RSRS ${QT_RSRS})

add_executable(
	Core
	MACOSX_BUNDLE 
	${QT_WRAP} 
	${UISrcs} 
	${MOCSrcs}
	${QT_SRC}
	${QT_INC}
	${CORE_INC}
	${CORE_SRC}
	${RCC_RSRS}
	${RC_FILE}
)

#Link all libraries
link_directories(${CMAKE_SOURCE_DIR}/lib)

target_link_libraries(
	Core
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets 
	Qt5::PrintSupport
	${VTK_LIBRARIES} 
	${VMTK_LIBRARIES} 
	${ITK_LIBRARIES} 
	Module
	InteractorStyles
	Widgets 
	GPURenderWithLUT
)


#Grouping in MSVC
SOURCE_GROUP(
	"Generated FIles" 
	FILES
	${RCC_RSRS}
	${UISrcs}
	${MOCSrcs}
)
SOURCE_GROUP(
	"UI FILES" 
	FILES 
	${QT_UI}
)
SOURCE_GROUP(
	"Header Files" 
	FILES 
	${QT_INC}
	${CORE_INC}
)
SOURCE_GROUP(
	"Source Files" 
	FILES 
	${QT_SRC}
	${CORE_SRC}
)
