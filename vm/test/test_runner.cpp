#define UNIT_TESTING
#include "mock_arduino.h"

// Include the VM implementation directly
// Since it's a .ino file, we treat it as a header for testing purposes
#include "../vm_complete.ino"

#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

MockSerial Serial;

static void print_registers(TinyVM &vm) {
    std::cout << "Regs: ";
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << "R" << i << "=" << vm.registers[i];
        if (i + 1 < NUM_REGISTERS) std::cout << ", ";
    }
    std::cout << std::endl;
}

// Function to parse vmcode file and convert to bytecode
std::vector<uint8_t> parse_vmcode_file(const std::string& filename) {
    // Map instruction names to opcodes - these match the opcodes in vm_complete.ino
    std::map<std::string, uint8_t> opcode_map = {
        {"NOP", 0x00},
        {"ADD", 0x01}, {"SUB", 0x02}, {"MUL", 0x03}, {"DIV", 0x04}, {"MOD", 0x05},
        {"AND", 0x06}, {"OR", 0x07}, {"XOR", 0x08}, {"NOT", 0x09}, {"CMP", 0x0A},
        {"SHL", 0x0B}, {"SHR", 0x0C},
        {"LOAD", 0x10}, {"LOADI", 0x11}, {"LOADI16", 0x12}, {"STORE", 0x13},
        {"LOAD_ADDR", 0x14}, {"PUSH", 0x15}, {"POP", 0x16}, {"PEEK", 0x17}, {"LOADM", 0x18},
        {"JMP", 0x20}, {"JZ", 0x21}, {"JNZ", 0x22}, {"JLT", 0x23}, {"JGT", 0x24},
        {"JLE", 0x25}, {"JGE", 0x26}, {"CALL", 0x27}, {"RET", 0x28}, {"HALT", 0x29},
        {"PRINT", 0x30}, {"TRAP", 0x31}
    };
    
    std::vector<uint8_t> bytecode;
    std::ifstream file(filename);
    std::string line;
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return bytecode;
    }
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string opcode_str;
        int arg1, arg2;
        
        if (iss >> opcode_str >> arg1 >> arg2) {
            auto it = opcode_map.find(opcode_str);
            if (it != opcode_map.end()) {
                bytecode.push_back(it->second); // opcode
                bytecode.push_back(static_cast<uint8_t>(arg1)); // arg1
                bytecode.push_back(static_cast<uint8_t>(arg2)); // arg2
            } else {
                std::cerr << "Warning: Unknown opcode " << opcode_str << std::endl;
            }
        }
    }
    
    file.close();
    return bytecode;
}

void test_program_vmcode() {
    // Load and parse the program.vmcode file
    std::string vmcode_path = "../../language/program.vmcode";
    std::vector<uint8_t> program = parse_vmcode_file(vmcode_path);
    
    if (program.empty()) {
        std::cout << "[FAIL] test_program_vmcode: Could not load or parse " << vmcode_path << std::endl;
        assert(false);
        return;
    }
    
    std::cout << "Running test_program_vmcode with " << program.size() << " bytes of bytecode..." << std::endl;
    
    // Print the bytecode for debugging
    std::cout << "Bytecode: ";
    for (size_t i = 0; i < program.size(); i += 3) {
        if (i + 2 < program.size()) {
            std::cout << "[" << (int)program[i] << " " << (int)program[i+1] << " " << (int)program[i+2] << "] ";
        }
    }
    std::cout << std::endl;
    
    // Run the program - this appears to be a loop that processes array elements
    // Based on the vmcode, it seems to store values 0-4 in heap[0]-heap[4], 
    // then iterates through them and prints each value
    TinyVM vm;
    vm.loadProgram(program.data(), program.size());
    vm.run();
    print_registers(vm);
    
    // The program doesn't have specific register expectations since it mainly
    // works with memory and prints output, so we just verify it runs without crashing
    std::cout << "test_program_vmcode completed successfully" << std::endl;
}

int main() {
    std::cout << "Starting VM Test with program.vmcode..." << std::endl;
    test_program_vmcode();
    std::cout << "Test completed!" << std::endl;
    return 0;
}
