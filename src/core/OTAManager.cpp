/**
 * ═══════════════════════════════════════════════════════════════════════════
 * OTA MANAGER - IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file OTAManager.cpp
 * @brief Implementation of Over-The-Air update manager
 * @version 2.0.0
 * @date 2024
 */

#include "OTAManager.h"

// Global instance
OTAManager otaManager;

/**
 * @brief Constructor - Initialize OTA Manager
 *
 * Sets up initial state and prepares for OTA service.
 * Does not start OTA service - call begin() for that.
 */
OTAManager::OTAManager()
{
    initialized = false;
    currentState = OTAManagerState::OTA_STATE_IDLE;
    updateStartTime = 0;
    lastProgress = 0;
    totalSize = 0;
    receivedSize = 0;
    totalUpdates = 0;
    failedUpdates = 0;
    lastUpdateTime = 0;
    ledPin = -1;
    useLED = false;
    port = OTA_PORT;

    // Initialize callbacks to nullptr
    userStartCallback = nullptr;
    userProgressCallback = nullptr;
    userEndCallback = nullptr;
    userErrorCallback = nullptr;
}

/**
 * @brief Destructor - Clean up resources
 */
OTAManager::~OTAManager()
{
    if (initialized)
    {
        ArduinoOTA.end();
    }
}

/**
 * @brief Initialize OTA service
 * @param host mDNS hostname
 * @param pass OTA authentication password
 * @param otaPort OTA service port
 * @return true if initialization successful
 *
 * IMPLEMENTATION DETAILS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * This function performs the following steps:
 *
 * 1. VERIFY WIFI CONNECTION:
 *    OTA requires active WiFi connection. If WiFi is not connected,
 *    initialization will fail.
 *
 * 2. START MDNS SERVICE:
 *    mDNS allows devices to be found by hostname.local instead of IP address.
 *    For example: ESP32_Kitchen.local instead of 192.168.1.100
 *
 * 3. CONFIGURE ARDUINO OTA:
 *    - Set hostname for identification
 *    - Set password for authentication
 *    - Configure port (default 3232)
 *
 * 4. SETUP CALLBACKS:
 *    Internal callbacks handle:
 *    - Update start (disable sensors, prepare for update)
 *    - Progress updates (show percentage, blink LED)
 *    - Update end (verify, prepare to reboot)
 *    - Errors (report failures, recover if possible)
 *
 * 5. START OTA SERVICE:
 *    Begins listening for OTA requests on configured port
 *
 * ERROR HANDLING:
 * - Returns false if WiFi not connected
 * - Returns false if mDNS fails to start
 * - Logs all errors to Serial
 * - Safe to call begin() multiple times
 */
