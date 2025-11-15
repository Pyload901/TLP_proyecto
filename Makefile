# Robot Language to Arduino Translator
# Clean architecture following BNF_Compiler_Machine pattern

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LEX = flex
YACC = bison

# Main translator target (clean architecture)
TARGET = robot_translator
OBJS = main.o compiler.o vm.o

# Source files
SOURCES = main.c compiler.c vm.c
HEADERS = compiler.h vm.h arduino_ir.h

.PHONY: all clean test translate

all: $(TARGET)

# Object files
main.o: main.c compiler.h vm.h
	$(CC) $(CFLAGS) -c main.c

compiler.o: compiler.c compiler.h arduino_ir.h
	$(CC) $(CFLAGS) -c compiler.c

vm.o: vm.c vm.h arduino_ir.h
	$(CC) $(CFLAGS) -c vm.c

# Link final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -lm

# Translate robot program
translate: $(TARGET)
	@echo "🤖 Translating robot program..."
	@echo "==============================="
	@echo ""
	./$(TARGET) my_robot.prog my_robot.ino
	@echo ""
	@echo "📄 Generated Arduino code:"
	@echo "========================="
	@cat my_robot.ino

# Test with sample programs
test: $(TARGET)
	@echo "Testing robot translator..."
	./$(TARGET) my_robot.prog test_output.ino
	@echo "Test completed. Check test_output.ino"

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
	rm -f my_robot.ino test_output.ino

# Help target
help:
	@echo "Robot Language to Arduino Translator"
	@echo "===================================="
	@echo ""
	@echo "Clean architecture following BNF_Compiler_Machine pattern"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the translator"
	@echo "  translate - Translate my_robot.prog to Arduino code"
	@echo "  test      - Run test translation"
	@echo "  clean     - Remove build files"
	@echo "  help      - Show this help"
	@echo ""
	@echo "Usage:"
	@echo "  ./robot_translator <input.prog> [output.ino]"
	@echo "  make translate"