#include "Logger.h"
#include <stdarg.h>

void Logger::info(const char* msg){
    Serial.println(msg);
}

void Logger::error(const char* msg){
    Serial.print("[ERROR] ");
    Serial.println(msg);
}

void Logger::debug(const char* msg){
    Serial.print("[DEBUG] ");
    Serial.println(msg);
}

static void vprintfHelper(const char* prefix, const char* fmt, va_list args) {
    char buf [256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    if (prefix) Serial.print(prefix);
    Serial.println(buf); 
}

void Logger::infof(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintfHelper(nullptr, fmt, args);
    va_end(args);
}

void Logger::errorf(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintfHelper("[ERROR] ", fmt, args);
    va_end(args);
}

void Logger::debugf(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vprintfHelper("[DEBUG] ", fmt, args);
    va_end(args);
}

void Logger::raw(const char* msg){
    Serial.print(msg);
}

void Logger::rawln(const char* msg){
    Serial.println(msg);
}

void Logger::raw(int value){
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    Serial.print(buf);
}

void Logger::raw(float value){
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", value);
    Serial.print(buf);
}

//for Hex methods 
void Logger::hexByte(uint8_t value){
    char buf[4];
    snprintf(buf, sizeof(buf), "%02X ", value);
    Serial.print(buf);
}