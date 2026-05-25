#!/usr/bin/env bash

# Fixture Generation Script
# Executes the fixture generator on all assembly files within the fixtures directory.

# Loop over directories in tests/fixtures
for dir in tests/fixtures/*; do
  if [ -d "$dir" ]; then
    echo "Processing $dir"

    # Get the directory name without the path
    dir_name=$(basename "$dir")
    
    # Exit if the directory is not found
    cd "$dir" || exit 1

    # The assembly fixture files
    fixture_files=()

    if [ "$dir_name" == "globals" ]; then
      fixture_files+=("${dir_name}One.asm")
      fixture_files+=("${dir_name}Two.asm")
      ../../../cmake-build-debug/bin/masm-fg "${fixture_files[@]}"
      cd - > /dev/null || exit 1
      continue
    else
      fixture_files+=("${dir_name}.asm")
    fi

    # Run fixture gen command with fixture files
    if [[ "${dir_name}" == "mmio_le" ]]; then
      ../../../cmake-build-debug/bin/masm-fg "${fixture_files[@]}" -l --raw-parse --no-sim
    elif [[ "${dir_name}" == "echointer" || "${dir_name}" == "mmio" ]]; then
      ../../../cmake-build-debug/bin/masm-fg "${fixture_files[@]}" --raw-parse --no-sim
    else
      ../../../cmake-build-debug/bin/masm-fg "${fixture_files[@]}" --raw-parse
    fi

    cd - > /dev/null || exit 1
  fi
done
