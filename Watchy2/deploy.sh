#!/bin/bash

exec arduino-cli compile -u \
	-p /dev/ttyACM0 \
	--config-file ./sketch.yaml \
	--profile watchy \
	.
