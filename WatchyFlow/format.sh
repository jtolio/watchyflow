#!/bin/bash

set -euo pipefail

STYLE="{
	BasedOnStyle: LLVM,
	AlignConsecutiveMacros: true,
	AlignConsecutiveAssignments: true,
	SortIncludes: false
}"

exec find -type f \( \
	-name "*.h" -or \
	-name "*.cpp" -or \
	-name "*.c" -or \
	-name "*.ino" -or \
	-name "*.h.example" \
\) -exec clang-format --style="$STYLE" -i "{}" \;