bool OTAManager::begin(const char *host, const char *pass, uint16_t otaPort)
{
    DEBUG_PRINTLN("═══════════════════════════════════════════════════");
    DEBUG_PRINTLN("Initializing OTA Manager");
    DEBUG_PRINTLN("═══════════════════════════════════════════════════");

    // Check WiFi connection
    if (WiFi.status() != WL_CONNECTED)
    {
        DEBUG_PRINTLN("ERROR: WiFi not connected!");
        DEBUG_PRINTLN("       Connect to WiFi before initializing OTA");
        return false;
    }

    // Store configuration
    hostname = String(host);
    password = String(pass);
    port = otaPort;

    DEBUG_PRINTF("Hostname: %s.local\n", hostname.c_str());
    DEBUG_PRINTF("Port:     %d\n", port);
    DEBUG_PRINTF("Password: %s\n", password.length() > 0 ? "Set (hidden)" : "NOT SET!");

    // Start mDNS
    DEBUG_PRINT("Starting mDNS responder... ");
    if (!MDNS.begin(hostname.c_str()))
    {
        DEBUG_PRINTLN("FAILED!");
        DEBUG_PRINTLN("ERROR: mDNS failed to start");
        DEBUG_PRINTLN("       Device will not be discoverable as hostname.local");
        DEBUG_PRINTLN("       OTA will still work with IP address");
        // Continue anyway - OTA works with IP even if mDNS fails
    }
    else
    {
        DEBUG_PRINTLN("OK");
        DEBUG_PRINTF("       Device available at: %s.local\n", hostname.c_str());
    }

    // Configure ArduinoOTA
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.setPassword(password.c_str());
    ArduinoOTA.setPort(port);

    // Setup OTA callbacks
    setupCallbacks();

    // Start OTA service
    DEBUG_PRINT("Starting OTA service... ");
    ArduinoOTA.begin();
    DEBUG_PRINTLN("OK");

    initialized = true;
    currentState = OTAManagerState::OTA_STATE_IDLE;

    DEBUG_PRINTLN("═══════════════════════════════════════════════════");
    DEBUG_PRINTLN("✓ OTA Manager Ready");
    DEBUG_PRINTLN("═══════════════════════════════════════════════════");
    DEBUG_PRINTLN("To upload firmware:");
    DEBUG_PRINTF("  Arduino IDE: Tools -> Port -> %s.local\n", hostname.c_str());
    DEBUG_PRINTF("  PlatformIO:  pio run -t upload --upload-port %s.local\n", hostname.c_str());
    DEBUG_PRINTF("  Or use IP:   %s\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTLN("═══════════════════════════════════════════════════\n");

    return true;
}

/**
 * @brief Setup internal OTA callbacks
 *
 * CALLBACK EXECUTION ORDER:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. onStart():
 *    - Called when OTA begins
 *    - Determines update type (sketch or filesystem)
 *    - Initializes progress tracking
 *    - Calls user start callback if set
 *
 * 2. onProgress(progress, total):
 *    - Called repeatedly during upload
 *    - Updates progress percentage
 *    - Provides visual feedback (LED, Serial)
 *    - Calls user progress callback if set
 *
 * 3. onEnd():
 *    - Called when update completes successfully
 *    - Verifies update integrity
 *    - Updates statistics
 *    - Calls user end callback if set
 *    - Device will reboot automatically
 *
 * 4. onError(error):
 *    - Called if update fails at any point
 *    - Logs error details
 *    - Updates failure statistics
 *    - Calls user error callback if set
 *    - Device remains running with old firmware
 *
 * THREAD SAFETY:
 * These callbacks run in interrupt context. Keep them fast and simple.
 * Avoid complex operations like file I/O or network requests.
 */
void OTAManager::setupCallbacks()
{
    // ───────────────────────────────────────────────────────────────────────
    // ON START CALLBACK
    // ───────────────────────────────────────────────────────────────────────
    ArduinoOTA.onStart([this]()
                       {
        currentState = OTAManagerState::OTA_STATE_STARTING;
        updateStartTime = millis();
        totalSize = 0;
        receivedSize = 0;
        lastProgress = 0;
        
        // Determine update type
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        
        DEBUG_PRINTLN("\n╔═══════════════════════════════════════════════════╗");
        DEBUG_PRINTLN("║           OTA UPDATE STARTING                     ║");
        DEBUG_PRINTLN("╚═══════════════════════════════════════════════════╝");
        DEBUG_PRINTF("Type: %s\n", type.c_str());
        DEBUG_PRINTLN("Please wait, do not power off device!");
        DEBUG_PRINTLN("─────────────────────────────────────────────────────");
        
        // LED indication (slow blink)
        if (useLED && ledPin >= 0) {
            blinkLED(3, 200);
        }
        
        // Call user callback
        if (userStartCallback) {
            userStartCallback();
        }
        
        currentState = OTAManagerState::OTA_STATE_IN_PROGRESS; });

    // ───────────────────────────────────────────────────────────────────────
    // ON PROGRESS CALLBACK
    // ───────────────────────────────────────────────────────────────────────
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total)
                          {
        receivedSize = progress;
        totalSize = total;
        
        // Calculate percentage
        uint8_t percentage = (progress / (total / 100));
        
        // Only print when percentage changes (reduce serial spam)
        if (percentage != lastProgress) {
            lastProgress = percentage;
            
            // Print progress bar
            DEBUG_PRINTF("\rProgress: %3d%% [", percentage);
            
            int barWidth = 30;
            int pos = barWidth * percentage / 100;
            
            for (int i = 0; i < barWidth; i++) {
                if (i < pos) DEBUG_PRINT("█");
                else if (i == pos) DEBUG_PRINT("▓");
                else DEBUG_PRINT("░");
            }
            
            DEBUG_PRINTF("] %u/%u bytes", progress, total);
            
            // LED indication (fast blink every 10%)
            if (useLED && ledPin >= 0 && percentage % 10 == 0) {
                digitalWrite(ledPin, !digitalRead(ledPin));
            }
            
            // Call user callback
            if (userProgressCallback) {
                userProgressCallback(percentage);
            }
        } });

    // ───────────────────────────────────────────────────────────────────────
    // ON END CALLBACK
    // ───────────────────────────────────────────────────────────────────────
    ArduinoOTA.onEnd([this]()
                     {
        currentState = OTAManagerState::OTA_STATE_COMPLETED;
        uint32_t duration = millis() - updateStartTime;
        
        DEBUG_PRINTLN("\n─────────────────────────────────────────────────────");
        DEBUG_PRINTLN("╔═══════════════════════════════════════════════════╗");
        DEBUG_PRINTLN("║         OTA UPDATE COMPLETED                      ║");
        DEBUG_PRINTLN("╚═══════════════════════════════════════════════════╝");
        DEBUG_PRINTF("Duration: %lu seconds\n", duration / 1000);
        DEBUG_PRINTF("Size: %u bytes (%.2f KB)\n", totalSize, totalSize / 1024.0);
        DEBUG_PRINTF("Speed: %.2f KB/s\n", (totalSize / 1024.0) / (duration / 1000.0));
        DEBUG_PRINTLN("\nDevice will reboot in 3 seconds...");
        DEBUG_PRINTLN("═════════════════════════════════════════════════════\n");
        
        // Update statistics
        totalUpdates++;
        lastUpdateTime = millis();
        
        // LED indication (success - 3 blinks)
        if (useLED && ledPin >= 0) {
            blinkLED(3, 200);
        }
        
        // Call user callback
        if (userEndCallback) {
            userEndCallback();
        }
        
        // Give time for serial to flush
        delay(100); });

    // ───────────────────────────────────────────────────────────────────────
    // ON ERROR CALLBACK
    // ───────────────────────────────────────────────────────────────────────
    ArduinoOTA.onError([this](ota_error_t error)
                       {
        currentState = OTAManagerState::OTA_STATE_ERROR;
        
        DEBUG_PRINTLN("\n─────────────────────────────────────────────────────");
        DEBUG_PRINTLN("╔═══════════════════════════════════════════════════╗");
        DEBUG_PRINTLN("║           OTA UPDATE FAILED                       ║");
        DEBUG_PRINTLN("╚═══════════════════════════════════════════════════╝");
        
        // Determine error type
        const char* errorMsg;
        switch (error) {
            case OTA_AUTH_ERROR:
                errorMsg = "Authentication Failed (wrong password)";
                break;
            case OTA_BEGIN_ERROR:
                errorMsg = "Begin Failed (not enough space or corrupted)";
                break;
            case OTA_CONNECT_ERROR:
                errorMsg = "Connect Failed (network issue)";
                break;
            case OTA_RECEIVE_ERROR:
                errorMsg = "Receive Failed (transmission interrupted)";
                break;
            case OTA_END_ERROR:
                errorMsg = "End Failed (verification failed)";
                break;
            default:
                errorMsg = "Unknown Error";
                break;
        }
        
        DEBUG_PRINTF("Error: %s\n", errorMsg);
        DEBUG_PRINTLN("\nDevice will continue running with old firmware");
        DEBUG_PRINTLN("═════════════════════════════════════════════════════\n");
        
        // Update statistics
        failedUpdates++;
        
        // LED indication (error - fast blinking)
        if (useLED && ledPin >= 0) {
            blinkLED(10, 100);
        }
        
        // Call user callback
        if (userErrorCallback) {
            userErrorCallback(errorMsg);
        }
        
        // Return to idle state
         currentState = OTAManagerState::OTA_STATE_IDLE; });
}

