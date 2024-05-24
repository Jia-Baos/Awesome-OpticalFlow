#!/bin/bash

mdy=`date +%m-%d-%Y`
echo "Date: $mdy"

echo "display the optflow..."
./flo2png.exe frame_0029_optflow.flo frame_0029_optflow.png
