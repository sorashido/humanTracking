cmake_minimum_required(VERSION 3.7)
project(cpp)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        main.cpp
        native/include/pxcsensemanager.h
)
add_executable(cpp ${SOURCE_FILES})

# OpenCVのパッケージを探す
find_package(OpenCV REQUIRED)
find_package(PythonLibs REQUIRED)

# OpenCVが見つかった場合のみ設定を行う
if(OpenCV_FOUND)
    # インクルードパスを指定
    include_directories(${OpenCV_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})

    # リンクするライブラリ指定
    target_link_libraries(cpp ${OpenCV_LIBS} ${PYTHON_LIBRARIES})
endif(OpenCV_FOUND)

# librealsense
include_directories(${/usr/local/include/librealsense2})
# リンクするライブラリ指定
set(REALSENSE_LIBS
        /usr/local/lib/librealsense-file.a
        /usr/local/lib/librealsense2.2.9.0.dylib
        /usr/local/lib/librealsense2.2.dylib
        /usr/local/lib/librealsense2.dylib
        )
target_link_libraries(cpp ${REALSENSE_LIBS})