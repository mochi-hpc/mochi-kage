# (C) 2020 The University of Chicago
# See COPYRIGHT in top-level directory.
cmake_minimum_required (VERSION 3.8)
project (kage C CXX)
set (CMAKE_CXX_STANDARD 17)
enable_testing ()

# library version set here (e.g. for shared libs).
set (KAGE_VERSION_MAJOR 0)
set (KAGE_VERSION_MINOR 1)
set (KAGE_VERSION_PATCH 0)
set (KAGE_VERSION
     "${KAGE_VERSION_MAJOR}.${KAGE_VERSION_MINOR}.${KAGE_VERSION_PATCH}")

add_definitions (-Wextra -Wall -Wpedantic)

add_library (coverage_config INTERFACE)

option (ENABLE_TESTS    "Build tests" OFF)
option (ENABLE_EXAMPLES "Build examples" OFF)
option (ENABLE_BEDROCK  "Build bedrock module" OFF)
option (ENABLE_COVERAGE "Build with coverage" OFF)
option (ENABLE_ZMQ      "Build with ZeroMQ support" ON)

# add our cmake module directory to the path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
     "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# link shared lib with full rpath
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# setup cache variables for ccmake
if (NOT CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release
         CACHE STRING "Choose the type of build." FORCE)
    set_property (CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                  "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif ()
set (CMAKE_PREFIX_PATH "" CACHE STRING "External dependencies path")
set (BUILD_SHARED_LIBS "ON" CACHE BOOL "Build a shared library")

if (ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options (coverage_config INTERFACE
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options (coverage_config INTERFACE --coverage)
    else ()
        target_link_libraries (coverage_config INTERFACE --coverage)
    endif ()
endif ()

find_package (PkgConfig REQUIRED)
# search fo thallium
find_package (thallium REQUIRED)
# search for nlohmann/json
find_package (nlohmann_json REQUIRED)
find_package (nlohmann_json_schema_validator REQUIRED)
# search for spdlog
find_package (spdlog REQUIRED)
# search for fmt
find_package (fmt REQUIRED)
# search of bedrock if needed
if (${ENABLE_BEDROCK})
    find_package (bedrock-module-api REQUIRED)
endif ()
# search for zmq if needed
if (${ENABLE_ZMQ})
    find_package (cppzmq REQUIRED)
endif ()

add_subdirectory (src)
if (${ENABLE_TESTS})
    enable_testing ()
    find_package (Catch2 3.6.0 QUIET)
    if (NOT Catch2_FOUND)
        include (FetchContent)
        FetchContent_Declare (
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG        v3.6.0
        )
        FetchContent_MakeAvailable (Catch2)
    endif ()
    add_subdirectory (tests)
endif (${ENABLE_TESTS})
if (${ENABLE_EXAMPLES})
    add_subdirectory (examples)
endif (${ENABLE_EXAMPLES})
