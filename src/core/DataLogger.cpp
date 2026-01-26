/**
 * ═══════════════════════════════════════════════════════════════════════════
 * DATA LOGGER - IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file DataLogger.cpp
 * @brief Implementation of SPIFFS data logging system
 * @version 2.0.0
 * @date 2024
 */

#include "DataLogger.h"

// Global instance
DataLogger dataLogger;

/**
 * @brief Constructor - Initialize data logger
 */
DataLogger::DataLogger()
{
    initialized = false;
    logDirectory = "/logs";
    maxLogSize = MAX_LOG_SIZE;
    enableRotation = LOG_ROTATION;
    enableTimestamp = true;
    totalWrites = 0;
    totalRotations = 0;
    failedWrites = 0;
    totalBytesWritten = 0;
    currentCategory = "";
}

/**
 * @brief Destructor - Clean up resources
 */
DataLogger::~DataLogger()
{
    if (currentFile)
    {
        currentFile.close();
    }
}

/**
 * @brief Initialize data logger
 * @param logDir Base directory for logs
 * @param maxSize Maximum log file size
 * @return true if initialization successful
 */
bool DataLogger::begin(const char *logDir, uint32_t maxSize)
{
    DEBUG_PRINTLN("═══════════════════════════════════════════════════");
    DEBUG_PRINTLN("Initializing Data Logger");
    DEBUG_PRINTLN("═══════════════════════════════════════════════════");

    // Check if SPIFFS is mounted
    if (!SPIFFS.begin())
    {
        DEBUG_PRINTLN("ERROR: SPIFFS not mounted!");
        DEBUG_PRINTLN("       Call SPIFFS.begin() before DataLogger.begin()");
        return false;
    }

    logDirectory = String(logDir);
    maxLogSize = maxSize;

    DEBUG_PRINTF("Log directory: %s\n", logDirectory.c_str());
    DEBUG_PRINTF("Max log size:  %u bytes (%.1f KB)\n", maxLogSize, maxLogSize / 1024.0);
    DEBUG_PRINTF("Auto-rotation: %s\n", enableRotation ? "Enabled" : "Disabled");
    DEBUG_PRINTF("Timestamps:    %s\n", enableTimestamp ? "Enabled" : "Disabled");

    // Create log directory if it doesn't exist
    // Note: SPIFFS doesn't have real directories, but we use path prefixes
    DEBUG_PRINT("Checking log directory... ");

    // Check if any log files exist with our prefix
    File root = SPIFFS.open("/");
    bool dirExists = false;
    if (root && root.isDirectory())
    {
        File file = root.openNextFile();
        while (file)
        {
            String filename = file.name();
            if (filename.startsWith(logDirectory))
            {
                dirExists = true;
                break;
            }
            file = root.openNextFile();
        }
    }

    if (!dirExists)
    {
        DEBUG_PRINTLN("Creating");
        // Create a marker file to establish directory
        String markerFile = logDirectory + "/.marker";
        File marker = SPIFFS.open(markerFile, FILE_WRITE);
        if (marker)
        {
            marker.println("Log directory marker");
            marker.close();
            DEBUG_PRINTLN("✓ Directory created");
        }
        else
        {
            DEBUG_PRINTLN("✗ Failed to create directory");
        }
    }
    else
    {
        DEBUG_PRINTLN("OK (exists)");
    }

    // Print filesystem info
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;

    DEBUG_PRINTLN("───────────────────────────────────────────────────");
    DEBUG_PRINTF("SPIFFS Total:  %u bytes (%.1f KB)\n", totalBytes, totalBytes / 1024.0);
    DEBUG_PRINTF("SPIFFS Used:   %u bytes (%.1f KB)\n", usedBytes, usedBytes / 1024.0);
    DEBUG_PRINTF("SPIFFS Free:   %u bytes (%.1f KB)\n", freeBytes, freeBytes / 1024.0);
    DEBUG_PRINTF("Usage:         %.1f%%\n", (float)usedBytes / totalBytes * 100);

    // Warn if low on space
    if (freeBytes < maxLogSize * 2)
    {
        DEBUG_PRINTLN("⚠️  WARNING: Low on storage space!");
        DEBUG_PRINTLN("    Consider deleting old logs or increasing SPIFFS size");
    }

    initialized = true;

    DEBUG_PRINTLN("═══════════════════════════════════════════════════");
    DEBUG_PRINTLN("✓ Data Logger Ready");
    DEBUG_PRINTLN("═══════════════════════════════════════════════════\n");

    return true;
}

