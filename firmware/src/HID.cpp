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

//resolves a single space-separated token (modifier, named key, or literal char) into a keycode
static uint8_t keycodeFor(const char* tok){
    if (strcmp(tok, "GUI") == 0 || strcmp(tok, "WINDOWS") == 0) return KEY_LEFT_GUI;
    if (strcmp(tok, "CTRL") == 0 || strcmp(tok, "CONTROL") == 0) return KEY_LEFT_CTRL;
    if (strcmp(tok, "ALT") == 0) return KEY_LEFT_ALT;
    if (strcmp(tok, "SHIFT") == 0) return KEY_LEFT_SHIFT;
    if (strcmp(tok, "ENTER") == 0) return KEY_RETURN;
    if (strcmp(tok, "TAB") == 0) return KEY_TAB;
    if (strcmp(tok, "ESCAPE") == 0 || strcmp(tok, "ESC") == 0) return KEY_ESC;
    if (strcmp(tok, "SPACE") == 0) return ' ';
    if (strcmp(tok, "BACKSPACE") == 0) return KEY_BACKSPACE;
    if (strcmp(tok, "DELETE") == 0) return KEY_DELETE;
    if (strcmp(tok, "UP") == 0) return KEY_UP_ARROW;
    if (strcmp(tok, "DOWN") == 0) return KEY_DOWN_ARROW;
    if (strcmp(tok, "LEFT") == 0) return KEY_LEFT_ARROW;
    if (strcmp(tok, "RIGHT") == 0) return KEY_RIGHT_ARROW;
    if (strcmp(tok, "F1") == 0) return KEY_F1;
    if (strcmp(tok, "F2") == 0) return KEY_F2;
    if (strcmp(tok, "F3") == 0) return KEY_F3;
    if (strcmp(tok, "F4") == 0) return KEY_F4;
    if (strcmp(tok, "F5") == 0) return KEY_F5;
    if (strcmp(tok, "F6") == 0) return KEY_F6;
    if (strcmp(tok, "F7") == 0) return KEY_F7;
    if (strcmp(tok, "F8") == 0) return KEY_F8;
    if (strcmp(tok, "F9") == 0) return KEY_F9;
    if (strcmp(tok, "F10") == 0) return KEY_F10;
    if (strcmp(tok, "F11") == 0) return KEY_F11;
    if (strcmp(tok, "F12") == 0) return KEY_F12;
    if (strcmp(tok, "CAPSLOCK") == 0) return KEY_CAPS_LOCK;
    if (strcmp(tok, "PRINTSCREEN") == 0) return HID_KEY_PRINT_SCREEN;
    //single literal character, e.g. the 'r' in "GUI r" or the 't' in "CTRL ALT T"
    if (strlen(tok) == 1) return (uint8_t)tok[0];
    return 0; //unresolved
}

//splits a line on spaces, presses every resolved key, then releases them in reverse order.
//this is what makes GUI r, CTRL ALT DELETE, ALT F4, CTRL ALT T, etc. all just work
//without needing a hardcoded branch per combo.
static void executeCombo(char* line){
    uint8_t codes[6];
    int count = 0;
    char* saveptr = NULL;

    char* tok = strtok_r(line, " ", &saveptr);
    while (tok != NULL && count < 6){
        uint8_t code = keycodeFor(tok);
        if (code == 0){
            Logger::errorf("[MIA] Unknown key: %s", tok);
        } else {
            codes[count] = code;
            count++;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    for (int i = 0; i < count; i++){
        Keyboard.press(codes[i]);
        delay(10);
    }
    delay(50);
    for (int i = count - 1; i >= 0; i--){
        Keyboard.release(codes[i]);
        delay(10);
    }
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

    // DELAY - pause execution
    else if (strcmp(keyword, "DELAY") == 0 && argument != NULL) {
        int ms = atoi(argument);
        delay(ms);
    }

    // REM - comment, do nothing
    else if (strcmp(keyword, "REM") == 0) {
        // Ignore comments
    }

    // everything else: treat as a key combo (single key or modifier+key(s)),
    // e.g. GUI r, CTRL ALT DELETE, ALT F4, ENTER, CTRL ALT T
    else {
        char fullLine[220];
        if (argument != NULL){
            snprintf(fullLine, sizeof(fullLine), "%s %s", keyword, argument);
        } else {
            snprintf(fullLine, sizeof(fullLine), "%s", keyword);
        }
        executeCombo(fullLine);
    }
}

void HID::executeCommands(char* commandstring){
    char* cmdCopy = strdup(commandstring);
    char* saveptr = NULL;
    char* singleCmd = strtok_r(cmdCopy, ",", &saveptr);

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
        singleCmd = strtok_r(NULL, ",", &saveptr);
    }
    free(cmdCopy);
    Keyboard.releaseAll();
}