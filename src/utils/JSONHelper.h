/**
 * ═══════════════════════════════════════════════════════════════════════════
 * JSON HELPER - JSON UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file JSONHelper.h
 * @brief Helper functions for working with JSON data
 * @version 2.0.0
 * @date 2024
 *
 * This utility provides convenience functions for common JSON operations:
 * - Creating JSON responses
 * - Parsing JSON requests
 * - Error handling
 * - Type conversions
 * - Validation
 *
 * WHY USE JSON?
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * JSON (JavaScript Object Notation) is ideal for IoT because:
 * - Human-readable
 * - Language-independent
 * - Widely supported
 * - Self-describing
 * - Hierarchical data structure
 * - Easy to parse
 * - Compatible with web interfaces
 *
 * EXAMPLE:
 * Instead of: "25.5,60,1013"  (what do these numbers mean?)
 * Use JSON: {"temp":25.5,"humidity":60,"pressure":1013}  (clear!)
 *
 * USAGE:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @code
 * #include "utils/JSONHelper.h"
 *
 * // Create JSON response
 * String json = JSONHelper::createSuccessResponse("Operation completed");
 *
 * // Create error response
 * String error = JSONHelper::createErrorResponse("Sensor failed");
 *
 * // Parse JSON string
 * StaticJsonDocument<256> doc;
 * if (JSONHelper::parseJSON(jsonString, doc)) {
 *     int value = doc["value"];
 * }
 *
 * // Create sensor data JSON
 * String data = JSONHelper::createSensorJSON(25.5, 60);
 * @endcode
 */

#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "../config.h"

/**
 * @brief JSON Helper Class
 *
 * Static utility class for JSON operations.
 * No need to instantiate - all methods are static.
 */
class JSONHelper
{
public:
    /**
     * @brief Create success response JSON
     * @param message Success message
     * @return JSON string
     *
     * Creates standardized success response:
     * {"success":true,"message":"Operation completed"}
     *
     * EXAMPLE:
     * @code
     * String response = JSONHelper::createSuccessResponse("Data saved");
     * webServer.send(200, "application/json", response);
     * @endcode
     */
    static String createSuccessResponse(const char *message);

    /**
     * @brief Create error response JSON
     * @param error Error message
     * @param code Optional error code
     * @return JSON string
     *
     * Creates standardized error response:
     * {"success":false,"error":"Sensor timeout","code":100}
     *
     * EXAMPLE:
     * @code
     * String response = JSONHelper::createErrorResponse("Invalid parameter", 400);
     * webServer.send(400, "application/json", response);
     * @endcode
     */
    static String createErrorResponse(const char *error, int code = -1);

    /**
     * @brief Create system status JSON
     * @return JSON string with system info
     *
     * Creates JSON with:
     * - Device name
     * - Firmware version
     * - Uptime
     * - Free heap
     * - WiFi status
     *
     * EXAMPLE OUTPUT:
     * @code
     * {
     *   "device":"ESP32_Device_1",
     *   "version":"2.0.0",
     *   "uptime":123456,
     *   "freeHeap":50000,
     *   "wifiRSSI":-45
     * }
     * @endcode
     */
    static String createStatusJSON();

    /**
     * @brief Create sensor data JSON
     * @param temp Temperature value
     * @param humidity Humidity value
     * @param pressure Pressure value (optional)
     * @return JSON string
     *
     * EXAMPLE:
     * @code
     * String json = JSONHelper::createSensorJSON(25.5, 60, 1013.25);
     * // Returns: {"temp":25.5,"humidity":60,"pressure":1013.25}
     * @endcode
     */
    static String createSensorJSON(float temp, float humidity, float pressure = 0);

    /**
     * @brief Parse JSON string into document
     * @param jsonString JSON string to parse
     * @param doc JsonDocument to populate
     * @return true if parsing successful
     *
     * EXAMPLE:
     * @code
     * StaticJsonDocument<256> doc;
     * if (JSONHelper::parseJSON(request->body(), doc)) {
     *     const char* cmd = doc["command"];
     *     int value = doc["value"];
     *     // Process command...
     * } else {
     *     Serial.println("JSON parse error!");
     * }
     * @endcode
     */
    static bool parseJSON(const char *jsonString, JsonDocument &doc);

