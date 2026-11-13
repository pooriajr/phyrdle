.PHONY: compile upload dictionary dictionary-check test-dictionary test-light-feedback
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
	python3 tests/test_answer_selection.py
	@set -e; test_binary=$$(mktemp /tmp/phyrdle-dictionary.XXXXXX); \
	trap 'rm -f "$$test_binary"' EXIT; \
	$(TEST_CXX) -O2 -std=c++17 -Wall -Wextra -pedantic -Itests/arduino_stub \
		tests/dictionary_test.cpp src/wordset/WordSet.cpp -o "$$test_binary"; \
	"$$test_binary"

test-light-feedback:
	@set -e; test_binary=$$(mktemp /tmp/tactle-light-feedback.XXXXXX); \
	trap 'rm -f "$$test_binary"' EXIT; \
	$(TEST_CXX) -std=c++11 -Wall -Wextra -Werror -pedantic \
		tests/light_feedback_test.cpp -o "$$test_binary"; \
	"$$test_binary"

compile:
	arduino-cli compile --fqbn $(FQBN) --build-property build.f_cpu=$(CPU_FREQUENCY) arduino_sketch.ino

upload: compile
	arduino-cli upload --port $(PORT) --fqbn $(FQBN) arduino_sketch.ino
