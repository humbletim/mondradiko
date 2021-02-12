# automatically enable VCPKG if ENV{USE_VCPKG} or -DUSE_VCPKG=ON is specified
# actual vcpkg dependencies are managed through vcpkg.json in the project root

# NOTE: this file needs to be included _before_ the first project() statement
#   in order to properly bootstrap the CMAKE_TOOLCHAIN_FILE

macro(enable_vcpkg_support)
  # enable vcpkg manifest mode
  set(ENV{VCPKG_FEATURE_FLAGS} manifests)

  # make sure x64 is used on windows (vcpkg otherwise defaults to x86-windows)
  if (WIN32)
    set(ENV{VCPKG_DEFAULT_TRIPLET} "x64-windows")
  endif()

  # enable use of our custom triplet overlays
  set(ENV{VCPKG_OVERLAY_TRIPLETS} ${CMAKE_CURRENT_LIST_DIR}/vcpkg-triplets)

  # enable use of our custom dependency portfiles
  set(ENV{VCPKG_OVERLAY_PORTS} ${CMAKE_CURRENT_LIST_DIR}/vcpkg-ports)

  # bootstrap the vcpkg toolchain file
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

  message(STATUS "***** VCPKG MODE ENABLED *****")
endmacro()

if (DEFINED ENV{USE_VCPKG} AND NOT DEFINED USE_VCPKG)
  set(USE_VCPKG $ENV{USE_VCPKG})
endif()

if (USE_VCPKG)
  enable_vcpkg_support()
endif()
