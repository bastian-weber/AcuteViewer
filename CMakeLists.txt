cmake_minimum_required (VERSION 3.5)

#=======================================================================#

#Opencv modules
set(OPENCV_MODULES_ACUTE_VIEWER "opencv_core" "opencv_highgui" "opencv_imgproc" "opencv_imgcodecs")
set(OPENCV_MODULES_WININSTALLER)

#Qt modules
set(QT_MODULES_ACUTE_VIEWER "Qt5::Core" "Qt5::Gui" "Qt5::Widgets" "Qt5::Network")
set(QT_MODULES_WININSTALLER "Qt5::Core" "Qt5::Gui" "Qt5::Widgets")
#these modules will only be linked on windows
#set(QT_MODULES_ACUTE_VIEWER_WIN "Qt5::WinExtras")

set(PROJECT_NAME AcuteViewer)
set(EXECUTABLE_NAME_1 AcuteViewer)
set(EXECUTABLE_NAME_2 WinInstaller)

#=======================================================================#

if(WIN32)
	set(TARGETS "${EXECUTABLE_NAME_1}" "${EXECUTABLE_NAME_2}")
	set(COPY_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/Release")
	set(COPY_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/Debug")
else()
	set(TARGETS "${EXECUTABLE_NAME_1}")
	#set exe output path
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")
	set(COPY_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	set(COPY_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(Macros)

project(${PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#set version information
add_definitions(-DAV_MAJOR_VERSION=1)
add_definitions(-DAV_MINOR_VERSION=6)

file(GLOB SOURCEFILES_ACUTE_VIEWER "./AcuteViewer/*.h"  "./AcuteViewer/*.hpp"  "./AcuteViewer/*.cpp")
file(GLOB SOURCEFILES_WININSTALLER "./WinInstaller/*.h"  "./WinInstaller/*.hpp"  "./WinInstaller/*.cpp")

if(WIN32)
	#add application icon ressource
	list(APPEND SOURCEFILES_ACUTE_VIEWER "${CMAKE_CURRENT_SOURCE_DIR}/AcuteViewer/main.rc")
	list(APPEND SOURCEFILES_WININSTALLER "${CMAKE_CURRENT_SOURCE_DIR}/WinInstaller/main.rc")
endif()

#OpenCV
set(OpenCV_STATIC OFF)
set(OPENCV_MODULES_ALL)
list(APPEND OPENCV_MODULES_ALL ${OPENCV_MODULES_ACUTE_VIEWER} ${OPENCV_MODULES_WININSTALLER})
list(REMOVE_DUPLICATES OPENCV_MODULES_ALL)
set(OPENCV_FILES "OpenCVConfig.cmake")
set(OPENCV_PATH_HINTS "." "./build" "./.." "./../.." "./../../.." "./share/OpenCV")
ask_for_path("OpenCV" "" "/usr/local" OPENCV_FILES OPENCV_PATH_HINTS)
if(OPENCV_ROOT_FOUND)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PATH_OPENCV_ROOT})
	find_package(OpenCV 3.1 REQUIRED)
endif()

#Qt
set(QT_MODULES_ALL)
list(APPEND QT_MODULES_ALL ${QT_MODULES_ACUTE_VIEWER} ${QT_MODULES_WININSTALLER})
if(WIN32)
	list(APPEND QT_MODULES_ALL ${QT_MODULES_ACUTE_VIEWER_WIN})
endif()
list(REMOVE_DUPLICATES QT_MODULES_ALL)
set(QT_FILES "lib/cmake/Qt5/Qt5Config.cmake")
set(QT_PATH_HINTS "." "./../../.." "./../.." "./..")
ask_for_path("Qt" "" "/opt/Qt5.6.0/5.6/gcc_64" QT_FILES QT_PATH_HINTS)
if(QT_ROOT_FOUND)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${PATH_QT_ROOT})
	foreach(module ${QT_MODULES_ALL})
		string(REGEX REPLACE "::" "" MODULE_NAME ${module})
		find_package(${MODULE_NAME} 5.6 REQUIRED)
	endforeach(module ${QT_MODULES_ALL})
endif(QT_ROOT_FOUND)

#Exiv2
if(WIN32)
	set(EXIV_FILES "include/exiv2.hpp")
else()
	set(EXIV_FILES "include/exiv2/exiv2.hpp")
endif()
set(EXIV_PATH_HINTS "." ".." "../../../.." "../..")
ask_for_path("Exiv" "" "/usr/local" EXIV_FILES EXIV_PATH_HINTS)
if(EXIV_ROOT_FOUND)
	if(MSVC)	
		if(${MSVC_VERSION} EQUAL 1800)
			set(PATH_COMPONENT "vc12")
		else()
			set(PATH_COMPONENT "vc14")
		endif()
	endif()
	set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" "${PATH_EXIV_ROOT}" "${PATH_EXIV_ROOT}/include" "${PATH_EXIV_ROOT}/include/exiv2" "${PATH_EXIV_ROOT}/bin/${PATH_COMPONENT}/x64")
	find_package(Exiv2 REQUIRED)
endif()

#enable openmp
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

################################################ Configuration ################################################

add_executable(${EXECUTABLE_NAME_1} ${SOURCEFILES_ACUTE_VIEWER})
if(WIN32)
	add_executable(${EXECUTABLE_NAME_2} ${SOURCEFILES_WININSTALLER})
endif()

#set entry point in case windows subsystem is used and enable whole program optimization
if(MSVC)
	set_target_properties(${EXECUTABLE_NAME_1} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
	set_target_properties(${EXECUTABLE_NAME_1} PROPERTIES LINK_FLAGS_RELEASE "/LTCG /SUBSYSTEM:WINDOWS")
	set_target_properties(${EXECUTABLE_NAME_2} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\" /level='requireAdministrator'")
	set_target_properties(${EXECUTABLE_NAME_2} PROPERTIES LINK_FLAGS_RELEASE "/LTCG /SUBSYSTEM:WINDOWS")	
	set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
endif(MSVC)

#gcc flags (C++11 and some Qt flag)
if(CMAKE_COMPILER_IS_GNUCC)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC")
endif(CMAKE_COMPILER_IS_GNUCC)
if(UNIX)
	#make executable find libs in same folder
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath='$ORIGIN/lib'")
endif()

#add custom target for copying ressources

#post build command for copying ressources
if(WIN32)
	foreach(EXECUTABLE ${TARGETS})
		add_custom_command(TARGET ${EXECUTABLE} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deploy_ressources/general/data "${PROJECT_BINARY_DIR}/data"
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deploy_ressources/general "${COPY_DIRECTORY_RELEASE}")
	endforeach()
elseif (UNIX)
	foreach(EXECUTABLE ${TARGETS})
		add_custom_command(TARGET ${EXECUTABLE} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deploy_ressources/general "${COPY_DIRECTORY_RELEASE}"
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/deploy_ressources/linux "${COPY_DIRECTORY_RELEASE}")
	endforeach()
endif()

set(RELEASE_DLLS)
set(DEBUG_DLLS)

set(LIB_SUBFOLDER "")
set(PLATFORMS_SUBFOLDER "")
if(UNIX)
	set(LIB_SUBFOLDER "/lib")
	set(PLATFORMS_SUBFOLDER "/plugins")
endif()

#add lib files and copy libs
message("Looking for libs, please wait.")

########################################### OpenCV Libs ###########################################

if((DEFINED OPENCV_ROOT_FOUND) OR UNIX)
	target_link_libraries(${EXECUTABLE_NAME_1} ${OPENCV_MODULES_ACUTE_VIEWER})
	#somehow include directories have to be added manually since OpenCV 3
	include_directories(${OpenCV_INCLUDE_DIRS})
	copydlls(RELEASE_DLLS DEBUG_DLLS OPENCV_MODULES_ALL)
endif()

########################################### Qt Libs ###########################################

IF(DEFINED QT_ROOT_FOUND)

	target_link_libraries(${EXECUTABLE_NAME_1} ${QT_MODULES_ACUTE_VIEWER})
	if(WIN32)
		target_link_libraries(${EXECUTABLE_NAME_2} ${QT_MODULES_WININSTALLER})
	endif()

	#copy the dlls
	copydlls(RELEASE_DLLS DEBUG_DLLS QT_MODULES_ALL)

	IF(MSVC)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/bin/icudt54.dll)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/bin/icuin54.dll)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/bin/icuuc54.dll)
		
		list(APPEND DEBUG_DLLS ${PATH_QT_ROOT}/bin/icudt54.dll)
		list(APPEND DEBUG_DLLS ${PATH_QT_ROOT}/bin/icuin54.dll)
		list(APPEND DEBUG_DLLS ${PATH_QT_ROOT}/bin/icuuc54.dll)
	ELSEIF(UNIX)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/lib/libicudata.so.56)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/lib/libicui18n.so.56)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/lib/libicuuc.so.56)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/lib/libQt5DBus.so.5)
		list(APPEND RELEASE_DLLS ${PATH_QT_ROOT}/lib/libQt5XcbQpa.so.5)
	ENDIF(MSVC)

	foreach(EXECUTABLE ${TARGETS})
		add_custom_command(TARGET ${EXECUTABLE} POST_BUILD 
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PATH_QT_ROOT}/plugins/platforms "${COPY_DIRECTORY_DEBUG}${PLATFORMS_SUBFOLDER}/platforms"
			COMMAND ${CMAKE_COMMAND} -E copy_directory ${PATH_QT_ROOT}/plugins/platforms "${COPY_DIRECTORY_RELEASE}${PLATFORMS_SUBFOLDER}/platforms")
	endforeach()

