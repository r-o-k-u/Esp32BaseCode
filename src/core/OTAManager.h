/**
 * ═══════════════════════════════════════════════════════════════════════════
 * OTA MANAGER - OVER-THE-AIR UPDATE SYSTEM
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file OTAManager.h
 * @brief Over-The-Air (OTA) firmware update manager for ESP32
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * WHAT IS OTA?
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * OTA (Over-The-Air) updates allow you to upload new firmware to ESP32
 * wirelessly over WiFi, without needing a USB connection.
 *
 * BENEFITS:
 * - Update devices remotely (no physical access needed)
 * - Deploy bug fixes quickly
 * - Add new features to deployed devices
 * - Update multiple devices on same network
 * - Ideal for devices in hard-to-reach locations
 *
 * HOW IT WORKS:
 * 1. ESP32 runs OTA service on network port 3232
 * 2. You compile new firmware on your computer
 * 3. Upload tool connects to ESP32 over WiFi
 * 4. New firmware is transmitted and written to flash
 * 5. ESP32 automatically reboots with new firmware
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * USAGE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * In your sketch:
 * @code
 * OTAManager otaManager;
 *
 * void setup() {
 *     WiFi.begin(ssid, password);
 *     otaManager.begin("MyESP32", "password123");
 * }
 *
 * void loop() {
 *     otaManager.handle();  // Must be called frequently!
 * }
 * @endcode
 *
 * To upload via OTA:
 * Arduino IDE: Tools -> Port -> <hostname> or <ip address>
 * PlatformIO: pio run -t upload --upload-port <hostname>.local
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SECURITY CONSIDERATIONS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - ALWAYS set a strong OTA password
 * - Only allow OTA on trusted networks
 * - Consider disabling OTA in production
 * - Use mDNS (.local) instead of IP when possible
 * - Monitor OTA attempts (success and failures)
 * - Implement rollback mechanism for failed updates
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include "../config.h"

/**
 * @brief OTA update states
 */
enum OTAManagerState
{
    OTA_STATE_IDLE = 0,    ///< No update in progress
    OTA_STATE_STARTING,    ///< Update starting
    OTA_STATE_IN_PROGRESS, ///< Update in progress
    OTA_STATE_COMPLETED,   ///< Update completed successfully
    OTA_STATE_ERROR        ///< Update failed
};

/**
 * @brief OTA Manager Class
 *
 * Handles all aspects of Over-The-Air firmware updates including:
 * - mDNS service advertisement
 * - Authentication
 * - Progress tracking
 * - Error handling
 * - Callbacks for update events
 *
 * This class wraps the ArduinoOTA library with enhanced features:
 * - Progress LED indication
 * - Status callbacks
 * - Better error reporting
 * - Update statistics
 * - Automatic reboot handling
 */
class OTAManager
{
private:
    // Configuration
    String hostname; ///< mDNS hostname (.local)
    String password; ///< OTA authentication password
    uint16_t port;   ///< OTA port (default 3232)

    // Status
    bool initialized;             ///< Initialization status
    OTAManagerState currentState; ///< Current OTA state
    uint32_t updateStartTime;     ///< Update start timestamp
    uint32_t lastProgress;        ///< Last progress value
    uint32_t totalSize;           ///< Total update size (bytes)
    uint32_t receivedSize;        ///< Received data size (bytes)

    // Statistics
    uint32_t totalUpdates;   ///< Total successful updates
    uint32_t failedUpdates;  ///< Total failed updates
    uint32_t lastUpdateTime; ///< Timestamp of last update

    // LED indication
    int ledPin;  ///< LED pin for status indication
    bool useLED; ///< Whether to use LED indication

    /**
     * @brief Setup OTA callbacks
     *
     * Configures ArduinoOTA library callbacks for:
     * - Start event
     * - Progress updates
     * - End event
     * - Error handling
     */
    void setupCallbacks();

    /**
     * @brief LED blink for OTA indication
     * @param times Number of blinks
     * @param delayMs Delay between blinks
     */
    void blinkLED(int times, int delayMs);

public:
    /**
     * @brief Constructor
     */
    OTAManager();

    /**
     * @brief Destructor
     */
    ~OTAManager();

    /**
     * @brief Initialize OTA service
     * @param host Hostname for mDNS (will be <host>.local)
     * @param pass Password for OTA authentication
     * @param otaPort OTA port (default 3232)
     * @return true if initialization successful
     *
     * INITIALIZATION PROCESS:
     * 1. Verify WiFi is connected
     * 2. Start mDNS responder
     * 3. Configure OTA hostname
     * 4. Set authentication password
     * 5. Setup callbacks
     * 6. Start OTA service
     * 7. Advertise service on network
     *
     * EXAMPLE:
     * @code
     * OTAManager ota;
     * if (ota.begin("ESP32_Kitchen", "myPassword123")) {
     *     Serial.println("OTA ready!");
     *     Serial.println("Upload at: ESP32_Kitchen.local");
     * }
     * @endcode
     *
     * TROUBLESHOOTING:
     * - Ensure WiFi is connected before calling begin()
     * - Check hostname is unique on network
     * - Verify firewall allows port 3232
     * - Test mDNS is working (.local domains resolve)
     */
    bool begin(const char *host, const char *pass, uint16_t otaPort = OTA_PORT);

    /**
     * @brief Handle OTA updates
     *
     * ⚠️ CRITICAL: Must be called frequently in main loop!
     *
     * This function processes incoming OTA requests and handles
     * the update process. If not called regularly, OTA will not work.
     *
     * CALL FREQUENCY:
     * - Call every loop iteration
     * - Don't use long delay() calls in loop
     * - Keep loop execution time under 100ms
     *
     * EXAMPLE:
     * @code
     * void loop() {
     *     otaManager.handle();  // Process OTA
     *     // ... rest of your code
     * }
     * @endcode
     */
    void handle();

    /**
     * @brief Enable/disable LED indication
     * @param pin LED GPIO pin
     * @param enable true to enable LED indication
     *
     * LED BLINK PATTERNS:
     * - Slow blink: OTA starting
     * - Fast blink: Receiving data
     * - Solid on: Verifying/writing
     * - 3 blinks: Success
     * - 10 fast blinks: Error
     *
     * EXAMPLE:
     * @code
     * otaManager.setLEDPin(LED_BUILTIN, true);
     * @endcode
     */
    void setLEDPin(int pin, bool enable = true);

    /**
     * @brief Get current OTA state
     * @return Current state (IDLE, IN_PROGRESS, etc.)
     */
    OTAManagerState getState() { return currentState; }

    /**
     * @brief Check if update is in progress
     * @return true if update is currently happening
     */
    bool isUpdating() { return currentState == OTA_STATE_IN_PROGRESS; }

    /**
     * @brief Get update progress percentage
     * @return Progress (0-100%)
     *
     * Returns 0 if no update in progress
     *
     * USAGE:
     * @code
     * if (ota.isUpdating()) {
     *     Serial.printf("Progress: %d%%\n", ota.getProgress());
     * }
     * @endcode
     */
    uint8_t getProgress();

    /**
     * @brief Get hostname
     * @return mDNS hostname
     */
    String getHostname() { return hostname; }

    /**
     * @brief Get OTA port
     * @return Port number
     */
    uint16_t getPort() { return port; }

    /**
     * @brief Check if OTA is initialized
     * @return true if ready to receive updates
     */
    bool isInitialized() { return initialized; }

    /**
     * @brief Get total successful updates
     * @return Update count
     */
    uint32_t getTotalUpdates() { return totalUpdates; }

    /**
     * @brief Get total failed updates
     * @return Failed update count
     */
    uint32_t getFailedUpdates() { return failedUpdates; }

    /**
     * @brief Get time of last update
     * @return Timestamp (millis)
     */
    uint32_t getLastUpdateTime() { return lastUpdateTime; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Print OTA status information
     *
     * Prints:
     * - Hostname and port
     * - Current state
     * - Update statistics
     * - Network information
     */
    void printStatus();

    /**
     * @brief Get status as string
     * @return Human-readable status
     */
    String getStatusString();

    // ═══════════════════════════════════════════════════════════════════════
    // CALLBACK FUNCTION TYPES
    // ═══════════════════════════════════════════════════════════════════════

    /**
     * Callback function types for custom handling
     *
     * These allow you to perform custom actions during OTA:
     * - Disable sensors before update
     * - Save state to flash
     * - Send notifications
     * - Update display
     *
     * EXAMPLE:
     * @code
     * void onOTAStart() {
     *     Serial.println("OTA starting, disabling sensors...");
     *     sensors.disable();
     *     display.showMessage("UPDATING...");
     * }
     *
     * void onOTAProgress(uint8_t progress) {
     *     display.showProgress(progress);
     * }
     *
     * void onOTAEnd() {
     *     Serial.println("OTA complete, rebooting...");
     * }
     *
     * void onOTAError(const char* error) {
     *     Serial.printf("OTA ERROR: %s\n", error);
     *     display.showError(error);
     * }
     *
     * void setup() {
     *     otaManager.setOnStart(onOTAStart);
     *     otaManager.setOnProgress(onOTAProgress);
     *     otaManager.setOnEnd(onOTAEnd);
     *     otaManager.setOnError(onOTAError);
     * }
     * @endcode
     */

    typedef void (*OTAStartCallback)();
    typedef void (*OTAProgressCallback)(uint8_t progress);
    typedef void (*OTAEndCallback)();
    typedef void (*OTAErrorCallback)(const char *error);

    /**
     * @brief Set callback for OTA start
     * @param callback Function to call when update starts
     */
    void setOnStart(OTAStartCallback callback);

    /**
     * @brief Set callback for OTA progress
     * @param callback Function to call with progress updates
     */
    void setOnProgress(OTAProgressCallback callback);

    /**
     * @brief Set callback for OTA end
     * @param callback Function to call when update completes
     */
    void setOnEnd(OTAEndCallback callback);

    /**
     * @brief Set callback for OTA error
     * @param callback Function to call on errors
     */
    void setOnError(OTAErrorCallback callback);

private:
    // User callbacks
    OTAStartCallback userStartCallback;
    OTAProgressCallback userProgressCallback;
    OTAEndCallback userEndCallback;
    OTAErrorCallback userErrorCallback;
};

#endif // OTA_MANAGER_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * OTA UPDATE GUIDE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * PREREQUISITES:
 * - ESP32 must be on same network as your computer
 * - OTA service must be running (otaManager.begin() called)
 * - Know the hostname or IP address of ESP32
 * - Have OTA password
 *
 * UPLOAD METHODS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. ARDUINO IDE:
 *    - Tools -> Port -> Select network port (hostname or IP)
 *    - Click Upload button
 *    - Enter OTA password if prompted
 *    - Wait for upload to complete
 *
 * 2. PLATFORMIO (CLI):
 *    pio run -t upload --upload-port ESP32_Kitchen.local
 *    Or:
 *    pio run -t upload --upload-port 192.168.1.100
 *
 * 3. PLATFORMIO (platformio.ini):
 *    Add to platformio.ini:
 *    upload_protocol = espota
 *    upload_port = ESP32_Kitchen.local
 *    upload_flags = --auth=password123
 *
 * 4. PYTHON SCRIPT (espota.py):
 *    python espota.py -i 192.168.1.100 -p 3232 -a password123 -f firmware.bin
 *
 * TROUBLESHOOTING:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * PORT NOT FOUND:
 * - Check ESP32 is powered on and connected to WiFi
 * - Verify ESP32 is on same network as computer
 * - Try using IP address instead of hostname
 * - Check mDNS is working (ping hostname.local)
 *
 * CONNECTION TIMEOUT:
 * - Check firewall not blocking port 3232
 * - Verify WiFi signal strength is good
 * - Try moving ESP32 closer to router
 * - Check ESP32 isn't under heavy load
 *
 * AUTHENTICATION FAILED:
 * - Verify OTA password is correct
 * - Check password doesn't have special characters
 * - Re-upload via USB with correct password
 *
 * UPDATE FAILED:
 * - Check ESP32 has enough flash space
 * - Verify firmware is for correct chip (ESP32 vs ESP32-S2 vs ESP32-C3)
 * - Check power supply is stable (weak power causes failures)
 * - Try uploading smaller firmware
 *
 * DEVICE WON'T BOOT AFTER UPDATE:
 * - Upload known-good firmware via USB
 * - Check Serial Monitor for error messages
 * - May need to erase flash and re-upload
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * BEST PRACTICES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * TESTING:
 * - Always test new firmware via USB first
 * - Have backup device with working firmware
 * - Test OTA on development board before production
 * - Keep USB access available for recovery
 *
 * DEPLOYMENT:
 * - Update one device at a time
 * - Monitor first device before updating others
 * - Keep changelog of firmware versions
 * - Have rollback plan if update fails
 *
 * SECURITY:
 * - Use strong OTA passwords
 * - Change default passwords
 * - Consider disabling OTA after deployment
 * - Use HTTPS for downloading firmware
 * - Verify firmware signatures (if implementing secure boot)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
