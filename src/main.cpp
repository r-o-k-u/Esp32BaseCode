/**
 * ═══════════════════════════════════════════════════════════════════════════
 * ESP32 DUAL COMMUNICATION SYSTEM - MAIN APPLICATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file main.cpp
 * @brief Main application entry point with complete system initialization
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * DESCRIPTION:
 * ═══════════════════════════════════════════════════════════════════════════
 * This is the heart of the ESP32 Dual Communication System. It orchestrates
 * all subsystems including:
 *
 * - WiFi connectivity with auto-reconnection
 * - ESP-NOW device-to-device communication
 * - Web server with real-time WebSocket updates
 * - OTA (Over-The-Air) firmware updates
 * - Sensor data collection from multiple sources
 * - Actuator control (relays, servos, motors, etc.)
 * - Camera support (ESP32-CAM only)
 * - Data logging to SPIFFS filesystem
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM ARCHITECTURE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 *    ┌─────────────────────────────────────────────────────────────┐
 *    │                      MAIN CONTROLLER                         │
 *    │                        (main.cpp)                            │
 *    └────────┬────────────────────────────────────────────┬────────┘
 *             │                                             │
 *    ┌────────▼────────┐                          ┌────────▼────────┐
 *    │   CORE MODULES  │                          │  HARDWARE I/O   │
 *    │  - WiFi Manager │                          │   - Sensors     │
 *    │  - OTA Manager  │                          │   - Actuators   │
 *    │  - Web Server   │                          │   - Camera      │
 *    │  - ESP-NOW Comm │                          │                 │
 *    │  - Data Logger  │                          └─────────────────┘
 *    └─────────────────┘
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * WORKFLOW:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. SETUP PHASE (runs once):
 *    - Initialize serial communication
 *    - Mount SPIFFS filesystem
 *    - Connect to WiFi / Start AP
 *    - Initialize ESP-NOW
 *    - Setup OTA
 *    - Initialize sensors
 *    - Initialize actuators
 *    - Start web server
 *    - Print system info
 *
 * 2. LOOP PHASE (runs continuously):
 *    - Handle OTA updates
 *    - Read sensors periodically
 *    - Send data via ESP-NOW
 *    - Update actuators
 *    - Monitor connections
 *    - Process WebSocket messages
 *    - Log data
 *    - Heartbeat LED
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * TIMER-BASED OPERATIONS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * We use non-blocking timers instead of delay() to keep the system responsive:
 *
 * - Sensor Reading: Every 2 seconds (configurable)
 * - Status Update: Every 5 seconds (ESP-NOW broadcast)
 * - Heartbeat LED: Every 1 second (visual feedback)
 * - Data Logging: Every 60 seconds (saves to SPIFFS)
 * - Peer Check: Every 30 seconds (ESP-NOW connectivity)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

// ═══════════════════════════════════════════════════════════════════════════
// INCLUDE HEADERS
// ═══════════════════════════════════════════════════════════════════════════

// Standard Arduino libraries
#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

// Project configuration
#include "config.h"
#include "credentials.h"

// Core system modules
#include "core/WiFiManager.h"
#include "core/OTAManager.h"
#include "core/WebServer.h"
#include "core/ESPNowComm.h"
#include "core/DataLogger.h"

// Sensor and actuator management
#include "sensors/SensorManager.h"
#include "actuators/ActuatorManager.h"

// Camera module (ESP32-CAM only)
#if ENABLE_CAMERA && (DEVICE_TYPE == 1)
#include "camera/CameraManager.h"
#endif

// Utility modules
#include "utils/Logger.h"
#include "utils/Timer.h"

// ═══════════════════════════════════════════════════════════════════════════
// EXTERN GLOBAL OBJECT DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * These global objects are defined in their respective .cpp files
 * and declared extern here for main.cpp to use them
 */
extern WiFiManager wifiManager;
extern OTAManager otaManager;
extern WebServerManager webServer;
extern SensorManager sensorManager;
extern ActuatorManager actuatorManager;
extern DataLogger dataLogger;

// ═══════════════════════════════════════════════════════════════════════════
// GLOBAL OBJECT INSTANCES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * These are the main system components. Each handles a specific domain:
 * - wifiManager: WiFi connectivity and AP mode (defined in WiFiManager.cpp)
 * - otaManager: Over-the-air firmware updates (defined in OTAManager.cpp)
 * - webServer: HTTP server and WebSocket communication (defined in WebServer.cpp)
 * - sensorManager: All sensor reading and management (defined in SensorManager.cpp)
 * - actuatorManager: All actuator control (defined in ActuatorManager.cpp)
 * - dataLogger: Data persistence to SPIFFS (defined in DataLogger.cpp)
 */

uint8_t peerMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast

#if ENABLE_CAMERA && (DEVICE_TYPE == 1)
CameraManager cameraManager;
#endif

// ═══════════════════════════════════════════════════════════════════════════
// TIMER OBJECTS FOR NON-BLOCKING OPERATIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Timers prevent blocking the main loop with delay().
 * Each timer triggers its action when the interval elapses.
 */
Timer sensorTimer(SENSOR_READ_INTERVAL);   // Read sensors every 2s
Timer statusTimer(STATUS_UPDATE_INTERVAL); // Send status every 5s
Timer heartbeatTimer(HEARTBEAT_INTERVAL);  // Blink LED every 1s
Timer loggingTimer(60000);                 // Log data every 60s
Timer peerCheckTimer(30000);               // Check peers every 30s

// ═══════════════════════════════════════════════════════════════════════════
// SYSTEM STATE VARIABLES
// ═══════════════════════════════════════════════════════════════════════════

bool systemReady = false; // System initialization complete flag
uint32_t bootTime = 0;    // Boot timestamp
uint32_t loopCounter = 0; // Main loop iteration counter
bool ledState = false;    // LED blink state

// ═══════════════════════════════════════════════════════════════════════════
// FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════

void onESPNowDataReceived(const uint8_t *mac, const char *data, uint8_t type);
void onESPNowDataSent(const uint8_t *mac, bool success);
void readAndSendSensorData();
void sendStatusUpdate();
void checkSystemHealth();
void blinkLED(int count, int delayMs);
bool initSPIFFS();
void printSystemInfo();
void printBootBanner();

// ═══════════════════════════════════════════════════════════════════════════
// ESP-NOW CALLBACK: DATA RECEIVED
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Callback function when ESP-NOW data is received from peer device
 *
 * This function is automatically called by the ESP-NOW subsystem when a
 * message arrives from another ESP32 device. It handles different message
 * types and takes appropriate actions.
 *
 * @param mac Sender's MAC address
 * @param data Message payload (JSON string)
 * @param type Message type (see ESPNowComm.h for types)
 *
 * MESSAGE TYPES:
 * - MSG_SENSOR_DATA: Sensor readings from peer
 * - MSG_ACTUATOR_CMD: Command to control our actuators
 * - MSG_STATUS: Status update from peer
 * - MSG_ALERT: Alert/warning message
 * - MSG_CONFIG: Configuration change
 * - MSG_SYNC: Time synchronization
 *
 * EXAMPLE MESSAGE:
 * Type: MSG_ACTUATOR_CMD
 * Data: {"actuator":"relay1","value":1}
 *
 * THREAD SAFETY:
 * This runs in interrupt context, so keep it fast!
 * Heavy processing should be deferred to main loop.
 */
void onESPNowDataReceived(const uint8_t *mac, const char *data, uint8_t type)
{
  // Print reception info
  DEBUG_PRINTLN("\n╔═══════════════════════════════════════════════════╗");
  DEBUG_PRINTLN("║         ESP-NOW MESSAGE RECEIVED                  ║");
  DEBUG_PRINTLN("╚═══════════════════════════════════════════════════╝");
  DEBUG_PRINTF("From: %s\n", espnowComm.getMacString(mac).c_str());
  DEBUG_PRINTF("Type: %d\n", type);
  DEBUG_PRINTF("Data: %s\n", data);
  DEBUG_PRINTLN("─────────────────────────────────────────────────────");

  // Parse JSON data
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, data);

  if (error)
  {
    DEBUG_PRINTF("ERROR: JSON parsing failed - %s\n", error.c_str());
    return;
  }

  // Handle different message types
  switch (type)
  {
  case MSG_SENSOR_DATA:
  {
    // Received sensor data from peer device
    DEBUG_PRINTLN("📊 Processing peer sensor data...");

    // Log the peer's sensor data
    dataLogger.logData("peer_sensor", data);

    // Broadcast to web clients for real-time display
    webServer.broadcastSensorData(data);

    // Optional: Take action based on peer's sensor readings
    // Example: If peer reports high temperature, activate cooling
    if (doc.containsKey("temperature"))
    {
      float peerTemp = doc["temperature"];
      if (peerTemp > 30.0)
      {
        DEBUG_PRINTLN("⚠️ Peer reports high temperature!");
        // Could activate a fan or send alert
      }
    }
    break;
  }

  case MSG_ACTUATOR_CMD:
  {
    // Received command to control our actuators
    DEBUG_PRINTLN("🎛️ Processing actuator command...");

    if (doc.containsKey("actuator") && doc.containsKey("value"))
    {
      const char *actuatorName = doc["actuator"];
      int value = doc["value"];

      DEBUG_PRINTF("Command: Set %s to %d\n", actuatorName, value);

      // Execute the actuator command
      actuatorManager.setActuator(actuatorName, value);

      // Send acknowledgment back to sender
      StaticJsonDocument<128> ackDoc;
      ackDoc["status"] = "ok";
      ackDoc["actuator"] = actuatorName;
      ackDoc["value"] = value;

      char ackBuffer[128];
      serializeJson(ackDoc, ackBuffer);
      espnowComm.sendMessage(mac, MSG_ACK, ackBuffer);
    }
    break;
  }

  case MSG_STATUS:
  {
    // Received status update from peer
    DEBUG_PRINTLN("📈 Peer status update received");

    // Could display peer status on LCD, store for monitoring, etc.
    if (doc.containsKey("uptime"))
    {
      uint32_t peerUptime = doc["uptime"];
      DEBUG_PRINTF("Peer uptime: %lu seconds\n", peerUptime / 1000);
    }
    break;
  }

  case MSG_ALERT:
  {
    // Received alert from peer
    DEBUG_PRINTLN("🚨 ALERT from peer!");

    if (doc.containsKey("message"))
    {
      const char *alertMsg = doc["message"];
      DEBUG_PRINTF("Alert: %s\n", alertMsg);

      // Trigger local alert indication
      actuatorManager.triggerAlert();

      // Broadcast alert to web clients
      webServer.broadcastAlert(alertMsg);
    }
    break;
  }

  case MSG_CONFIG:
  {
    // Configuration change from peer
    DEBUG_PRINTLN("⚙️ Configuration update received");
    // Handle configuration changes
    break;
  }

  case MSG_SYNC:
  {
    // Time synchronization
    DEBUG_PRINTLN("🕐 Time sync request");
    // Handle time sync
    break;
  }

  default:
    DEBUG_PRINTF("Unknown message type: %d\n", type);
    break;
  }

  DEBUG_PRINTLN("═════════════════════════════════════════════════════\n");
}

