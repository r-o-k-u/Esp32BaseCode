/**
 * @file WebServer.cpp
 * @brief Web server manager implementation
 */

#include "WebServer.h"
#include "sensors/SensorManager.h"
#include "actuators/ActuatorManager.h"
#include "ESPNowComm.h"
#include "WiFiManager.h"
#include "OTAManager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <Update.h>

// External references (define these in your main.cpp)
extern SensorManager sensorManager;
extern ActuatorManager actuatorManager;
extern WiFiManager wifiManager;
extern OTAManager otaManager;
extern ESPNowComm espnowComm;
extern DataLogger dataLogger;

// Global instance
WebServerManager webServer;

/**
 * @brief Constructor
 */
WebServerManager::WebServerManager()
{
    server = nullptr;
    ws = nullptr;
    clientCount = 0;
    totalRequests = 0;
    totalWSMessages = 0;
    serverStartTime = 0;
    authEnabled = false;
    authUsername = "";
    authPassword = "";
    initialized = false;
    spiffsAvailable = false;

    // Initialize client array
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].id = 0;
        clients[i].connectTime = 0;
        clients[i].lastPing = 0;
        clients[i].authenticated = false;
    }
}

/**
 * @brief Initialize web server
 * @return true if started successfully
 */
bool WebServerManager::begin()
{
    return begin(80, 81); // Default ports
}

/**
 * @brief Initialize and start web server with custom ports
 * @param port HTTP port
 * @param wsPort WebSocket port (not used directly but kept for compatibility)
 * @return true if started successfully
 */
bool WebServerManager::begin(uint16_t port, uint16_t wsPort)
{
    Serial.println("═══════════════════════════════════════════════════");
    Serial.println("Initializing Web Server");
    Serial.println("═══════════════════════════════════════════════════");

    // Initialize SPIFFS first
    spiffsAvailable = initSPIFFS();

    // Create server instance
    server = new AsyncWebServer(port);
    ws = new AsyncWebSocket("/ws");

    Serial.printf("HTTP Port:      %d\n", port);
    Serial.printf("WebSocket Path: /ws\n");
    Serial.printf("SPIFFS:         %s\n", spiffsAvailable ? "Available" : "Not Available");

    // Setup WebSocket
    setupWebSocket();

    // Add WebSocket handler to server
    server->addHandler(ws);

    // Setup HTTP routes
    setupRoutes();

    // Start server
    server->begin();
    serverStartTime = millis();
    initialized = true;

    Serial.println("═══════════════════════════════════════════════════");
    Serial.println("✓ Web Server Started");
    Serial.println("═══════════════════════════════════════════════════");
    Serial.printf("Access at: http://%s\n", WiFi.localIP().toString().c_str());
    if (spiffsAvailable)
    {
        Serial.println("Static files available from SPIFFS");
    }
    Serial.println("═══════════════════════════════════════════════════\n");

    return true;
}

/**
 * @brief Initialize SPIFFS
 */
bool WebServerManager::initSPIFFS()
{
    Serial.println("\n=== SPIFFS INITIALIZATION ===");

    // Try to mount SPIFFS
    Serial.print("Mounting SPIFFS... ");
    if (!SPIFFS.begin(true))
    {
        Serial.println("FAILED");
        return false;
    }
    Serial.println("SUCCESS");

    // Get SPIFFS info
    size_t total = SPIFFS.totalBytes();
    size_t used = SPIFFS.usedBytes();
    Serial.printf("Total space: %d bytes\n", total);
    Serial.printf("Used space: %d bytes\n", used);
    Serial.printf("Free space: %d bytes\n", total - used);

    // List ALL files with their full paths
    Serial.println("\n=== ALL FILES IN SPIFFS ===");
    File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("Failed to open root directory");
        return false;
    }

    bool filesFound = false;
    File file = root.openNextFile();

    while (file)
    {
        filesFound = true;
        // Use file.path() to get the FULL path
        String filePath = String(file.path());
        Serial.printf("  %-50s %8d bytes\n", filePath.c_str(), file.size());
        file = root.openNextFile();
    }

    if (!filesFound)
    {
        Serial.println("  No files found!");
        Serial.println("\n⚠ SPIFFS is empty! Upload files using:");
        Serial.println("  pio run --target uploadfs");
        return false;
    }

    // Check for index.html
    Serial.println("\n=== CHECKING FOR index.html ===");

    bool hasIndex = SPIFFS.exists("/index.html");
    Serial.printf("  /index.html: %s\n", hasIndex ? "✓ EXISTS" : "✗ NOT FOUND");

    if (!hasIndex)
    {
        Serial.println("\n⚠ WARNING: index.html not found at root level!");
        Serial.println("  Your files might be in a subdirectory.");
        Serial.println("  This will use fallback HTML instead.");
    }

    Serial.printf("\nSPIFFS Ready: %s\n", hasIndex ? "YES ✓" : "NO ✗");
    Serial.println("=== END SPIFFS INIT ===\n");

    return hasIndex;
}

/**
 * @brief Setup WebSocket handlers
 */
void WebServerManager::setupWebSocket()
{
    ws->onEvent([this](AsyncWebSocket *server,
                       AsyncWebSocketClient *client,
                       AwsEventType type,
                       void *arg,
                       uint8_t *data,
                       size_t len)
                { this->onWebSocketEvent(server, client, type, arg, data, len); });
}

/**
 * @brief WebSocket event handler
 */
void WebServerManager::onWebSocketEvent(AsyncWebSocket *server,
                                        AsyncWebSocketClient *client,
                                        AwsEventType type,
                                        void *arg,
                                        uint8_t *data,
                                        size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n",
                      client->id(), client->remoteIP().toString().c_str());
        addClient(client);

        // Send initial data to new client
        {
            StaticJsonDocument<512> doc;
            doc["type"] = "connected";
            doc["clientId"] = client->id();
            doc["message"] = "Connected to ESP32";
            doc["spiffs"] = spiffsAvailable;

            char buffer[512];
            serializeJson(doc, buffer);
            client->text(buffer);
        }
        break;

    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        removeClient(client);
        break;

    case WS_EVT_DATA:
        processWebSocketMessage(client, data, len);
        totalWSMessages++;
        break;

    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

