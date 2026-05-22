#!/bin/bash
set -euo pipefail

# Create the directory for storing built packages
mkdir -p dist

# Loop over all supported packaging targets and build them within a container
for DISTRO in fedora ubuntu; do
  echo "Building package for ${DISTRO}..."
  podman build -f packaging/Containerfile.${DISTRO} -t masm-build-${DISTRO} .

  # Instantiate the built image
  CONTAINER=$(podman create masm-build-${DISTRO})
  # Get the exact path of the built package
  PKG_PATH=$(podman run --rm masm-build-${DISTRO} find /build/build -maxdepth 1 -regextype posix-extended -regex '.*masm-.*(rpm|deb)')
  podman cp "${CONTAINER}:${PKG_PATH}" dist

  TAR_PATH=$(podman run --rm masm-build-${DISTRO} find /build/build -maxdepth 1 -regextype posix-extended -regex '.*masm-.*tar\.gz')
  if [[ -n "${TAR_PATH}" ]]; then
    podman cp "${CONTAINER}:${TAR_PATH}" dist
  fi

  podman rm "${CONTAINER}"
done