/**
 * @brief Get log filename for category
 * @param category Log category
 * @return Full file path
 */
String DataLogger::getLogFilename(const char *category)
{
    return logDirectory + "/" + String(category) + ".log";
}

/**
 * @brief Get rotated log filename
 * @param category Log category
 * @return Rotated file path
 */
String DataLogger::getRotatedFilename(const char *category)
{
    return logDirectory + "/" + String(category) + "_old.log";
}

/**
 * @brief Check if log file needs rotation
 * @param category Log category
 * @return true if rotation needed
 */
bool DataLogger::needsRotation(const char *category)
{
    if (!enableRotation)
        return false;

    String filename = getLogFilename(category);
    if (!SPIFFS.exists(filename))
        return false;

    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
        return false;

    size_t size = file.size();
    file.close();

    return (size >= maxLogSize);
}

/**
 * @brief Rotate log file
 * @param category Log category
 * @return true if rotation successful
 */
bool DataLogger::rotateLog(const char *category)
{
    String currentFilename = getLogFilename(category);
    String rotatedFilename = getRotatedFilename(category);

    DEBUG_PRINTF("Rotating log: %s\n", category);

    // Delete old rotated file if exists
    if (SPIFFS.exists(rotatedFilename))
    {
        SPIFFS.remove(rotatedFilename);
    }

    // Rename current to rotated
    if (SPIFFS.rename(currentFilename, rotatedFilename))
    {
        totalRotations++;
        DEBUG_PRINTLN("✓ Log rotated successfully");
        return true;
    }
    else
    {
        DEBUG_PRINTLN("✗ Log rotation failed");
        return false;
    }
}

/**
 * @brief Format log entry as JSON
 * @param data Data string
 * @return Formatted JSON string
 */
String DataLogger::formatLogEntry(const char *data)
{
    StaticJsonDocument<512> doc;

    // Add timestamp if enabled
    if (enableTimestamp)
    {
        doc["timestamp"] = millis();
    }

    // Try to parse data as JSON
    StaticJsonDocument<256> dataDoc;
    DeserializationError error = deserializeJson(dataDoc, data);

    if (!error)
    {
        // Data is valid JSON, add as object
        doc["data"] = dataDoc.as<JsonObject>();
    }
    else
    {
        // Data is plain text, add as string
        doc["data"] = data;
    }

    // Serialize to string
    String output;
    serializeJson(doc, output);
    output += "\n"; // Add newline for readability

    return output;
}

/**
 * @brief Write entry to file
 * @param filename File path
 * @param data Data to write
 * @return true if write successful
 */
bool DataLogger::writeToFile(const char *filename, const char *data)
{
    // Open file in append mode
    File file = SPIFFS.open(filename, FILE_APPEND);

    if (!file)
    {
        DEBUG_PRINTF("ERROR: Failed to open %s for writing\n", filename);
        failedWrites++;
        return false;
    }

    // Write data
    size_t bytesWritten = file.print(data);
    file.close();

    if (bytesWritten > 0)
    {
        totalBytesWritten += bytesWritten;
        return true;
    }
    else
    {
        failedWrites++;
        return false;
    }
}

/**
 * @brief Log data to specified category
 * @param category Log category
 * @param data Data to log
 * @return true if logged successfully
 */
bool DataLogger::logData(const char *category, const char *data)
{
    if (!initialized)
    {
        DEBUG_PRINTLN("ERROR: DataLogger not initialized!");
        return false;
    }

    // Check if rotation needed
    if (needsRotation(category))
    {
        rotateLog(category);
    }

    // Format log entry
    String formattedEntry = formatLogEntry(data);

    // Write to file
    String filename = getLogFilename(category);
    bool success = writeToFile(filename.c_str(), formattedEntry.c_str());

    if (success)
    {
        totalWrites++;

#if DEBUG_MODE
// Only print in debug mode to reduce serial spam
// DEBUG_PRINTF("Logged to %s: %s", category, data);
#endif
    }
    else
    {
        DEBUG_PRINTF("Failed to log to %s\n", category);
    }

    return success;
}

/**
 * @brief Log sensor data
 * @param jsonData Sensor data as JSON
 * @return true if logged successfully
 */
bool DataLogger::logSensorData(const char *jsonData)
{
    return logData("sensors", jsonData);
}

/**
 * @brief Log event message
 * @param message Event description
 * @return true if logged successfully
 */
