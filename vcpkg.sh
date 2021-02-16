#!/bin/bash

# NOTE: to invoke manually while specifying custom overlays and install:
# ./vcpkg/vcpkg \
#   --x-install-root=$PWD/builddir/vcpkg_installed \
#   --overlay-ports=$PWD/cmake/vcpkg-ports \
#   --overlay-triplets=$PWD/cmake/vcpkg-triplets \
#   --triplet=x64-linux

set -ea
CURRENT_SOURCE_DIR=$(git rev-parse --show-toplevel)
OS=${OS:-Linux} # note: OS == Windows_NT in git for windows bash prompt

# make sure x64 is used (otherwise on windows vcpkg defaults to x86-windows)
if [[ "$OS" == "Windows_NT" ]]; then
  VCPKG_DEFAULT_TRIPLET=x64-windows
  VCPKG_EXEC=$CURRENT_SOURCE_DIR/vcpkg/vcpkg.exe
  if [[ ! -x $VCPKG_EXEC ]]; then
    echo "bootstrapping vcpkg..."
    $CURRENT_SOURCE_DIR/vcpkg/bootstrap-vcpkg.bat
  fi
else
  VCPKG_DEFAULT_TRIPLET=x64-linux
  VCPKG_EXEC=$CURRENT_SOURCE_DIR/vcpkg/vcpkg
  if [[ ! -x $VCPKG_EXEC ]]; then
    echo "bootstrapping vcpkg..."
    $CURRENT_SOURCE_DIR/vcpkg/bootstrap-vcpkg.sh
  fi
fi

# enable use of custom triplet overlays
VCPKG_OVERLAY_TRIPLETS=$CURRENT_SOURCE_DIR/cmake/vcpkg-triplets
# enable use custom portfiles
VCPKG_OVERLAY_PORTS=$CURRENT_SOURCE_DIR/cmake/vcpkg-ports

# for var in "${!VCPKG_@}"; do
#     printf '%s=%s\n' "$var" "${!var}"
# done

$VCPKG_EXEC --x-install-root=$CURRENT_SOURCE_DIR/builddir/vcpkg_installed $*
