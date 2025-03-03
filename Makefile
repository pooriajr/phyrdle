.PHONY: upload

upload:
	arduino-cli compile --fqbn arduino:avr:mega arduino_sketch.ino && \
	arduino-cli upload -p /dev/cu.usbmodem101 --fqbn arduino:avr:mega arduino_sketch.ino 