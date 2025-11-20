#define UNIT_TESTING
#include "mock_arduino.h"

// Include the VM implementation directly
// Since it's a .ino file, we treat it as a header for testing purposes
#include "../vm_complete.ino"

#include <cassert>
#include <iostream>

MockSerial Serial;

void test_addition() {
    std::cout << "Running test_addition..." << std::endl;
    TinyVM vm;
    
    // Program: R0 = 10, R1 = 20, ADD R0, R1 -> R0 should be 30
    uint8_t program[] = {
        LOAD, 0, 10,
        LOAD, 1, 20,
        ADD,  0, 1,
        HALT, 0, 0
    };

    vm.loadProgram(program, sizeof(program));
    vm.run();

    assert(vm.registers[0] == 30);
    std::cout << "test_addition PASSED" << std::endl;
}

void test_subtraction() {
    std::cout << "Running test_subtraction..." << std::endl;
    TinyVM vm;
    
    // Program: R0 = 50, R1 = 20, SUB R0, R1 -> R0 should be 30
    uint8_t program[] = {
        LOAD, 0, 50,
        LOAD, 1, 20,
        SUB,  0, 1,
        HALT, 0, 0
    };

    vm.loadProgram(program, sizeof(program));
    vm.run();

    assert(vm.registers[0] == 30);
    std::cout << "test_subtraction PASSED" << std::endl;
}

int main() {
    std::cout << "Starting VM Tests..." << std::endl;
    
    test_addition();
    test_subtraction();
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
