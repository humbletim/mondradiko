# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later

if (NOT WIN32)
  find_package(PkgConfig REQUIRED)
endif()
if (USE_VCPKG)
  find_package(Flatbuffers CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
  find_package(lz4 CONFIG REQUIRED)
  find_package(OpenXR CONFIG REQUIRED)
  find_package(SDL2 CONFIG REQUIRED)
  find_package(xxHash CONFIG REQUIRED)
  find_package(wasmtime CONFIG REQUIRED)
  find_package(msdfgen CONFIG REQUIRED)
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

if (USE_VCPKG)
  target_link_libraries(mondradiko-lib PUBLIC flatbuffers::flatbuffers)
  target_link_libraries(mondradiko-lib PUBLIC lz4::lz4)
  target_link_libraries(mondradiko-lib PUBLIC OpenXR::headers OpenXR::openxr_loader OpenXR::openxr-all-supported)
  target_link_libraries(mondradiko-lib PUBLIC SDL2::SDL2main SDL2::SDL2-static)
  target_link_libraries(mondradiko-lib PUBLIC xxHash::xxhash)
endif()

if(TRACY_ENABLE)
  if(UNIX)
    target_link_libraries(mondradiko-lib PUBLIC dl)
    target_link_libraries(mondradiko-lib PUBLIC pthread)
  endif()
endif()

# Add C++ std::filesystem
if (NOT MSVC)
   target_link_libraries(mondradiko-lib PUBLIC stdc++fs)
endif()

if (NOT USE_VCPKG)
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

target_link_libraries(mondradiko-lib PUBLIC GameNetworkingSockets::GameNetworkingSockets)

target_link_libraries(mondradiko-lib PUBLIC Vulkan::Vulkan)

target_link_libraries(mondradiko-lib PUBLIC msdfgen::msdfgen)
target_link_libraries(mondradiko-lib PUBLIC msdfgen::msdfgen-ext)

target_link_libraries(mondradiko-lib PUBLIC wasmtime::wasmtime)
