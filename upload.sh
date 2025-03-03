#!/bin/bash

# Script to compile and upload Arduino sketch
echo "Compiling and uploading arduino_sketch.ino..."

arduino-cli compile --fqbn arduino:avr:mega arduino_sketch.ino && 
arduino-cli upload -p /dev/cu.usbmodem101 --fqbn arduino:avr:mega arduino_sketch.ino

if [ $? -eq 0 ]; then
  echo "Upload successful!"
else
  echo "Error during compilation or upload. Check the output above for details."
fi 