/**
 * @brief Process incoming WebSocket message
 */
void WebServerManager::processWebSocketMessage(AsyncWebSocketClient *client,
                                               uint8_t *data,
                                               size_t len)
{
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, (char *)data);

    if (error)
    {
        Serial.println("WebSocket: JSON parse error");
        return;
    }

    const char *type = doc["type"];
    if (!type)
        return;

    Serial.printf("WebSocket message type: %s\n", type);

    // GET STATUS
    if (strcmp(type, "getStatus") == 0)
    {
        StaticJsonDocument<1536> response;
        response["type"] = "status";
        response["device"] = DEVICE_NAME;
        response["version"] = FIRMWARE_VERSION;
        response["uptime"] = millis();
        response["freeHeap"] = ESP.getFreeHeap();
        response["heapSize"] = ESP.getHeapSize();
        response["heapUsage"] = (100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()));
        response["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        response["wifiRSSI"] = WiFi.RSSI();
        response["ip"] = WiFi.localIP().toString();
        response["mac"] = WiFi.macAddress();
        response["ssid"] = WiFi.SSID();
        response["sensorCount"] = sensorManager.getSensorCount();
        response["clients"] = clientCount;

        // WiFi info
        JsonObject wifi = response.createNestedObject("wifi");
        wifi["connected"] = WiFi.status() == WL_CONNECTED;
        wifi["apMode"] = WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;

        // OTA info
        JsonObject ota = response.createNestedObject("ota");
        ota["updating"] = otaManager.isUpdating();
        ota["progress"] = otaManager.getProgress();

        // ESP-NOW stats
        JsonObject espnow = response.createNestedObject("espnow");
        uint32_t sent, received, failed;
        espnowComm.getStatistics(sent, received, failed);
        espnow["sent"] = sent;
        espnow["received"] = received;
        espnow["failed"] = failed;
        espnow["peers"] = espnowComm.getPeerCount();

        char buffer[1536];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // GET SENSOR DATA
    else if (strcmp(type, "getSensorData") == 0)
    {
        StaticJsonDocument<1024> response;
        sensorManager.getAllSensorData(response.to<JsonObject>());
        response["type"] = "sensor";

        char buffer[1024];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // SET ACTUATOR
    else if (strcmp(type, "setActuator") == 0)
    {
        const char *actuator = doc["actuator"];
        int value = doc["value"];

        if (actuator)
        {
            actuatorManager.setActuator(actuator, value);

            StaticJsonDocument<256> response;
            response["type"] = "actuatorSet";
            response["actuator"] = actuator;
            response["value"] = value;
            response["success"] = true;

            char buffer[256];
            serializeJson(response, buffer);
            ws->textAll(buffer);
        }
    }
    // GET ACTUATOR STATUS
    else if (strcmp(type, "getActuatorStatus") == 0)
    {
        String status = actuatorManager.getStatus();

        StaticJsonDocument<1024> response;
        deserializeJson(response, status);
        response["type"] = "actuatorStatus";

        char buffer[1024];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // GET PEERS
    else if (strcmp(type, "getPeers") == 0)
    {
        StaticJsonDocument<1024> response;
        response["type"] = "peers";
        JsonArray peers = response.createNestedArray("peers");

        uint8_t peerCount = espnowComm.getPeerCount();
        for (uint8_t i = 0; i < peerCount; i++)
        {
            PeerInfo *peer = espnowComm.getPeerInfo(i);
            if (peer && peer->active)
            {
                JsonObject peerObj = peers.createNestedObject();
                peerObj["mac"] = espnowComm.getMacString(peer->mac);
                peerObj["name"] = peer->name;
                peerObj["lastSeen"] = peer->lastSeen;
                peerObj["messagesSent"] = peer->messagesSent;
                peerObj["messagesReceived"] = peer->messagesReceived;
                peerObj["connected"] = (millis() - peer->lastSeen) < 60000;
            }
        }

        char buffer[1024];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // SEND TO PEER
    else if (strcmp(type, "sendToPeer") == 0)
    {
        const char *peerMac = doc["peer"];
        JsonVariant messageVar = doc["message"];

        if (peerMac && !messageVar.isNull())
        {
            uint8_t mac[6];
            if (sscanf(peerMac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                       &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6)
            {

                String messageStr;
                serializeJson(messageVar, messageStr);

                bool success = espnowComm.sendMessage(mac, MSG_CUSTOM, messageStr.c_str());

                dataLogger.logEvent(("Sent to " + String(peerMac) + ": " + messageStr).c_str());

                StaticJsonDocument<256> response;
                response["type"] = "espnowMessage";
                response["direction"] = "sent";
                response["peer"] = peerMac;
                response["message"] = messageVar;
                response["success"] = success;

                char buffer[256];
                serializeJson(response, buffer);
                ws->textAll(buffer);
            }
        }
    }
    // TRIGGER ALERT
    else if (strcmp(type, "triggerAlert") == 0)
    {
        const char *message = doc["message"] | "Alert triggered";

        actuatorManager.triggerAlert();

        StaticJsonDocument<256> response;
        response["type"] = "alert";
        response["message"] = message;

        char buffer[256];
        serializeJson(response, buffer);
        ws->textAll(buffer);

        espnowComm.sendToAllPeers(MSG_ALERT, message);
    }
    // WiFi SCAN
    else if (strcmp(type, "wifiScan") == 0)
    {
        int n = WiFi.scanNetworks();

        StaticJsonDocument<2048> response;
        response["type"] = "wifiNetworks";
        JsonArray networks = response.createNestedArray("networks");

        for (int i = 0; i < n; i++)
        {
            JsonObject net = networks.createNestedObject();
            net["ssid"] = WiFi.SSID(i);
            net["rssi"] = WiFi.RSSI(i);
            net["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured";
        }

        char buffer[2048];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // WiFi CONNECT
    else if (strcmp(type, "wifiConnect") == 0)
    {
        const char *ssid = doc["ssid"];
        const char *password = doc["password"];

        if (ssid)
        {
            WiFi.begin(ssid, password);

            StaticJsonDocument<128> response;
            response["type"] = "wifiConnecting";
            response["ssid"] = ssid;

            char buffer[128];
            serializeJson(response, buffer);
            client->text(buffer);
        }
    }
    // LIST FILES
    else if (strcmp(type, "listFiles") == 0)
    {
        listSPIFFSFiles(client);
    }
    // GET CONFIG
    else if (strcmp(type, "getConfig") == 0)
    {
        StaticJsonDocument<512> response;
        response["type"] = "config";
        response["deviceName"] = DEVICE_NAME;
        response["sensorInterval"] = SENSOR_READ_INTERVAL;

        char buffer[512];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // SAVE CONFIG
    else if (strcmp(type, "saveConfig") == 0)
    {
        File configFile = SPIFFS.open("/config.json", FILE_WRITE);
        if (configFile)
        {
            serializeJson(doc, configFile);
            configFile.close();

            StaticJsonDocument<128> response;
            response["type"] = "configSaved";
            response["success"] = true;

            char buffer[128];
            serializeJson(response, buffer);
            client->text(buffer);
        }
    }
    // RESTART
    else if (strcmp(type, "restart") == 0)
    {
        StaticJsonDocument<128> response;
        response["type"] = "restarting";

        char buffer[128];
        serializeJson(response, buffer);
        client->text(buffer);

        delay(1000);
        ESP.restart();
    }
}

/**
 * @brief List SPIFFS files to WebSocket client
 */
void WebServerManager::listSPIFFSFiles(AsyncWebSocketClient *client)
{
    StaticJsonDocument<1024> response;
    response["type"] = "fileList";
    JsonArray files = response.createNestedArray("files");

    if (spiffsAvailable)
    {
        File root = SPIFFS.open("/");
        File file = root.openNextFile();

        while (file)
        {
            JsonObject fileObj = files.createNestedObject();
            fileObj["name"] = file.name();
            fileObj["size"] = file.size();
            file = root.openNextFile();
        }
    }

    char buffer[1024];
    serializeJson(response, buffer);
    client->text(buffer);
}

/**
 * @brief Add client to tracking
 */
void WebServerManager::addClient(AsyncWebSocketClient *client)
{
    if (clientCount >= MAX_CLIENTS)
    {
        Serial.println("WARNING: Max clients reached");
        return;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].id == 0)
        {
            clients[i].id = client->id();
            clients[i].ip = client->remoteIP();
            clients[i].connectTime = millis();
            clients[i].lastPing = millis();
            clients[i].authenticated = !authEnabled;
            clientCount++;
            break;
        }
    }
}

/**
 * @brief Remove client from tracking
 */
void WebServerManager::removeClient(AsyncWebSocketClient *client)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].id == client->id())
        {
            clients[i].id = 0;
            clientCount--;
            break;
        }
    }
}

/**
 * @brief Setup all HTTP routes
 */
void WebServerManager::setupRoutes()
{
    // ───────────────────────────────────────────────────────────────────────
    // STATIC FILE SERVING (SPIFFS)
    // ───────────────────────────────────────────────────────────────────────
    if (spiffsAvailable)
    {
        Serial.println("Setting up SPIFFS file server...");
        server->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
        Serial.println("✓ SPIFFS static file server configured");
    }

    // ───────────────────────────────────────────────────────────────────────
    // DEBUG ENDPOINT
    // ───────────────────────────────────────────────────────────────────────
    server->on("/debug/files", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        String response = "<!DOCTYPE html><html><head><title>SPIFFS Files</title>";
        response += "<style>body {font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5;}";
        response += "h1 {color: #333;} ul {list-style-type: none; padding: 0;}";
        response += "li {padding: 8px; margin: 5px 0; background: white; border-radius: 4px;}</style></head><body>";
        response += "<h1>🗂️ SPIFFS Files Debug</h1>";
        
        if (webServer.spiffsAvailable) {
            response += "<p><strong>SPIFFS Status:</strong> ✓ Available</p>";
            response += "<h2>All Files:</h2><ul>";
            File root = SPIFFS.open("/");
            File file = root.openNextFile();
            
            int fileCount = 0;
            while(file){
                String filePath = String(file.path());
                response += "<li>📄 <strong>" + filePath + "</strong> (" + String(file.size()) + " bytes)";
                response += " <a href='" + filePath + "' target='_blank'>Open</a></li>";
                file = root.openNextFile();
                fileCount++;
            }
            response += "</ul><p>Total files: " + String(fileCount) + "</p>";
        } else {
            response += "<p><strong>SPIFFS Status:</strong> ✗ Not Available</p>";
        }
        
        response += "<hr><p><a href='/'>← Back to Dashboard</a></p></body></html>";
        request->send(200, "text/html", response); });

    // ───────────────────────────────────────────────────────────────────────
    // SYSTEM STATUS API (Enhanced with WiFi & OTA info)
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<1536> doc;
        doc["device"] = DEVICE_NAME;
        doc["version"] = FIRMWARE_VERSION;
        doc["uptime"] = millis();
        doc["freeHeap"] = ESP.getFreeHeap();
        doc["heapSize"] = ESP.getHeapSize();
        doc["heapUsage"] = (100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()));
        doc["cpuUsage"] = 0; // Placeholder
        doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        doc["wifiRSSI"] = WiFi.RSSI();
        doc["ip"] = WiFi.localIP().toString();
        doc["mac"] = WiFi.macAddress();
        doc["ssid"] = WiFi.SSID();
        doc["clients"] = webServer.clientCount;
        doc["spiffs"] = webServer.spiffsAvailable;
        doc["sensorCount"] = sensorManager.getSensorCount();
        
        // Storage info
        size_t totalBytes = SPIFFS.totalBytes();
        size_t usedBytes = SPIFFS.usedBytes();
        doc["storageTotal"] = totalBytes;
        doc["storageUsed"] = usedBytes;
        doc["storageUsage"] = (usedBytes * 100 / totalBytes);
        
        // WiFi Manager info
        JsonObject wifi = doc.createNestedObject("wifi");
        wifi["connected"] = WiFi.status() == WL_CONNECTED;
        wifi["ssid"] = WiFi.SSID();
        wifi["rssi"] = WiFi.RSSI();
        wifi["ip"] = WiFi.localIP().toString();
        wifi["gateway"] = WiFi.gatewayIP().toString();
        wifi["subnet"] = WiFi.subnetMask().toString();
        wifi["dns"] = WiFi.dnsIP().toString();
        wifi["apMode"] = WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
        if (wifi["apMode"]) {
            wifi["apSSID"] = WiFi.softAPSSID();
            wifi["apIP"] = WiFi.softAPIP().toString();
            wifi["apClients"] = WiFi.softAPgetStationNum();
        }
        
        // OTA Manager info
        JsonObject ota = doc.createNestedObject("ota");
        ota["initialized"] = otaManager.isInitialized();
        ota["hostname"] = otaManager.getHostname();
        ota["port"] = otaManager.getPort();
        ota["updating"] = otaManager.isUpdating();
        ota["progress"] = otaManager.getProgress();
        ota["totalUpdates"] = otaManager.getTotalUpdates();
        ota["failedUpdates"] = otaManager.getFailedUpdates();
        
        // ESP-NOW statistics
        JsonObject espnow = doc.createNestedObject("espnow");
        uint32_t sent, received, failed;
        espnowComm.getStatistics(sent, received, failed);
        espnow["sent"] = sent;
        espnow["received"] = received;
        espnow["failed"] = failed;
        espnow["peers"] = espnowComm.getPeerCount();

