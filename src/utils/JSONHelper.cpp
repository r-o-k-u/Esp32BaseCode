/**
 * ═══════════════════════════════════════════════════════════════════════════
 * JSON HELPER - IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file JSONHelper.cpp
 * @brief Implementation of JSON utility functions
 * @version 2.0.0
 * @date 2024
 */

#include "JSONHelper.h"

/**
 * @brief Create success response JSON
 */
String JSONHelper::createSuccessResponse(const char *message)
{
    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = message;

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Create error response JSON
 */
String JSONHelper::createErrorResponse(const char *error, int code)
{
    StaticJsonDocument<128> doc;
    doc["success"] = false;
    doc["error"] = error;
    if (code >= 0)
    {
        doc["code"] = code;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Create system status JSON
 */
String JSONHelper::createStatusJSON()
{
    StaticJsonDocument<512> doc;

    doc["device"] = DEVICE_NAME;
    doc["version"] = FIRMWARE_VERSION;
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["heapSize"] = ESP.getHeapSize();
    doc["chipModel"] = ESP.getChipModel();
    doc["cpuFreq"] = ESP.getCpuFreqMHz();
    doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
    if (WiFi.status() == WL_CONNECTED)
    {
        doc["wifiRSSI"] = WiFi.RSSI();
        doc["ip"] = WiFi.localIP().toString();
    }

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Create sensor data JSON
 */
String JSONHelper::createSensorJSON(float temp, float humidity, float pressure)
{
    StaticJsonDocument<256> doc;

    doc["temperature"] = temp;
    doc["humidity"] = humidity;
    if (pressure > 0)
    {
        doc["pressure"] = pressure;
    }
    doc["timestamp"] = millis();

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Parse JSON string into document
 */
bool JSONHelper::parseJSON(const char *jsonString, JsonDocument &doc)
{
    DeserializationError error = deserializeJson(doc, jsonString);
    return !error;
}

/**
 * @brief Parse JSON with error reporting
 */
bool JSONHelper::parseJSONWithError(const char *jsonString, JsonDocument &doc, char *errorMsg)
{
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error)
    {
        snprintf(errorMsg, 128, "JSON parse error: %s", error.c_str());
        return false;
    }

    return true;
}

/**
 * @brief Validate JSON has required fields
 */
bool JSONHelper::validateFields(JsonDocument &doc, const char *fields[], int count)
{
    for (int i = 0; i < count; i++)
    {
        if (!doc.containsKey(fields[i]))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Safely get string value with default
 */
String JSONHelper::getString(JsonDocument &doc, const char *key, const char *defaultValue)
{
    if (doc.containsKey(key))
    {
        return doc[key].as<String>();
    }
    return String(defaultValue);
}

/**
 * @brief Safely get integer value with default
 */
int JSONHelper::getInt(JsonDocument &doc, const char *key, int defaultValue)
{
    if (doc.containsKey(key))
    {
        return doc[key].as<int>();
    }
    return defaultValue;
}

/**
 * @brief Safely get float value with default
 */
float JSONHelper::getFloat(JsonDocument &doc, const char *key, float defaultValue)
{
    if (doc.containsKey(key))
    {
        return doc[key].as<float>();
    }
    return defaultValue;
}

/**
 * @brief Safely get boolean value with default
 */
bool JSONHelper::getBool(JsonDocument &doc, const char *key, bool defaultValue)
{
    if (doc.containsKey(key))
    {
        return doc[key].as<bool>();
    }
    return defaultValue;
}

/**
 * @brief Create JSON array from integer array
 */
String JSONHelper::createIntArray(int *values, int count)
{
    StaticJsonDocument<512> doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < count; i++)
    {
        array.add(values[i]);
    }

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Create JSON array from float array
 */
String JSONHelper::createFloatArray(float *values, int count)
{
    StaticJsonDocument<512> doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < count; i++)
    {
        array.add(values[i]);
    }

    String output;
    serializeJson(doc, output);
    return output;
}

/**
 * @brief Pretty print JSON document
 */
void JSONHelper::prettyPrint(JsonDocument &doc)
{
    serializeJsonPretty(doc, Serial);
    Serial.println();
}

/**
 * @brief Calculate required buffer size for JSON
 */
size_t JSONHelper::calculateSize(JsonDocument &doc)
{
    return measureJson(doc);
}

/**
 * @brief Merge two JSON documents
 */
void JSONHelper::merge(JsonDocument &dest, JsonDocument &src)
{
    JsonObject destObj = dest.as<JsonObject>();
    JsonObject srcObj = src.as<JsonObject>();

    for (JsonPair kv : srcObj)
    {
        destObj[kv.key()] = kv.value();
    }
}

/**
 * @brief Create timestamp JSON entry
 */
String JSONHelper::createTimestamp()
{
    StaticJsonDocument<64> doc;
    doc["timestamp"] = millis();

    String output;
    serializeJson(doc, output);
    return output;
}