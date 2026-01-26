/**
 * @file WebServer.cpp
 * @brief Web server manager implementation
 */

#include "WebServer.h"

// Global instance
WebServerManager webServer;

/**
 * @brief Constructor
 */
WebServerManager::WebServerManager()
{
    server = nullptr;
    ws = nullptr;
    initialized = false;
}

/**
 * @brief Initialize web server
 * @return true if started successfully
 */
bool WebServerManager::begin()
{
    // Create server instance
    server = new AsyncWebServer(80);
    ws = new AsyncWebSocket("/ws");

    // Add WebSocket handler
    server->addHandler(ws);

    // Add simple routes
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/html", "<h1>ESP32 Web Server</h1>"); });

    // Start server
    server->begin();
    initialized = true;

    Serial.println("Web server started on port 80");
    return true;
}

/**
 * @brief Broadcast sensor data to WebSocket clients
 * @param data JSON data string
 */
void WebServerManager::broadcastSensorData(const char *data)
{
    if (ws && initialized)
    {
        ws->textAll(data);
    }
}

/**
 * @brief Broadcast status to WebSocket clients
 * @param data JSON data string
 */
void WebServerManager::broadcastStatus(const char *data)
{
    if (ws && initialized)
    {
        ws->textAll(data);
    }
}

/**
 * @brief Broadcast alert to WebSocket clients
 * @param data JSON data string
 */
void WebServerManager::broadcastAlert(const char *data)
{
    if (ws && initialized)
    {
        ws->textAll(data);
    }
}

/**
 * @brief Handle web server tasks
 */
void WebServerManager::handle()
{
    // AsyncWebServer handles requests automatically
    // No need for manual handling
}

/**
 * @brief Print web server status
 */
void WebServerManager::printStatus()
{
    Serial.println("=== Web Server Status ===");
    Serial.printf("Status: %s\n", initialized ? "Running" : "Stopped");
    if (initialized)
    {
        Serial.println("Port: 80");
        Serial.println("WebSocket: /ws");
    }
    Serial.println("========================");
}

/**
 * @brief Get server URL
 * @return URL string
 */
String WebServerManager::getURL()
{
    if (initialized)
    {
        return "http://" + WiFi.localIP().toString();
    }
    return "Server not running";
}
