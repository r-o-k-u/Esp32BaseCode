/**
 * @file SensorManager.cpp
 * @brief Sensor manager implementation
 */

#include "SensorManager.h"

// Global instance
SensorManager sensorManager;

/**
 * @brief Constructor
 */
SensorManager::SensorManager()
{
    initialized = false;
}

/**
 * @brief Initialize sensor manager
 * @return true if successful
 */
bool SensorManager::begin()
{
    Serial.println("Initializing Sensor Manager...");

    // For now, just return true - sensors will be added as needed
    initialized = true;
    Serial.println("✓ Sensor Manager initialized");
    return true;
}

/**
 * @brief Get all sensor data
 */
void SensorManager::getAllSensorData(JsonDocument &doc)
{
    // Add basic sensor data
    doc["temperature"] = 25.0;
    doc["humidity"] = 50.0;
    doc["pressure"] = 1013.25;
    doc["motion"] = false;
    doc["lightLevel"] = 500;
    doc["soilMoisture"] = 600;
}

/**
 * @brief Get temperature
 */
float SensorManager::getTemperature()
{
    return 25.0;
}

/**
 * @brief Get humidity
 */
float SensorManager::getHumidity()
{
    return 50.0;
}

/**
 * @brief Get pressure
 */
float SensorManager::getPressure()
{
    return 1013.25;
}

/**
 * @brief Get altitude
 */
float SensorManager::getAltitude()
{
    return 100.0;
}

/**
 * @brief Get motion status
 */
bool SensorManager::getMotion()
{
    return false;
}

/**
 * @brief Get light level
 */
int SensorManager::getLightLevel()
{
    return 500;
}

/**
 * @brief Get soil moisture
 */
int SensorManager::getSoilMoisture()
{
    return 600;
}

/**
 * @brief Print sensor status
 */
void SensorManager::printStatus()
{
    Serial.println("Sensor Manager Status:");
    Serial.printf("Initialized: %s\n", initialized ? "Yes" : "No");
}

/**
 * @brief Get sensor count
 */
uint8_t SensorManager::getSensorCount()
{
    return 3; // Basic sensors
}
