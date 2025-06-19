cmake_minimum_required(VERSION 3.21)

project(icon-count-editor VERSION 0.1.0)

if (PROJECT_IS_TOP_LEVEL)
    add_library(icon-count-editor SHARED src/IconCountEditor.cpp)
else()
    add_library(icon-count-editor STATIC src/IconCountEditor.cpp)
endif()

target_compile_features(icon-count-editor PRIVATE cxx_std_20)
target_include_directories(icon-count-editor PRIVATE include)

if (PROJECT_IS_TOP_LEVEL)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)

    if (NOT DEFINED ENV{GEODE_SDK})
        message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
    else()
        message(STATUS "Found Geode: $ENV{GEODE_SDK}")
    endif()

    add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

    target_compile_definitions(${PROJECT_NAME} PRIVATE GEODE_MOD_ID="")
    target_link_libraries(${PROJECT_NAME} geode-sdk)
endif()
