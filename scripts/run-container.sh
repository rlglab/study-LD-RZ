#!/bin/bash

CURRENT_DIR=$(pwd)
podman run --rm -it -v "$CURRENT_DIR":/workspace -w /workspace --network="host" rockmanray/gorzone
