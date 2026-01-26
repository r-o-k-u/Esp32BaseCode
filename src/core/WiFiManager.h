/**
 * @file WiFiManager.h
 * @brief WiFi connection manager for ESP32
 *
 * Handles WiFi connection, AP mode, and network configuration.
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager
{
private:
    bool connected;
    String ssid;
    String password;

public:
    WiFiManager();

    // WiFi connection
    bool begin(const char *ssid, const char *password);
    bool isConnected();
    String getSSID();
    String getIP();

    // AP mode
    void startAP(const char *ssid, const char *password = nullptr);
    String getAPSSID();
    String getAPIP();

    // Utility
    void disconnect();
    void printStatus();
};

extern WiFiManager wifiManager; // Global instance

#endif // WIFI_MANAGER_H