bool DataLogger::logEvent(const char *message)
{
    return logData("events", message);
}

/**
 * @brief Log error message
 * @param error Error description
 * @return true if logged successfully
 */
bool DataLogger::logError(const char *error)
{
    DEBUG_PRINTF("ERROR LOGGED: %s\n", error);
    return logData("errors", error);
}

/**
 * @brief Log actuator state change
 * @param actuator Actuator name
 * @param state New state
 * @return true if logged successfully
 */
bool DataLogger::logActuator(const char *actuator, int state)
{
    StaticJsonDocument<128> doc;
    doc["actuator"] = actuator;
    doc["state"] = state;

    char buffer[128];
    serializeJson(doc, buffer);

    return logData("actuators", buffer);
}

/**
 * @brief Read log file contents
 * @param category Log category to read
 * @param maxLines Maximum lines to read (0 = all)
 * @return Log contents as String
 */
String DataLogger::readLog(const char *category, uint16_t maxLines)
{
    String filename = getLogFilename(category);

    if (!SPIFFS.exists(filename))
    {
        return "Log file not found: " + String(category);
    }

    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        return "Error opening log file";
    }

    String content = "";
    uint16_t lineCount = 0;

    // If maxLines is 0, read all
    if (maxLines == 0)
    {
        content = file.readString();
    }
    else
    {
        // Read last N lines (simple implementation)
        // For production, use circular buffer or seek to end
        while (file.available() && lineCount < maxLines)
        {
            content += file.readStringUntil('\n') + "\n";
            lineCount++;
        }
    }

    file.close();
    return content;
}

/**
 * @brief Get log file size
 * @param category Log category
 * @return File size in bytes
 */
uint32_t DataLogger::getLogSize(const char *category)
{
    String filename = getLogFilename(category);

    if (!SPIFFS.exists(filename))
    {
        return 0;
    }

    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        return 0;
    }

    size_t size = file.size();
    file.close();

    return size;
}

/**
 * @brief Check if log file exists
 * @param category Log category
 * @return true if file exists
 */
bool DataLogger::logExists(const char *category)
{
    String filename = getLogFilename(category);
    return SPIFFS.exists(filename);
}

/**
 * @brief Delete log file
 * @param category Log category to delete
 * @return true if deleted successfully
 */
bool DataLogger::deleteLog(const char *category)
{
    String filename = getLogFilename(category);

    if (!SPIFFS.exists(filename))
    {
        DEBUG_PRINTF("Log %s does not exist\n", category);
        return false;
    }

    bool success = SPIFFS.remove(filename);

    if (success)
    {
        DEBUG_PRINTF("✓ Deleted log: %s\n", category);
    }
    else
    {
        DEBUG_PRINTF("✗ Failed to delete log: %s\n", category);
    }

    return success;
}

/**
 * @brief Delete all logs
 * @return true if all deleted successfully
 */
bool DataLogger::deleteAllLogs()
{
    DEBUG_PRINTLN("Deleting all logs...");

    File root = SPIFFS.open(logDirectory);
    if (!root || !root.isDirectory())
    {
        DEBUG_PRINTLN("ERROR: Cannot open log directory");
        return false;
    }

    int deletedCount = 0;
    int failedCount = 0;

    File file = root.openNextFile();
    while (file)
    {
        String filename = file.name();
        file.close();

        // Only delete .log files
        if (filename.endsWith(".log"))
        {
            if (SPIFFS.remove(filename))
            {
                deletedCount++;
                DEBUG_PRINTF("  Deleted: %s\n", filename.c_str());
            }
            else
            {
                failedCount++;
                DEBUG_PRINTF("  Failed: %s\n", filename.c_str());
            }
        }

        file = root.openNextFile();
    }

    DEBUG_PRINTF("✓ Deleted %d log files\n", deletedCount);
    if (failedCount > 0)
    {
        DEBUG_PRINTF("⚠️  Failed to delete %d files\n", failedCount);
    }

    return (failedCount == 0);
}

/**
 * @brief Export log as CSV
 * @param category Log category
 * @param csvFilename Output CSV filename
 * @return true if export successful
 */
