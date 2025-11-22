#define UNIT_TESTING
#include "mock_arduino.h"
#include "../vm_complete.ino"
#include <iostream>
#include <fstream>
#include <vector>

MockSerial Serial;

static void print_registers(TinyVM &vm) {
    std::cout << "Regs: ";
    for (int i = 0; i < NUM_REGISTERS; ++i) {
        std::cout << "R" << i << "=" << vm.registers[i];
        if (i + 1 < NUM_REGISTERS) std::cout << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> program(size);
    if (!file.read((char*)program.data(), size)) {
        std::cerr << "Failed to read file" << std::endl;
        return 1;
    }

    TinyVM vm;
    vm.loadProgram(program.data(), size);
    vm.run();
    
    print_registers(vm);

    return 0;
}