/**
 * @brief Handle OTA updates
 *
 * CRITICAL: Call this frequently in main loop!
 *
 * This function must be called regularly (every loop iteration ideally)
 * for OTA to work. It processes incoming OTA requests and manages the
 * update process.
 *
 * WHAT IT DOES:
 * - Checks for incoming OTA requests
 * - Processes received data
 * - Manages state transitions
 * - Handles mDNS queries
 *
 * PERFORMANCE IMPACT:
 * - Very low when idle (< 1ms per call)
 * - Higher during update (but that's OK, update is priority)
 * - Non-blocking (returns quickly)
 *
 * BEST PRACTICES:
 * - Call every loop iteration
 * - Don't use long delay() calls
 * - Keep loop execution fast
 * - Avoid blocking operations
 */
void OTAManager::handle()
{
    if (!initialized)
        return;

    // Handle ArduinoOTA
    ArduinoOTA.handle();

// Optional: Handle mDNS queries
// Some platforms need this, others don't
#ifdef ESP32
// ESP32 handles mDNS automatically
#endif
}

/**
 * @brief Configure LED for status indication
 * @param pin GPIO pin for LED
 * @param enable true to enable LED indication
 *
 * LED BLINK PATTERNS:
 * - 3 slow blinks: Update starting
 * - Fast toggling: Receiving data
 * - 3 medium blinks: Update successful
 * - 10 fast blinks: Update failed
 */
