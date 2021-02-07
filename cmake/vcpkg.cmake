# originally based on https://github.com/sutambe/cpptruths/blob/master/cpp0x/vcpkg_test/cmake/AutoVcpkg.cmake (Apache 2.0)
# 2021.02.07 humbletim -- updated to support --overlay-triplets and --overlay-ports subdirs and simplified vcpkg_install(a b c) use

set(VCPKG_SUBDIR ${CMAKE_CURRENT_LIST_DIR}) # assumes vcpkg-overlays/ and vcpkg-ports/ live next to this .cmake file

macro(vcpkg_install)
  if(NOT EXISTS ${VCPKG_ROOT})
    message("Cloning vcpkg in ${VCPKG_ROOT}")
    execute_process(COMMAND git clone https://github.com/Microsoft/vcpkg.git ${VCPKG_ROOT})
    execute_process(COMMAND git checkout 2020.11 WORKING_DIRECTORY ${VCPKG_ROOT})
  endif()

  if(NOT EXISTS ${VCPKG_ROOT}/README.md)
    message(FATAL_ERROR "***** FATAL ERROR: Could not clone vcpkg *****")
  endif()

  if(WIN32)
    set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg.exe)
    set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.bat)
  else()
    set(VCPKG_EXEC ${VCPKG_ROOT}/vcpkg)
    set(VCPKG_BOOTSTRAP ${VCPKG_ROOT}/bootstrap-vcpkg.sh)
  endif()

  if(NOT EXISTS ${VCPKG_EXEC})
    message("Bootstrapping vcpkg in ${VCPKG_ROOT}")
    execute_process(COMMAND ${VCPKG_BOOTSTRAP} WORKING_DIRECTORY ${VCPKG_ROOT})
  endif()

  if(NOT EXISTS ${VCPKG_EXEC})
    message(FATAL_ERROR "***** FATAL ERROR: Could not bootstrap vcpkg *****")
  endif()

  set(CMAKE_TOOLCHAIN_FILE ${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "")

  message(STATUS "***** Checking project third party dependencies in ${VCPKG_ROOT} *****")
  execute_process(COMMAND ${VCPKG_EXEC} install --overlay-ports=${VCPKG_SUBDIR}/vcpkg-ports --overlay-triplets=${VCPKG_SUBDIR}/vcpkg-triplets ${ARGN} WORKING_DIRECTORY ${VCPKG_ROOT})
  include(${CMAKE_TOOLCHAIN_FILE})
endmacro()
