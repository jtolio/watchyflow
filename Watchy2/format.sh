#!/bin/bash

set -euo pipefail

STYLE="{
	BasedOnStyle: LLVM,
	AlignConsecutiveMacros: true,
	AlignConsecutiveAssignments: true,
	SortIncludes: false
}"

for file in *.{h,cpp,c,ino}{,.example}; do
	if [ -e "$file" ]; then
		clang-format --style="$STYLE" -i "$file"
	fi
done