    /**
     * @brief Parse JSON with error reporting
     * @param jsonString JSON string to parse
     * @param doc JsonDocument to populate
     * @param errorMsg Buffer for error message
     * @return true if parsing successful
     *
     * EXAMPLE:
     * @code
     * StaticJsonDocument<256> doc;
     * char error[128];
     * if (!JSONHelper::parseJSONWithError(jsonStr, doc, error)) {
     *     Serial.printf("Parse error: %s\n", error);
     * }
     * @endcode
     */
    static bool parseJSONWithError(const char *jsonString, JsonDocument &doc, char *errorMsg);

    /**
     * @brief Validate JSON has required fields
     * @param doc JsonDocument to validate
     * @param fields Array of required field names
     * @param count Number of fields
     * @return true if all fields present
     *
     * EXAMPLE:
     * @code
     * const char* required[] = {"command", "value"};
     * if (!JSONHelper::validateFields(doc, required, 2)) {
     *     return createErrorResponse("Missing required fields");
     * }
     * @endcode
     */
    static bool validateFields(JsonDocument &doc, const char *fields[], int count);

    /**
     * @brief Safely get string value with default
     * @param doc JsonDocument
     * @param key Field name
     * @param defaultValue Default if missing
     * @return String value
     *
     * EXAMPLE:
     * @code
     * String cmd = JSONHelper::getString(doc, "command", "unknown");
     * // Returns "unknown" if "command" field doesn't exist
     * @endcode
     */
    static String getString(JsonDocument &doc, const char *key, const char *defaultValue = "");

    /**
     * @brief Safely get integer value with default
     * @param doc JsonDocument
     * @param key Field name
     * @param defaultValue Default if missing
     * @return Integer value
     */
    static int getInt(JsonDocument &doc, const char *key, int defaultValue = 0);

    /**
     * @brief Safely get float value with default
     * @param doc JsonDocument
     * @param key Field name
     * @param defaultValue Default if missing
     * @return Float value
     */
    static float getFloat(JsonDocument &doc, const char *key, float defaultValue = 0.0);

    /**
     * @brief Safely get boolean value with default
     * @param doc JsonDocument
     * @param key Field name
     * @param defaultValue Default if missing
     * @return Boolean value
     */
    static bool getBool(JsonDocument &doc, const char *key, bool defaultValue = false);

    /**
     * @brief Create JSON array from integer array
     * @param values Array of integers
     * @param count Array size
     * @return JSON string
     *
     * EXAMPLE:
     * @code
     * int data[] = {1, 2, 3, 4, 5};
     * String json = JSONHelper::createIntArray(data, 5);
     * // Returns: [1,2,3,4,5]
     * @endcode
     */
    static String createIntArray(int *values, int count);

    /**
     * @brief Create JSON array from float array
     * @param values Array of floats
     * @param count Array size
     * @return JSON string
     *
     * EXAMPLE:
     * @code
     * float temps[] = {25.5, 26.0, 25.8};
     * String json = JSONHelper::createFloatArray(temps, 3);
     * // Returns: [25.5,26.0,25.8]
     * @endcode
     */
    static String createFloatArray(float *values, int count);

    /**
     * @brief Pretty print JSON document
     * @param doc JsonDocument to print
     *
     * Prints formatted JSON to Serial for debugging
     *
     * EXAMPLE:
     * @code
     * StaticJsonDocument<256> doc;
     * doc["temp"] = 25.5;
     * doc["humidity"] = 60;
     * JSONHelper::prettyPrint(doc);
     * // Output:
     * // {
     * //   "temp": 25.5,
     * //   "humidity": 60
     * // }
     * @endcode
     */
    static void prettyPrint(JsonDocument &doc);

    /**
     * @brief Calculate required buffer size for JSON
     * @param doc JsonDocument
     * @return Required buffer size
     *
     * Useful for determining if buffer is large enough
     *
     * EXAMPLE:
     * @code
     * size_t size = JSONHelper::calculateSize(doc);
     * if (size > 256) {
     *     Serial.println("Warning: JSON too large!");
     * }
     * @endcode
     */
    static size_t calculateSize(JsonDocument &doc);

