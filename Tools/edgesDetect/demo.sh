#!/bin/bash

mdy=`date +%m-%d-%Y`
echo "Date: $mdy"

echo "Use the SED to detect edges..."
./edgesGet.exe frame_0029.png model.yml