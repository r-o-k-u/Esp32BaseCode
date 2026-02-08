// extern WebServerManager webServer; // Global instance

// #endif // WEB_SERVER_H
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
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "DataLogger.h"

// Configuration
#define MAX_CLIENTS 10
#ifndef DEVICE_NAME
#define DEVICE_NAME "ESP32-Device"
#endif
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "2.0.0"
#endif

// Forward declarations (define these in your main files)
class SensorManager;
class ActuatorManager;
class ESPNowComm;
class DataLogger;

// Client information structure
struct ClientInfo
{
    uint32_t id;
    IPAddress ip;
    unsigned long connectTime;
    unsigned long lastPing;
    bool authenticated;
};

class WebServerManager
{
private:
    AsyncWebServer *server;
    AsyncWebSocket *ws;

    // Client management
    ClientInfo clients[MAX_CLIENTS];
    uint8_t clientCount;

    // Statistics
    uint32_t totalRequests;
    uint32_t totalWSMessages;
    unsigned long serverStartTime;

    // Authentication
    bool authEnabled;
    String authUsername;
    String authPassword;

    // SPIFFS management
    bool spiffsAvailable;
    bool initSPIFFS();
    void listSPIFFSFiles(AsyncWebSocketClient *client);
    String getContentType(String filename);

    bool initialized;

    // Private methods
    void setupWebSocket();
    void setupRoutes();
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len);
    void processWebSocketMessage(AsyncWebSocketClient *client, uint8_t *data, size_t len);
    void addClient(AsyncWebSocketClient *client);
    void removeClient(AsyncWebSocketClient *client);
    void cleanupClients();
    void broadcast(const String &message);

public:
    WebServerManager();

    // Web server initialization
    bool begin();
    bool begin(uint16_t port, uint16_t wsPort = 81);

    // WebSocket broadcasting
    void broadcastSensorData(const char *data);
    void broadcastStatus(const char *data);
    void broadcastAlert(const char *data);

    // Client management
    void disconnectAllClients();
    ClientInfo *getClientInfo(uint8_t index);
    uint8_t getClientCount() { return clientCount; }

    // Statistics
    uint32_t getUptime();
    uint32_t getRequestCount() { return totalRequests; }
    uint32_t getWSMessageCount() { return totalWSMessages; }
    void resetStatistics();

    // Authentication
    void setAuthentication(bool enable, const char *username = "admin", const char *password = "admin");

    // Utility
    void handle();
    void printStatus();
    String getURL();

    // Server control
    void stop();
    bool isRunning() { return initialized; }
};

extern WebServerManager webServer; // Global instance
extern DataLogger dataLogger;      // Global data logger instance

#endif // WEB_SERVER_H
