# automatically enable VCPKG if USE_VCPKG environment or -DUSE_VCPKG=ON is specified
# note: this uses vcpkg fancy new (experimental) manifests mode -- see vcpkg.json in root project dir for deps
macro(enable_vcpkg_support)
  set(ENV{VCPKG_FEATURE_FLAGS} manifests)
  if (WIN32)
    set(ENV{VCPKG_DEFAULT_TRIPLET} "x64-windows")
  endif()
  set(ENV{VCPKG_OVERLAY_PORTS} ${CMAKE_CURRENT_LIST_DIR}/vcpkg-ports)
  set(ENV{VCPKG_OVERLAY_TRIPLETS} ${CMAKE_CURRENT_LIST_DIR}/vcpkg-triplets)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "")
  message(STATUS "***** VCPKG MODE ENABLED *****")
endmacro()

if (ENV{USE_VCPKG} AND NOT DEFINED USE_VCPKG)
  set(USE_VCPKG ENV{USE_VCPKG})
endif()

if (USE_VCPKG)
  enable_vcpkg_support()
endif()
