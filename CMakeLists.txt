project(easyai_sample_mark)

cmake_minimum_required(VERSION 3.12)

#project setting
project(easyai_sample_mark LANGUAGES C CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
SET(CMAKE_INCLUDE_SYSTEM_FLAG_C "-I")
SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-I" CACHE STRING "" FORCE)

#include dependencies
include(cmake/dependencies.cmake)

#include_directories
include_directories(${PROJECT_BINARY_DIR})
message(STATUS "PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

set(CMAKE_AUTOMOC ON) # For meta object compiler
set(CMAKE_AUTORCC ON) # Resource files
set(CMAKE_AUTOUIC ON) # UI files
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_SOURCE_DIR})
message(STATUS "easyai_sample_mark: ${PROJECT_SOURCE_DIR}")

#collect source files
file(GLOB QT_SRCS
	"${PROJECT_SOURCE_DIR}/autoSampleMark/*.cpp"
	"${PROJECT_SOURCE_DIR}/baseAlgorithm/*.cpp"
	"${PROJECT_SOURCE_DIR}/dataType/*.cpp"
	"${PROJECT_SOURCE_DIR}/deepLearning/*.cpp"
	"${PROJECT_SOURCE_DIR}/drawShape/*.cpp"
	"${PROJECT_SOURCE_DIR}/drawWidget/*.cpp"
	"${PROJECT_SOURCE_DIR}/helpers/*.cpp"
	"${PROJECT_SOURCE_DIR}/multipletracking/*.cpp"
    "${PROJECT_SOURCE_DIR}/multipletracking/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/multipletracking/*/*/*.cpp"
    "${PROJECT_SOURCE_DIR}/paramWindow/*.cpp"
    "${PROJECT_SOURCE_DIR}/pcTools/*.cpp"
    "${PROJECT_SOURCE_DIR}/sampleMarkParam/*.cpp"
    "${PROJECT_SOURCE_DIR}/saveData/*.cpp"
    "${PROJECT_SOURCE_DIR}/saveMarkData/*.cpp"
    "${PROJECT_SOURCE_DIR}/utilityGUI/customWindow/*.cpp"
    "${PROJECT_SOURCE_DIR}/utilityGUI/QtAwesome/*.cpp"
    "${PROJECT_SOURCE_DIR}/videoTools/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.cpp"
	)
# file(GLOB_RECURSE QT_SRCS "${PROJECT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE QT_HDRS "${PROJECT_SOURCE_DIR}/*.h")
file(GLOB QT_UIS "${PROJECT_SOURCE_DIR}/*.ui")
file(GLOB QT_QRSCS "${PROJECT_SOURCE_DIR}/*.qrc")

qt5_wrap_cpp(MOC ${QT_HDRS})
qt5_wrap_ui(UI_HDRS ${QT_UIS})
qt5_add_resources(RCC ${QT_QRSCS})

add_definitions(
    ${Qt5Core_DEFINITIONS}
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS}
)

add_executable(${PROJECT_NAME} ${QT_SRCS} ${QT_HDRS} ${MOC} ${UI_HDRS} ${RCC})
target_link_libraries(${PROJECT_NAME}
	${Boost_LIBRARIES}
	${OpenCV_LIBRARIES}
	${VTK_LIBRARIES}
	${PCL_LIBRARIES}
    Qt5::Widgets
    Qt5::Core
	Qt5::Gui
    Qt5::Xml
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::OpenGL
    pthread
    GL
)