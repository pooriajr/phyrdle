.PHONY: compile upload

FQBN := arduino:avr:nano
PORT ?= /dev/cu.usbserial-10
CPU_FREQUENCY := 8000000L

compile:
	arduino-cli compile --fqbn $(FQBN) --build-property build.f_cpu=$(CPU_FREQUENCY) arduino_sketch.ino

upload: compile
	arduino-cli upload --port $(PORT) --fqbn $(FQBN) arduino_sketch.ino