bool DataLogger::exportAsCSV(const char *category, const char *csvFilename)
{
    String logFilename = getLogFilename(category);

    if (!SPIFFS.exists(logFilename))
    {
        DEBUG_PRINTLN("ERROR: Log file not found");
        return false;
    }

    File logFile = SPIFFS.open(logFilename, FILE_READ);
    if (!logFile)
    {
        DEBUG_PRINTLN("ERROR: Cannot open log file");
        return false;
    }

    File csvFile = SPIFFS.open(csvFilename, FILE_WRITE);
    if (!csvFile)
    {
        logFile.close();
        DEBUG_PRINTLN("ERROR: Cannot create CSV file");
        return false;
    }

    // Write CSV header
    csvFile.println("Timestamp,Data");

    // Convert each JSON line to CSV
    while (logFile.available())
    {
        String line = logFile.readStringUntil('\n');

        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, line);

        if (!error)
        {
            uint32_t timestamp = doc["timestamp"];
            String data = doc["data"].as<String>();

            csvFile.printf("%u,%s\n", timestamp, data.c_str());
        }
    }

    logFile.close();
    csvFile.close();

    DEBUG_PRINTF("✓ Exported to CSV: %s\n", csvFilename);
    return true;
}

/**
 * @brief Get list of all log files
 * @param buffer Buffer to store filenames
 * @param maxFiles Maximum files to list
 * @return Number of files found
 */
uint8_t DataLogger::listLogs(String *buffer, uint8_t maxFiles)
{
    File root = SPIFFS.open(logDirectory);
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    uint8_t count = 0;
    File file = root.openNextFile();

    while (file && count < maxFiles)
    {
        String filename = file.name();

        if (filename.endsWith(".log"))
        {
            buffer[count] = filename;
            count++;
        }

        file = root.openNextFile();
    }

    return count;
}

/**
 * @brief Get total storage used by logs
 * @return Total bytes used
 */
uint32_t DataLogger::getTotalLogSize()
{
    File root = SPIFFS.open(logDirectory);
    if (!root || !root.isDirectory())
    {
        return 0;
    }

    uint32_t totalSize = 0;
    File file = root.openNextFile();

    while (file)
    {
        if (String(file.name()).endsWith(".log"))
        {
            totalSize += file.size();
        }
        file = root.openNextFile();
    }

    return totalSize;
}

/**
 * @brief Get available SPIFFS space
 * @return Free bytes
 */
uint32_t DataLogger::getAvailableSpace()
{
    return SPIFFS.totalBytes() - SPIFFS.usedBytes();
}

/**
 * @brief Enable/disable automatic file rotation
 * @param enable true to enable rotation
 */
void DataLogger::setRotation(bool enable)
{
    enableRotation = enable;
    DEBUG_PRINTF("Log rotation: %s\n", enable ? "Enabled" : "Disabled");
}

/**
 * @brief Set maximum log file size
 * @param size Size in bytes
 */
void DataLogger::setMaxLogSize(uint32_t size)
{
    maxLogSize = size;
    DEBUG_PRINTF("Max log size set to: %u bytes (%.1f KB)\n", size, size / 1024.0);
}

/**
 * @brief Enable/disable timestamps
 * @param enable true to include timestamps
 */
void DataLogger::setTimestamp(bool enable)
{
    enableTimestamp = enable;
    DEBUG_PRINTF("Timestamps: %s\n", enable ? "Enabled" : "Disabled");
}

/**
 * @brief Reset statistics
 */
void DataLogger::resetStatistics()
{
    totalWrites = 0;
    totalRotations = 0;
    failedWrites = 0;
    totalBytesWritten = 0;
    DEBUG_PRINTLN("Statistics reset");
}

/**
 * @brief Print logger status
 */
void DataLogger::printStatus()
{
    DEBUG_PRINTLN("┌───────────────────────────────────────────────────┐");
    DEBUG_PRINTLN("│            DATA LOGGER STATUS                     │");
    DEBUG_PRINTLN("├───────────────────────────────────────────────────┤");
    DEBUG_PRINTF("│ Status:        %-30s │\n", initialized ? "Ready" : "Not initialized");
    DEBUG_PRINTF("│ Log Directory: %-30s │\n", logDirectory.c_str());
    DEBUG_PRINTF("│ Max Log Size:  %-25u KB │\n", maxLogSize / 1024);
    DEBUG_PRINTF("│ Auto-rotation: %-30s │\n", enableRotation ? "Enabled" : "Disabled");
    DEBUG_PRINTF("│ Timestamps:    %-30s │\n", enableTimestamp ? "Enabled" : "Disabled");
    DEBUG_PRINTLN("├───────────────────────────────────────────────────┤");
    DEBUG_PRINTF("│ Total Writes:  %-30u │\n", totalWrites);
    DEBUG_PRINTF("│ Failed Writes: %-30u │\n", failedWrites);
    DEBUG_PRINTF("│ Rotations:     %-30u │\n", totalRotations);
    DEBUG_PRINTF("│ Bytes Written: %-25u KB │\n", totalBytesWritten / 1024);
    DEBUG_PRINTLN("├───────────────────────────────────────────────────┤");
    DEBUG_PRINTF("│ Total Log Size:%-25u KB │\n", getTotalLogSize() / 1024);
    DEBUG_PRINTF("│ Free Space:    %-25u KB │\n", getAvailableSpace() / 1024);
    DEBUG_PRINTLN("└───────────────────────────────────────────────────┘");
}

