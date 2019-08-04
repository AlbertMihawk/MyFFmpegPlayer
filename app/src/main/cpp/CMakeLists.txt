
cmake_minimum_required(VERSION 3.4.1)


file(GLOB SOURCE ./*.cpp)
add_library( # Sets the name of the library.
        myffmpegplayer
        SHARED
        ${SOURCE})

find_library( # Sets the name of the path variable.
        log-lib
        log)

include_directories(include)
message(WARNING "DIR: ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI}")
#message(WARNING "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
#message(WARNING "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
set(my_lib_path ${CMAKE_SOURCE_DIR}/../../../libs/${ANDROID_ABI})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${my_lib_path}")
target_link_libraries( # Specifies the target library.
        myffmpegplayer

        avformat
        avcodec
        avfilter
        avutil
        swresample
        swscale

        z
        OpenSLES
        android
        ${log-lib}
        )