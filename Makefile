.PHONY: compile upload dictionary dictionary-check test-dictionary
.DEFAULT_GOAL := compile

FQBN := arduino:avr:nano
PORT ?= /dev/cu.usbserial-10
CPU_FREQUENCY := 8000000L
TEST_CXX ?= c++

dictionary:
	python3 words/build_dictionary.py

dictionary-check:
	python3 words/build_dictionary.py --check

test-dictionary: dictionary-check
	@set -e; test_binary=$$(mktemp /tmp/phyrdle-dictionary.XXXXXX); \
	trap 'rm -f "$$test_binary"' EXIT; \
	$(TEST_CXX) -O2 -std=c++17 -Wall -Wextra -pedantic -Itests/arduino_stub \
		tests/dictionary_test.cpp src/wordset/WordSet.cpp -o "$$test_binary"; \
	"$$test_binary"

compile:
	arduino-cli compile --fqbn $(FQBN) --build-property build.f_cpu=$(CPU_FREQUENCY) arduino_sketch.ino

upload: compile
	arduino-cli upload --port $(PORT) --fqbn $(FQBN) arduino_sketch.ino
