#!/bin/bash

readonly PKGS="g++ make cmake rapidjson-dev librestbed-dev python3"

for PKG in $PKGS; do
    if ! dpkg -s "$PKG" &>/dev/null; then
        echo "Package '$PKG' not installed!"
        exit 1
    fi
done

if [ -f src/deviceMonitor/device-monitor ]; then
    cmake . && make -j 2
    [ $? -ne 0 ] && exit 1
fi

exit 0