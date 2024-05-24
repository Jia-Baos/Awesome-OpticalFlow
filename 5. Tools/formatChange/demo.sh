#!/bin/bash

mdy=`date +%m-%d-%Y`
echo "Date: $mdy"

echo "change image's format..."
./formatChange.exe frame_0029.png frame_0029.ppm
./formatChange.exe frame_0030.png frame_0030.ppm