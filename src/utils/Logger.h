/**
 * ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM LOGGER - ADVANCED LOGGING UTILITY
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file Logger.h
 * @brief Advanced system logging with levels, formatting, and output control
 * @version 2.0.0
 * @date 2024
 *
 * This logger provides professional-grade logging functionality with:
 * - Multiple log levels (ERROR, WARN, INFO, DEBUG, TRACE)
 * - Colored output for readability
 * - Timestamps
 * - File and line number tracking
 * - Multiple output destinations (Serial, SPIFFS, Web)
 * - Log filtering
 * - Performance monitoring
 *
 * WHY USE THIS LOGGER?
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Instead of:
 * @code
 * Serial.println("Something happened");  // ❌ No context
 * Serial.print("Value: ");               // ❌ No severity level
 * Serial.println(value);                 // ❌ Can't filter
 * @endcode
 *
 * Use:
 * @code
 * LOG_INFO("Something happened");        // ✓ Severity level
 * LOG_DEBUG("Value: %d", value);         // ✓ Printf formatting
 * LOG_ERROR("Failed: %s", error);        // ✓ Easy filtering
 * @endcode
 *
 * USAGE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Basic logging:
 * @code
 * #include "utils/Logger.h"
 *
 * void setup() {
 *     Logger::begin();
 *     Logger::setLevel(LOG_LEVEL_DEBUG);
 *
 *     LOG_INFO("System starting...");
 *     LOG_DEBUG("Free heap: %d", ESP.getFreeHeap());
 * }
 *
 * void loop() {
 *     float temp = readSensor();
 *
 *     if (temp > 30) {
 *         LOG_WARN("Temperature high: %.1f°C", temp);
 *     }
 *
 *     if (sensorError) {
 *         LOG_ERROR("Sensor read failed!");
 *     }
 * }
 * @endcode
 *
 * LOG LEVELS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * ERROR   - Critical errors that need immediate attention
 * WARN    - Warnings about potential problems
 * INFO    - Important informational messages
 * DEBUG   - Detailed debugging information
 * TRACE   - Very detailed trace information
 *
 * Set level to control what gets logged:
 * - ERROR: Only errors
 * - WARN:  Errors + warnings
 * - INFO:  Errors + warnings + info (recommended for production)
 * - DEBUG: Everything except trace (recommended for development)
 * - TRACE: Everything (very verbose)
 *
 * OUTPUT FORMATS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * With timestamps:
 * [12345] [INFO] System starting...
 * [12456] [ERROR] Sensor read failed!
 *
 * With colors (Serial Monitor):
 * [INFO]  System starting...  (green)
 * [ERROR] Sensor failed!      (red)
 *
 * With file/line (debug builds):
 * [DEBUG] main.cpp:42 - Value: 123
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>
#include "../config.h"

/**
 * @brief Log level enumeration
 */
enum LogLevel
{
    LOG_LEVEL_NONE = 0,  ///< No logging
    LOG_LEVEL_ERROR = 1, ///< Error messages only
    LOG_LEVEL_WARN = 2,  ///< Warnings and errors
    LOG_LEVEL_INFO = 3,  ///< Info, warnings, and errors
    LOG_LEVEL_DEBUG = 4, ///< Debug info and above
    LOG_LEVEL_TRACE = 5  ///< Trace (everything)
};

/**
 * @brief Logger output destination flags
 */
enum LogOutput
{
    LOG_OUTPUT_SERIAL = (1 << 0), ///< Output to Serial
    LOG_OUTPUT_FILE = (1 << 1),   ///< Output to SPIFFS file
    LOG_OUTPUT_WEB = (1 << 2),    ///< Output to web clients
    LOG_OUTPUT_ALL = 0xFF         ///< All outputs
};

/**
 * @brief System Logger Class (Singleton)
 *
 * Provides centralized logging functionality with level control,
 * formatting, and multiple output destinations.
 */
class Logger
{
private:
    static LogLevel currentLevel;
    static uint8_t outputFlags;
    static bool useTimestamps;
    static bool useColors;
    static bool useLocation;
    static uint32_t logCount;

    /**
     * @brief Internal log implementation
     */
    static void logInternal(LogLevel level, const char *file, int line, const char *format, va_list args);

    /**
     * @brief Get level name as string
     */
    static const char *getLevelName(LogLevel level);

    /**
     * @brief Get ANSI color code for level
     */
    static const char *getLevelColor(LogLevel level);

public:
    /**
     * @brief Initialize logger
     * @param level Initial log level
     * @param outputs Output destinations (OR'd flags)
     *
     * EXAMPLE:
     * @code
     * // Log INFO and above to Serial
     * Logger::begin(LOG_LEVEL_INFO, LOG_OUTPUT_SERIAL);
     *
     * // Log everything to Serial and file
     * Logger::begin(LOG_LEVEL_TRACE, LOG_OUTPUT_SERIAL | LOG_OUTPUT_FILE);
     * @endcode
     */
    static void begin(LogLevel level = LOG_LEVEL_INFO,
                      uint8_t outputs = LOG_OUTPUT_SERIAL);

