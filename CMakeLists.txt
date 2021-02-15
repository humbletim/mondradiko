# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later
cmake_minimum_required(VERSION 3.18)

###############################################################################
# note: this preconfiguration needs to appear before first call to project()

# allow cmake to find locally defined modules under cmake/
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# arrange for Release builds by default (TODO: maybe only for CI environments)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release'"
      "(to override add -DCMAKE_BUILD_TYPE=Debug to override).")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if (WIN32)
  # FIXME(humbletim): this policy override and setting is needed in order to
  #     link _Mondradiko_ Debug builds against the release versions of VCPKG
  #     dependencies and MSVCRT. The assumption here is that most people making
  #     Debug versions of mondradiko-client etc. are interested in debugging
  #     this project's source code, not the underlying dependencies or MSVCRT.
  # dynamically link against non-debug version of msvcrtxxx.dll (/MD)
  cmake_policy(SET CMP0091 NEW)
  set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
endif()

# VCPKG automation
include(optional-vcpkg-setup)
###############################################################################

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

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/config.toml)
  # Copy runtime config into binary folder
  configure_file(example-config.toml config.toml COPYONLY)
endif()

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
