# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)
project(Mondradiko VERSION 0.0.0)

option(TRACY_ENABLE "Enable Tracy profiling." OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
if (MSVC)
  set(MSVC_DISABLED_WARNINGS_LIST
    "C4710" # function '<x>' not inlined
    "C4711" # function <x> selected for inline expansion
    "C4464" # relative include path contains '..'
    "C4514" #unreferenced inline function has been removed
    "C4244" #conversion from 'double' to 'T', possible loss of data
    "C5045" "C4820" "C5027" "C4623" "C5026" "C4265" "C4625" "C4626" "C4365" "C4583" "C4267" "C4774" "C4061" "C4201" "C4242" "C4100" "C5204" "C4702" "C4668" "C4305" "C5039" "C5219" "C4191" "C4458" "C4582"
  )
  message(WARNING "FIXME: DISABLING LOTS OF C++ WARNINGS IN ORDER TO TROUBLESHOOT GHA CI... UNDO! ${MSVC_DISABLED_WARNINGS_LIST}")
  string(REPLACE "C" " -wd" MSVC_DISABLED_WARNINGS_STR ${MSVC_DISABLED_WARNINGS_LIST})
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_DISABLED_WARNINGS_STR}")
endif()

# if (WIN32 AND MSVC_VERSION GREATER_EQUAL "1900")
#   add_compile_options("/std:c++latest")
# endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(DEFINED VCPKG_ROOT)
  include(vcpkg)
  vcpkg_install(vulkan-sdk-minimal glslang flatbuffers openxr-loader sdl2[vulkan] xxhash lz4 gamenetworkingsockets glm freetype wasmtime)
endif()

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

add_compile_options(-Wall)

add_subdirectory(log)
add_subdirectory(types)
add_subdirectory(bindings)
add_subdirectory(assets)
add_subdirectory(core)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(bundler)
