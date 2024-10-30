# CMakeLists.txt
cmake_minimum_required(VERSION 3.22.1)

project(native-renderer)

# Include FetchContent
include(FetchContent)

# Declare GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
        GIT_SHALLOW TRUE
)

# Make GLM available
FetchContent_MakeAvailable(glm)

# Set the path to your cpp directory relative to CMakeLists.txt location
set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Add OpenGL ES and EGL libraries
find_library(GLES2-lib GLESv2)
find_library(EGL-lib EGL)
find_library(android-lib android)
find_library(log-lib log)

# Debug print paths to verify
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "GLM Source Dir: ${glm_SOURCE_DIR}")

# Include directories
include_directories(
        ${CPP_DIR}/include
        ${glm_SOURCE_DIR}
)

# Add source files
add_library(native-renderer SHARED
        ${CPP_DIR}/gl_renderer.cpp
        ${CPP_DIR}/jni_bridge.cpp
)

# Link libraries
target_link_libraries(native-renderer
        ${GLES2-lib}
        ${EGL-lib}
        ${android-lib}
        ${log-lib}
        glm
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

    # Add debug messages for path verification
    target_compile_definitions(native-renderer PRIVATE
            GLM_INCLUDE_PATH="${glm_SOURCE_DIR}"
    )
endif()