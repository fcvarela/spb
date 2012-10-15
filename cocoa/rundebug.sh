#!/bin/bash

export DYLD_PRINT_STATISTICS=1
sudo dtruss -f -t open Vaalbara.app/Contents/MacOS/Vaalbara