/**
 * @brief Check logger health
 * @return true if healthy
 */
bool DataLogger::isHealthy()
{
    if (!initialized)
        return false;

    // Check available space (at least 10%)
    uint32_t totalSpace = SPIFFS.totalBytes();
    uint32_t freeSpace = getAvailableSpace();
    float freePercent = (float)freeSpace / totalSpace * 100;

    if (freePercent < 10)
    {
        DEBUG_PRINTLN("⚠️  WARNING: Low storage space!");
        return false;
    }

    // Check write success rate
    if (totalWrites > 0)
    {
        float successRate = (float)(totalWrites - failedWrites) / totalWrites * 100;
        if (successRate < 95)
        {
            DEBUG_PRINTLN("⚠️  WARNING: Low write success rate!");
            return false;
        }
    }

    return true;
}

/**
 * @brief Compact log file
 * @param category Log category
 * @param keepLines Number of recent lines to keep
 * @return true if compacted successfully
 */
bool DataLogger::compactLog(const char *category, uint16_t keepLines)
{
    String filename = getLogFilename(category);
    String tempFilename = filename + ".tmp";

    if (!SPIFFS.exists(filename))
    {
        return false;
    }

    File inputFile = SPIFFS.open(filename, FILE_READ);
    if (!inputFile)
    {
        return false;
    }

    // Count total lines
    uint16_t totalLines = 0;
    while (inputFile.available())
    {
        inputFile.readStringUntil('\n');
        totalLines++;
    }

    // Calculate lines to skip
    uint16_t skipLines = (totalLines > keepLines) ? (totalLines - keepLines) : 0;

    // Rewind
    inputFile.seek(0);

    // Create temp file with kept lines
    File outputFile = SPIFFS.open(tempFilename, FILE_WRITE);
    if (!outputFile)
    {
        inputFile.close();
        return false;
    }

    uint16_t currentLine = 0;
    while (inputFile.available())
    {
        String line = inputFile.readStringUntil('\n');
        if (currentLine >= skipLines)
        {
            outputFile.println(line);
        }
        currentLine++;
    }

    inputFile.close();
    outputFile.close();

    // Replace original with compacted
    SPIFFS.remove(filename);
    SPIFFS.rename(tempFilename, filename);

    DEBUG_PRINTF("✓ Compacted %s: kept %u/%u lines\n", category, keepLines, totalLines);
    return true;
}

/**
 * @brief Search logs for pattern
 * @param category Log category
 * @param pattern Search string
 * @param maxResults Maximum results to return
 * @return Matching lines as String
 */
String DataLogger::searchLog(const char *category, const char *pattern, uint16_t maxResults)
{
    String filename = getLogFilename(category);

    if (!SPIFFS.exists(filename))
    {
        return "Log not found";
    }

    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        return "Error opening log";
    }

    String results = "";
    uint16_t matchCount = 0;

    while (file.available() && matchCount < maxResults)
    {
        String line = file.readStringUntil('\n');
        if (line.indexOf(pattern) >= 0)
        {
            results += line + "\n";
            matchCount++;
        }
    }

    file.close();

    if (matchCount == 0)
    {
        return "No matches found";
    }

    return results;
}

/**
 * @brief Get log entry count
 * @param category Log category
 * @return Number of entries
 */
uint32_t DataLogger::getEntryCount(const char *category)
{
    String filename = getLogFilename(category);

    if (!SPIFFS.exists(filename))
    {
        return 0;
    }

    File file = SPIFFS.open(filename, FILE_READ);
    if (!file)
    {
        return 0;
    }

    uint32_t count = 0;
    while (file.available())
    {
        file.readStringUntil('\n');
        count++;
    }

    file.close();
    return count;
}

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * END OF DATA LOGGER IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 */
