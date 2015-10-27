cmake_minimum_required (VERSION 3.0.2)

#=======================================================================#

#Opencv modules
set(OPENCV_MODULES_SIMPLE_VIEWER "opencv_core" "opencv_highgui" "opencv_imgproc" "opencv_imgcodecs")
set(OPENCV_MODULES_WININSTALLER)

#Qt modules
set(QT_MODULES_SIMPLE_VIEWER "Qt5::Core" "Qt5::Gui" "Qt5::Widgets")
set(QT_MODULES_WININSTALLER "Qt5::Core" "Qt5::Gui" "Qt5::Widgets")
#these modules will only be linked on windows
#set(QT_MODULES_SIMPLE_VIEWER_WIN "Qt5::WinExtras")

#=======================================================================#

set(PROJECT_NAME SimpleViewer)
set(EXECUTABLE_NAME_1 SimpleViewer)
set(EXECUTABLE_NAME_2 WinInstaller)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Macros)

project(${PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SOURCEFILES_SIMPLE_VIEWER "./SimpleViewer/*.h"  "./SimpleViewer/*.hpp"  "./SimpleViewer/*.cpp")
file(GLOB SOURCEFILES_WININSTALLER "./WinInstaller/*.h"  "./WinInstaller/*.hpp"  "./WinInstaller/*.cpp")

if(WIN32)
	#add application icon ressource
	list(APPEND SOURCEFILES_SIMPLE_VIEWER "${CMAKE_CURRENT_SOURCE_DIR}/SimpleViewer/main.rc")
	list(APPEND SOURCEFILES_WININSTALLER "${CMAKE_CURRENT_SOURCE_DIR}/WinInstaller/main.rc")
endif()

#OpenCV
set(OpenCV_STATIC OFF)
set(OPENCV_MODULES_ALL)
list(APPEND OPENCV_MODULES_ALL ${OPENCV_MODULES_SIMPLE_VIEWER} ${OPENCV_MODULES_WININSTALLER})
list(REMOVE_DUPLICATES OPENCV_MODULES_ALL)
set(OPENCV_FILES "OpenCVConfig.cmake")
set(OPENCV_PATH_HINTS "." "./build" "./.." "./../.." "./../../.." "./share/OpenCV")
ask_for_path("OpenCV" "" "/usr/local" OPENCV_FILES OPENCV_PATH_HINTS)
if(OPENCV_ROOT_FOUND)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PATH_OPENCV_ROOT})
	find_package(OpenCV 3.0 REQUIRED)
endif()

#Qt
set(QT_MODULES_ALL)
list(APPEND QT_MODULES_ALL ${QT_MODULES_SIMPLE_VIEWER} ${QT_MODULES_WININSTALLER})
if(WIN32)
	list(APPEND QT_MODULES_ALL ${QT_MODULES_SIMPLE_VIEWER_WIN})
endif()
list(REMOVE_DUPLICATES QT_MODULES_ALL)
set(QT_FILES "lib/cmake/Qt5/Qt5Config.cmake")
set(QT_PATH_HINTS "." "./../../.." "./../.." "./..")
ask_for_path("Qt" "" "~/Qt5.5.0/5.5/gcc_64" QT_FILES QT_PATH_HINTS)
if(QT_ROOT_FOUND)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PATH_QT_ROOT})
	foreach(module ${QT_MODULES_ALL})
		string(REGEX REPLACE "::" "" MODULE_NAME ${module})
		find_package(${MODULE_NAME} 5.5 REQUIRED)
	endforeach(module ${QT_MODULES_ALL})
endif(QT_ROOT_FOUND)

#Exiv2
set(EXIV_FILES "/include/exiv2.hpp" "/bin/vc12/x64/Release/libexiv2.dll")
set(EXIV_PATH_HINTS "." ".." "../../../..")
ask_for_path("Exiv" "" "/usr/local/lib" EXIV_FILES EXIV_PATH_HINTS)
if(EXIV_ROOT_FOUND)
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${PATH_EXIV_ROOT}" "${PATH_EXIV_ROOT}/include" "${PATH_EXIV_ROOT}/bin/vc12/x64")
	find_package(Exiv2 REQUIRED)
