# header-only library
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO KhronosGroup/Vulkan-Headers
    REF v1.2.162
    SHA512 7b33f6ef32f1ef34d3b37ad38b077a3a2ebcef62612371afa1b6f4910815bbab8418a837c49b3872b57d2e5012c15623521121f5d62836e46d6f20773841d7b0
    HEAD_REF master
)

vcpkg_configure_cmake(
    SOURCE_PATH ${SOURCE_PATH}
    PREFER_NINJA
)

vcpkg_install_cmake()

# #file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
# file(INSTALL ${SOURCE_PATH}/LICENSE.txt DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT}/ RENAME copyright)
# 
# # This must be vulkan as other vulkan packages expect it there.
# file(COPY "${SOURCE_PATH}/include/vulkan/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/vulkan")

# Handle copyright
file(INSTALL "${SOURCE_PATH}/LICENSE.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