#if ENABLE_CAMERA
        doc["hasCamera"] = true;
#else
        doc["hasCamera"] = false;
#endif
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // ───────────────────────────────────────────────────────────────────────
    // WIFI MANAGER ENDPOINTS
    // ───────────────────────────────────────────────────────────────────────

    // Scan WiFi Networks
    server->on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        int n = WiFi.scanNetworks();
        
        StaticJsonDocument<2048> doc;
        JsonArray networks = doc.createNestedArray("networks");
        
        for (int i = 0; i < n; i++) {
            JsonObject net = networks.createNestedObject();
            net["ssid"] = WiFi.SSID(i);
            net["rssi"] = WiFi.RSSI(i);
            net["encryption"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Secured";
            net["channel"] = WiFi.channel(i);
        }
        
        doc["count"] = n;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Connect to WiFi Network
    server->on("/api/wifi/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<256> doc;
        deserializeJson(doc, (char*)data);
        
        const char* ssid = doc["ssid"];
        const char* password = doc["password"];
        
        if (ssid) {
            // Attempt to connect
            WiFi.begin(ssid, password);
            
            // Wait up to 10 seconds
            int timeout = 0;
            while (WiFi.status() != WL_CONNECTED && timeout < 20) {
                delay(500);
                timeout++;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                request->send(200, "application/json", "{\"success\":true,\"ip\":\"" + WiFi.localIP().toString() + "\"}");
            } else {
                request->send(200, "application/json", "{\"success\":false,\"error\":\"Connection failed\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing SSID\"}");
        } });

    // Disconnect WiFi
    server->on("/api/wifi/disconnect", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        WiFi.disconnect();
        request->send(200, "application/json", "{\"success\":true}"); });

    // Get WiFi Status
    server->on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<512> doc;
        doc["connected"] = WiFi.status() == WL_CONNECTED;
        doc["ssid"] = WiFi.SSID();
        doc["rssi"] = WiFi.RSSI();
        doc["ip"] = WiFi.localIP().toString();
        doc["mac"] = WiFi.macAddress();
        doc["gateway"] = WiFi.gatewayIP().toString();
        doc["subnet"] = WiFi.subnetMask().toString();
        doc["dns"] = WiFi.dnsIP().toString();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Start Access Point
    server->on("/api/wifi/ap/start", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<256> doc;
        deserializeJson(doc, (char*)data);
        
        const char* ssid = doc["ssid"] | AP_SSID;
        const char* password = doc["password"] | AP_PASSWORD;
        
        WiFi.softAP(ssid, password);
        
        StaticJsonDocument<256> response;
        response["success"] = true;
        response["ssid"] = ssid;
        response["ip"] = WiFi.softAPIP().toString();
        
        char buffer[256];
        serializeJson(response, buffer);
        request->send(200, "application/json", buffer); });

    // Stop Access Point
    server->on("/api/wifi/ap/stop", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        WiFi.softAPdisconnect(true);
        request->send(200, "application/json", "{\"success\":true}"); });

    // ───────────────────────────────────────────────────────────────────────
    // OTA UPDATE ENDPOINTS
    // ───────────────────────────────────────────────────────────────────────

    // OTA Status
    server->on("/api/ota/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<512> doc;
        doc["initialized"] = otaManager.isInitialized();
        doc["hostname"] = otaManager.getHostname();
        doc["port"] = otaManager.getPort();
        doc["updating"] = otaManager.isUpdating();
        doc["progress"] = otaManager.getProgress();
        doc["state"] = otaManager.getStatusString();
        doc["totalUpdates"] = otaManager.getTotalUpdates();
        doc["failedUpdates"] = otaManager.getFailedUpdates();
        doc["lastUpdate"] = otaManager.getLastUpdateTime();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Trigger OTA Update (for web-based OTA)
    server->on("/api/ota/update", HTTP_POST, [](AsyncWebServerRequest *request)
               { request->send(200, "application/json", "{\"success\":true,\"message\":\"Upload firmware file\"}"); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
               {
            if (!index) {
                Serial.printf("OTA Update Start: %s\n", filename.c_str());
                
                // Start update
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            }
            
            // Write data
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
            
            if (final) {
                if (Update.end(true)) {
                    Serial.printf("OTA Update Success: %u bytes\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            } });

    // ───────────────────────────────────────────────────────────────────────
    // SENSOR DATA API
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/sensors", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<1024> doc;
        sensorManager.getAllSensorData(doc.to<JsonObject>());
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // ───────────────────────────────────────────────────────────────────────
    // ACTUATOR CONTROL API (Enhanced)
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/actuator", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }
        
        const char* actuator = doc["actuator"];
        
        if (!actuator) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing actuator\"}");
            return;
        }
        
        // Handle different actuator types
        if (doc.containsKey("value")) {
            int value = doc["value"];
            actuatorManager.setActuator(actuator, value);
        }
        else if (doc.containsKey("r") && doc.containsKey("g") && doc.containsKey("b")) {
            int r = doc["r"];
            int g = doc["g"];
            int b = doc["b"];
            actuatorManager.setRGBColor(r, g, b);
        }
        else if (doc.containsKey("angle")) {
            int angle = doc["angle"];
            actuatorManager.setActuator(actuator, angle);
        }
        
        // Broadcast state change to all WebSocket clients
        StaticJsonDocument<256> response;
        response["type"] = "actuatorSet";
        response["actuator"] = actuator;
        if (doc.containsKey("value")) response["value"] = doc["value"];
        if (doc.containsKey("r")) {
            response["r"] = doc["r"];
            response["g"] = doc["g"];
            response["b"] = doc["b"];
        }
        response["success"] = true;
        
        char buffer[256];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer);
        
        request->send(200, "application/json", "{\"success\":true}"); });

    // Get Actuator Status
    server->on("/api/actuators/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        String status = actuatorManager.getStatus();
        request->send(200, "application/json", status); });

    // Reset All Actuators
    server->on("/api/actuators/reset", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        actuatorManager.loadDefaultConfiguration();
        
        request->send(200, "application/json", "{\"success\":true}");
        
        StaticJsonDocument<128> doc;
        doc["type"] = "actuatorsReset";
        char buffer[128];
        serializeJson(doc, buffer);
        webServer.ws->textAll(buffer); });

    // Emergency Stop
    server->on("/api/actuators/emergency-stop", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        actuatorManager.emergencyStop();
        
        request->send(200, "application/json", "{\"success\":true}");
        
        StaticJsonDocument<128> doc;
        doc["type"] = "alert";
        doc["message"] = "Emergency stop activated";
        char buffer[128];
        serializeJson(doc, buffer);
        webServer.ws->textAll(buffer); });

    // ───────────────────────────────────────────────────────────────────────
    // ESP-NOW PEERS API
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/peers", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<1024> doc;
        JsonArray peers = doc.createNestedArray("peers");
        
        uint8_t peerCount = espnowComm.getPeerCount();
        for (uint8_t i = 0; i < peerCount; i++) {
            PeerInfo* peer = espnowComm.getPeerInfo(i);
            if (peer && peer->active) {
                JsonObject peerObj = peers.createNestedObject();
                peerObj["mac"] = espnowComm.getMacString(peer->mac);
                peerObj["name"] = peer->name;
                peerObj["active"] = peer->active;
                peerObj["lastSeen"] = peer->lastSeen;
                peerObj["messagesSent"] = peer->messagesSent;
                peerObj["messagesReceived"] = peer->messagesReceived;
                peerObj["connected"] = (millis() - peer->lastSeen) < 60000;
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Send ESP-NOW Message
    server->on("/api/peers/send", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }
        
        const char* peerMac = doc["peer"];
        const char* message = doc["message"];
        
        if (!peerMac || !message) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameters\"}");
            return;
        }
        
        uint8_t mac[6];
        if (sscanf(peerMac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid MAC address\"}");
            return;
        }
        
        bool success = espnowComm.sendMessage(mac, MSG_CUSTOM, message);
        
        if (success) {
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"error\":\"Send failed\"}");
        } });

    // ───────────────────────────────────────────────────────────────────────
    // LOGS API
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        String category = "events";
        if (request->hasParam("category")) {
            category = request->getParam("category")->value();
        }
        
        String logs = dataLogger.readLog(category.c_str(), 100);
        request->send(200, "text/plain", logs); });

    server->on("/api/logs", HTTP_DELETE, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        dataLogger.deleteAllLogs();
        request->send(200, "application/json", "{\"success\":true}"); });

    // ───────────────────────────────────────────────────────────────────────
    // CONFIGURATION API
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/config", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<512> doc;
        doc["deviceName"] = DEVICE_NAME;
        doc["sensorInterval"] = SENSOR_READ_INTERVAL;
        doc["enableLogging"] = ENABLE_DATA_LOGGING;
        doc["enableESPNow"] = ENABLE_ESPNOW;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    server->on("/api/config", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);
        
        if (error) {
            request->send(400, "application/json", "{\"success\":false}");
            return;
        }
        
        File configFile = SPIFFS.open("/config.json", FILE_WRITE);
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(500, "application/json", "{\"success\":false}");
        } });

    // ───────────────────────────────────────────────────────────────────────
    // DATA EXPORT API
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/export", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<2048> doc;
        
        JsonObject system = doc.createNestedObject("system");
        system["device"] = DEVICE_NAME;
        system["version"] = FIRMWARE_VERSION;
        system["uptime"] = millis();
        system["freeHeap"] = ESP.getFreeHeap();
        
        JsonObject sensors = doc.createNestedObject("sensors");
        sensorManager.getAllSensorData(sensors);
        
        JsonObject espnow = doc.createNestedObject("espnow");
        uint32_t sent, received, failed;
        espnowComm.getStatistics(sent, received, failed);
        espnow["sent"] = sent;
        espnow["received"] = received;
        espnow["failed"] = failed;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // ───────────────────────────────────────────────────────────────────────
    // SYSTEM CONTROL ENDPOINTS
    // ───────────────────────────────────────────────────────────────────────
    server->on("/api/restart", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        request->send(200, "text/plain", "Restarting...");
        delay(1000);
        ESP.restart(); });

    server->on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        SPIFFS.remove("/config.json");
        dataLogger.deleteAllLogs();
        request->send(200, "application/json", "{\"success\":true}");
        delay(1000);
        ESP.restart(); });

    server->on("/api/alert", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<256> doc;
        deserializeJson(doc, (char*)data);
        
        const char* message = doc["message"] | "Alert triggered";
        
        actuatorManager.triggerAlert();
        
        StaticJsonDocument<256> alert;
        alert["type"] = "alert";
        alert["message"] = message;
        char buffer[256];
        serializeJson(alert, buffer);
        webServer.ws->textAll(buffer);
        
        espnowComm.sendToAllPeers(MSG_ALERT, message);
        
        request->send(200, "application/json", "{\"success\":true}"); });

    server->on("/api/files", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        
        StaticJsonDocument<2048> doc;
        doc["spiffs"] = webServer.spiffsAvailable;
        JsonArray files = doc.createNestedArray("files");
        
        if (webServer.spiffsAvailable) {
            File root = SPIFFS.open("/");
            File file = root.openNextFile();
            
            while(file){
                JsonObject fileObj = files.createNestedObject();
                fileObj["name"] = file.path();
                fileObj["size"] = file.size();
                file = root.openNextFile();
            }
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // ───────────────────────────────────────────────────────────────────────
    // FALLBACK HOMEPAGE (if SPIFFS not available)
    // ───────────────────────────────────────────────────────────────────────
    if (!spiffsAvailable)
    {
        server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
            webServer.totalRequests++;
            
            String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 IoT Dashboard</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
            padding: 20px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 20px;
            backdrop-filter: blur(10px);
        }

        .header h1 {
            font-size: 2.5rem;
            margin-bottom: 10px;
        }

        .status-bar {
            display: flex;
            justify-content: space-between;
            background: white;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }

        .status-item {
            text-align: center;
            flex: 1;
        }

        .status-label {
            font-size: 0.9rem;
            color: #666;
            margin-bottom: 5px;
        }

        .status-value {
            font-size: 1.8rem;
            font-weight: bold;
            color: #333;
        }

        .card-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }

        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            transition: transform 0.3s ease;
        }

        .card:hover {
            transform: translateY(-5px);
        }

        .card h2 {
            color: #333;
            margin-bottom: 20px;
            font-size: 1.5rem;
        }

        .sensor-value {
            font-size: 2.5rem;
            font-weight: bold;
            color: #667eea;
            text-align: center;
            margin: 20px 0;
        }

        .sensor-unit {
            font-size: 1rem;
            color: #666;
        }

        .sensor-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
        }

        .sensor-item {
            padding: 15px;
            background: #f8f9fa;
            border-radius: 10px;
        }

        .sensor-name {
            font-size: 0.9rem;
            color: #666;
            margin-bottom: 5px;
        }

        .control-panel {
            display: flex;
            gap: 10px;
            flex-wrap: wrap;
        }

        .btn {
            padding: 12px 24px;
            border: none;
            border-radius: 10px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            flex: 1;
            min-width: 120px;
        }

        .btn-primary {
            background: #667eea;
            color: white;
        }

        .btn-primary:hover {
            background: #5a67d8;
        }

        .btn-secondary {
            background: #48bb78;
            color: white;
        }

        .btn-secondary:hover {
            background: #38a169;
        }

        .btn-danger {
            background: #f56565;
            color: white;
        }

        .btn-danger:hover {
            background: #e53e3e;
        }

        .btn-warning {
            background: #ed8936;
            color: white;
        }

        .btn-warning:hover {
            background: #dd6b20;
        }

        .log {
            height: 200px;
            overflow-y: auto;
            background: #f8f9fa;
            border-radius: 10px;
            padding: 15px;
            font-family: monospace;
            font-size: 0.9rem;
        }

        .log-entry {
            padding: 5px 0;
            border-bottom: 1px solid #e2e8f0;
        }

        .log-time {
            color: #666;
        }

        .log-message {
            color: #333;
        }

        .ws-status {
            display: inline-block;
            padding: 5px 10px;
            border-radius: 20px;
            font-size: 0.9rem;
            font-weight: 600;
        }

        .ws-connected {
            background: #c6f6d5;
            color: #22543d;
        }

        .ws-disconnected {
            background: #fed7d7;
            color: #742a2a;
        }

        @media (max-width: 768px) {
            .status-bar {
                flex-direction: column;
                gap: 15px;
            }

            .sensor-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌡️ ESP32 IoT Dashboard</h1>
            <p>Real-time sensor monitoring and control</p>
            <p><em>Note: Using fallback interface. Upload files to SPIFFS for enhanced UI.</em></p>
        </div>

        <div class="status-bar">
            <div class="status-item">
                <div class="status-label">Device</div>
                <div class="status-value" id="deviceName">ESP32</div>
            </div>
            <div class="status-item">
                <div class="status-label">IP Address</div>
                <div class="status-value" id="ipAddress">%IP%</div>
            </div>
            <div class="status-item">
                <div class="status-label">Uptime</div>
                <div class="status-value" id="uptime">0s</div>
            </div>
            <div class="status-item">
                <div class="status-label">Heap Memory</div>
                <div class="status-value" id="heap">0 KB</div>
            </div>
            <div class="status-item">
                <div class="status-label">WebSocket</div>
                <div class="status-value">
                    <span id="wsStatus" class="ws-status ws-disconnected">Disconnected</span>
                </div>
            </div>
        </div>

        <div class="card-grid">
            <div class="card">
                <h2>📊 Sensor Data</h2>
                <div class="sensor-value" id="tempValue">--.-</div>
                <div class="sensor-unit">Temperature (°C)</div>

                <div class="sensor-grid">
                    <div class="sensor-item">
                        <div class="sensor-name">Humidity</div>
                        <div class="sensor-value" id="humidityValue">--%</div>
                    </div>
                    <div class="sensor-item">
                        <div class="sensor-name">Pressure</div>
                        <div class="sensor-value" id="pressureValue">---- hPa</div>
                    </div>
                </div>

                <div style="margin-top: 20px;">
                    <button class="btn btn-primary" onclick="getSensorData()">
                        🔄 Refresh Sensors
                    </button>
                </div>
            </div>

            <div class="card">
                <h2>⚡ Control Panel</h2>
                <div class="control-panel">
                    <button class="btn btn-secondary" onclick="controlActuator('led', 1)">
                        💡 LED ON
                    </button>
                    <button class="btn btn-danger" onclick="controlActuator('led', 0)">
                        LED OFF
                    </button>
                    <button class="btn btn-warning" onclick="restartDevice()">
                        🔄 Restart
                    </button>
                </div>

                <div style="margin-top: 20px;">
                    <h3>System Log</h3>
                    <div class="log" id="systemLog">
                        <div class="log-entry">
                            <span class="log-time">[00:00:00]</span>
                            <span class="log-message">System started</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>🔧 System Information</h2>
            <div class="sensor-grid">
                <div class="sensor-item">
                    <div class="sensor-name">Firmware Version</div>
                    <div class="sensor-value">%VERSION%</div>
                </div>
                <div class="sensor-item">
                    <div class="sensor-name">WiFi RSSI</div>
                    <div class="sensor-value" id="rssiValue">-- dBm</div>
                </div>
                <div class="sensor-item">
                    <div class="sensor-name">Connected Clients</div>
                    <div class="sensor-value" id="clientCount">0</div>
                </div>
                <div class="sensor-item">
                    <div class="sensor-name">Filesystem</div>
                    <div class="sensor-value" id="fsStatus">Not Available</div>
                </div>
            </div>
        </div>
    </div>

    <script>
        const ip = "%IP%";
        let ws = null;
        let logCount = 0;
        const maxLogs = 20;

        function addLog(message) {
            const now = new Date();
            const time = `[${now.getHours().toString().padStart(2, '0')}:${now.getMinutes().toString().padStart(2, '0')}:${now.getSeconds().toString().padStart(2, '0')}]`;
            const logDiv = document.getElementById('systemLog');

            const logEntry = document.createElement('div');
            logEntry.className = 'log-entry';
            logEntry.innerHTML = `<span class="log-time">${time}</span> <span class="log-message">${message}</span>`;

            logDiv.prepend(logEntry);

            // Limit number of logs
            if (logDiv.children.length > maxLogs) {
                logDiv.removeChild(logDiv.lastChild);
            }

            logCount++;
        }

        function connectWS() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                addLog('WebSocket already connected');
                return;
            }

            addLog('Connecting to WebSocket...');
            ws = new WebSocket(`ws://${ip}/ws`);

            ws.onopen = () => {
                addLog('✓ WebSocket connected');
                document.getElementById('wsStatus').textContent = 'Connected';
                document.getElementById('wsStatus').className = 'ws-status ws-connected';
                updateStatus();

                // Request initial data
                setTimeout(() => {
                    getStatus();
                    getSensorData();
                }, 500);
            };

            ws.onmessage = (event) => {
                try {
                    const data = JSON.parse(event.data);

                    if (data.type === 'status') {
                        updateStatusDisplay(data);
                    } else if (data.type === 'sensor') {
                        updateSensorDisplay(data);
                    } else if (data.type === 'connected') {
                        addLog(`Connected with ID: ${data.clientId}`);
                    } else if (data.type === 'actuatorSet') {
                        addLog(`Actuator ${data.actuator} set to ${data.value}`);
                    } else if (data.type === 'alert') {
                        addLog(`⚠ Alert: ${JSON.stringify(data)}`);
                    }
                } catch (e) {
                    console.error('Error parsing WebSocket message:', e);
                }
            };

            ws.onclose = () => {
                addLog('✗ WebSocket disconnected');
                document.getElementById('wsStatus').textContent = 'Disconnected';
                document.getElementById('wsStatus').className = 'ws-status ws-disconnected';
            };

            ws.onerror = (error) => {
                addLog('❌ WebSocket error occurred');
                console.error('WebSocket error:', error);
            };
        }

        function updateStatusDisplay(data) {
            document.getElementById('uptime').textContent = Math.floor(data.uptime / 1000) + 's';
            document.getElementById('heap').textContent = Math.floor(data.freeHeap / 1024) + ' KB';
            document.getElementById('rssiValue').textContent = data.wifiRSSI + ' dBm';
            document.getElementById('clientCount').textContent = data.clients || 0;
            document.getElementById('fsStatus').textContent = data.spiffs ? 'Available' : 'Not Available';
            document.getElementById('deviceName').textContent = data.device || 'ESP32';
        }

        function updateSensorDisplay(data) {
            if (data.temperature !== undefined) {
                document.getElementById('tempValue').textContent = data.temperature.toFixed(1);
            }
            if (data.humidity !== undefined) {
                document.getElementById('humidityValue').textContent = data.humidity.toFixed(1) + '%';
            }
            if (data.pressure !== undefined) {
                document.getElementById('pressureValue').textContent = data.pressure.toFixed(1) + ' hPa';
            }
            addLog('Sensor data updated');
        }

        function updateStatus() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'getStatus'}));
                // Auto-refresh every 10 seconds
                setTimeout(updateStatus, 10000);
            }
        }

        function getStatus() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'getStatus'}));
            } else {
                alert('Please connect WebSocket first');
            }
        }

        function getSensorData() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({type: 'getSensorData'}));
                addLog('Requesting sensor data...');
            } else {
                alert('Please connect WebSocket first');
            }
        }

        function controlActuator(actuator, value) {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({
                    type: 'setActuator',
                    actuator: actuator,
                    value: value
                }));
                addLog(`Sending control: ${actuator} = ${value}`);
            } else {
                alert('Please connect WebSocket first');
            }
        }

        function restartDevice() {
            if (confirm('Are you sure you want to restart the device?')) {
                fetch('/api/restart', { method: 'POST' })
                    .then(() => {
                        addLog('Device restart initiated...');
                    })
                    .catch(err => {
                        addLog('Error restarting device');
                        console.error(err);
                    });
            }
        }

        // Auto-connect on page load
        window.addEventListener('load', () => {
            connectWS();

            // Add initial logs
            addLog('Dashboard initialized');
            addLog('Device IP: ' + ip);
            addLog('Firmware: %VERSION%');

            // Update IP display
            document.getElementById('ipAddress').textContent = ip;
        });

        // Handle page visibility change
        document.addEventListener('visibilitychange', () => {
            if (!document.hidden && (!ws || ws.readyState !== WebSocket.OPEN)) {
                connectWS();
            }
        });
    </script>
