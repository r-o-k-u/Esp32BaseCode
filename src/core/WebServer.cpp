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
    // LED CONTROL
    else if (strcmp(type, "ledControl") == 0)
    {
        const char *command = doc["command"];
        int value = doc["value"] | 0;

        if (strcmp(command, "on") == 0)
        {
            actuatorManager.setLED(true);
        }
        else if (strcmp(command, "off") == 0)
        {
            actuatorManager.setLED(false);
        }
        else if (strcmp(command, "toggle") == 0)
        {
            actuatorManager.setLED(!actuatorManager.getLED());
        }
        else if (strcmp(command, "brightness") == 0)
        {
            actuatorManager.setActuator("led", value);
        }

        StaticJsonDocument<128> response;
        response["type"] = "ledStatus";
        response["state"] = actuatorManager.getLED();
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // RGB LED CONTROL
    else if (strcmp(type, "rgbControl") == 0)
    {
        const char *command = doc["command"];

        if (strcmp(command, "color") == 0)
        {
            int r = doc["r"] | 0;
            int g = doc["g"] | 0;
            int b = doc["b"] | 0;
            actuatorManager.setRGBColor(r, g, b);
        }
        else if (strcmp(command, "effect") == 0)
        {
            const char *effect = doc["effect"];
            if (effect)
            {
                actuatorManager.executeScene(effect);
            }
        }
        else if (strcmp(command, "brightness") == 0)
        {
            int brightness = doc["value"] | 255;
            actuatorManager.setRGBBrightness(brightness);
        }
        else if (strcmp(command, "off") == 0)
        {
            actuatorManager.setRGBColor(0, 0, 0);
        }

        StaticJsonDocument<256> response;
        response["type"] = "rgbStatus";
        response["r"] = actuatorManager.getStatus().indexOf("\"r\":") >= 0 ? 1 : 0;
        char buffer[256];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // BUZZER CONTROL
    else if (strcmp(type, "buzzerControl") == 0)
    {
        const char *command = doc["command"];

        if (strcmp(command, "on") == 0)
        {
            actuatorManager.setBuzzer(true);
        }
        else if (strcmp(command, "off") == 0)
        {
            actuatorManager.setBuzzer(false);
        }
        else if (strcmp(command, "tone") == 0)
        {
            int frequency = doc["frequency"] | 1000;
            int duration = doc["duration"] | 500;
            actuatorManager.playTone(frequency, duration);
        }
        else if (strcmp(command, "melody") == 0)
        {
            const char *melody = doc["melody"];
            if (melody)
            {
                actuatorManager.playMelody(melody);
            }
        }

        StaticJsonDocument<128> response;
        response["type"] = "buzzerStatus";
        response["state"] = actuatorManager.getStatus().indexOf("\"buzzer\":true") >= 0;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // MOTOR CONTROL
    else if (strcmp(type, "motorControl") == 0)
    {
        const char *command = doc["command"];

        if (strcmp(command, "forward") == 0)
        {
            int speed = doc["speed"] | 100;
            actuatorManager.setMotorDirection(true);
            actuatorManager.setMotorSpeed(speed);
        }
        else if (strcmp(command, "backward") == 0)
        {
            int speed = doc["speed"] | 100;
            actuatorManager.setMotorDirection(false);
            actuatorManager.setMotorSpeed(speed);
        }
        else if (strcmp(command, "stop") == 0)
        {
            actuatorManager.stopMotor();
        }
        else if (strcmp(command, "speed") == 0)
        {
            int speed = doc["speed"] | 0;
            actuatorManager.setMotorSpeed(speed);
        }
        else if (strcmp(command, "brake") == 0)
        {
            actuatorManager.stopMotor();
        }
        else if (strcmp(command, "rampUp") == 0)
        {
            int duration = doc["duration"] | 2000;
            actuatorManager.rampMotorUp(duration);
        }
        else if (strcmp(command, "rampDown") == 0)
        {
            int duration = doc["duration"] | 2000;
            actuatorManager.rampMotorDown(duration);
        }

        StaticJsonDocument<128> response;
        response["type"] = "motorStatus";
        response["speed"] = actuatorManager.getSpeed();
        response["direction"] = actuatorManager.getDirection() ? "forward" : "backward";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // SERVO CONTROL
    else if (strcmp(type, "servoControl") == 0)
    {
        int servo = doc["servo"] | 1;
        const char *command = doc["command"];

        if (strcmp(command, "angle") == 0)
        {
            int angle = doc["angle"] | 90;
            actuatorManager.setServoAngle(servo, angle);
        }
        else if (strcmp(command, "sweep") == 0)
        {
            int startAngle = doc["start"] | 0;
            int endAngle = doc["end"] | 180;
            int speed = doc["speed"] | 10;
            actuatorManager.sweepServo(servo, startAngle, endAngle, speed);
        }

        StaticJsonDocument<128> response;
        response["type"] = "servoStatus";
        response["servo"] = servo;
        response["angle"] = actuatorManager.getServoAngle(servo);
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // RELAY CONTROL
    else if (strcmp(type, "relayControl") == 0)
    {
        int relay = doc["relay"] | 1;
        const char *command = doc["command"];

        if (strcmp(command, "on") == 0)
        {
            actuatorManager.setRelay(relay, true);
        }
        else if (strcmp(command, "off") == 0)
        {
            actuatorManager.setRelay(relay, false);
        }
        else if (strcmp(command, "toggle") == 0)
        {
            actuatorManager.toggleRelay(relay);
        }
        else if (strcmp(command, "pulse") == 0)
        {
            int duration = doc["duration"] | 1000;
            actuatorManager.pulseRelay(relay, duration);
        }
        else if (strcmp(command, "allOn") == 0)
        {
            for (int i = 1; i <= 3; i++)
            {
                actuatorManager.setRelay(i, true);
            }
        }
        else if (strcmp(command, "allOff") == 0)
        {
            for (int i = 1; i <= 3; i++)
            {
                actuatorManager.setRelay(i, false);
            }
        }
        else if (strcmp(command, "cycle") == 0)
        {
            int times = doc["times"] | 3;
            int interval = doc["interval"] | 500;
            actuatorManager.cycleRelays(times, interval);
        }

        StaticJsonDocument<256> response;
        response["type"] = "relayStatus";
        response["relay1"] = actuatorManager.getRelay(1);
        response["relay2"] = actuatorManager.getRelay(2);
        response["relay3"] = actuatorManager.getRelay(3);
        char buffer[256];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // SCENE CONTROL
    else if (strcmp(type, "sceneControl") == 0)
    {
        const char *command = doc["command"];
        const char *sceneName = doc["scene"];

        if (strcmp(command, "execute") == 0 && sceneName)
        {
            actuatorManager.executeScene(sceneName);
        }
        else if (strcmp(command, "save") == 0 && sceneName)
        {
            actuatorManager.saveCurrentScene(sceneName);
        }
        else if (strcmp(command, "list") == 0)
        {
            actuatorManager.loadSceneList();
        }

        StaticJsonDocument<128> response;
        response["type"] = "sceneExecuted";
        response["scene"] = sceneName ? sceneName : "";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // EMERGENCY STOP
    else if (strcmp(type, "emergencyStop") == 0)
    {
        actuatorManager.emergencyStop();

        StaticJsonDocument<128> response;
        response["type"] = "emergencyStop";
        response["success"] = true;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);

        // Also broadcast alert
        StaticJsonDocument<128> alert;
        alert["type"] = "alert";
        alert["message"] = "Emergency stop activated";
        char alertBuffer[128];
        serializeJson(alert, alertBuffer);
        ws->textAll(alertBuffer);
    }
    // TEST ACTUATOR
    else if (strcmp(type, "testActuator") == 0)
    {
        const char *actuator = doc["actuator"];
        int duration = doc["duration"] | 1000;

        if (actuator)
        {
            // Store original state and test
            bool originalState = false;

            if (strcmp(actuator, "led") == 0)
            {
                originalState = actuatorManager.getLED();
                actuatorManager.setLED(true);
                delay(duration);
                actuatorManager.setLED(originalState);
            }

            StaticJsonDocument<128> response;
            response["type"] = "actuatorTest";
            response["actuator"] = actuator;
            response["success"] = true;
            char buffer[128];
            serializeJson(response, buffer);
            client->text(buffer);
        }
    }
    // BLINK ACTUATOR
    else if (strcmp(type, "blinkActuator") == 0)
    {
        const char *actuator = doc["actuator"];
        int times = doc["times"] | 3;
        int interval = doc["interval"] | 500;

        if (actuator && strcmp(actuator, "led") == 0)
        {
            actuatorManager.blinkLED(times, interval);
        }

        StaticJsonDocument<128> response;
        response["type"] = "actuatorBlink";
        response["actuator"] = actuator;
        response["times"] = times;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // RAMP MOTOR
    else if (strcmp(type, "rampMotor") == 0)
    {
        const char *direction = doc["direction"];
        int duration = doc["duration"] | 2000;

        if (strcmp(direction, "up") == 0)
        {
            actuatorManager.rampMotorUp(duration);
        }
        else if (strcmp(direction, "down") == 0)
        {
            actuatorManager.rampMotorDown(duration);
        }

        StaticJsonDocument<128> response;
        response["type"] = "motorRamp";
        response["direction"] = direction;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // SWEEP SERVO
    else if (strcmp(type, "sweepServo") == 0)
    {
        int servo = doc["servo"] | 1;
        int speed = doc["speed"] | 10;

        actuatorManager.sweepServo(servo, 0, 180, speed);

        StaticJsonDocument<128> response;
        response["type"] = "servoSweep";
        response["servo"] = servo;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // CYCLE RELAYS
    else if (strcmp(type, "cycleRelays") == 0)
    {
        int times = doc["times"] | 3;
        int interval = doc["interval"] | 500;

        actuatorManager.cycleRelays(times, interval);

        StaticJsonDocument<128> response;
        response["type"] = "relaysCycle";
        response["times"] = times;
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // PLAY MELODY
    else if (strcmp(type, "playMelody") == 0)
    {
        const char *melody = doc["melody"];
        if (melody)
        {
            actuatorManager.playMelody(melody);
        }

        StaticJsonDocument<128> response;
        response["type"] = "melodyPlaying";
        response["melody"] = melody ? melody : "";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // EXECUTE SCENE
    else if (strcmp(type, "executeScene") == 0)
    {
        const char *scene = doc["scene"];
        if (scene)
        {
            actuatorManager.executeScene(scene);
        }

        StaticJsonDocument<128> response;
        response["type"] = "sceneExecuted";
        response["scene"] = scene ? scene : "";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // SAVE SCENE
    else if (strcmp(type, "saveScene") == 0)
    {
        const char *name = doc["name"];
        if (name)
        {
            actuatorManager.saveCurrentScene(name);
        }

        StaticJsonDocument<128> response;
        response["type"] = "sceneSaved";
        response["name"] = name ? name : "";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
    }
    // GET SCENES
    else if (strcmp(type, "getScenes") == 0)
    {
        actuatorManager.loadSceneList();

        StaticJsonDocument<256> response;
        response["type"] = "scenesList";

        // Create default scenes array
        JsonArray scenes = response.createNestedArray("scenes");
        scenes.add("welcome");
        scenes.add("alert");
        scenes.add("party");
        scenes.add("calm");
        scenes.add("security");
        scenes.add("rainbow");
        scenes.add("pulse");
        scenes.add("breathe");
        scenes.add("chase");
        scenes.add("fire");

        char buffer[256];
        serializeJson(response, buffer);
        client->text(buffer);
    }
    // RGB EFFECT
    else if (strcmp(type, "rgbEffect") == 0)
    {
        const char *effect = doc["effect"];

        if (effect)
        {
            // Execute the effect via scene system
            if (strcmp(effect, "rainbow") == 0)
            {
                actuatorManager.executeScene("rainbow");
            }
            else if (strcmp(effect, "pulse") == 0)
            {
                actuatorManager.executeScene("pulse");
            }
            else if (strcmp(effect, "chase") == 0)
            {
                actuatorManager.executeScene("chase");
            }
            else if (strcmp(effect, "fire") == 0)
            {
                actuatorManager.executeScene("fire");
            }
            else if (strcmp(effect, "stop") == 0)
            {
                actuatorManager.setRGBColor(0, 0, 0);
            }
        }

        StaticJsonDocument<128> response;
        response["type"] = "rgbEffectStarted";
        response["effect"] = effect ? effect : "";
        char buffer[128];
        serializeJson(response, buffer);
        ws->textAll(buffer);
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

    // BUZZER CONTROL API
    server->on("/api/buzzer", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* action = doc["action"];

        if (!action) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing action\"}");
            return;
        }

        bool success = false;

        if (strcmp(action, "playTone") == 0) {
            int frequency = doc["frequency"] | 1000;
            int duration = doc["duration"] | 500;
            actuatorManager.getBuzzerController()->playTone(frequency, duration);
            success = true;
        }
        else if (strcmp(action, "playMelody") == 0) {
            const char* melody = doc["melody"];
            if (melody) {
                if (strcmp(melody, "startup") == 0) {
                    // Startup melody
                    int notes[] = {523, 659, 784};
                    int durations[] = {200, 200, 400};
                    actuatorManager.getBuzzerController()->playMelody(notes, durations, 3);
                }
                else if (strcmp(melody, "alert") == 0) {
                    // Alert melody
                    int notes[] = {880, 440, 880, 440};
                    int durations[] = {200, 200, 200, 200};
                    actuatorManager.getBuzzerController()->playMelody(notes, durations, 4);
                }
                else if (strcmp(melody, "emergency") == 0) {
                    // Emergency melody
                    int notes[] = {800, 600, 800, 600};
                    int durations[] = {100, 100, 100, 100};
                    actuatorManager.getBuzzerController()->playMelody(notes, durations, 4);
                }
                success = true;
            }
        }
        else if (strcmp(action, "setState") == 0) {
            bool state = doc["state"] | false;
            actuatorManager.getBuzzerController()->setState(state);
            success = true;
        }
        else if (strcmp(action, "beep") == 0) {
            int frequency = doc["frequency"] | 1000;
            int duration = doc["duration"] | 500;
            actuatorManager.getBuzzerController()->playBeep(frequency, duration);
            success = true;
        }
        else if (strcmp(action, "errorSound") == 0) {
            actuatorManager.getBuzzerController()->playErrorSound();
            success = true;
        }
        else if (strcmp(action, "successSound") == 0) {
            actuatorManager.getBuzzerController()->playSuccessSound();
            success = true;
        }
        else if (strcmp(action, "alertSound") == 0) {
            actuatorManager.getBuzzerController()->playAlertSound();
            success = true;
        }
        else if (strcmp(action, "siren") == 0) {
            int duration = doc["duration"] | 2000;
            actuatorManager.getBuzzerController()->sirenSound(duration);
            success = true;
        }
        else if (strcmp(action, "stop") == 0) {
            actuatorManager.getBuzzerController()->stopTone();
            success = true;
        }

        if (success) {
            request->send(200, "application/json", "{\"success\":true}");

            // Broadcast buzzer action to WebSocket clients
            StaticJsonDocument<256> response;
            response["type"] = "buzzerAction";
            response["action"] = action;
            if (doc.containsKey("frequency")) response["frequency"] = doc["frequency"];
            if (doc.containsKey("duration")) response["duration"] = doc["duration"];
            response["success"] = true;

            char buffer[256];
            serializeJson(response, buffer);
            webServer.ws->textAll(buffer);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid action\"}");
        } });

    // BUZZER STATUS API
    server->on("/api/buzzer/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        doc["type"] = "buzzerStatus";
        doc["state"] = actuatorManager.getBuzzerController()->getState();
        doc["playing"] = actuatorManager.getBuzzerController()->isPlaying();
        doc["playTime"] = actuatorManager.getBuzzerController()->getPlayTime();

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // BLINK LED API
    server->on("/api/led/blink", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int times = doc["times"] | 3;
        int interval = doc["interval"] | 500;

        actuatorManager.blinkLED(times, interval);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "ledBlink";
        wsResponse["times"] = times;
        wsResponse["interval"] = interval;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // MOTOR RAMP API
    server->on("/api/motor/ramp", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* direction = doc["direction"];
        int duration = doc["duration"] | 2000;

        if (strcmp(direction, "up") == 0) {
            actuatorManager.rampMotorUp(duration);
        }
        else if (strcmp(direction, "down") == 0) {
            actuatorManager.rampMotorDown(duration);
        }
        else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid direction\"}");
            return;
        }

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "motorRamp";
        wsResponse["direction"] = direction;
        wsResponse["duration"] = duration;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // SERVO SWEEP API
    server->on("/api/servo/sweep", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int servo = doc["servo"] | 1;
        int startAngle = doc["startAngle"] | 0;
        int endAngle = doc["endAngle"] | 180;
        int speed = doc["speed"] | 10;

        actuatorManager.sweepServo(servo, startAngle, endAngle, speed);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "servoSweep";
        wsResponse["servo"] = servo;
        wsResponse["startAngle"] = startAngle;
        wsResponse["endAngle"] = endAngle;
        wsResponse["speed"] = speed;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // CYCLE RELAYS API
    server->on("/api/relays/cycle", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int times = doc["times"] | 3;
        int interval = doc["interval"] | 500;

        actuatorManager.cycleRelays(times, interval);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "relaysCycle";
        wsResponse["times"] = times;
        wsResponse["interval"] = interval;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // PLAY MELODY API
    server->on("/api/buzzer/melody", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* melody = doc["melody"];

        if (!melody) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing melody\"}");
            return;
        }

        actuatorManager.playMelody(melody);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "buzzerMelody";
        wsResponse["melody"] = melody;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // SAVE SCENE API
    server->on("/api/scenes/save", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* sceneName = doc["name"];

        if (!sceneName) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing scene name\"}");
            return;
        }

        actuatorManager.saveCurrentScene(sceneName);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "sceneSaved";
        wsResponse["name"] = sceneName;
        wsResponse["success"] = true;

        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // LIST SCENES API
    server->on("/api/scenes/list", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        actuatorManager.loadSceneList();

        request->send(200, "application/json", "{\"success\":true}"); });

    // RGB LED CONTROL API
    server->on("/api/rgbled/color", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int r = doc["r"] | doc["red"] | 0;
        int g = doc["g"] | doc["green"] | 0;
        int b = doc["b"] | doc["blue"] | 0;

        actuatorManager.setRGBColor(r, g, b);

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<256> response;
        response["type"] = "rgbColorSet";
        response["r"] = r;
        response["g"] = g;
        response["b"] = b;
        response["success"] = true;

        char buffer[256];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // RGB LED EFFECT API
    server->on("/api/rgbled/effect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* effect = doc["effect"];

        if (effect) {
            if (strcmp(effect, "rainbow") == 0) {
                // Start rainbow effect
                request->send(200, "application/json", "{\"success\":true,\"effect\":\"rainbow\"}");
            }
            else if (strcmp(effect, "pulse") == 0) {
                request->send(200, "application/json", "{\"success\":true,\"effect\":\"pulse\"}");
            }
            else if (strcmp(effect, "chase") == 0) {
                request->send(200, "application/json", "{\"success\":true,\"effect\":\"chase\"}");
            }
            else if (strcmp(effect, "fire") == 0) {
                request->send(200, "application/json", "{\"success\":true,\"effect\":\"fire\"}");
            }
            else if (strcmp(effect, "stop") == 0) {
                actuatorManager.setRGBColor(0, 0, 0);
                request->send(200, "application/json", "{\"success\":true,\"effect\":\"stopped\"}");
            }
            else {
                request->send(400, "application/json", "{\"success\":false,\"error\":\"Unknown effect\"}");
            }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing effect\"}");
        }

        // Broadcast effect change
        StaticJsonDocument<128> wsResponse;
        wsResponse["type"] = "rgbEffect";
        wsResponse["effect"] = effect;
        char buffer[128];
        serializeJson(wsResponse, buffer);
        webServer.ws->textAll(buffer); });

    // RGB LED BRIGHTNESS API
    server->on("/api/rgbled/brightness", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int brightness = doc["brightness"] | 255;
        actuatorManager.setRGBBrightness(brightness);

        request->send(200, "application/json", "{\"success\":true,\"brightness\":" + String(brightness) + "}"); });

    // RGB LED STATUS API
    server->on("/api/rgbled/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<512> doc;
        doc["type"] = "rgbledStatus";
        
        // Get current RGB values from actuator manager
        String status = actuatorManager.getStatus();
        StaticJsonDocument<512> statusDoc;
        deserializeJson(statusDoc, status);
        
        if (statusDoc.containsKey("rgb")) {
            doc["rgb"] = statusDoc["rgb"];
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // SERVO ANGLE API
    server->on("/api/servo/angle", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int servo = doc["servo"] | 1;
        int angle = doc["angle"] | 90;

        // Validate angle
        if (angle < 0) angle = 0;
        if (angle > 180) angle = 180;

        actuatorManager.setServoAngle(servo, angle);

        request->send(200, "application/json", "{\"success\":true,\"servo\":" + String(servo) + ",\"angle\":" + String(angle) + "}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "servoAngleSet";
        response["servo"] = servo;
        response["angle"] = angle;
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // SERVO STATUS API
    server->on("/api/servo/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        doc["type"] = "servoStatus";
        doc["servo1"] = actuatorManager.getServoAngle(1);
        doc["servo2"] = actuatorManager.getServoAngle(2);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // RELAY STATE API
    server->on("/api/relay/state", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int relay = doc["relay"] | 1;
        bool state = doc["state"] | false;

        if (relay < 1 || relay > 3) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid relay number (1-3)\"}");
            return;
        }

        actuatorManager.setRelay(relay, state);

        request->send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"state\":" + String(state ? "true" : "false") + "}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "relayStateChanged";
        response["relay"] = relay;
        response["state"] = state;
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // RELAY PULSE API
    server->on("/api/relay/pulse", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int relay = doc["relay"] | 1;
        int duration = doc["duration"] | 1000;

        if (relay < 1 || relay > 3) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid relay number\"}");
            return;
        }

        actuatorManager.pulseRelay(relay, duration);

        request->send(200, "application/json", "{\"success\":true,\"relay\":" + String(relay) + ",\"duration\":" + String(duration) + "}"); });

    // RELAY STATUS API
    server->on("/api/relay/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        doc["type"] = "relayStatus";
        doc["relay1"] = actuatorManager.getRelay(1);
        doc["relay2"] = actuatorManager.getRelay(2);
        doc["relay3"] = actuatorManager.getRelay(3);

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // MOTOR SPEED API
    server->on("/api/motor/speed", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        int speed = doc["speed"] | 0;
        
        // Validate speed
        if (speed < 0) speed = 0;
        if (speed > 100) speed = 100;

        // Get direction if provided
        const char* direction = doc["direction"];
        if (direction) {
            if (strcmp(direction, "forward") == 0) {
                actuatorManager.setMotorDirection(true);
            } else if (strcmp(direction, "backward") == 0) {
                actuatorManager.setMotorDirection(false);
            }
        }

        actuatorManager.setMotorSpeed(speed);

        request->send(200, "application/json", "{\"success\":true,\"speed\":" + String(speed) + "}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "motorSpeedSet";
        response["speed"] = speed;
        response["direction"] = actuatorManager.getDirection() ? "forward" : "backward";
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // MOTOR DIRECTION API
    server->on("/api/motor/direction", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"JSON parse error\"}");
            return;
        }

        const char* direction = doc["direction"];

        if (!direction) {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing direction\"}");
            return;
        }

        bool forward = (strcmp(direction, "forward") == 0);
        actuatorManager.setMotorDirection(forward);

        request->send(200, "application/json", "{\"success\":true,\"direction\":\"" + String(direction) + "\"}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "motorDirectionSet";
        response["direction"] = direction;
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // MOTOR STOP API
    server->on("/api/motor/stop", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        actuatorManager.stopMotor();

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "motorStopped";
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // MOTOR BRAKE API
    server->on("/api/motor/brake", HTTP_POST, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        actuatorManager.stopMotor();
        // For L298N, setting both IN1 and IN2 HIGH creates a short brake

        request->send(200, "application/json", "{\"success\":true}");

        // Broadcast to WebSocket clients
        StaticJsonDocument<128> response;
        response["type"] = "motorBrake";
        response["success"] = true;

        char buffer[128];
        serializeJson(response, buffer);
        webServer.ws->textAll(buffer); });

    // MOTOR STATUS API
    server->on("/api/motor/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<256> doc;
        doc["type"] = "motorStatus";
        doc["speed"] = actuatorManager.getSpeed();
        doc["direction"] = actuatorManager.getDirection() ? "forward" : "backward";

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

    // Get Actuator Status
    server->on("/api/actuators/status", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        String status = actuatorManager.getStatus();
        request->send(200, "application/json", status); });

    // Get Actuator Log
    server->on("/api/actuators/log", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        String log = dataLogger.readLog("actuators", 100);
        request->send(200, "text/plain", log); });

    // Clear Actuator Log
    server->on("/api/actuators/log", HTTP_DELETE, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;
        dataLogger.deleteLog("actuators");
        request->send(200, "application/json", "{\"success\":true}"); });

    // Export Actuator Data
    server->on("/api/actuators/export", HTTP_GET, [](AsyncWebServerRequest *request)
               {
        webServer.totalRequests++;

        StaticJsonDocument<2048> doc;

        // System info
        JsonObject system = doc.createNestedObject("system");
        system["device"] = DEVICE_NAME;
        system["version"] = FIRMWARE_VERSION;
        system["uptime"] = millis();
        system["freeHeap"] = ESP.getFreeHeap();
        system["ip"] = WiFi.localIP().toString();

        // Actuator status
        String status = actuatorManager.getStatus();
        StaticJsonDocument<1024> statusDoc;
        deserializeJson(statusDoc, status);
        doc["actuators"] = statusDoc;

        // Activity log
        JsonArray activityLog = doc.createNestedArray("activityLog");
        String log = dataLogger.readLog("events", 50);
        // Parse log lines and add to array
        int start = 0;
        int end = log.indexOf('\n');
        while (end != -1 && activityLog.size() < 50) {
            String line = log.substring(start, end);
            if (line.length() > 0) {
                JsonObject entry = activityLog.createNestedObject();
                entry["timestamp"] = millis();
                entry["activity"] = line;
            }
            start = end + 1;
            end = log.indexOf('\n', start);
        }

        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response); });

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

            String html = "<!DOCTYPE html><html><head><title>ESP32 IoT Dashboard</title>";
            html += "<style>body {font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5;}";
            html += "h1 {color: #333;} ul {list-style-type: none; padding: 0;}";
            html += "li {padding: 8px; margin: 5px 0; background: white; border-radius: 4px;}</style></head><body>";
            html += "<h1>🗂️ SPIFFS Files Debug</h1>";

            if (webServer.spiffsAvailable) {
                html += "<p><strong>SPIFFS Status:</strong> ✓ Available</p>";
                html += "<h2>All Files:</h2><ul>";
                File root = SPIFFS.open("/");
                File file = root.openNextFile();

                int fileCount = 0;
                while(file){
                    String filePath = String(file.path());
                    html += "<li>📄 <strong>" + filePath + "</strong> (" + String(file.size()) + " bytes)";
                    html += " <a href='" + filePath + "' target='_blank'>Open</a></li>";
                    file = root.openNextFile();
                    fileCount++;
                }
                html += "</ul><p>Total files: " + String(fileCount) + "</p>";
            } else {
                html += "<p><strong>SPIFFS Status:</strong> ✗ Not Available</p>";
            }

            html += "<hr><p><a href='/'>← Back to Dashboard</a></p></body></html>";
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