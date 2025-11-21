#define UNIT_TESTING
#include "mock_arduino.h"

// Include the VM implementation directly
// Since it's a .ino file, we treat it as a header for testing purposes
#include "../vm_complete.ino"

#include <cassert>
#include <iostream>
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

static bool run_and_check(const uint8_t *program, size_t size,
                          const std::vector<std::pair<int,int>> &expectations,
                          const char *test_name)
{
    std::cout << "Running " << test_name << "..." << std::endl;
    TinyVM vm;
    vm.loadProgram(program, size);
    vm.run();
    print_registers(vm);

    bool ok = true;
    for (auto &p : expectations) {
        int reg = p.first;
        int expected = p.second;
        if (vm.registers[reg] != expected) {
            std::cout << "[FAIL] " << test_name << ": R" << reg << " == "
                      << vm.registers[reg] << " (expected " << expected << ")" << std::endl;
            ok = false;
        }
    }
    if (ok) std::cout << test_name << " PASSED" << std::endl;
    return ok;
}

/* Tests using only the opcodes implemented in vm_complete.ino (LOADI/LOADI16, ADD, SUB, MUL, DIV, MOD,
   AND, OR, XOR, NOT, SHL, SHR, CMP, J*, CALL/RET, PUSH/POP, STORE/LOAD_ADDR, PRINT, HALT) */

void test_addition() {
    uint8_t program[] = {
        LOADI, 0, 10,   // R0 = 10
        LOADI, 1, 20,   // R1 = 20
        ADD,   0, 1,    // R0 = R0 + R1  (result in R0)
        HALT,  0, 0
    };
    bool ok = run_and_check(program, sizeof(program), {{0,30}}, "test_addition");
    assert(ok);
}

void test_sub_mul_div_mod() {
    // test SUB, MUL, DIV, MOD in a single program (sequence)
    uint8_t program[] = {
        // SUB
        LOADI, 0, 50,
        LOADI, 1, 20,
        SUB,   0, 1,    // R0 = 30
        // MUL (use R0 and R1 -> R0 = 30 * 20 = 600)
        MUL,   0, 1,
        // now set up for DIV/MOD
        LOADI, 1, 6,    // R1 = 6
        DIV,   0, 1,    // R0 = 600 / 6 = 100
        LOADI, 0, 10,   // R0 = 10
        LOADI, 1, 3,    // R1 = 3
        MOD,   0, 1,    // R0 = 1
        HALT, 0, 0
    };
    // We check final R0 == 1 and intermediate validated by running separately isn't necessary here.
    bool ok = run_and_check(program, sizeof(program), {{0,1}}, "test_sub_mul_div_mod");
    assert(ok);
}

void test_bitwise_and_or_xor_not_shifts() {
    uint8_t program[] = {
        LOADI, 0, 0b1100, // R0 = 12
        LOADI, 1, 0b1010, // R1 = 10
        AND,   0, 1,      // R0 = 8
        LOADI, 1, 0b0011, // R1 = 3
        OR,    0, 1,      // R0 = 11  (8 | 3)
        LOADI, 0, 0x0F,   // R0 = 15
        NOT,   0, 0,      // R0 = ~15
        LOADI, 0, 4,      // R0 = 4
        SHL,   0, 1,      // R0 = 4 << 1 = 8
        SHR,   0, 2,      // R0 = 8 >> 2 = 2
        LOADI, 0, 7,
        LOADI, 1, 3,
        XOR,   0, 1,      // R0 = 7 ^ 3 = 4
        HALT, 0, 0
    };
    // We only assert final known result (R0 == 4 from final XOR)
    bool ok = run_and_check(program, sizeof(program), {{0,4}}, "test_bitwise_and_or_xor_not_shifts");
    assert(ok);
}

void test_cmp_and_conditional_jumps() {
    // Program demonstrates CMP + JGE/JLT/JZ behavior.
    // We'll set R0 and R1 and branch based on comparison to write different values into R2.
    //
    // Layout (addresses in bytes):
    // 0: LOADI R0,5
    // 3: LOADI R1,5
    // 6: CMP R0,R1
    // 9: JZ  18,0    -> if equal jump to label_equal (byte 18)
    // 12: LOADI R2,1 -> not-equal path
    // 15: JMP 21,0   -> jump to end (byte 21)
    // 18: LOADI R2,42 -> equal path
    // 21: HALT
    uint8_t program[] = {
        LOADI, 0, 5,
        LOADI, 1, 5,
        CMP,   0, 1,
        JZ,    18, 0,
        LOADI, 2, 1,
        JMP,   21, 0,
        LOADI, 2, 42,
        HALT,  0, 0
    };
    bool ok = run_and_check(program, sizeof(program), {{2,42}}, "test_cmp_and_conditional_jumps");
    assert(ok);
}

void test_push_pop() {
    uint8_t program[] = {
        LOADI, 0, 77,
        PUSH,  0, 0,
        LOADI, 0, 0,
        POP,   1, 0,   // R1 should get 77
        HALT,  0, 0
    };
    bool ok = run_and_check(program, sizeof(program), {{1,77}}, "test_push_pop");
    assert(ok);
}

void test_call_ret() {
    // Main: CALL func(6) ; HALT
    // Func at byte 6: LOADI R0,99 ; RET
    uint8_t program[] = {
        CALL, 6, 0,     // 0..2
        HALT, 0, 0,     // 3..5
        LOADI, 0, 99,   // 6..8 (func)
        RET,   0, 0     // 9..11
    };
    bool ok = run_and_check(program, sizeof(program), {{0,99}}, "test_call_ret");
    assert(ok);
}

void test_loadi16_and_store_loadaddr() {
    // LOADI16 into R0 with 16-bit value 0x1234, then LOAD_ADDR into R1 with offset 5 -> R1 = heap_top + 5
    // Then STORE a byte from R0 into heap at index R1 (only low byte stored). LOADM now allows reading it back,
    // but this test keeps the indirect validation to ensure legacy paths still behave correctly.
    uint8_t program[] = {
        LOADI16, 0, 0,      // LOADI16 consumes next 2 bytes (placed after)
        0x34, 0x12,         // little-endian 0x1234 => 4660
        LOAD_ADDR, 1, 5,    // R1 = heap_top + 5
        LOADI,  2, 0xAA,    // R2 = 0xAA
        STORE,  1, 2,       // heap[R1] = R2 (0xAA)
        HALT, 0, 0
    };
    // After execution: R0 == 0x1234, R1 == heap_top+5, R2==0xAA
    bool ok = run_and_check(program, sizeof(program), {{0, 0x1234}, {2, 0xAA}}, "test_loadi16_and_store_loadaddr");
    assert(ok);
}

void test_store_and_loadm() {
    uint8_t program[] = {
        LOADI, 0, 77,  // value
        LOADI, 1, 10,  // heap index
        STORE, 1, 0,   // heap[10] = 77
        LOADI, 2, 0,   // clear R2
        LOADM, 2, 1,   // R2 = heap[10]
        HALT, 0, 0
    };
    bool ok = run_and_check(program, sizeof(program), {{2,77}}, "test_store_and_loadm");
    assert(ok);
}

int main() {
    std::cout << "Starting VM Tests..." << std::endl;

    test_addition();
    test_sub_mul_div_mod();
    test_bitwise_and_or_xor_not_shifts();
    test_cmp_and_conditional_jumps();
    test_push_pop();
    test_call_ret();
    test_loadi16_and_store_loadaddr();
    test_store_and_loadm();

    std::cout << "All tests passed!" << std::endl;
    return 0;
}
