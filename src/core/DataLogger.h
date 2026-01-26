/**
 * ═══════════════════════════════════════════════════════════════════════════
 * DATA LOGGER - SPIFFS-BASED DATA PERSISTENCE SYSTEM
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file DataLogger.h
 * @brief Data logging system with SPIFFS storage for ESP32
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * WHAT IS DATA LOGGING?
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Data logging saves sensor readings, events, and system information to the
 * ESP32's flash storage (SPIFFS filesystem). This allows you to:
 *
 * - Track sensor data over time
 * - Analyze historical trends
 * - Debug system behavior
 * - Create data backups
 * - Export data for analysis
 * - Maintain event logs
 *
 * FEATURES:
 * - Automatic file rotation when size limit reached
 * - Multiple log categories (sensors, errors, events, etc.)
 * - JSON format for easy parsing
 * - CSV export capability
 * - Timestamp support
 * - Log compression (optional)
 * - Automatic cleanup of old logs
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * USAGE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Basic usage:
 * @code
 * DataLogger logger;
 *
 * void setup() {
 *     SPIFFS.begin();
 *     logger.begin();
 * }
 *
 * void loop() {
 *     // Log sensor data
 *     String data = "{\"temp\":25.5,\"humidity\":60}";
 *     logger.logData("sensors", data.c_str());
 *
 *     // Log events
 *     logger.logEvent("Motion detected in zone 1");
 *
 *     // Log errors
 *     logger.logError("Sensor timeout on DHT22");
 * }
 * @endcode
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * FILE STRUCTURE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * /logs/
 *   sensors.log          <- Current sensor data log
 *   sensors_old.log      <- Rotated sensor log
 *   events.log           <- System events
 *   errors.log           <- Error messages
 *   custom.log           <- Custom category logs
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * LOG FORMAT:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Each log entry is a JSON line:
 * {"timestamp":123456,"data":{"temp":25.5,"humidity":60}}
 * {"timestamp":123789,"data":{"temp":26.0,"humidity":58}}
 *
 * This format is:
 * - Easy to parse (JSON)
 * - Human-readable
 * - Machine-processable
 * - Compatible with data analysis tools
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../config.h"

/**
 * @brief Log categories for organizing data
 */
enum LogCategory
{
    LOG_SENSORS = 0, ///< Sensor readings
    LOG_EVENTS,      ///< System events
    LOG_ERRORS,      ///< Error messages
    LOG_ACTUATORS,   ///< Actuator states
    LOG_NETWORK,     ///< Network activity
    LOG_DEBUG,       ///< Debug information
    LOG_CUSTOM       ///< Custom category
};

/**
 * @brief Log entry structure
 */
struct LogEntry
{
    uint32_t timestamp; ///< Entry timestamp (millis)
    String category;    ///< Log category
    String data;        ///< Log data (JSON string)
};

/**
 * @brief Data Logger Class
 *
 * Comprehensive logging system with:
 * - Multiple log categories
 * - Automatic file rotation
 * - Size management
 * - JSON and CSV export
 * - Search and filtering
 * - Statistics tracking
 */
class DataLogger
{
private:
    // Configuration
    bool initialized;     ///< Initialization status
    String logDirectory;  ///< Base log directory
    uint32_t maxLogSize;  ///< Maximum log file size
    bool enableRotation;  ///< Auto-rotate logs when full
    bool enableTimestamp; ///< Include timestamps in logs

    // Statistics
    uint32_t totalWrites;       ///< Total log writes
    uint32_t totalRotations;    ///< Total file rotations
    uint32_t failedWrites;      ///< Failed write attempts
    uint32_t totalBytesWritten; ///< Total bytes written

    // File handles
    File currentFile;       ///< Current open file
    String currentCategory; ///< Current log category

    /**
     * @brief Get log filename for category
     * @param category Log category
     * @return Full file path
     */
    String getLogFilename(const char *category);

    /**
     * @brief Get rotated log filename
     * @param category Log category
     * @return Rotated file path
     */
    String getRotatedFilename(const char *category);

    /**
     * @brief Check if log file needs rotation
     * @param category Log category
     * @return true if rotation needed
     */
    bool needsRotation(const char *category);

    /**
     * @brief Rotate log file
     * @param category Log category
     * @return true if rotation successful
     */
    bool rotateLog(const char *category);

    /**
     * @brief Write entry to file
     * @param filename File path
     * @param data Data to write
     * @return true if write successful
     */
    bool writeToFile(const char *filename, const char *data);

    /**
     * @brief Format log entry as JSON
     * @param data Data string
     * @return Formatted JSON string
     */
    String formatLogEntry(const char *data);

public:
    /**
     * @brief Constructor
     */
    DataLogger();

    /**
     * @brief Destructor
     */
    ~DataLogger();

    /**
     * @brief Initialize data logger
     * @param logDir Base directory for logs (default "/logs")
     * @param maxSize Maximum log file size (default 100KB)
     * @return true if initialization successful
     *
     * INITIALIZATION:
     * 1. Verify SPIFFS is mounted
     * 2. Create log directory if doesn't exist
     * 3. Scan existing logs
     * 4. Setup configuration
     *
     * EXAMPLE:
     * @code
     * DataLogger logger;
     * if (!logger.begin()) {
     *     Serial.println("Logger init failed!");
     * }
     * @endcode
     */
    bool begin(const char *logDir = "/logs", uint32_t maxSize = MAX_LOG_SIZE);

