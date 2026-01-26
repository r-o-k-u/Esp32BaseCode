/**
 * @file WiFiManager.cpp
 * @brief WiFi connection manager implementation
 */

#include "WiFiManager.h"

// Global instance
WiFiManager wifiManager;

/**
 * @brief Constructor
 */
WiFiManager::WiFiManager()
{
    connected = false;
    ssid = "";
    password = "";
}

/**
 * @brief Initialize WiFi connection
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @return true if connected successfully
 */
bool WiFiManager::begin(const char *ssid, const char *password)
{
    this->ssid = String(ssid);
    this->password = String(password);

    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    // Wait for connection with timeout
    uint32_t startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 10000)
    {
        delay(500);
        Serial.print(".");
    }

    connected = (WiFi.status() == WL_CONNECTED);

    if (connected)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nWiFi connection failed!");
    }

    return connected;
}

/**
 * @brief Check if WiFi is connected
 * @return true if connected
 */
bool WiFiManager::isConnected()
{
    return connected;
}

/**
 * @brief Get current SSID
 * @return SSID string
 */
String WiFiManager::getSSID()
{
    return ssid;
}

/**
 * @brief Get current IP address
 * @return IP address string
 */
String WiFiManager::getIP()
{
    return WiFi.localIP().toString();
}

/**
 * @brief Start Access Point mode
 * @param ssid AP SSID
 * @param password AP password (optional)
 */
void WiFiManager::startAP(const char *ssid, const char *password)
{
    Serial.print("Starting AP mode: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

/**
 * @brief Get AP SSID
 * @return AP SSID string
 */
String WiFiManager::getAPSSID()
{
    return WiFi.softAPSSID();
}

/**
 * @brief Get AP IP address
 * @return AP IP address string
 */
String WiFiManager::getAPIP()
{
    return WiFi.softAPIP().toString();
}

/**
 * @brief Disconnect from WiFi
 */
void WiFiManager::disconnect()
{
    WiFi.disconnect();
    connected = false;
    Serial.println("WiFi disconnected");
}

/**
 * @brief Print WiFi status information
 */
void WiFiManager::printStatus()
{
    Serial.println("=== WiFi Status ===");
    Serial.printf("Status: %s\n", connected ? "Connected" : "Disconnected");
    if (connected)
    {
        Serial.printf("SSID: %s\n", ssid.c_str());
        Serial.printf("IP: %s\n", getIP().c_str());
        Serial.printf("Signal: %d dBm\n", WiFi.RSSI());
    }
    Serial.println("==================");
}
