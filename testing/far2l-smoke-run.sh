#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

mkdir -p /tmp/far2l-smoke/output
APP="$1"
if [ "$APP" = "" ]; then
	echo 'Please specify path to far2l binary as argument'
	echo 'Note that far2l must be built with -DTESTING=Yes'
	exit 1
fi
BINARY="$SCRIPT_DIR/far2l-smoke"
if [ ! -f "$BINARY" ] || [ "$BINARY" -ot "$SCRIPT_DIR/far2l-smoke.go" ]; then
	echo '--->' Prepare
	cd "$SCRIPT_DIR"
	go get far2l-smoke
	echo PREPARE: downloading modules
	go mod download
	echo PREPARE: building
	go build
	echo PREPARE: done
fi

echo 'Cleaning up...'
for test in "$SCRIPT_DIR"/tests/*; do
	if [ -d "$test" ]; then
		rm -rf "$test"/workdir
	fi
done

if [ "$2" == "clean" ]; then
	exit 0
fi

echo 'Starting tests:' "$SCRIPT_DIR"/tests/"$2"*
for test in "$SCRIPT_DIR"/tests/*; do
	if [ -d "$test" ]; then
		mkdir -p "$test"/workdir
		if [ -e "$test"/initdir ]; then
			cp -r -f "$test"/initdir/* "$test"/workdir/
		fi
	fi
done

"$BINARY" "$APP" "$SCRIPT_DIR"/tests/"$2"*
