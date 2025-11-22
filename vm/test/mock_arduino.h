#pragma once

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>
#include <thread>
#include <chrono>
#include <unordered_map>

// Mock Serial class
class MockSerial {
public:
    void begin(long baud) {
        std::cout << "[Serial] Initialized at " << baud << " baud" << std::endl;
    }

    void print(const char* str) {
        std::cout << str;
    }

    void print(const std::string &str) {
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

    void println(const std::string &str) {
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

// --- Arduino constants ---
#ifndef HIGH
#define HIGH 1
#endif
#ifndef LOW
#define LOW 0
#endif
#ifndef INPUT
#define INPUT 0
#endif
#ifndef OUTPUT
#define OUTPUT 1
#endif

// Minimal String wrapper used by the VM (compatible with Arduino String usage)
struct String {
    std::string s;
    String() = default;
    String(const char* c): s(c ? c : "") {}
    String(const std::string& ss): s(ss) {}
    String(int v): s(std::to_string(v)) {}
    String(long v): s(std::to_string(v)) {}
    String(const String &o): s(o.s) {}
    String operator+(const String &o) const { return String(s + o.s); }
    String operator+(const char *c) const { return String(s + (c ? c : "")); }
    String operator+(int v) const { return String(s + std::to_string(v)); }
    const char* c_str() const { return s.c_str(); }
    operator std::string() const { return s; }
};

// Internal mock state for pins / ADC
inline std::unordered_map<int,int> __mock_digital_state;
inline std::unordered_map<int,int> __mock_digital_mode;
inline std::unordered_map<int,int> __mock_analog_values;

// Helper setters for tests
inline void mock_set_analog_read(int pin, int value) { __mock_analog_values[pin] = value; }
inline void mock_set_digital_read(int pin, int value) { __mock_digital_state[pin] = value ? HIGH : LOW; }
inline void mock_clear_pin(int pin) { __mock_digital_state.erase(pin); __mock_analog_values.erase(pin); __mock_digital_mode.erase(pin); }

// Arduino-like functions (simple, test-friendly)
inline void digitalWrite(int pin, int value) {
    __mock_digital_state[pin] = value ? HIGH : LOW;
}
inline int digitalRead(int pin) {
    auto it = __mock_digital_state.find(pin);
    return (it != __mock_digital_state.end()) ? it->second : LOW;
}
inline void pinMode(int pin, int mode) {
    __mock_digital_mode[pin] = mode ? OUTPUT : INPUT;
}
inline int analogRead(int pin) {
    auto it = __mock_analog_values.find(pin);
    return (it != __mock_analog_values.end()) ? it->second : 0;
}
inline void analogWrite(int pin, int value) {
    // store as "digital state" for inspection (mock)
    __mock_digital_state[pin] = value;
}
inline void pwm_write_pin(int pin, int pwm) {
    __mock_digital_state[pin] = pwm;
}
