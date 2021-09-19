#!/bin/bash

./run_common.sh || exit 1

python3 ./src/deviceSimulator/device_simulator.py
