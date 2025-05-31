#!/usr/bin/env bash

# Loop over directories in test/fixtures
for dir in test/fixtures/*; do
  if [ -d "$dir" ]; then
    # Get the directory name without the path
    dir_name=$(basename "$dir")

    cd "$dir" || exit 1

    # The assembly fixture files
    fixture_files=()

    if [ "$dir_name" == "globals" ]; then
      fixture_files+=("${dir_name}One.asm")
      fixture_files+=("${dir_name}Two.asm")
    else
      fixture_files+=("${dir_name}.asm")
    fi

    # Run fixture gen command with fixture files
    ../../../cmake-build-debug/masm-fg "${fixture_files[@]}"

    cd - > /dev/null || exit 1
  fi
done