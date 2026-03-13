cmake_minimum_required(VERSION 3.21)

project(AcuteViewer VERSION 1.6)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

#=======================================================================#
# Version definitions
#=======================================================================#

add_definitions(-DAV_MAJOR_VERSION=${PROJECT_VERSION_MAJOR})
add_definitions(-DAV_MINOR_VERSION=${PROJECT_VERSION_MINOR})

#=======================================================================#
# Dependencies
#=======================================================================#

find_package(OpenCV 4 REQUIRED COMPONENTS core highgui imgproc imgcodecs)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(Exiv2 REQUIRED)
find_package(OpenMP)

#=======================================================================#
# Sources
#=======================================================================#

file(GLOB SOURCEFILES_ACUTE_VIEWER
    "AcuteViewer/*.h"
    "AcuteViewer/*.hpp"
    "AcuteViewer/*.cpp"
)

file(GLOB SOURCEFILES_WININSTALLER
    "WinInstaller/*.h"
    "WinInstaller/*.hpp"
    "WinInstaller/*.cpp"
)

if(WIN32)
    list(APPEND SOURCEFILES_ACUTE_VIEWER "${CMAKE_CURRENT_SOURCE_DIR}/AcuteViewer/main.rc")
    list(APPEND SOURCEFILES_WININSTALLER "${CMAKE_CURRENT_SOURCE_DIR}/WinInstaller/main.rc")
endif()

#=======================================================================#
# Targets
#=======================================================================#

add_executable(AcuteViewer ${SOURCEFILES_ACUTE_VIEWER})

target_link_libraries(AcuteViewer PRIVATE
    ${OpenCV_LIBS}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Exiv2::exiv2lib
)

if(OpenMP_CXX_FOUND)
    target_link_libraries(AcuteViewer PRIVATE OpenMP::OpenMP_CXX)
endif()

if(WIN32)
    add_executable(WinInstaller ${SOURCEFILES_WININSTALLER})

    target_link_libraries(WinInstaller PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
    )
endif()

#=======================================================================#
# Compiler/linker flags
#=======================================================================#

if(MSVC)
    target_link_options(AcuteViewer PRIVATE
        /ENTRY:mainCRTStartup
        $<$<CONFIG:Release>:/LTCG /SUBSYSTEM:WINDOWS>
    )
    target_compile_options(AcuteViewer PRIVATE
        $<$<CONFIG:Release>:/GL>
    )
    target_compile_definitions(AcuteViewer PRIVATE NOMINMAX)

    if(WIN32)
        target_link_options(WinInstaller PRIVATE
            /ENTRY:mainCRTStartup
            $<$<CONFIG:Release>:/LTCG /SUBSYSTEM:WINDOWS>
        )
        target_compile_options(WinInstaller PRIVATE
            $<$<CONFIG:Release>:/GL>
        )
        target_compile_definitions(WinInstaller PRIVATE NOMINMAX)
    endif()
endif()

if(UNIX)
    # Make executable find libs in the same folder at runtime
    target_compile_options(AcuteViewer PRIVATE -fPIC)
    target_link_options(AcuteViewer PRIVATE "LINKER:-rpath=$ORIGIN/lib")
endif()

#=======================================================================#
# Post-build: copy deploy resources
#=======================================================================#

set(ALL_TARGETS AcuteViewer)
if(WIN32)
    list(APPEND ALL_TARGETS WinInstaller)
endif()

foreach(TARGET_NAME ${ALL_TARGETS})
    if(WIN32)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/deploy_ressources/general/data"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/deploy_ressources/general"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    elseif(UNIX)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/deploy_ressources/general"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/deploy_ressources/linux"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        )
    endif()
endforeach()

#=======================================================================#
# Post-build: copy runtime DLLs (Windows only)
#=======================================================================#

if(WIN32)
    # Only AcuteViewer has third-party dependencies other than QT
    foreach(TARGET_NAME AcuteViewer)
        # Copy all non-Qt runtime DLLs via generator expression
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_RUNTIME_DLLS:${TARGET_NAME}>
                "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            COMMAND_EXPAND_LISTS
        )
    endforeach()

    foreach(TARGET_NAME ${ALL_TARGETS})
        # Let windeployqt handle Qt DLLs and plugins (platform drivers etc.)
        find_program(WINDEPLOYQT windeployqt HINTS "${Qt6_DIR}/../../../bin")
        if(WINDEPLOYQT)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${WINDEPLOYQT}
                    --no-translations
                    $<$<CONFIG:Debug>:--debug>
                    $<$<CONFIG:Release>:--release>
                    "$<TARGET_FILE:${TARGET_NAME}>"
                WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
        endif()
    endforeach()
endif()

#=======================================================================#
# Install
#=======================================================================#

# Install executables and collect their runtime dependencies into a set
install(TARGETS AcuteViewer
    RUNTIME_DEPENDENCY_SET app_deps
    DESTINATION .
)
if(WIN32)
    install(TARGETS WinInstaller
        RUNTIME_DEPENDENCY_SET app_deps
        DESTINATION .
    )
endif()

# Install all third-party DLLs collected in the dependency set,
# excluding Windows system DLLs
install(RUNTIME_DEPENDENCY_SET app_deps
    PRE_EXCLUDE_REGEXES
        "api-ms-.*"
        "ext-ms-.*"
        "hvsifiletrust\\.dll"
        "pdmutilities\\.dll"
    POST_EXCLUDE_REGEXES
        ".*[Ss]ystem32[/\\\\].*\\.dll"
        ".*[Ww]in[Ss][Xx][Ss][/\\\\].*\\.dll"
    DESTINATION .
)

# Install deploy resources
install(DIRECTORY "${CMAKE_SOURCE_DIR}/deploy_ressources/general/"
    DESTINATION .
)
if(WIN32)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/deploy_ressources/general/data"
        DESTINATION .
    )
elseif(UNIX)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/deploy_ressources/linux/"
        DESTINATION .
    )
endif()

install(FILES
    "${CMAKE_SOURCE_DIR}/LICENSE.txt"
    "${CMAKE_SOURCE_DIR}/THIRD-PARTY-NOTICES.txt"
    DESTINATION .
)

# On Windows, run windeployqt at install time to pull in Qt plugins
if(WIN32 AND WINDEPLOYQT)
    foreach(TARGET_NAME ${ALL_TARGETS})
        install(CODE "
            message(STATUS \"Running windeployqt for ${TARGET_NAME}...\")
            execute_process(
                COMMAND \"${WINDEPLOYQT}\"
                    --no-translations
                    --dir \"\${CMAKE_INSTALL_PREFIX}\"
                    \"\${CMAKE_INSTALL_PREFIX}/${TARGET_NAME}.exe\"
            )
        ")
    endforeach()
endif()