    /**
     * @brief Log data to specified category
     * @param category Log category (e.g., "sensors", "events")
     * @param data Data to log (JSON string recommended)
     * @return true if logged successfully
     *
     * This is the main logging function. Use it to log any data.
     *
     * EXAMPLES:
     * @code
     * // Log sensor data
     * logger.logData("sensors", "{\"temp\":25.5,\"hum\":60}");
     *
     * // Log custom event
     * logger.logData("events", "System reboot");
     *
     * // Log with timestamp (automatic)
     * logger.logData("debug", "Loop iteration: 1000");
     * @endcode
     *
     * FEATURES:
     * - Automatic timestamp addition
     * - JSON formatting
     * - Auto-rotation when file full
     * - Error handling
     */
    bool logData(const char *category, const char *data);

    /**
     * @brief Log sensor data (convenience wrapper)
     * @param jsonData Sensor data as JSON string
     * @return true if logged successfully
     *
     * Shortcut for logging sensor data.
     * Equivalent to: logData("sensors", jsonData)
     */
    bool logSensorData(const char *jsonData);

    /**
     * @brief Log event message
     * @param message Event description
     * @return true if logged successfully
     *
     * Logs system events like:
     * - "WiFi connected"
     * - "OTA update started"
     * - "Sensor calibration complete"
     *
     * EXAMPLE:
     * @code
     * logger.logEvent("Motion detected in hallway");
     * logger.logEvent("Temperature threshold exceeded");
     * @endcode
     */
    bool logEvent(const char *message);

    /**
     * @brief Log error message
     * @param error Error description
     * @return true if logged successfully
     *
     * Logs errors for debugging:
     * - "Sensor read timeout"
     * - "Network connection lost"
     * - "File system full"
     *
     * EXAMPLE:
     * @code
     * if (!sensor.read()) {
     *     logger.logError("DHT22 sensor read failed");
     * }
     * @endcode
     */
    bool logError(const char *error);

    /**
     * @brief Log actuator state change
     * @param actuator Actuator name
     * @param state New state
     * @return true if logged successfully
     *
     * EXAMPLE:
     * @code
     * logger.logActuator("relay1", 1);  // Relay turned on
     * logger.logActuator("servo1", 90); // Servo moved to 90°
     * @endcode
     */
    bool logActuator(const char *actuator, int state);

    /**
     * @brief Read log file contents
     * @param category Log category to read
     * @param maxLines Maximum lines to read (0 = all)
     * @return Log contents as String
     *
     * Reads log file and returns contents.
     * Useful for displaying logs in web interface or serial.
     *
     * EXAMPLE:
     * @code
     * String logs = logger.readLog("sensors", 10);  // Last 10 lines
     * Serial.println(logs);
     * @endcode
     */
    String readLog(const char *category, uint16_t maxLines = 0);

    /**
     * @brief Get log file size
     * @param category Log category
     * @return File size in bytes
     */
    uint32_t getLogSize(const char *category);

    /**
     * @brief Check if log file exists
     * @param category Log category
     * @return true if file exists
     */
    bool logExists(const char *category);

    /**
     * @brief Delete log file
     * @param category Log category to delete
     * @return true if deleted successfully
     *
     * CAUTION: This permanently deletes the log file!
     *
     * EXAMPLE:
     * @code
     * if (logger.getLogSize("debug") > 500000) {
     *     logger.deleteLog("debug");  // Clear large debug log
     * }
     * @endcode
     */
    bool deleteLog(const char *category);

    /**
     * @brief Delete all logs
     * @return true if all deleted successfully
     *
     * CAUTION: This deletes ALL log files!
     * Use for factory reset or cleanup.
     */
    bool deleteAllLogs();

    /**
     * @brief Export log as CSV
     * @param category Log category
     * @param csvFilename Output CSV filename
     * @return true if export successful
     *
     * Converts JSON log to CSV format for analysis in Excel/Sheets.
     *
     * EXAMPLE:
     * @code
     * logger.exportAsCSV("sensors", "/export/sensors.csv");
     * // Can then download via web interface
     * @endcode
     */
    bool exportAsCSV(const char *category, const char *csvFilename);

    /**
     * @brief Get list of all log files
     * @param buffer Buffer to store filenames
     * @param maxFiles Maximum files to list
     * @return Number of files found
     *
     * EXAMPLE:
     * @code
     * String files[10];
     * int count = logger.listLogs(files, 10);
     * for (int i = 0; i < count; i++) {
     *     Serial.println(files[i]);
     * }
     * @endcode
     */
    uint8_t listLogs(String *buffer, uint8_t maxFiles);

    /**
     * @brief Get total storage used by logs
     * @return Total bytes used
     */
    uint32_t getTotalLogSize();

    /**
     * @brief Get available SPIFFS space
     * @return Free bytes
     */
    uint32_t getAvailableSpace();

