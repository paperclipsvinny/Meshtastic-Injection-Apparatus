#include "HID.h"
#include "Logger.h"
#include "USB.h"
#include "USBHIDKeyboard.h"

//Keyboard setup
static USBHIDKeyboard Keyboard;

void HID::begin(){
    USB.begin();
    Keyboard.begin();
}

void HID::executeCommand(char* keyword, char* argument){
    Logger::raw(keyword);
    if (argument != NULL){
        Logger::raw(argument);
    }
Logger::rawln();

//STRING
if (strcmp(keyword, "STRING") == 0 && argument != NULL){
    Keyboard.print(argument);
}

//STRINGLN(type text and enter)
    else if (strcmp(keyword, "STRINGLN") == 0 && argument != NULL){
        Keyboard.println(argument);
    }

    // ENTER
    else if (strcmp(keyword, "ENTER") == 0) {
        Keyboard.write(KEY_RETURN);
    }
    // TAB
    else if (strcmp(keyword, "TAB") == 0) {
        Keyboard.write(KEY_TAB);
    }
    // ESCAPE
    else if (strcmp(keyword, "ESCAPE") == 0 || strcmp(keyword, "ESC") == 0) {
        Keyboard.write(KEY_ESC);
    }
    // SPACE
    else if (strcmp(keyword, "SPACE") == 0) {
        Keyboard.write(' ');
    }
    // BACKSPACE
    else if (strcmp(keyword, "BACKSPACE") == 0) {
        Keyboard.write(KEY_BACKSPACE);
    }
    // DELETE
    else if (strcmp(keyword, "DELETE") == 0) {
        Keyboard.write(KEY_DELETE);
    }
    // UP / DOWN / LEFT / RIGHT
    else if (strcmp(keyword, "UP") == 0) {
        Keyboard.write(KEY_UP_ARROW);
    }
    else if (strcmp(keyword, "DOWN") == 0) {
        Keyboard.write(KEY_DOWN_ARROW);
    }
    else if (strcmp(keyword, "LEFT") == 0) {
        Keyboard.write(KEY_LEFT_ARROW);
    }
    else if (strcmp(keyword, "RIGHT") == 0) {
        Keyboard.write(KEY_RIGHT_ARROW);
    }
    // F1-F12
    else if (strcmp(keyword, "F1") == 0) { Keyboard.write(KEY_F1); }
    else if (strcmp(keyword, "F2") == 0) { Keyboard.write(KEY_F2); }
    else if (strcmp(keyword, "F3") == 0) { Keyboard.write(KEY_F3); }
    else if (strcmp(keyword, "F4") == 0) { Keyboard.write(KEY_F4); }
    else if (strcmp(keyword, "F5") == 0) { Keyboard.write(KEY_F5); }
    else if (strcmp(keyword, "F6") == 0) { Keyboard.write(KEY_F6); }
    else if (strcmp(keyword, "F7") == 0) { Keyboard.write(KEY_F7); }
    else if (strcmp(keyword, "F8") == 0) { Keyboard.write(KEY_F8); }
    else if (strcmp(keyword, "F9") == 0) { Keyboard.write(KEY_F9); }
    else if (strcmp(keyword, "F10") == 0) { Keyboard.write(KEY_F10); }
    else if (strcmp(keyword, "F11") == 0) { Keyboard.write(KEY_F11); }
    else if (strcmp(keyword, "F12") == 0) { Keyboard.write(KEY_F12); }
    // DELAY - pause execution
    else if (strcmp(keyword, "DELAY") == 0 && argument != NULL) {
        int ms = atoi(argument);
        delay(ms);
    }
    // GUI / WINDOWS - Windows key combos
    else if (strcmp(keyword, "GUI") == 0 || strcmp(keyword, "WINDOWS") == 0) {
        Keyboard.press(KEY_LEFT_GUI);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.press(argument[0]);
            Keyboard.release(argument[0]);
        }
        Keyboard.release(KEY_LEFT_GUI);
    }
    // CTRL - Control key combos
    else if (strcmp(keyword, "CTRL") == 0 || strcmp(keyword, "CONTROL") == 0) {
        Keyboard.press(KEY_LEFT_CTRL);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.press(argument[0]);
            Keyboard.release(argument[0]);
        }
        Keyboard.release(KEY_LEFT_CTRL);
    }
    // ALT - Alt key combos
    else if (strcmp(keyword, "ALT") == 0) {
        Keyboard.press(KEY_LEFT_ALT);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.press(argument[0]);
            Keyboard.release(argument[0]);
        }
        Keyboard.release(KEY_LEFT_ALT);
    }
    // SHIFT - Shift key combos
    else if (strcmp(keyword, "SHIFT") == 0) {
        Keyboard.press(KEY_LEFT_SHIFT);
        if (argument != NULL && strlen(argument) > 0) {
            Keyboard.press(argument[0]);
            Keyboard.release(argument[0]);
        }
        Keyboard.release(KEY_LEFT_SHIFT);
    }
    // CAPSLOCK
    else if (strcmp(keyword, "CAPSLOCK") == 0) {
        Keyboard.write(KEY_CAPS_LOCK);
    }
    // PRINTSCREEN
    else if (strcmp(keyword, "PRINTSCREEN") == 0) {
        Keyboard.write(HID_KEY_PRINT_SCREEN);
    }
    // REM - comment, do nothing
    else if (strcmp(keyword, "REM") == 0) {
        // Ignore comments
    }
    // Unknown command
    else {
        Logger::errorf("[MIA] Unknown command: %s", keyword);
    }

    Keyboard.releaseAll();
}

void HID::executeCommands(char* commandstring){
    char* cmdCopy = strdup(commandstring);
    char* singleCmd = strtok(cmdCopy, ",");

    while (singleCmd != NULL){
        while (*singleCmd == ' ') singleCmd++;
        char* end = singleCmd + strlen(singleCmd) - 1;
        while (end > singleCmd && *end == ' '){
            *end = '\0';
            end--;
        }
        if (strlen(singleCmd) > 0){
            char* space = strchr(singleCmd, ' ');
            char* argument = NULL;    
                        
            if (space != NULL){
                *space = '\0';
                argument = space + 1;
                while (*argument == ' ') argument++;
            }
            executeCommand(singleCmd, argument);
            delay(100);
        }
        singleCmd = strtok(NULL, ",");
    }
    free(cmdCopy);
}
