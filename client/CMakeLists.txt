# Copyright (c) 2020-2021 the Mondradiko contributors.
# SPDX-License-Identifier: LGPL-3.0-or-later

include(FindPackageHelper)
find_package_system_first(mondradiko::cli11 VCPKG "CLI11" VCPKG_MODULES CLI11::CLI11)

add_executable(mondradiko-client client_main.cc)
target_link_libraries(mondradiko-client mondradiko-core)
target_link_libraries(mondradiko-client mondradiko::cli11)
