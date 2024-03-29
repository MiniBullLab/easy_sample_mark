
set(OpenCV_DIR /usr/local/share/OpenCV)
find_package(OpenCV REQUIRED CONFIG)
message(STATUS "Found OpenCV ${OpenCV_VERSION}")
message(STATUS "${OpenCV_INCLUDE_DIRS}, ${OpenCV_LIBRARIES}")
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIBRARIES})

find_package(Boost COMPONENTS system filesystem thread date_time iostreams serialization chrono REQUIRED)
message(STATUS "Found Boost ${Boost_VERSION}")
message(STATUS "${Boost_INCLUDE_DIRS}, ${Boost_LIBRARY_DIRS}")
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN_INCLUDE_DIRS})

set(VTK_DIR /usr/local/lib/cmake/vtk-8.1)
find_package(VTK REQUIRED CONFIG)
include(${VTK_USE_FILE})
message(STATUS "Found VTK ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${VTK_BUILD_VERSION}")
message(STATUS "${VTK_INCLUDE_DIRS}, ${VTK_LIBRARIES}")
include_directories(${VTK_INCLUDE_DIRS})
link_directories(${VTK_LIBRARIES})

set(PCL_DIR /usr/local/share/pcl-1.9)
find_package(PCL REQUIRED COMPONENTS common io visualization CONFIG)
message(STATUS "Found PCL ${PCL_VERSION}")
message(STATUS "${PCL_INCLUDE_DIRS}, ${PCL_LIBRARIES}")
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARIES})
# Fix a compilation bug under ubuntu 16.04 (Xenial)
list(REMOVE_ITEM PCL_LIBRARIES "vtkproj4")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
message(STATUS "${OPENGL_INCLUDE_DIR}, ${GLEW_INCLUDE_DIR}")
message(STATUS "${OPENGL_LIBRARIES} , ${GLEW_LIBRARIES}")
include_directories(${OPENGL_INCLUDE_DIR})

set(Qt5_DIR /opt/qt512/lib/cmake/Qt5)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL Xml Multimedia MultimediaWidgets)
message(STATUS "Found Qt5: ${Qt5_VERSION}")
#message(STATUS "Qt5Core: ${Qt5Core_INCLUDE_DIRS}, ${Qt5Core_LIBRARIES}")
#include_directories(${Qt5Core_INCLUDE_DIRS})
#message(STATUS "Qt5Widgets: ${Qt5Widgets_INCLUDE_DIRS}, ${Qt5Widgets_LIBRARIES}")
#include_directories(${Qt5Widgets_INCLUDE_DIRS})
#message(STATUS "Qt5Gui: ${Qt5Gui_INCLUDE_DIRS}, ${Qt5Gui_LIBRARIES}")
#include_directories(${Qt5Gui_INCLUDE_DIRS})
