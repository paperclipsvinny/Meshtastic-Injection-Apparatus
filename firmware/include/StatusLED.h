#pragma once
#include <Arduino.h>

class StatusLED{
    public: 
        static void begin(uint8_t pin);
        static void blinkReceived(); // the 3x fast blink "packet recieved" pattern
    private:
        static uint8_t LEDPin;
};