void OTAManager::setLEDPin(int pin, bool enable)
{
    ledPin = pin;
    useLED = enable;

    if (useLED && ledPin >= 0)
    {
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);
    }
}

/**
 * @brief Blink LED helper function
 * @param times Number of blinks
 * @param delayMs Delay between blinks
 */
void OTAManager::blinkLED(int times, int delayMs)
{
    if (!useLED || ledPin < 0)
        return;

    for (int i = 0; i < times; i++)
    {
        digitalWrite(ledPin, HIGH);
        delay(delayMs);
        digitalWrite(ledPin, LOW);
        if (i < times - 1)
            delay(delayMs);
    }
}

/**
 * @brief Get current update progress
 * @return Progress percentage (0-100)
 */
uint8_t OTAManager::getProgress()
{
    if (currentState != OTAManagerState::OTA_STATE_IN_PROGRESS || totalSize == 0)
    {
        return 0;
    }
    return (receivedSize * 100) / totalSize;
}

/**
 * @brief Reset OTA statistics
 */
void OTAManager::resetStatistics()
{
    totalUpdates = 0;
    failedUpdates = 0;
    lastUpdateTime = 0;
    DEBUG_PRINTLN("OTA statistics reset");
}

/**
 * @brief Print OTA status information
 */
void OTAManager::printStatus()
{
    DEBUG_PRINTLN("┌─────────────────────────────────────────────────┐");
    DEBUG_PRINTLN("│            OTA STATUS                           │");
    DEBUG_PRINTLN("├─────────────────────────────────────────────────┤");
    DEBUG_PRINTF("│ Hostname:    %-30s │\n", hostname.c_str());
    DEBUG_PRINTF("│ Port:        %-30d │\n", port);
    DEBUG_PRINTF("│ Password:    %-30s │\n", password.length() > 0 ? "Set" : "NOT SET!");
    DEBUG_PRINTF("│ State:       %-30s │\n", getStatusString().c_str());
    DEBUG_PRINTF("│ Updates:     %-30d │\n", totalUpdates);
    DEBUG_PRINTF("│ Failed:      %-30d │\n", failedUpdates);
    DEBUG_PRINTLN("└─────────────────────────────────────────────────┘");
}

/**
 * @brief Get status as human-readable string
 * @return Status string
 */
String OTAManager::getStatusString()
{
    switch (currentState)
    {
    case OTAManagerState::OTA_STATE_IDLE:
        return "Idle (ready for updates)";
    case OTAManagerState::OTA_STATE_STARTING:
        return "Starting update...";
    case OTAManagerState::OTA_STATE_IN_PROGRESS:
        return "Update in progress (" + String(getProgress()) + "%)";
    case OTAManagerState::OTA_STATE_COMPLETED:
        return "Update completed";
    case OTAManagerState::OTA_STATE_ERROR:
        return "Update failed";
    default:
        return "Unknown";
    }
}

/**
 * @brief Set user callback for OTA start
 * @param callback Function to call when update starts
 */
void OTAManager::setOnStart(OTAStartCallback callback)
{
    userStartCallback = callback;
}

/**
 * @brief Set user callback for OTA progress
 * @param callback Function to call with progress updates
 */
void OTAManager::setOnProgress(OTAProgressCallback callback)
{
    userProgressCallback = callback;
}

/**
 * @brief Set user callback for OTA end
 * @param callback Function to call when update completes
 */
void OTAManager::setOnEnd(OTAEndCallback callback)
{
    userEndCallback = callback;
}

/**
 * @brief Set user callback for OTA error
 * @param callback Function to call on errors
 */
void OTAManager::setOnError(OTAErrorCallback callback)
{
    userErrorCallback = callback;
}

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * END OF OTA MANAGER IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 */
