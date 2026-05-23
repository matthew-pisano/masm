#!/bin/bash

TARGET="${1}"

set -euo pipefail

# Create the directory for storing built packages
mkdir -p dist

# Loop over all supported packaging targets and build them within a container
for DISTRO in fedora ubuntu windows; do
  if [[ -n "${TARGET}" && "${TARGET}" != "${DISTRO}" ]]; then
    continue
  fi

  echo "Building package for ${DISTRO}..."
  podman build -f packaging/Containerfile.${DISTRO} -t masm-build-${DISTRO} .

  # Instantiate the built image
  CONTAINER=$(podman create masm-build-${DISTRO})
  # Get the exact path of the built package
  PKG_PATH=$(podman run --rm masm-build-${DISTRO} find /build/build -maxdepth 1 -regextype posix-extended -regex '.*masm.*(rpm|deb|exe)')
  podman cp "${CONTAINER}:${PKG_PATH}" dist

  if [[ "${DISTRO}" != "fedora" && "${DISTRO}" != "windows" ]]; then
    continue
  fi

  ZIP_PATH=$(podman run --rm masm-build-${DISTRO} find /build/build -maxdepth 1 -regextype posix-extended -regex '.*masm-.*zip')
  podman cp "${CONTAINER}:${ZIP_PATH}" dist

 if [[ "${DISTRO}" != "fedora" ]]; then
    continue
  fi

  TAR_PATH=$(podman run --rm masm-build-${DISTRO} find /build/build -maxdepth 1 -regextype posix-extended -regex '.*masm-.*tar\.gz')
  podman cp "${CONTAINER}:${TAR_PATH}" dist

  WHEEL_PATH=$(podman run --rm masm-build-${DISTRO} find /build/python/dist -maxdepth 1 -regextype posix-extended -regex '.*pymasm-.*whl')
  podman cp "${CONTAINER}:${WHEEL_PATH}" dist

  podman rm "${CONTAINER}"
done
