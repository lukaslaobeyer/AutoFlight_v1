# May be required to run CMake with  -DPYTHON_EXECUTABLE=/usr/bin/python3 -DPYTHON_LIBRARY=/usr/lib/x86_64-linux-gnu/libpython3.4m.so -DPYTHON_INCLUDE_DIR=/usr/include/python3.4 -DPYTHON_INCLUDE_DIR2=/usr/include/python3.4


cmake_minimum_required(VERSION 2.8.9)

project(AutoFlight)

# Update build version number
IF(BUILD_NUMBER)
    add_definitions(-DAUTOFLIGHT_BUILD="${BUILD_NUMBER}")
ENDIF()

# Build configurations
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -g -O0")
    #                                      ^^^^^^ for debugging only
ELSE()
    set(Boost_USE_DEBUG_RUNTIME OFF)
    set(Boost_USE_DEBUG_PYTHON OFF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -O2 -s")
ENDIF()
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Set executable output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/release)
# Add cmake/modules directory to CMAKE_MODULE_PATH
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

set(QT_USE_QTWEBKIT TRUE)

# Tell cmake where Qt is
#set(CMAKE_PREFIX_PATH "/home/lukas/workspace/libs/qt5/qtbase")
IF(MINGW)
  set(CMAKE_PREFIX_PATH "C:/Qt/5.4/mingw491_32")
ELSE()
  set(CMAKE_PREFIX_PATH "/opt/Qt5.5.1/5.5/gcc_64")
ENDIF()

# Include libraries in the ../lib folder
include_directories("${PROJECT_SOURCE_DIR}/lib/include/gamepad")
include_directories("${PROJECT_SOURCE_DIR}/lib/include/")

# Include libdrone
include_directories("${PROJECT_SOURCE_DIR}/../libdrone/include")

# Find the QtWidgets library
add_definitions(-DQT_NO_KEYWORDS)
find_package(Qt5Widgets)
find_package(Qt5OpenGL)
find_package(Qt5Multimedia)

# Find the Boost libraries
IF(MINGW)
  set(BOOST_ROOT "C:/boost/build")
ENDIF()
set(Python_ADDITIONAL_VERSIONS 3)
IF(MINGW)
  find_package(Boost 1.53 COMPONENTS program_options system python filesystem thread chrono date_time timer REQUIRED)
ELSE()
  find_package(Boost 1.53 COMPONENTS program_options system python-py34 filesystem thread chrono date_time timer REQUIRED)
ENDIF()
include_directories(${Boost_INCLUDE_DIR})

# Find OpenCV
IF(MINGW)
  set(OpenCV_DIR "C:/opencv")
ELSE()
  set(OpenCV_DIR "/opt/opencv/share/OpenCV")
ENDIF()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Find Eigen
IF(MINGW)
  include_directories("C:/eigen3/include/eigen3")
ELSE()
  find_package(Eigen3 REQUIRED)
  include_directories(${EIGEN3_INCLUDE_DIR})
ENDIF()

# Find yaml-cpp
IF(MINGW)
    set(YAMLCPP_LIBRARY "C:/yaml-cpp/lib/libyaml-cpp.a")
    include_directories("C:/yaml-cpp/include")
ELSE()
    find_package(YamlCpp)
ENDIF()

# Find FFMPEG and dependencies
#find_package(FFMPEG REQUIRED)
#include_directories(${FFMPEG_INCLUDE_DIR})
#find_package(ZLIB REQUIRED)
IF(MINGW)
#  set(FFMPEG_LIBRARIES
#      "C:/ffmpeg/bin/libavcodec.dll"
#      "C:/ffmpeg/bin/lib/libavformat.dll"
#      "C:/ffmpeg/bin/libavutil.dll"
#      "C:/ffmpeg/bin/libswscale.dll"
#      "C:/ffmpeg/bin/libswresample.dll" )
  include_directories("C:/ffmpeg/include")
ELSE()
  set(FFMPEG_LIBRARIES
      "/opt/ffmpeg/lib/libavcodec.so"
      "/opt/ffmpeg/lib/libavformat.so"
      "/opt/ffmpeg/lib/libavutil.so"
      "/opt/ffmpeg/lib/libswscale.so"
      "/opt/ffmpeg/lib/libswresample.so" )
  include_directories("/opt/ffmpeg/include")
ENDIF()

# Find Python
set(Python_ADDITIONAL_VERSIONS 3.4)
find_package(PythonLibs 3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

# Numpy on Windows
IF(MINGW)
  include_directories("C:/Python34/Lib/site-packages/numpy/core/include")
ENDIF()

file(GLOB AutoFlight_SRC
    "src/*.h"
    "src/*.cpp"
    "src/widgets/*.h"
    "src/widgets/*.cpp"
    "src/dialogs/*.h"
    "src/dialogs/*.cpp"
    "src/input/*.h"
    "src/input/*.cpp"
    "src/autoscript/*.h"
    "src/autoscript/*.cpp"
    "src/autoscript/opencv/*.h"
    "src/autoscript/opencv/*.cpp"
    "src/autoscript/opencv/*.hpp"
    "src/tools/*.h"
    "src/tools/*.cpp"
    "src/mavlink/*.h"
    "src/mavlink/*.cpp"
    "src/imgprocui/*.h"
    "src/imgprocui/*.cpp"
    "src/imgproc/*.h"
    "src/imgproc/*.cpp"
    "src/imgprocui/nodeseditor/*.h"
    "src/imgprocui/nodeseditor/*.cpp"

    "lib/include/qcustomplot/*.h"
    "lib/include/qcustomplot/*.cpp"
    "lib/include/pugixml/*.hpp"
    "lib/include/pugixml/*.cpp"
    "lib/include/glm/*.hpp"
    "lib/include/glm/core/*.hpp"
    "lib/include/mavlink/*.h"
    "lib/include/mavlink/common/*.h"
    "lib/include/ftp/*.h"
    "lib/include/ftp/*.cpp"
    "lib/include/april/*.h"
    "lib/include/april/*.cpp"
)

# Add the resources
qt5_add_resources(AutoFlight_RCCS "${PROJECT_SOURCE_DIR}/src/resources.qrc")

# Create the AutoFlight executable
add_executable(AutoFlight ${AutoFlight_SRC} ${AutoFlight_RCCS})

IF(MINGW)
  target_link_libraries(AutoFlight opengl32 ws2_32)
ELSE()
  target_link_libraries(AutoFlight GL x264)
ENDIF()

target_link_libraries(AutoFlight ${Boost_LIBRARIES})
target_link_libraries(AutoFlight ${OpenCV_LIBS})
target_link_libraries(AutoFlight ${FFMPEG_LIBRARIES})
target_link_libraries(AutoFlight ${PYTHON_LIBRARIES})
target_link_libraries(AutoFlight ${YAMLCPP_LIBRARY})

# Link libraries in ../lib
IF(MINGW)
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/lib/release-win32/libstem_gamepad.a" winmm)
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/lib/release-win32/libqscintilla2.a")
ELSE()
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/lib/release-linux64/libstem_gamepad.a")
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/lib/release-linux64/libqscintilla2.a")
ENDIF()

# Link libdrone
IF(MINGW)
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/../libdrone/build/libdrone/drone.dll")
ELSE()
  target_link_libraries(AutoFlight "${PROJECT_SOURCE_DIR}/../libdrone/build/libdrone/libdrone.so")
ENDIF()

# Use the Widgets module from Qt 5
qt5_use_modules(AutoFlight Widgets PrintSupport WebKit WebKitWidgets OpenGL Multimedia)
