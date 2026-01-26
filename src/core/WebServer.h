/**
 * @file WebServer.h
 * @brief Web server manager for ESP32
 *
 * Handles HTTP requests, WebSocket communication, and web interface.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

class WebServerManager
{
private:
    AsyncWebServer *server;
    AsyncWebSocket *ws;
    bool initialized;

public:
    WebServerManager();

    // Web server initialization
    bool begin();

    // WebSocket broadcasting
    void broadcastSensorData(const char *data);
    void broadcastStatus(const char *data);
    void broadcastAlert(const char *data);

    // Utility
    void handle();
    void printStatus();
    String getURL();
};

extern WebServerManager webServer; // Global instance

#endif // WEB_SERVER_H
