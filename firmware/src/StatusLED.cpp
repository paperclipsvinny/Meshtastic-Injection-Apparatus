#include "StatusLED.h"

uint8_t StatusLED::LEDPin = 0;
void StatusLED::begin(uint8_t pin){
    LEDPin = pin;
    pinMode(LEDPin, OUTPUT);
}

void StatusLED::blinkReceived(){
    //fast blink for visual feedback when packets recieved
    for (int i = 0; i < 3; i++){
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
    }
}
    