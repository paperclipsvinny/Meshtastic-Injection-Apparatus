#pragma once
#include <Arduino.h>

class Logger {
public: 
    static void info(const char* msg);
    static void error(const char* msg);
    static void debug(const char* msg);

    //printf-style - always ends the line by adding a new line.
    static void infof(const char* fmt, ...);
    static void errorf(const char* fmt, ...);
    static void debugf(const char* fmt, ...);

    //raw passthrough style, for building a line piece by piece.
    static void raw(const char* msg);
    static void raw(int value);
    static void raw(float value);
    static void rawln(const char* msg = "");

    //buffer for converting hex
    static void hexByte(uint8_t value);
};