</body>
</html>
)rawliteral";

            // Replace placeholders
            html.replace("%VERSION%", FIRMWARE_VERSION);
            html.replace("%IP%", WiFi.localIP().toString());

            request->send(200, "text/html", html); });
    }

    // ───────────────────────────────────────────────────────────────────────
    // 404 HANDLER - MUST BE LAST
    // ───────────────────────────────────────────────────────────────────────
    server->onNotFound([](AsyncWebServerRequest *request)
                       {
        webServer.totalRequests++;
        String message = "404 - Not Found\n\nURI: " + request->url();
        request->send(404, "text/plain", message); });
}

/**
 * @brief Get content type based on file extension
 */
String WebServerManager::getContentType(String filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/pdf";
    else if (filename.endsWith(".zip"))
        return "application/zip";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    return "text/plain";
}

/**
 * @brief Broadcast message to all WebSocket clients
 */
void WebServerManager::broadcast(const String &message)
{
    if (ws && initialized)
    {
        ws->textAll(message);
    }
}

/**
 * @brief Broadcast sensor data
 */
void WebServerManager::broadcastSensorData(const char *jsonData)
{
    if (ws && initialized)
    {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, jsonData);
        if (!error)
        {
            doc["type"] = "sensor";
            char buffer[1024];
            serializeJson(doc, buffer);
            ws->textAll(buffer);
        }
    }
}