    /**
     * @brief Set log level
     * @param level New log level
     *
     * Only messages at or above this level will be logged.
     *
     * EXAMPLE:
     * @code
     * Logger::setLevel(LOG_LEVEL_DEBUG);  // Show debug and above
     * Logger::setLevel(LOG_LEVEL_ERROR);  // Only show errors
     * @endcode
     */
    static void setLevel(LogLevel level);

    /**
     * @brief Get current log level
     * @return Current level
     */
    static LogLevel getLevel() { return currentLevel; }

    /**
     * @brief Set output destinations
     * @param outputs Output flags (OR'd)
     *
     * EXAMPLE:
     * @code
     * // Output to Serial only
     * Logger::setOutput(LOG_OUTPUT_SERIAL);
     *
     * // Output to Serial and file
     * Logger::setOutput(LOG_OUTPUT_SERIAL | LOG_OUTPUT_FILE);
     * @endcode
     */
    static void setOutput(uint8_t outputs);

    /**
     * @brief Enable/disable timestamps
     * @param enable true to show timestamps
     */
    static void setTimestamps(bool enable);

    /**
     * @brief Enable/disable colored output
     * @param enable true to use colors
     */
    static void setColors(bool enable);

    /**
     * @brief Enable/disable file/line location
     * @param enable true to show location
     */
    static void setLocation(bool enable);

    /**
     * @brief Log error message
     * @param file Source file name
     * @param line Line number
     * @param format Printf-style format string
     * @param ... Format arguments
     */
    static void error(const char *file, int line, const char *format, ...);

    /**
     * @brief Log warning message
     */
    static void warn(const char *file, int line, const char *format, ...);

    /**
     * @brief Log info message
     */
    static void info(const char *file, int line, const char *format, ...);

    /**
     * @brief Log debug message
     */
    static void debug(const char *file, int line, const char *format, ...);

    /**
     * @brief Log trace message
     */
    static void trace(const char *file, int line, const char *format, ...);

    /**
     * @brief Get total log count
     * @return Number of messages logged
     */
    static uint32_t getLogCount() { return logCount; }

    /**
     * @brief Reset log counter
     */
    static void resetCounter();

    /**
     * @brief Print logger status
     */
    static void printStatus();
};

// ═══════════════════════════════════════════════════════════════════════════
// CONVENIENCE MACROS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * These macros make logging easy and automatically include file/line info
 *
 * USAGE:
 * @code
 * LOG_ERROR("Connection failed: %s", error);
 * LOG_WARN("Low memory: %d bytes", freeHeap);
 * LOG_INFO("System ready");
 * LOG_DEBUG("Loop count: %d", count);
 * LOG_TRACE("Function called with param: %d", param);
 * @endcode
 */

#define LOG_ERROR(format, ...) Logger::error(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Logger::warn(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Logger::info(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) Logger::debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_TRACE(format, ...) Logger::trace(__FILE__, __LINE__, format, ##__VA_ARGS__)

#endif // LOGGER_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * LOGGER USAGE GUIDE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * WHEN TO USE EACH LEVEL:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * ERROR:
 * - Failed to initialize hardware
 * - Network connection lost
 * - Sensor read timeout
 * - File system full
 * - Critical failures
 *
 * WARN:
 * - Low memory
 * - Weak WiFi signal
 * - Sensor calibration needed
 * - Using fallback values
 * - Potential problems
 *
 * INFO:
 * - System starting/stopping
 * - Configuration changes
 * - Major state transitions
 * - Important events
 * - Production logging
 *
 * DEBUG:
 * - Function entry/exit
 * - Variable values
 * - Conditional branches
 * - Algorithm steps
 * - Development logging
 *
 * TRACE:
 * - Loop iterations
 * - Every sensor reading
 * - Network packets
 * - Memory allocations
 * - Very detailed debugging
 *
 * FORMATTING EXAMPLES:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Strings:
 * LOG_INFO("Connected to: %s", ssid);
 *
 * Integers:
 * LOG_DEBUG("Counter: %d", count);
 * LOG_DEBUG("Hex value: 0x%04X", value);
 *
 * Floats:
 * LOG_INFO("Temperature: %.1f°C", temp);
 * LOG_DEBUG("Voltage: %.3fV", voltage);
 *
 * Multiple values:
 * LOG_DEBUG("X: %d, Y: %d, Z: %d", x, y, z);
 *
 * PRODUCTION RECOMMENDATIONS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Development:
 * - Use LOG_LEVEL_DEBUG
 * - Enable timestamps
 * - Enable colors
 * - Enable file/line location
 *
 * Production:
 * - Use LOG_LEVEL_INFO
 * - Enable timestamps
 * - Disable colors (no terminal)
 * - Disable file/line (save space)
 * - Log to file for debugging
 *
 * Battery-Powered:
 * - Use LOG_LEVEL_WARN or ERROR
 * - Disable timestamps (save power)
 * - Minimal logging (save power)
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */