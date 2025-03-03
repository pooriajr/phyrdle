#!/bin/bash

# Script to monitor Arduino sketch
echo "Monitoring"

arduino-cli monitor -p /dev/cu.usbmodem101 -b arduino:avr:mega