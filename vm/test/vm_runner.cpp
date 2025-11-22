#define UNIT_TESTING
#include "mock_arduino.h"
#include "../vm_complete.ino"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
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

// Map mnemonic string to Opcode enum
static std::map<std::string, uint8_t> opcode_map = {
    {"NOP", NOP}, {"ADD", ADD}, {"SUB", SUB}, {"MUL", MUL}, {"DIV", DIV},
    {"MOD", MOD}, {"AND", AND}, {"OR", OR}, {"XOR", XOR}, {"NOT", NOT},
    {"CMP", CMP}, {"SHL", SHL}, {"SHR", SHR},
    {"LOAD", LOAD}, {"LOADI", LOADI}, {"LOADI16", LOADI16}, {"STORE", STORE},
    {"LOAD_ADDR", LOAD_ADDR}, {"PUSH", PUSH}, {"POP", POP}, {"PEEK", PEEK},
    {"LOADM", LOADM},
    {"JMP", JMP}, {"JZ", JZ}, {"JNZ", JNZ}, {"JLT", JLT}, {"JGT", JGT},
    {"JLE", JLE}, {"JGE", JGE}, {"CALL", CALL}, {"RET", RET}, {"HALT", HALT},
    {"PRINT", PRINT}, {"TRAP", TRAP}
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::vector<uint8_t> program;
    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string mnemonic;
        int arg1, arg2;

        if (!(ss >> mnemonic >> arg1 >> arg2)) {
            // Try parsing without args if it failed (though format says 3 columns)
            // Actually format is "%-7s %3u %3u\n" so it should always have 3 parts
            // But let's be safe.
            continue; 
        }

        if (opcode_map.find(mnemonic) == opcode_map.end()) {
            std::cerr << "Unknown mnemonic: " << mnemonic << std::endl;
            return 1;
        }

        program.push_back(opcode_map[mnemonic]);
        program.push_back((uint8_t)arg1);
        program.push_back((uint8_t)arg2);
    }

    TinyVM vm;
    vm.loadProgram(program.data(), program.size());
    vm.run();
    
    print_registers(vm);

    return 0;
}
