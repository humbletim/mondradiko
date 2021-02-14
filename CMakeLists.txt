# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' (run cmake with -DCMAKE_BUILD_TYPE=Debug to override).")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(optional-vcpkg-setup)

project(Mondradiko VERSION 0.0.0)
set(Mondradiko_COPYRIGHT "Copyright (c) 2020-2021 the Mondradiko contributors.")
set(Mondradiko_LICENSE "SPDX-License-Identifier: LGPL-3.0-or-later")

option(TRACY_ENABLE "Enable Tracy profiling." OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (MSVC)
  include(msvc-disable-warnings)
endif()

# if (WIN32 AND MSVC_VERSION GREATER_EQUAL "1900")
#   add_compile_options("/std:c++latest")
# endif()

include(SPIR-V)
include(flatc)

# Enable Tracy profiling
if(${TRACY_ENABLE})
  add_compile_definitions(TRACY_ENABLE)
endif()

# Copy runtime config into binary folder
configure_file(example-config.toml config.toml COPYONLY)

# Let all parts of the source base access each other's headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
# Let all parts of the source base access each other's generated files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(lib)

if (NOT WIN32)
  add_compile_options(-Wall)
endif()

add_subdirectory(log)
add_subdirectory(types)
add_subdirectory(bindings)
add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(bundler)