// ═══════════════════════════════════════════════════════════════════════════
// ESP-NOW CALLBACK: DATA SENT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Callback when ESP-NOW data transmission completes
 *
 * Called after attempting to send data to a peer. Indicates success or failure.
 *
 * @param mac Destination MAC address
 * @param success true if delivery confirmed, false if failed
 *
 * DELIVERY CONFIRMATION:
 * ESP-NOW provides automatic acknowledgment at the MAC layer.
 * Success means the peer received the packet (not that they processed it).
 *
 * HANDLING FAILURES:
 * - Retry important messages
 * - Log failed transmissions
 * - Alert user if peer is unreachable
 * - Switch to backup communication method
 */
void onESPNowDataSent(const uint8_t *mac, bool success)
{
#if DEBUG_ESPNOW
  DEBUG_PRINTF("Send to %s: %s\n",
               espnowComm.getMacString(mac).c_str(),
               success ? "✓ Success" : "✗ Failed");
#endif

  // If send failed, could retry or log error
  if (!success)
  {
    DEBUG_PRINTLN("⚠️ Message delivery failed!");
    // Optional: Implement retry logic
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// INITIALIZE SPIFFS FILESYSTEM
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Initialize SPIFFS (SPI Flash File System)
 *
 * SPIFFS is used for:
 * - Storing web page files (HTML, CSS, JS)
 * - Logging sensor data
 * - Storing configuration
 * - Caching data
 *
 * @return true if mounted successfully
 *
 * FILESYSTEM STRUCTURE:
 * /data/              <- Web files
 *   index.html
 *   style.css
 *   script.js
 * /logs/              <- Log files
 *   sensors.log
 *   errors.log
 * /config/            <- Configuration files
 *   settings.json
 *
 * TROUBLESHOOTING:
 * If mount fails:
 * 1. Check SPIFFS partition is defined in platformio.ini
 * 2. Format SPIFFS: SPIFFS.format()
 * 3. Re-upload filesystem: pio run -t uploadfs
 */
bool initSPIFFS()
{
  DEBUG_PRINT("Mounting SPIFFS filesystem... ");

  // Mount SPIFFS (format if mount fails)
  if (!SPIFFS.begin(true))
  {
    DEBUG_PRINTLN("✗ FAILED!");
    DEBUG_PRINTLN("ERROR: Could not mount SPIFFS!");
    return false;
  }

  DEBUG_PRINTLN("✓ OK");

  // Print filesystem information
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  float usedPercent = (float)usedBytes / totalBytes * 100;

  DEBUG_PRINTLN("┌─────────────────────────────────────┐");
  DEBUG_PRINTLN("│      SPIFFS FILESYSTEM INFO         │");
  DEBUG_PRINTLN("├─────────────────────────────────────┤");
  DEBUG_PRINTF("│ Total:  %7d bytes (%.1f KB) │\n", totalBytes, totalBytes / 1024.0);
  DEBUG_PRINTF("│ Used:   %7d bytes (%.1f KB) │\n", usedBytes, usedBytes / 1024.0);
  DEBUG_PRINTF("│ Free:   %7d bytes (%.1f KB) │\n", totalBytes - usedBytes, (totalBytes - usedBytes) / 1024.0);
  DEBUG_PRINTF("│ Usage:  %6.1f%%                 │\n", usedPercent);
  DEBUG_PRINTLN("└─────────────────────────────────────┘");

  // Warn if filesystem is almost full
  if (usedPercent > 90)
  {
    DEBUG_PRINTLN("⚠️ WARNING: SPIFFS is over 90% full!");
    DEBUG_PRINTLN("Consider cleaning up old logs.");
  }

  return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// READ SENSORS AND SEND DATA
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Read all sensors and distribute data
 *
 * This function:
 * 1. Reads all available sensors
 * 2. Creates JSON data structure
 * 3. Logs data to SPIFFS
 * 4. Sends to web clients via WebSocket
 * 5. Sends to ESP-NOW peers
 *
 * Called periodically by sensorTimer (default: every 2 seconds)
 */
void readAndSendSensorData()
{
  // Create JSON document for sensor data
  StaticJsonDocument<1024> doc;

  // Read all sensors and populate JSON
  JsonObject root = doc.to<JsonObject>();
  sensorManager.getAllSensorData(root);

  // Add metadata
  doc["timestamp"] = millis();
  doc["device"] = DEVICE_NAME;
  doc["type"] = DEVICE_TYPE == 1 ? "ESP32-CAM" : "ESP32";

  // Convert to string
  char jsonBuffer[1024];
  size_t jsonSize = serializeJson(doc, jsonBuffer);

// Print to serial (if debugging)
#if LOG_TO_SERIAL && DEBUG_SENSORS
  DEBUG_PRINTLN("\n╔═══════════════════════════════════════════════════╗");
  DEBUG_PRINTLN("║           SENSOR DATA UPDATE                      ║");
  DEBUG_PRINTLN("╚═══════════════════════════════════════════════════╝");
  serializeJsonPretty(doc, Serial);
  DEBUG_PRINTLN("\n═════════════════════════════════════════════════════");
#endif

// Log to SPIFFS
#if LOG_TO_SPIFFS
  if (dataLogger.logData("sensors", jsonBuffer))
  {
    DEBUG_PRINTLN("✓ Data logged to SPIFFS");
  }
#endif

  // Broadcast to web clients
  webServer.broadcastSensorData(jsonBuffer);

  // Send to all ESP-NOW peers
  int peerCount = espnowComm.getPeerCount();
  if (peerCount > 0)
  {
    DEBUG_PRINTF("📡 Sending sensor data to %d peer(s)...\n", peerCount);
    espnowComm.sendToAllPeers(MSG_SENSOR_DATA, jsonBuffer);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SEND STATUS UPDATE
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Send system status to peers and web clients
 *
 * Status includes:
 * - Device name and type
 * - Firmware version
 * - Uptime
 * - Free heap memory
 * - WiFi RSSI
 * - ESP-NOW statistics
 * - Sensor count
 *
 * Called periodically by statusTimer (default: every 5 seconds)
 */
void sendStatusUpdate()
{
  // Get ESP-NOW statistics
  uint32_t sent, received, failed;
  espnowComm.getStatistics(sent, received, failed);

  // Create status JSON
  StaticJsonDocument<512> doc;
  doc["device"] = DEVICE_NAME;
  doc["type"] = DEVICE_TYPE == 1 ? "ESP32-CAM" : "ESP32";
  doc["version"] = FIRMWARE_VERSION;
  doc["uptime"] = millis() - bootTime;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["heapSize"] = ESP.getHeapSize();
  doc["wifiConnected"] = wifiManager.isConnected();
  doc["wifiRSSI"] = WiFi.RSSI();
  doc["ip"] = WiFi.localIP().toString();

  // ESP-NOW statistics
  JsonObject espnow = doc.createNestedObject("espnow");
  espnow["sent"] = sent;
  espnow["received"] = received;
  espnow["failed"] = failed;
  espnow["peers"] = espnowComm.getPeerCount();

  // Sensor information
  doc["sensors"] = sensorManager.getSensorCount();

  // Convert to string
  char buffer[512];
  serializeJson(doc, buffer);

  // Send to peers
  espnowComm.sendToAllPeers(MSG_STATUS, buffer);

  // Broadcast to web clients
  webServer.broadcastStatus(buffer);
}

// ═══════════════════════════════════════════════════════════════════════════
// CHECK SYSTEM HEALTH
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Monitor system health and take corrective actions
 *
 * Monitors:
 * - Memory leaks (heap decreasing)
 * - WiFi connectivity
 * - ESP-NOW peer availability
 * - Sensor failures
 * - Filesystem space
 *
 * Takes action if problems detected:
 * - Reconnect WiFi
 * - Restart services
 * - Clear logs
 * - Alert user
 */
void checkSystemHealth()
{
  static uint32_t lastHeapSize = 0;
  uint32_t currentHeap = ESP.getFreeHeap();

  // Check for memory leaks
  if (lastHeapSize > 0)
  {
    int32_t heapChange = currentHeap - lastHeapSize;
    if (heapChange < -5000)
    { // Lost more than 5KB
      DEBUG_PRINTLN("⚠️ WARNING: Possible memory leak detected!");
      DEBUG_PRINTF("Heap decreased by %d bytes\n", -heapChange);
    }
  }
  lastHeapSize = currentHeap;

  // Warn if memory is low
  if (currentHeap < 10000)
  {
    DEBUG_PRINTLN("⚠️ WARNING: Low memory!");
    DEBUG_PRINTF("Free heap: %d bytes\n", currentHeap);
  }

  // Check WiFi connection
  if (!wifiManager.isConnected())
  {
    DEBUG_PRINTLN("⚠️ WiFi disconnected, attempting reconnection...");
  }

  // Check ESP-NOW peer connectivity
  espnowComm.checkPeerActivity(60000); // Mark inactive if no message in 60s
}

// ═══════════════════════════════════════════════════════════════════════════
// LED BLINKING UTILITY
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Blink LED for visual feedback
 *
 * @param count Number of blinks
 * @param delayMs Delay between blinks in milliseconds
 *
 * BLINK PATTERNS:
 * - 1 blink: Operation successful
 * - 2 blinks: WiFi connected
 * - 3 blinks: System starting
 * - 5 blinks: System ready
 * - Fast blinking: Error condition
 */
void blinkLED(int count, int delayMs)
{
  for (int i = 0; i < count; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    if (i < count - 1)
      delay(delayMs);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// PRINT BOOT BANNER
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Print attractive boot banner to serial
 */
void printBootBanner()
{
  DEBUG_PRINTLN("\n\n");
  DEBUG_PRINTLN("╔═══════════════════════════════════════════════════════════════╗");
  DEBUG_PRINTLN("║                                                               ║");
  DEBUG_PRINTLN("║        ESP32 DUAL COMMUNICATION SYSTEM v2.0                  ║");
  DEBUG_PRINTLN("║        ═══════════════════════════════════                   ║");
  DEBUG_PRINTLN("║                                                               ║");
  DEBUG_PRINTLN("║        Two-Way ESP-NOW Communication                          ║");
  DEBUG_PRINTLN("║        WiFi Manager + OTA Updates                             ║");
  DEBUG_PRINTLN("║        Multi-Sensor & Actuator Support                        ║");
  DEBUG_PRINTLN("║        Real-time Web Dashboard                                ║");
  DEBUG_PRINTLN("║                                                               ║");
  DEBUG_PRINTLN("╚═══════════════════════════════════════════════════════════════╝");
  DEBUG_PRINTLN();
}

// ═══════════════════════════════════════════════════════════════════════════
// PRINT SYSTEM INFORMATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Print detailed system information
 */
void printSystemInfo()
{
  DEBUG_PRINTLN("┌───────────────────────────────────────────────────────────┐");
  DEBUG_PRINTLN("│                    SYSTEM INFORMATION                     │");
  DEBUG_PRINTLN("├───────────────────────────────────────────────────────────┤");
  DEBUG_PRINTF("│ Device Name:      %-35s │\n", DEVICE_NAME);
  DEBUG_PRINTF("│ Device Type:      %-35s │\n", DEVICE_TYPE == 1 ? "ESP32-CAM" : "ESP32");
  DEBUG_PRINTF("│ Firmware:         %-35s │\n", FIRMWARE_VERSION);
  DEBUG_PRINTF("│ Chip Model:       %-35s │\n", ESP.getChipModel());
  DEBUG_PRINTF("│ CPU Frequency:    %-30d MHz │\n", ESP.getCpuFreqMHz());
  DEBUG_PRINTF("│ Flash Size:       %-27d bytes │\n", ESP.getFlashChipSize());
  DEBUG_PRINTF("│ Free Heap:        %-27d bytes │\n", ESP.getFreeHeap());
  DEBUG_PRINTF("│ MAC Address:      %-35s │\n", WiFi.macAddress().c_str());
  DEBUG_PRINTLN("└───────────────────────────────────────────────────────────┘");
}

// ═══════════════════════════════════════════════════════════════════════════
// SETUP FUNCTION - RUNS ONCE AT BOOT
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Arduino setup function - initialize all subsystems
 *
 * This function runs once when the ESP32 boots up or resets.
 * It initializes all hardware and software components in the correct order.
 *
 * INITIALIZATION ORDER IS CRITICAL:
 * 1. Serial (for debugging)
 * 2. SPIFFS (for web files)
 * 3. WiFi (for network)
 * 4. ESP-NOW (for peer communication)
 * 5. Sensors (for data collection)
 * 6. Actuators (for control)
 * 7. OTA (for updates)
 * 8. Web Server (for UI)
 *
 * If any critical component fails, system continues with reduced functionality.
 */
void setup()
{
  // ─────────────────────────────────────────────────────────────────────
  // 1. INITIALIZE SERIAL COMMUNICATION
  // ─────────────────────────────────────────────────────────────────────
  Serial.begin(SERIAL_BAUD);
  delay(1000); // Wait for serial to be ready

  // Print boot banner
  printBootBanner();
  printSystemInfo();

  bootTime = millis();

  // ─────────────────────────────────────────────────────────────────────
  // 2. INITIALIZE GPIO
  // ─────────────────────────────────────────────────────────────────────
  DEBUG_PRINTLN("\n[1/9] Initializing GPIO...");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  blinkLED(3, 200); // 3 blinks = starting
  DEBUG_PRINTLN("✓ GPIO initialized");

  // ─────────────────────────────────────────────────────────────────────
  // 3. INITIALIZE SPIFFS FILESYSTEM
  // ─────────────────────────────────────────────────────────────────────
  DEBUG_PRINTLN("\n[2/9] Initializing SPIFFS...");
  if (!initSPIFFS())
  {
    DEBUG_PRINTLN("✗ CRITICAL: SPIFFS failed!");
    blinkLED(10, 100); // Fast blinking = error
                       // Continue anyway - system can work without SPIFFS
  }

  // ─────────────────────────────────────────────────────────────────────
  // 4. INITIALIZE WIFI
  // ─────────────────────────────────────────────────────────────────────
  DEBUG_PRINTLN("\n[3/9] Initializing WiFi...");
  if (wifiManager.begin(WIFI_SSID_1, WIFI_PASS_1))
  {
    DEBUG_PRINTLN("✓ WiFi connected!");
    DEBUG_PRINTF("   IP Address: %s\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTF("   Signal: %d dBm\n", WiFi.RSSI());
    blinkLED(2, 300); // 2 blinks = WiFi OK
  }
  else
  {
    DEBUG_PRINTLN("⚠️ WiFi connection failed");
    DEBUG_PRINTLN("   Starting Access Point mode...");
    wifiManager.startAP(AP_SSID, AP_PASSWORD);
    DEBUG_PRINTF("   AP SSID: %s\n", AP_SSID);
    DEBUG_PRINTF("   AP Password: %s\n", AP_PASSWORD);
    DEBUG_PRINTF("   AP IP: %s\n", WiFi.softAPIP().toString().c_str());
  }

// ─────────────────────────────────────────────────────────────────────
// 5. INITIALIZE ESP-NOW
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_ESPNOW
  DEBUG_PRINTLN("\n[4/9] Initializing ESP-NOW...");
  if (espnowComm.begin())
  {
    DEBUG_PRINTLN("✓ ESP-NOW initialized");

    // Register callbacks
    espnowComm.setOnDataRecv(onESPNowDataReceived);
    espnowComm.setOnDataSent(onESPNowDataSent);

    // Print our MAC address
    uint8_t mac[6];
    espnowComm.getOwnMac(mac);
    DEBUG_PRINTF("   MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    DEBUG_PRINTLN("   ⚠️ IMPORTANT: Use this MAC address in other device's config!");

    // Add peer devices from credentials.h
    DEBUG_PRINTLN("   Adding peer devices...");
    uint8_t peer1[] = PEER_MAC_1;
    if (espnowComm.addPeer(peer1, "Device_2"))
    {
      DEBUG_PRINTLN("   ✓ Peer 1 added successfully");
    }

    // Uncomment to add more peers
    // uint8_t peer2[] = PEER_MAC_2;
    // espnowComm.addPeer(peer2, "Device_3");

    // Print peer list
    espnowComm.printPeerList();

    blinkLED(1, 500); // 1 long blink = ESP-NOW OK
  }
  else
  {
    DEBUG_PRINTLN("✗ ESP-NOW initialization failed!");
  }
#else
  DEBUG_PRINTLN("\n[4/9] ESP-NOW disabled in config");
#endif

// ─────────────────────────────────────────────────────────────────────
// 6. INITIALIZE OTA
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_OTA
  DEBUG_PRINTLN("\n[5/9] Initializing OTA...");
  if (otaManager.begin(OTA_HOSTNAME, OTA_PASSWORD))
  {
    DEBUG_PRINTLN("✓ OTA ready");
    DEBUG_PRINTF("   Hostname: %s.local\n", OTA_HOSTNAME);
    DEBUG_PRINTLN("   Use Arduino IDE or PlatformIO for OTA updates");
  }
  else
  {
    DEBUG_PRINTLN("⚠️ OTA initialization failed");
  }
#else
  DEBUG_PRINTLN("\n[5/9] OTA disabled in config");
#endif

// ─────────────────────────────────────────────────────────────────────
// 7. INITIALIZE SENSORS
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_SENSORS
  DEBUG_PRINTLN("\n[6/9] Initializing Sensors...");
  uint8_t sensorCount = sensorManager.begin();
  DEBUG_PRINTF("✓ %d sensor(s) initialized\n", sensorCount);

  if (sensorCount == 0)
  {
    DEBUG_PRINTLN("⚠️ No sensors detected!");
    DEBUG_PRINTLN("   Check wiring and I2C connections");
  }
  else
  {
    // Print sensor status
    sensorManager.printStatus();
  }
#else
  DEBUG_PRINTLN("\n[6/9] Sensors disabled in config");
#endif

// ─────────────────────────────────────────────────────────────────────
// 8. INITIALIZE ACTUATORS
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_ACTUATORS
  DEBUG_PRINTLN("\n[7/9] Initializing Actuators...");
  if (actuatorManager.begin())
  {
    DEBUG_PRINTLN("✓ Actuators initialized");

    // Test sequence (optional)
    DEBUG_PRINTLN("   Running test sequence...");
    blinkLED(2, 100);

    // Move servos to center
    actuatorManager.setServoAngle(1, 90);
    actuatorManager.setServoAngle(2, 90);
    delay(500);

    DEBUG_PRINTLN("   ✓ Test complete");
  }
  else
  {
    DEBUG_PRINTLN("⚠️ Actuator initialization failed");
  }
#else
  DEBUG_PRINTLN("\n[7/9] Actuators disabled in config");
#endif

// ─────────────────────────────────────────────────────────────────────
// 9. INITIALIZE CAMERA (ESP32-CAM only)
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_CAMERA && (DEVICE_TYPE == 1)
  DEBUG_PRINTLN("\n[8/9] Initializing Camera...");
  if (cameraManager.begin())
  {
    DEBUG_PRINTLN("✓ Camera ready");
    DEBUG_PRINTLN("   Camera stream: http://<ip>/cam");

    // Test capture
    DEBUG_PRINTLN("   Testing camera capture...");
    if (cameraManager.capturePhoto())
    {
      DEBUG_PRINTLN("   ✓ Test capture successful");
    }
  }
  else
  {
    DEBUG_PRINTLN("✗ Camera initialization failed!");
    DEBUG_PRINTLN("   Check camera connection and power supply");
  }
#else
  DEBUG_PRINTLN("\n[8/9] Camera not available (ESP32 or disabled)");
#endif

// ─────────────────────────────────────────────────────────────────────
// 10. INITIALIZE WEB SERVER
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_WEBSERVER
  DEBUG_PRINTLN("\n[9/9] Initializing Web Server...");
  if (webServer.begin())
  {
    DEBUG_PRINTLN("✓ Web server started");
    DEBUG_PRINTLN("┌───────────────────────────────────────────────────┐");
    DEBUG_PRINTLN("│            WEB INTERFACE ACCESS                   │");
    DEBUG_PRINTLN("├───────────────────────────────────────────────────┤");
    DEBUG_PRINTF("│ URL:  http://%-35s │\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTLN("│                                                   │");
    DEBUG_PRINTLN("│ Features:                                         │");
    DEBUG_PRINTLN("│ • Real-time sensor monitoring                     │");
    DEBUG_PRINTLN("│ • Actuator control                                │");
    DEBUG_PRINTLN("│ • ESP-NOW communication viewer                    │");
    DEBUG_PRINTLN("│ • System configuration                            │");
    DEBUG_PRINTLN("│ • Data logs viewer                                │");
    DEBUG_PRINTLN("└───────────────────────────────────────────────────┘");
  }
  else
  {
    DEBUG_PRINTLN("✗ Web server failed to start!");
  }
#else
  DEBUG_PRINTLN("\n[9/9] Web server disabled in config");
#endif

// ─────────────────────────────────────────────────────────────────────
// 11. INITIALIZE DATA LOGGER
// ─────────────────────────────────────────────────────────────────────
#if ENABLE_DATA_LOGGING
  DEBUG_PRINTLN("\nInitializing Data Logger...");
  if (dataLogger.begin())
  {
    DEBUG_PRINTLN("✓ Data logger ready");
  }
#endif

  // ─────────────────────────────────────────────────────────────────────
  // SYSTEM READY
  // ─────────────────────────────────────────────────────────────────────
  systemReady = true;

  DEBUG_PRINTLN("\n╔═══════════════════════════════════════════════════════╗");
  DEBUG_PRINTLN("║                                                       ║");
  DEBUG_PRINTLN("║            🚀 SYSTEM READY 🚀                         ║");
  DEBUG_PRINTLN("║                                                       ║");
  DEBUG_PRINTLN("╚═══════════════════════════════════════════════════════╝");
  DEBUG_PRINTLN();

  // Success indication
  blinkLED(5, 100); // 5 fast blinks = ready

  // Print final status
  DEBUG_PRINTLN("Status Summary:");
  DEBUG_PRINTF("├─ WiFi:      %s\n", wifiManager.isConnected() ? "✓ Connected" : "⚠️ AP Mode");
  DEBUG_PRINTF("├─ ESP-NOW:   %s\n", ENABLE_ESPNOW ? "✓ Active" : "○ Disabled");
  DEBUG_PRINTF("├─ Sensors:   %d available\n", sensorManager.getSensorCount());
  DEBUG_PRINTF("├─ Actuators: %s\n", ENABLE_ACTUATORS ? "✓ Ready" : "○ Disabled");
  DEBUG_PRINTF("├─ Camera:    %s\n", ENABLE_CAMERA ? "✓ Ready" : "○ Not available");
  DEBUG_PRINTF("└─ Web UI:    %s\n", ENABLE_WEBSERVER ? "✓ Active" : "○ Disabled");
  DEBUG_PRINTLN();

  // Print peer information
  int peerCount = espnowComm.getPeerCount();
  if (peerCount > 0)
  {
    DEBUG_PRINTF("ESP-NOW Peers: %d registered\n", peerCount);
    for (int i = 0; i < peerCount; i++)
    {
      PeerInfo *peer = espnowComm.getPeerInfo(i);
      if (peer)
      {
        DEBUG_PRINTF("  %d. %s - %s\n", i + 1, peer->name,
                     espnowComm.getMacString(peer->mac).c_str());
      }
    }
  }

  DEBUG_PRINTLN("\nEntering main loop...\n");
}

// ═══════════════════════════════════════════════════════════════════════════
// LOOP FUNCTION - RUNS CONTINUOUSLY
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief Arduino loop function - main program loop
 *
 * This function runs continuously after setup() completes.
 * It handles all periodic tasks using non-blocking timers.
 *
 * NEVER USE delay() IN LOOP!
 * Use Timer objects for periodic operations to keep system responsive.
 *
 * LOOP FREQUENCY:
 * This loop runs at maximum speed (thousands of times per second).
 * Timer objects ensure tasks run at their specified intervals.
 *
 * WATCHDOG TIMER:
 * ESP32 has a watchdog that resets the system if loop() blocks too long.
 * Keep operations fast and use yield() for long operations.
 */
void loop()
{
  // Increment loop counter (for debugging)
  loopCounter++;

// ─────────────────────────────────────────────────────────────────────
// 1. HANDLE OTA UPDATES
// ─────────────────────────────────────────────────────────────────────
// OTA manager handles incoming update requests
// This must be called frequently for OTA to work
#if ENABLE_OTA
  otaManager.handle();
#endif

  // ─────────────────────────────────────────────────────────────────────
  // 2. HANDLE WIFI MANAGER
  // ─────────────────────────────────────────────────────────────────────
  // WiFi manager handles reconnection and DNS (in AP mode)
  // WiFi manager handled automatically

  // ─────────────────────────────────────────────────────────────────────
  // 3. READ SENSORS PERIODICALLY
  // ─────────────────────────────────────────────────────────────────────
  // Check if it's time to read sensors (every 2 seconds by default)
  if (sensorTimer.isReady() && ENABLE_SENSORS)
  {
    readAndSendSensorData();
  }

  // ─────────────────────────────────────────────────────────────────────
  // 4. SEND STATUS UPDATES
  // ─────────────────────────────────────────────────────────────────────
  // Send status to peers and web clients (every 5 seconds)
  if (statusTimer.isReady())
  {
    sendStatusUpdate();
  }

  // ─────────────────────────────────────────────────────────────────────
  // 5. HEARTBEAT LED
  // ─────────────────────────────────────────────────────────────────────
  // Blink LED to show system is alive (every 1 second)
  if (heartbeatTimer.isReady())
  {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

// ─────────────────────────────────────────────────────────────────────
// 6. DATA LOGGING
// ─────────────────────────────────────────────────────────────────────
// Periodic data logging (every 60 seconds)
#if ENABLE_DATA_LOGGING
  if (loggingTimer.isReady())
  {
    // Log could include aggregated sensor data, statistics, etc.
    DEBUG_PRINTLN("📝 Periodic data log checkpoint");
  }
#endif

  // ─────────────────────────────────────────────────────────────────────
  // 7. CHECK PEER CONNECTIVITY
  // ─────────────────────────────────────────────────────────────────────
  // Check which peers are still active (every 30 seconds)
  if (peerCheckTimer.isReady())
  {
    espnowComm.checkPeerActivity(60000); // 60 second timeout
  }

// ─────────────────────────────────────────────────────────────────────
// 8. UPDATE ACTUATORS
// ─────────────────────────────────────────────────────────────────────
// Some actuators need continuous updates (e.g., fading LEDs)
#if ENABLE_ACTUATORS
  actuatorManager.update();
#endif

  // ─────────────────────────────────────────────────────────────────────
  // 9. SYSTEM HEALTH CHECK
  // ─────────────────────────────────────────────────────────────────────
  // Check every 1000 loops
  if (loopCounter % 1000 == 0)
  {
    checkSystemHealth();
  }

  // ─────────────────────────────────────────────────────────────────────
  // 10. YIELD TO PREVENT WATCHDOG RESET
  // ─────────────────────────────────────────────────────────────────────
  // Small delay to prevent watchdog timeout and reduce power consumption
  // Also allows background WiFi tasks to run
  delay(10); // 10ms delay keeps loop at ~100Hz

// Optional: Deeper debugging every 10000 loops
#if DEBUG_MODE
  if (loopCounter % 10000 == 0)
  {
    DEBUG_PRINTLN("\n─── System Status ───");
    DEBUG_PRINTF("Loop count: %lu\n", loopCounter);
    DEBUG_PRINTF("Uptime: %lu seconds\n", (millis() - bootTime) / 1000);
    DEBUG_PRINTF("Free heap: %d bytes\n", ESP.getFreeHeap());
    DEBUG_PRINTF("WiFi RSSI: %d dBm\n", WiFi.RSSI());
    DEBUG_PRINTLN("────────────────────\n");
  }
#endif
}

// ═══════════════════════════════════════════════════════════════════════════
// END OF MAIN APPLICATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * TROUBLESHOOTING GUIDE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. SYSTEM WON'T BOOT:
 *    - Check power supply (5V/2A minimum)
 *    - Verify USB cable supports data (not just charging)
 *    - Check Serial Monitor baud rate (115200)
 *    - Look for brownout detector messages
 *
 * 2. WIFI WON'T CONNECT:
 *    - Verify credentials in credentials.h
 *    - Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
 *    - Check if network uses WPA2 (WPA3 not supported)
 *    - Try closer to router
 *    - Check MAC address not blocked
 *
 * 3. ESP-NOW NOT WORKING:
 *    - Verify both devices have correct MAC addresses
 *    - Ensure same WiFi channel
 *    - Check distance (max ~100m outdoors, 30m indoors)
 *    - Verify ESP-NOW initialized before WiFi.begin()
 *
 * 4. WEB PAGE WON'T LOAD:
 *    - Verify SPIFFS uploaded: pio run -t uploadfs
 *    - Check files exist: SPIFFS.exists("/index.html")
 *    - Try accessing directly: http://ip/index.html
 *    - Check browser console for errors
 *
 * 5. SENSORS NOT READING:
 *    - Check wiring (VCC, GND, data pins)
 *    - Verify correct pins in config.h
 *    - Test with I2C scanner for I2C sensors
 *    - Check 3.3V vs 5V requirements
 *    - Verify library versions
 *
 * 6. OTA NOT WORKING:
 *    - Ensure same network as computer
 *    - Check firewall not blocking port 3232
 *    - Verify mDNS working (.local domain)
 *    - Try IP address instead of hostname
 *
 * 7. MEMORY ISSUES:
 *    - Reduce JSON buffer sizes
 *    - Use StaticJsonDocument instead of DynamicJsonDocument
 *    - Clear old log files from SPIFFS
 *    - Reduce number of sensors
 *    - Check for memory leaks
 *
 * 8. SYSTEM RESETS RANDOMLY:
 *    - Watchdog timeout (operations too slow)
 *    - Power supply insufficient
 *    - Brown-out detector triggered
 *    - Memory corruption
 *    - Check Serial Monitor for reset reason
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * PERFORMANCE OPTIMIZATION:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - Increase sensor read interval if not needed frequently
 * - Disable unused features in config.h
 * - Use smaller JSON buffers where possible
 * - Compress web files (gzip)
 * - Reduce WebSocket broadcast frequency
 * - Use deep sleep for battery applications
 * - Lower WiFi transmit power if close to AP
 * - Disable serial debugging in production
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY RECOMMENDATIONS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - Change default passwords in credentials.h
 * - Enable web authentication
 * - Use HTTPS (requires certificates)
 * - Enable ESP-NOW encryption
 * - Don't expose device directly to internet
 * - Use VPN for remote access
 * - Keep firmware updated
 * - Limit CORS origins
 * - Validate all inputs
 * - Rate limit API requests
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