endif()

#enable openmp
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

################################################ Configuration ################################################

add_executable(${EXECUTABLE_NAME_1} ${SOURCEFILES_SIMPLE_VIEWER})
add_executable(${EXECUTABLE_NAME_2} ${SOURCEFILES_WININSTALLER})

#set entry point in case windows subsystem is used and enable whole program optimization
if(MSVC)
	set_target_properties(${EXECUTABLE_NAME_1} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\" /SUBSYSTEM:WINDOWS")
	set_target_properties(${EXECUTABLE_NAME_1} PROPERTIES LINK_FLAGS_RELEASE "/LTCG")
	set_target_properties(${EXECUTABLE_NAME_2} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\"  /SUBSYSTEM:WINDOWS /level='requireAdministrator'")
	set_target_properties(${EXECUTABLE_NAME_2} PROPERTIES LINK_FLAGS_RELEASE "/LTCG")	
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
endif(MSVC)

#gcc flags (C++11 and some Qt flag)
if(CMAKE_COMPILER_IS_GNUCC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC")
endif(CMAKE_COMPILER_IS_GNUCC)

#add custom target for copying ressources
add_custom_target(COPY_RESSOURCES)
add_custom_command(TARGET COPY_RESSOURCES POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${PROJECT_BINARY_DIR}/data)
add_dependencies(${EXECUTABLE_NAME_1} COPY_RESSOURCES)
add_dependencies(${EXECUTABLE_NAME_2} COPY_RESSOURCES)

#add lib files and copy dlls

########################################### OpenCV Libs ###########################################

if((DEFINED OPENCV_ROOT_FOUND) OR UNIX)
	if(MSVC)
		message("Copying dlls, please wait.")
	endif()
	target_link_libraries(${EXECUTABLE_NAME_1} ${OPENCV_MODULES_SIMPLE_VIEWER})
	#somehow include directories have to be added manually since OpenCV 3
	include_directories(${OpenCV_INCLUDE_DIRS})
	if(MSVC)
		copydlls(OPENCV_MODULES_ALL)
	endif(MSVC)
endif()

########################################### Qt Libs ###########################################

IF(DEFINED QT_ROOT_FOUND)

	target_link_libraries(${EXECUTABLE_NAME_1} ${QT_MODULES_SIMPLE_VIEWER})
	target_link_libraries(${EXECUTABLE_NAME_2} ${QT_MODULES_WININSTALLER})

	#copy the dlls
	IF(MSVC)

		copydlls(QT_MODULES_ALL)	

		file(COPY ${PATH_QT_ROOT}/bin/icudt54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY ${PATH_QT_ROOT}/bin/icuin54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY ${PATH_QT_ROOT}/bin/icuuc54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
		file(COPY ${PATH_QT_ROOT}/plugins/platforms DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)

		file(COPY ${PATH_QT_ROOT}/bin/icudt54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${PATH_QT_ROOT}/bin/icuin54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${PATH_QT_ROOT}/bin/icuuc54.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
		file(COPY ${PATH_QT_ROOT}/plugins/platforms DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

	ENDIF(MSVC)

ENDIF(DEFINED QT_ROOT_FOUND)

########################################## Exvi2 Libs ##########################################

if(DEFINED EXIV_ROOT_FOUND)
	target_link_libraries(${EXECUTABLE_NAME_1} "exiv2")

	if(MSVC)
		set(EXIV2_MODULES "exiv2")
		copydlls(EXIV2_MODULES)
	endif(MSVC)
endif()

IF(OPENCV_ROOT_FOUND AND QT_ROOT_FOUND AND EXIV_ROOT_FOUND)
	message("Everything fine, ready to generate.")
ENDIF()