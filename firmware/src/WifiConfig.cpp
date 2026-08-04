#include "WifiConfig.h"

void WifiConfig::begin() {
    prefs.begin("wifi", false);
    apEnabled = prefs.getBool("enabled", true);   // matches your current temporary true default
    ssid = prefs.getString("ssid", ssid);
    password = prefs.getString("password", password);
}

void WifiConfig::setSsid(const String& newSsid) {
    ssid = newSsid;
    prefs.putString("ssid", ssid);
}

void WifiConfig::setPassword(const String& newPassword) {
    password = newPassword;
    prefs.putString("password", password);
}

void WifiConfig::setEnabled(bool enabled) {
    apEnabled = enabled;
    prefs.putBool("enabled", apEnabled);
}