    /**
     * @brief Enable/disable automatic file rotation
     * @param enable true to enable rotation
     */
    void setRotation(bool enable);

    /**
     * @brief Set maximum log file size
     * @param size Size in bytes
     *
     * When log reaches this size, it will be rotated.
     * Default: 100KB
     * Recommended: 50-200KB depending on available space
     */
    void setMaxLogSize(uint32_t size);

    /**
     * @brief Enable/disable timestamps
     * @param enable true to include timestamps
     *
     * Timestamps use millis() since boot.
     * For real timestamps, sync with NTP server first.
     */
    void setTimestamp(bool enable);

    /**
     * @brief Get total number of writes
     * @return Write count
     */
    uint32_t getTotalWrites() { return totalWrites; }

    /**
     * @brief Get total number of rotations
     * @return Rotation count
     */
    uint32_t getTotalRotations() { return totalRotations; }

    /**
     * @brief Get failed write count
     * @return Failed write count
     */
    uint32_t getFailedWrites() { return failedWrites; }

    /**
     * @brief Get total bytes written
     * @return Bytes written
     */
    uint32_t getTotalBytesWritten() { return totalBytesWritten; }

    /**
     * @brief Reset statistics
     */
    void resetStatistics();

    /**
     * @brief Print logger status
     *
     * Prints:
     * - Log directory
     * - Available space
     * - Log file sizes
     * - Statistics
     */
    void printStatus();

    /**
     * @brief Check logger health
     * @return true if healthy (enough space, no errors)
     *
     * Health check criteria:
     * - SPIFFS mounted
     * - Adequate free space (>10%)
     * - Write success rate >95%
     * - No critical errors
     */
    bool isHealthy();

    /**
     * @brief Compact log file (remove old entries)
     * @param category Log category
     * @param keepLines Number of recent lines to keep
     * @return true if compacted successfully
     *
     * Keeps only the most recent entries, deletes older ones.
     * Useful for maintaining log size without full rotation.
     *
     * EXAMPLE:
     * @code
     * // Keep only last 1000 sensor readings
     * logger.compactLog("sensors", 1000);
     * @endcode
     */
    bool compactLog(const char *category, uint16_t keepLines);

    /**
     * @brief Search logs for pattern
     * @param category Log category
     * @param pattern Search string
     * @param maxResults Maximum results to return
     * @return Matching lines as String
     *
     * Simple text search in logs.
     * Case-sensitive by default.
     *
     * EXAMPLE:
     * @code
     * // Find all temperature readings over 30°C
     * String results = logger.searchLog("sensors", "\"temp\":3", 10);
     * Serial.println(results);
     * @endcode
     */
    String searchLog(const char *category, const char *pattern, uint16_t maxResults = 10);

    /**
     * @brief Get log entry count
     * @param category Log category
     * @return Number of entries in log
     */
    uint32_t getEntryCount(const char *category);
};

#endif // DATA_LOGGER_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * DATA LOGGING BEST PRACTICES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. LOG FREQUENCY:
 *    - Don't log too frequently (wears out flash)
 *    - Typical: Every 10-60 seconds for sensors
 *    - Events: As they occur
 *    - Errors: Always log immediately
 *
 * 2. DATA FORMAT:
 *    - Use JSON for structured data
 *    - Keep entries small (<500 bytes)
 *    - Use consistent field names
 *    - Include units in field names (tempC, humidityPct)
 *
 * 3. FILE MANAGEMENT:
 *    - Enable automatic rotation
 *    - Monitor available space
 *    - Clean up old logs periodically
 *    - Export important data before deletion
 *
 * 4. CATEGORIES:
 *    - Separate logs by type
 *    - Sensors in one file
 *    - Events in another
 *    - Makes searching easier
 *
 * 5. PERFORMANCE:
 *    - Batch writes when possible
 *    - Don't write in interrupt handlers
 *    - Close files after writing
 *    - Check return values
 *
 * 6. DEBUGGING:
 *    - Use debug category for troubleshooting
 *    - Disable debug logs in production
 *    - Monitor failed write count
 *    - Check filesystem health
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * SPIFFS LIMITATIONS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * - Limited write cycles (~100,000 per block)
 * - No wear leveling (unlike LittleFS)
 * - Slower than SD card
 * - Limited total size (typically 1-2MB)
 * - No directories (flat file structure)
 *
 * ALTERNATIVES:
 * - LittleFS: Better wear leveling
 * - SD card: Much larger capacity
 * - Cloud storage: Unlimited (needs network)
 * - MQTT: Real-time logging to server
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * EXAMPLE USE CASES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * GREENHOUSE MONITOR:
 * - Log temperature/humidity every 5 minutes
 * - Log watering events
 * - Log alerts (too hot, too dry)
 * - Export weekly CSV for analysis
 *
 * SECURITY SYSTEM:
 * - Log motion detection events
 * - Log door open/close
 * - Log camera triggers
 * - Keep logs for 7 days
 *
 * ENERGY MONITOR:
 * - Log power consumption every minute
 * - Log peak usage times
 * - Calculate daily/monthly totals
 * - Export for billing analysis
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */