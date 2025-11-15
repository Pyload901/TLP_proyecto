CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LEX = flex
YACC = bison

# Arduino Translator
ARDUINO_TARGET = arduino_translator
ARDUINO_OBJS = arduino_translator.o arduino_vm.o arduino_compiler.o

# Simple File Translator
SIMPLE_TARGET = simple_translator
SIMPLE_OBJS = simple_translator.o arduino_vm.o arduino_compiler.o

# Source files
SOURCES = arduino_translator.c simple_translator.c arduino_vm.c arduino_compiler.c
HEADERS = arduino_ir.h

.PHONY: all clean test demo translate

all: $(ARDUINO_TARGET) $(SIMPLE_TARGET)

# Object files
arduino_translator.o: arduino_translator.c arduino_ir.h
	$(CC) $(CFLAGS) -c arduino_translator.c

arduino_vm.o: arduino_vm.c arduino_ir.h
	$(CC) $(CFLAGS) -c arduino_vm.c

arduino_compiler.o: arduino_compiler.c arduino_ir.h
	$(CC) $(CFLAGS) -c arduino_compiler.c

simple_translator.o: simple_translator.c arduino_ir.h
	$(CC) $(CFLAGS) -c simple_translator.c

# Link final executables
$(ARDUINO_TARGET): $(ARDUINO_OBJS)
	$(CC) $(CFLAGS) -o $(ARDUINO_TARGET) $(ARDUINO_OBJS) -lm

$(SIMPLE_TARGET): $(SIMPLE_OBJS)
	$(CC) $(CFLAGS) -o $(SIMPLE_TARGET) $(SIMPLE_OBJS) -lm

# Demo target
demo: $(ARDUINO_TARGET)
	@echo "Running Arduino translator demo..."
	@echo "=================================="
	@echo ""
	@echo "Translating to Arduino..."
	@echo ""
	./$(ARDUINO_TARGET) demo_output.ino
	@echo ""
	@echo "Generated Arduino code (demo_output.ino):"
	@echo "========================================="
	@cat demo_output.ino

# Translate a custom program file
translate: $(SIMPLE_TARGET)
	@echo "Translating custom robot program..."
	@echo "==================================="
	@echo ""
	./$(SIMPLE_TARGET) my_robot.prog my_robot.ino
	@echo ""
	@echo "Generated Arduino code (my_robot.ino):"
	@echo "======================================"
	@cat my_robot.ino

# Test with simple program
test: $(ARDUINO_TARGET)
	@echo "Testing Arduino translator..."
	./$(ARDUINO_TARGET) test_output.ino
	@echo "Test completed. Check test_output.ino"

# Clean build files
clean:
	rm -f $(ARDUINO_OBJS) $(SIMPLE_OBJS) $(ARDUINO_TARGET) $(SIMPLE_TARGET)
	rm -f demo_output.ino test_output.ino my_robot.ino

# Help target
help:
	@echo "Arduino Language Translator Makefile"
	@echo "===================================="
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build the Arduino translator"
	@echo "  demo    - Run a demonstration of the translator"
	@echo "  test    - Run tests with sample programs"
	@echo "  clean   - Remove all build files"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make all               # Build the translator"
	@echo "  make demo              # See it in action"
	@echo "  ./arduino_translator output.ino"
	@echo ""
	@echo "The translator converts programs written in your custom language"
	@echo "into Arduino C++ code that can be uploaded to Arduino boards."