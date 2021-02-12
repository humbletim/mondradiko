message("== preflight check tools folder: ${CURRENT_INSTALLED_DIR}/tools")

vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO google/flatbuffers
    REF v1.12.0
    SHA512 8a0b88d739fa4694a69d3630140fe89fdd70d50bba4dadd1758d9aa2920cda16700bcafb8d89fe2a09ac907d3f378240c3cb4abc7106318136799836aba4b063
    HEAD_REF master
    PATCHES
        ignore_use_of_cmake_toolchain_file.patch
        no-werror.patch
		fix-uwp-build.patch
)

set(OPTIONS)
if(VCPKG_TARGET_IS_UWP)
    list(APPEND OPTIONS -DFLATBUFFERS_BUILD_FLATC=OFF -DFLATBUFFERS_BUILD_FLATHASH=OFF)
endif()

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
    OPTIONS
        -DFLATBUFFERS_BUILD_TESTS=OFF
        -DFLATBUFFERS_BUILD_GRPCTEST=OFF
        ${OPTIONS}
)

vcpkg_install_cmake()
vcpkg_fixup_cmake_targets(CONFIG_PATH lib/cmake/flatbuffers)
vcpkg_copy_tool_dependencies(${CURRENT_PACKAGES_DIR}/tools/flatbuffers)

file(GLOB flatc_path ${CURRENT_PACKAGES_DIR}/bin/flatc*)
message("== flatc_path ${flatc_path}")
# #if(flatc_path)
make_directory(${CURRENT_PACKAGES_DIR}/tools/flatbuffers)
get_filename_component(flatc_executable ${flatc_path} NAME)
message("== flatc_executable ${flatc_executable}")
file(
    RENAME
    ${flatc_path}
    ${CURRENT_PACKAGES_DIR}/tools/flatbuffers/${flatc_executable}
)
message("== installed as: ${CURRENT_PACKAGES_DIR}/tools/flatbuffers/${flatc_executable}")
# 
# 
# #endif()

file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/debug/include)
file(REMOVE_RECURSE ${CURRENT_PACKAGES_DIR}/bin ${CURRENT_PACKAGES_DIR}/debug/bin)

# Handle copyright
file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT} RENAME copyright)


# list(APPEND CMAKE_PROGRAM_PATH ${CURRENT_INSTALLED_DIR}/tools)
# file(GLOB _VCPKG_TOOLS_DIRS ${CURRENT_INSTALLED_DIR}/tools/*)
# foreach(_VCPKG_TOOLS_DIR IN LISTS _VCPKG_TOOLS_DIRS)
#     if(IS_DIRECTORY "${_VCPKG_TOOLS_DIR}")
#         list(APPEND CMAKE_PROGRAM_PATH "${_VCPKG_TOOLS_DIR}")
#         message("++ tools folder: ${_VCPKG_TOOLS_DIR}")
#     endif()
# endforeach()
# 
# find_program(FLATC_COMMAND flatc)
# 
# message("== find_program(FLATC_COMMAND flatc): ${FLATC_COMMAND}")
