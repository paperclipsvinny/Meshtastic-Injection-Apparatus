#pragma once
#include <Arduino.h>
#include <Preferences.h>

class WifiConfig {
public:
    bool apEnabled = true;   // off until the user turns it on during setup
    String ssid = "Mesh Injection Apparatus";
    String password = "Mesh-Inject-7f3K9pQ2"; //hardcoded for now

    void begin(); 
    void setSsid(const String& newSsid);
    void setPassword(const String& newPassword);
    void setEnabled(bool enabled);

private: 
    Preferences prefs;
};

