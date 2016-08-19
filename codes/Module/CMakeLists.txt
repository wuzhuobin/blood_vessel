
project(Module)
SET(
  CMAKE_PREFIX_PATH
  "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x64"
  )


#ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

#VTK
find_package(VTK REQUIRED)
if (NOT VTK_FOUND)
	message(FATAL_ERROR "error: VTK is not found")
endif()
include(${VTK_USE_FILE})

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

#link other libraries
link_directories(
	${PROJECT_SOURCE_DIR}/lib
	)

include_directories(
	${PROJECT_BINARY_DIR}
	${MAIN_INCLUDE_DIRS}
	)

SET(
	LIBRARY_OUTPUT_PATH
	${CMAKE_SOURCE_DIR}/lib
	${EXECUTABLE_OUTPUT_PATH}
)

# SET(
	# MODULE_INC_LIST
	# ${CMAKE_SOURCE_DIR}/include/Define.h
 # )

SET(
	MODULE_SRC_LIST
)

SET(MODULE_QT_INC 
	${PROJECT_SOURCE_DIR}/ModuleWidget.h
	${PROJECT_SOURCE_DIR}/MyThread.h
)

SET(MODULE_QT_SRC 
	${PROJECT_SOURCE_DIR}/ModuleWidget.cxx
	${PROJECT_SOURCE_DIR}/MyThread.cxx
)

SET(MODULE_QT_UI 
	${PROJECT_SOURCE_DIR}/ModuleWidget.ui
)

#Generate moc and ui file
QT5_WRAP_UI(UISrcs ${MODULE_QT_UI})
QT5_WRAP_CPP(MOCSrcs ${MODULE_QT_INC})

add_library(
	Module 
	${MODULE_SRC_LIST}
	${MODULE_INC_LIST}
	${UISrcs}
	${MOCSrcs} 
	${MODULE_QT_SRC} 
	${MODULE_QT_INC}
	)

#link libraries
target_link_libraries(
	Module
	${ITK_LIBRARIES}  
	${VTK_LIBRARIES}
	Qt5::Widgets 
	Qt5::PrintSupport
	)

#Grouping in MSVC
SOURCE_GROUP(
	"Generated Files"
	FILES 
	${UISrcs}
	${MOCSrcs}
	)
SOURCE_GROUP(
	"UI FILES" 	
	FILES 
	${MODULE_QT_UI}
	)
SOURCE_GROUP(
	"Header Files" 
	FILES 
	${MODULE_QT_INC}
	${MODULE_INC_LIST}
	)
SOURCE_GROUP(
	"Source Files" 
	FILES 
	${MODULE_QT_SRC}
	${MODULE_SRC_LIST}
	)

