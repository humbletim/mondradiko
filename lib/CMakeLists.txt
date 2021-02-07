# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(PkgConfig REQUIRED)
if (DEFINED VCPKG_ROOT)
  find_package(Flatbuffers CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
  find_package(lz4 CONFIG REQUIRED)
  find_package(OpenXR CONFIG REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
  find_package(xxHash CONFIG REQUIRED)
else()
  pkg_check_modules(SDL2 REQUIRED sdl2)
  pkg_check_modules(OPENXR REQUIRED openxr)
  pkg_check_modules(XXHASH REQUIRED libxxhash)
  pkg_check_modules(LZ4 REQUIRED liblz4)
endif()

find_package(GameNetworkingSockets REQUIRED)
find_package(Threads REQUIRED)

include(FindVulkan)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(MONDRADIKO_LIB_SRC library_implementation.cc)

if(${TRACY_ENABLE})
  list(APPEND MONDRADIKO_LIB_SRC tracy/TracyClient.cpp)
endif()

add_library(mondradiko-lib STATIC ${MONDRADIKO_LIB_SRC})

if (DEFINED VCPKG_ROOT)
  target_link_libraries(mondradiko-lib PUBLIC flatbuffers::flatbuffers)
  target_link_libraries(mondradiko-lib PUBLIC lz4::lz4)
  target_link_libraries(mondradiko-lib PUBLIC OpenXR::headers OpenXR::openxr_loader OpenXR::openxr-all-supported)
  target_link_libraries(mondradiko-lib PUBLIC SDL2::SDL2main SDL2::SDL2-static)
  target_link_libraries(mondradiko-lib PUBLIC xxHash::xxhash)
endif()

if(${TRACY_ENABLE})
  if(${UNIX})
    target_link_libraries(mondradiko-lib PUBLIC dl)
    target_link_libraries(mondradiko-lib PUBLIC pthread)
  endif()
endif()

# Add C++ std::filesystem
target_link_libraries(mondradiko-lib PUBLIC stdc++fs)

if (NOT DEFINED VCPKG_ROOT)
  target_link_libraries(mondradiko-lib PUBLIC ${SDL2_LIBRARIES})
  target_include_directories(mondradiko-lib PUBLIC ${SDL2_INCLUDE_DIRS})
  target_compile_options(mondradiko-lib PUBLIC ${SDL2_CFLAGS_OTHER})

  target_link_libraries(mondradiko-lib PUBLIC ${OPENXR_LIBRARIES})
  target_include_directories(mondradiko-lib PUBLIC ${OPENXR_INCLUDE_DIRS})
  target_compile_options(mondradiko-lib PUBLIC ${OPENXR_CFLAGS_OTHER})

  target_link_libraries(mondradiko-lib PUBLIC ${XXHASH_LIBRARIES})
  target_include_directories(mondradiko-lib PUBLIC ${XXHASH_INCLUDE_DIRS})
  target_compile_options(mondradiko-lib PUBLIC ${XXHASH_CFLAGS_OTHER})

  target_link_libraries(mondradiko-lib PUBLIC ${LZ4_LIBRARIES})
  target_include_directories(mondradiko-lib PUBLIC ${LZ4_INCLUDE_DIRS})
  target_compile_options(mondradiko-lib PUBLIC ${LZ4_CFLAGS_OTHER})
endif()

target_link_libraries(mondradiko-lib PUBLIC GameNetworkingSockets::GameNetworkingSockets GameNetworkingSockets::GameNetworkingSockets_s)

target_link_libraries(mondradiko-lib PUBLIC ${Vulkan_LIBRARIES})
target_include_directories(mondradiko-lib PUBLIC ${Vulkan_INCLUDE_DIRS})
target_compile_options(mondradiko-lib PUBLIC ${Vulkan_CFLAGS_OTHER})

# Add msdfgen
add_subdirectory(msdfgen)
target_link_libraries(mondradiko-lib PUBLIC msdfgen::msdfgen)
target_link_libraries(mondradiko-lib PUBLIC msdfgen::msdfgen-ext)

# Add Wasmtime
# TODO(marceline-cramer) Download the appropriate package per-OS and architecture
# TODO(marceline-cramer) Move this to its own module
# TODO(marceline-cramer) Make this a target, so that it automatically redownloads
set(WASMTIME_C_API_PATH "${CMAKE_CURRENT_BINARY_DIR}/wasmtime-v0.21.0-x86_64-linux-c-api/")
set(WASMTIME_C_API_LIB "${WASMTIME_C_API_PATH}/lib/")
target_include_directories(mondradiko-lib PUBLIC "${WASMTIME_C_API_PATH}/include/")

find_library(WASMTIME wasmtime PATHS "${WASMTIME_C_API_LIB}")
if(NOT WASMTIME)
  message(STATUS "libwasmtime not found; automatically downloading it.")

  set(WASMTIME_C_API_URL https://github.com/bytecodealliance/wasmtime/releases/download/v0.21.0/wasmtime-v0.21.0-x86_64-linux-c-api.tar.xz)
  set(WASMTIME_C_API_FILE "${CMAKE_CURRENT_BINARY_DIR}/wasmtime-v0.21.0-x86_64-linux-c-api.tar.xz")

  file(DOWNLOAD ${WASMTIME_C_API_URL} ${WASMTIME_C_API_FILE})
  file(ARCHIVE_EXTRACT INPUT ${WASMTIME_C_API_FILE})

  find_library(WASMTIME wasmtime PATHS "${WASMTIME_C_API_LIB}")
  if(NOT WASMTIME)
    message(FATAL_ERROR "Failed to automatically download and link libwasmtime")
  endif()
endif()

target_link_libraries(mondradiko-lib PUBLIC ${WASMTIME})
