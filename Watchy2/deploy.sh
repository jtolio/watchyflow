#!/bin/bash

exec arduino-cli compile -u -j 0 \
	-p /dev/ttyACM0 \
	--config-file ./sketch.yaml \
	--profile watchy \
	.