    /**
     * @brief Merge two JSON documents
     * @param dest Destination document
     * @param src Source document
     *
     * Copies all fields from src to dest
     *
     * EXAMPLE:
     * @code
     * StaticJsonDocument<512> combined;
     * StaticJsonDocument<256> sensors, status;
     * sensors["temp"] = 25.5;
     * status["uptime"] = 123456;
     *
     * JSONHelper::merge(combined, sensors);
     * JSONHelper::merge(combined, status);
     * // combined now has both temp and uptime
     * @endcode
     */
    static void merge(JsonDocument &dest, JsonDocument &src);

    /**
     * @brief Create timestamp JSON entry
     * @return JSON with timestamp
     *
     * EXAMPLE:
     * @code
     * String ts = JSONHelper::createTimestamp();
     * // Returns: {"timestamp":123456}
     * @endcode
     */
    static String createTimestamp();
};

#endif // JSON_HELPER_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * JSON BEST PRACTICES
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. DOCUMENT SIZE:
 *    - Use StaticJsonDocument for known sizes
 *    - Use DynamicJsonDocument for variable sizes
 *    - Calculate required size: https://arduinojson.org/v6/assistant/
 *    - Typical sizes:
 *      * Simple response: 128-256 bytes
 *      * Sensor data: 256-512 bytes
 *      * Complex data: 1024-2048 bytes
 *
 * 2. FIELD NAMING:
 *    - Use camelCase: "sensorValue" not "sensor_value"
 *    - Keep names short but descriptive
 *    - Be consistent across your API
 *    - Avoid special characters
 *
 * 3. ERROR HANDLING:
 *    - Always check parseJSON return value
 *    - Validate required fields exist
 *    - Provide meaningful error messages
 *    - Use proper HTTP status codes
 *
 * 4. PERFORMANCE:
 *    - Reuse JsonDocument objects when possible
 *    - Don't create documents in loops
 *    - Use appropriate document size
 *    - Serialize directly to String or Stream
 *
 * 5. MEMORY:
 *    - Monitor heap usage
 *    - Clear() documents when done
 *    - Prefer StaticJsonDocument (stack) over Dynamic (heap)
 *    - Watch for memory leaks
 *
 * COMMON PATTERNS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * Pattern 1: API Response
 * @code
 * void handleRequest(AsyncWebServerRequest *request) {
 *     if (operationSuccessful) {
 *         String response = JSONHelper::createSuccessResponse("Done");
 *         request->send(200, "application/json", response);
 *     } else {
 *         String response = JSONHelper::createErrorResponse("Failed");
 *         request->send(500, "application/json", response);
 *     }
 * }
 * @endcode
 *
 * Pattern 2: Command Processing
 * @code
 * void processCommand(const char* jsonStr) {
 *     StaticJsonDocument<256> doc;
 *     if (!JSONHelper::parseJSON(jsonStr, doc)) {
 *         Serial.println("Invalid JSON");
 *         return;
 *     }
 *
 *     String cmd = JSONHelper::getString(doc, "command", "");
 *     int value = JSONHelper::getInt(doc, "value", 0);
 *
 *     // Process command...
 * }
 * @endcode
 *
 * Pattern 3: Data Aggregation
 * @code
 * String getCompleteStatus() {
 *     StaticJsonDocument<1024> doc;
 *
 *     // Add system info
 *     doc["device"] = DEVICE_NAME;
 *     doc["uptime"] = millis();
 *
 *     // Add sensor data
 *     JsonObject sensors = doc.createNestedObject("sensors");
 *     sensors["temp"] = readTemperature();
 *     sensors["humidity"] = readHumidity();
 *
 *     // Add actuator states
 *     JsonObject actuators = doc.createNestedObject("actuators");
 *     actuators["relay1"] = getRelayState(1);
 *     actuators["relay2"] = getRelayState(2);
 *
 *     String output;
 *     serializeJson(doc, output);
 *     return output;
 * }
 * @endcode
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
