#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>

// Mock Serial class
class MockSerial {
public:
    void begin(long baud) {
        std::cout << "[Serial] Initialized at " << baud << " baud" << std::endl;
    }

    void print(const char* str) {
        std::cout << str;
    }

    void print(int val, int format = 10) {
        if (format == 16) std::cout << std::hex << val << std::dec;
        else std::cout << val;
    }
    
    void print(char c) {
        std::cout << c;
    }

    void println(const char* str) {
        std::cout << str << std::endl;
    }

    void println(int val, int format = 10) {
        if (format == 16) std::cout << std::hex << val << std::dec << std::endl;
        else std::cout << val << std::endl;
    }
    
    void println() {
        std::cout << std::endl;
    }

    bool available() {
        return false; // Mock: no input available
    }

    int parseInt() {
        return 0; // Mock: return 0
    }

    void flush() {
        std::cout.flush();
    }
};

extern MockSerial Serial;

// Mock Arduino functions
inline void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#define HEX 16
#define DEC 10
