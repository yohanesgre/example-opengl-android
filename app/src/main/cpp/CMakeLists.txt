# CMakeLists.txt
cmake_minimum_required(VERSION 3.22.1)

project(native-renderer)

# Add OpenGL ES and EGL libraries
find_library(GLES2-lib GLESv2)
find_library(EGL-lib EGL)
find_library(android-lib android)
find_library(log-lib log)

# Set source files directory
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Include directories
include_directories(${SOURCE_DIR}/include)

# Add source files
add_library(native-renderer SHARED
        ${SOURCE_DIR}/matrix.cpp
        ${SOURCE_DIR}/gl_renderer.cpp
        ${SOURCE_DIR}/jni_bridge.cpp
)

# Link libraries
target_link_libraries(native-renderer
        ${GLES2-lib}
        ${EGL-lib}
        ${android-lib}
        ${log-lib}
)

# Set C++ standard
set_target_properties(native-renderer PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

# Enable debug symbols in Debug build
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(native-renderer PRIVATE -g)
endif()