ENDIF(DEFINED QT_ROOT_FOUND)

########################################## Exvi2 Libs ##########################################

if(DEFINED EXIV_ROOT_FOUND)
	target_link_libraries(${EXECUTABLE_NAME_1} "exiv2")

	set(EXIV2_MODULES "exiv2")
	copydlls(RELEASE_DLLS DEBUG_DLLS EXIV2_MODULES)
endif()

#copy libs
if(DEFINED RELEASE_DLLS AND DEFINED DEBUG_DLLS)

	set(CREATE_LIB_DIR_COMMAND "-E" "make_directory" "${COPY_DIRECTORY_RELEASE}/lib")

	set(COPY_COMMAND_RELEASE "-E" "copy_if_different")
	set(COPY_COMMAND_DEBUG "-E" "copy_if_different")

	list(REMOVE_DUPLICATES RELEASE_DLLS)
	list(REMOVE_DUPLICATES DEBUG_DLLS)

	foreach(DLL ${RELEASE_DLLS})
		list(APPEND COPY_COMMAND_RELEASE "\"${DLL}\"")
	endforeach()
	foreach(DLL ${DEBUG_DLLS})
		list(APPEND COPY_COMMAND_DEBUG "\"${DLL}\"")
	endforeach()

	list(APPEND COPY_COMMAND_RELEASE "\"${COPY_DIRECTORY_RELEASE}${LIB_SUBFOLDER}\"")
	list(APPEND COPY_COMMAND_DEBUG "\"${COPY_DIRECTORY_DEBUG}${LIB_SUBFOLDER}\"")

	foreach(EXECUTABLE ${TARGETS})
		if(UNIX)
			add_custom_command(TARGET ${EXECUTABLE} POST_BUILD COMMAND ${CMAKE_COMMAND} ${CREATE_LIB_DIR_COMMAND})	
		endif()		
		add_custom_command(TARGET ${EXECUTABLE} POST_BUILD COMMAND ${CMAKE_COMMAND} ${COPY_COMMAND_RELEASE})
		if(MSVC)
			add_custom_command(TARGET ${EXECUTABLE} POST_BUILD COMMAND ${CMAKE_COMMAND} ${COPY_COMMAND_DEBUG})
		endif()
	endforeach()

endif()

IF(OPENCV_ROOT_FOUND AND QT_ROOT_FOUND AND EXIV_ROOT_FOUND)
	message("Everything fine, ready to generate.")
ENDIF()
