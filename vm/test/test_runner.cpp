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

void test_print_loop() {
    std::cout << "Running test_print_loop..." << std::endl;
    TinyVM vm;
    
    // Program: Print numbers 1 to 10
    // R0: Counter (1)
    // R1: Limit (11)
    // R2: Increment (1)
    // R3: Temp for comparison
    uint8_t program[] = {
        // Init
        LOAD, 0, 1,     // 0: R0 = 1
        LOAD, 1, 11,    // 3: R1 = 11
        LOAD, 2, 1,     // 6: R2 = 1
        
        // Loop (Address 9)
        MOV, 3, 0,      // 9: R3 = R0
        LT, 3, 1,       // 12: R3 = (R3 < R1)
        JZ, 3, 27,      // 15: If R3 == 0 (False), Jump to End (27)
        
        PRINT, 0, 0,    // 18: Print R0
        ADD, 0, 2,      // 21: R0 = R0 + R2
        JMP, 0, 9,      // 24: Jump to Loop (9)
        
        // End (Address 27)
        HALT, 0, 0      // 27: Halt
    };

    vm.loadProgram(program, sizeof(program));
    vm.run();

    assert(vm.registers[0] == 11);
    std::cout << "test_print_loop PASSED" << std::endl;
}

int main() {
    std::cout << "Starting VM Tests..." << std::endl;
    
    test_addition();
    test_subtraction();
    test_print_loop();
    
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
