#pragma once
#include <Arduino.h>

class HID {
    public:
    static void begin();
    static void executeCommand(char* keyword, char* argument);
    static void executeCommands(char* commandstring);
};