/**
 * @brief Broadcast status
 */
void WebServerManager::broadcastStatus(const char *jsonData)
{
    if (ws && initialized)
    {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, jsonData);
        if (!error)
        {
            doc["type"] = "status";
            char buffer[512];
            serializeJson(doc, buffer);
            ws->textAll(buffer);
        }
    }
}

/**
 * @brief Broadcast alert
 */
void WebServerManager::broadcastAlert(const char *jsonData)
{
    if (ws && initialized)
    {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, jsonData);
        if (!error)
        {
            doc["type"] = "alert";
            char buffer[256];
            serializeJson(doc, buffer);
            ws->textAll(buffer);
        }
    }
}

/**
 * @brief Handle web server tasks
 */
void WebServerManager::handle()
{
    // AsyncWebServer handles requests automatically
    // No need for manual handling

    // Clean up disconnected clients periodically
    static unsigned long lastCleanup = 0;
    if (millis() - lastCleanup > 30000)
    { // Every 30 seconds
        lastCleanup = millis();
        cleanupClients();
    }
}

/**
 * @brief Clean up old client entries
 */
void WebServerManager::cleanupClients()
{
    // This is handled automatically by AsyncWebSocket
    // We just reset our tracking array for clients that are no longer connected
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

/**
 * @brief Print web server status
 */
void WebServerManager::printStatus()
{
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│          WEB SERVER STATUS                      │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.printf("│ Status:         %-28s │\n", initialized ? "Running" : "Stopped");
    Serial.printf("│ Port:           %-28d │\n", 80);
    Serial.printf("│ WebSocket:      /ws                               │\n");
    Serial.printf("│ SPIFFS:         %-28s │\n", spiffsAvailable ? "Available" : "Not Available");
    if (initialized)
    {
        Serial.printf("│ Uptime:         %-23lu sec │\n", getUptime() / 1000);
        Serial.printf("│ HTTP Requests:  %-28u │\n", totalRequests);
        Serial.printf("│ WS Messages:    %-28u │\n", totalWSMessages);
        Serial.printf("│ Connected Clients: %-23u │\n", clientCount);
    }
    Serial.println("└─────────────────────────────────────────────────┘");
}

/**
 * @brief Get server uptime
 */
uint32_t WebServerManager::getUptime()
{
    return millis() - serverStartTime;
}

/**
 * @brief Get client information
 */
ClientInfo *WebServerManager::getClientInfo(uint8_t index)
{
    if (index >= MAX_CLIENTS)
        return nullptr;
    if (clients[index].id == 0)
        return nullptr;
    return &clients[index];
}

/**
 * @brief Disconnect all clients
 */
void WebServerManager::disconnectAllClients()
{
    if (ws && initialized)
    {
        ws->closeAll();
    }
    clientCount = 0;
}

/**
 * @brief Reset statistics
 */
void WebServerManager::resetStatistics()
{
    totalRequests = 0;
    totalWSMessages = 0;
    Serial.println("Web server statistics reset");
}

/**
 * @brief Set authentication
 */
void WebServerManager::setAuthentication(bool enable, const char *username, const char *password)
{
    authEnabled = enable;
    if (enable)
    {
        authUsername = String(username);
        authPassword = String(password);
        Serial.println("Web authentication enabled");
    }
    else
    {
        Serial.println("Web authentication disabled");
    }
}

/**
 * @brief Stop web server
 */
void WebServerManager::stop()
{
    disconnectAllClients();

    if (server)
    {
        server->end();
        delete server;
        server = nullptr;
    }

    if (ws)
    {
        delete ws;
        ws = nullptr;
    }

    if (spiffsAvailable)
    {
        SPIFFS.end();
    }

    initialized = false;
    spiffsAvailable = false;
    Serial.println("Web server stopped");
}