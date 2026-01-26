/**
 * ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM LOGGER - IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file Logger.cpp
 * @brief Implementation of advanced logging system
 * @version 2.0.0
 * @date 2024
 */

#include "Logger.h"

// Initialize static members
LogLevel Logger::currentLevel = LOG_LEVEL_INFO;
uint8_t Logger::outputFlags = LOG_OUTPUT_SERIAL;
bool Logger::useTimestamps = true;
bool Logger::useColors = true;
bool Logger::useLocation = false;
uint32_t Logger::logCount = 0;

// ANSI color codes for terminal output
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_GRAY "\033[90m"

/**
 * @brief Initialize logger
 */
void Logger::begin(LogLevel level, uint8_t outputs)
{
    currentLevel = level;
    outputFlags = outputs;
    logCount = 0;

    Serial.println("\n═══════════════════════════════════════════════════");
    Serial.println("System Logger Initialized");
    Serial.println("═══════════════════════════════════════════════════");
    Serial.printf("Log Level:    %s\n", getLevelName(level));
    Serial.printf("Outputs:      %s%s%s\n",
                  (outputs & LOG_OUTPUT_SERIAL) ? "Serial " : "",
                  (outputs & LOG_OUTPUT_FILE) ? "File " : "",
                  (outputs & LOG_OUTPUT_WEB) ? "Web " : "");
    Serial.printf("Timestamps:   %s\n", useTimestamps ? "Enabled" : "Disabled");
    Serial.printf("Colors:       %s\n", useColors ? "Enabled" : "Disabled");
    Serial.printf("Location:     %s\n", useLocation ? "Enabled" : "Disabled");
    Serial.println("═══════════════════════════════════════════════════\n");
}

/**
 * @brief Set log level
 */
void Logger::setLevel(LogLevel level)
{
    currentLevel = level;
    Serial.printf("Log level set to: %s\n", getLevelName(level));
}

/**
 * @brief Set output destinations
 */
void Logger::setOutput(uint8_t outputs)
{
    outputFlags = outputs;
}

/**
 * @brief Enable/disable timestamps
 */
void Logger::setTimestamps(bool enable)
{
    useTimestamps = enable;
}

/**
 * @brief Enable/disable colored output
 */
void Logger::setColors(bool enable)
{
    useColors = enable;
}

/**
 * @brief Enable/disable file/line location
 */
void Logger::setLocation(bool enable)
{
    useLocation = enable;
}

/**
 * @brief Get level name as string
 */
const char *Logger::getLevelName(LogLevel level)
{
    switch (level)
    {
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_WARN:
        return "WARN ";
    case LOG_LEVEL_INFO:
        return "INFO ";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_TRACE:
        return "TRACE";
    default:
        return "NONE ";
    }
}

/**
 * @brief Get ANSI color code for level
 */
const char *Logger::getLevelColor(LogLevel level)
{
    switch (level)
    {
    case LOG_LEVEL_ERROR:
        return COLOR_RED;
    case LOG_LEVEL_WARN:
        return COLOR_YELLOW;
    case LOG_LEVEL_INFO:
        return COLOR_GREEN;
    case LOG_LEVEL_DEBUG:
        return COLOR_CYAN;
    case LOG_LEVEL_TRACE:
        return COLOR_GRAY;
    default:
        return COLOR_RESET;
    }
}

/**
 * @brief Internal log implementation
 */
void Logger::logInternal(LogLevel level, const char *file, int line, const char *format, va_list args)
{
    // Check if this level should be logged
    if (level > currentLevel)
    {
        return;
    }

    logCount++;

    // Build log message
    char buffer[256];
    char *ptr = buffer;
    int remaining = sizeof(buffer);

    // Add timestamp
    if (useTimestamps)
    {
        int written = snprintf(ptr, remaining, "[%lu] ", millis());
        ptr += written;
        remaining -= written;
    }

    // Add color (if enabled)
    if (useColors && (outputFlags & LOG_OUTPUT_SERIAL))
    {
        int written = snprintf(ptr, remaining, "%s", getLevelColor(level));
        ptr += written;
        remaining -= written;
    }

    // Add level
    int written = snprintf(ptr, remaining, "[%s] ", getLevelName(level));
    ptr += written;
    remaining -= written;

    // Add file/line (if enabled)
    if (useLocation)
    {
        // Extract just filename (not full path)
        const char *filename = strrchr(file, '/');
        if (!filename)
            filename = strrchr(file, '\\');
        if (!filename)
            filename = file;
        else
            filename++; // Skip the slash

        written = snprintf(ptr, remaining, "%s:%d - ", filename, line);
        ptr += written;
        remaining -= written;
    }

    // Add user message
    written = vsnprintf(ptr, remaining, format, args);
    ptr += written;
    remaining -= written;

    // Add color reset
    if (useColors && (outputFlags & LOG_OUTPUT_SERIAL))
    {
        snprintf(ptr, remaining, "%s", COLOR_RESET);
    }

    // Output to Serial
    if (outputFlags & LOG_OUTPUT_SERIAL)
    {
        Serial.println(buffer);
    }

    // Output to file (if enabled)
    if (outputFlags & LOG_OUTPUT_FILE)
    {
        // TODO: Implement file logging
        // Could use DataLogger here
    }

    // Output to web (if enabled)
    if (outputFlags & LOG_OUTPUT_WEB)
    {
        // TODO: Broadcast to web clients
        // Could use WebServer broadcast here
    }
}

/**
 * @brief Log error message
 */
void Logger::error(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_ERROR, file, line, format, args);
    va_end(args);
}

/**
 * @brief Log warning message
 */
void Logger::warn(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_WARN, file, line, format, args);
    va_end(args);
}

/**
 * @brief Log info message
 */
void Logger::info(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_INFO, file, line, format, args);
    va_end(args);
}

/**
 * @brief Log debug message
 */
void Logger::debug(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_DEBUG, file, line, format, args);
    va_end(args);
}

/**
 * @brief Log trace message
 */
void Logger::trace(const char *file, int line, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    logInternal(LOG_LEVEL_TRACE, file, line, format, args);
    va_end(args);
}

/**
 * @brief Reset log counter
 */
void Logger::resetCounter()
{
    logCount = 0;
}

/**
 * @brief Print logger status
 */
void Logger::printStatus()
{
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│            LOGGER STATUS                        │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.printf("│ Current Level: %-28s │\n", getLevelName(currentLevel));
    Serial.printf("│ Total Logs:    %-28u │\n", logCount);
    Serial.printf("│ Timestamps:    %-28s │\n", useTimestamps ? "Enabled" : "Disabled");
    Serial.printf("│ Colors:        %-28s │\n", useColors ? "Enabled" : "Disabled");
    Serial.printf("│ Location:      %-28s │\n", useLocation ? "Enabled" : "Disabled");
    Serial.println("└─────────────────────────────────────────────────┘");
}

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * END OF LOGGER IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 */