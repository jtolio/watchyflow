#!/bin/bash

watchyversion="$1"
set -euo pipefail

if [ "x$watchyversion" == "x" ]; then
	watchyversion="v3"
fi

exec arduino-cli compile -u -j 0 \
	-p /dev/ttyACM0 \
	--config-file ./sketch.yaml \
	--profile watchy_$watchyversion \
	.
