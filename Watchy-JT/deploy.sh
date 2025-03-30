#!/bin/bash

exec arduino-cli compile -v -u \
	-p /dev/ttyACM0 \
	--config-file ./sketch.yaml \
	--profile watchy \
	.
