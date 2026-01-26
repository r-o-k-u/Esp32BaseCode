/**
 * @file SoilMoistureSensor.cpp
 * @brief Soil moisture sensor implementation
 * @author Your Name
 * @version 2.0
 */

#include "SoilMoistureSensor.h"
#include "../utils/Logger.h"

SoilMoistureSensor::SoilMoistureSensor(uint8_t sensorPin, int sampleCount)
    : pin(sensorPin), rawValue(0), voltage(0.0f), moisturePercentage(0.0f),
      dryValue(0), wetValue(4095), samples(sampleCount), sampleIndex(0)
{
    sampleBuffer = new int[samples];
    for (int i = 0; i < samples; i++)
    {
        sampleBuffer[i] = 0;
    }
}

SoilMoistureSensor::~SoilMoistureSensor()
{
    delete[] sampleBuffer;
}

bool SoilMoistureSensor::begin()
{
    pinMode(pin, INPUT);
    DEBUG_PRINTLN("[SOIL] Soil moisture sensor initialized on pin " + String(pin));
    return true;
}

bool SoilMoistureSensor::readMoisture()
{
    // Read raw ADC value
    int raw = analogRead(pin);

    // Store in circular buffer
    sampleBuffer[sampleIndex] = raw;
    sampleIndex = (sampleIndex + 1) % samples;

    // Calculate average raw value
    rawValue = getAverageRaw();

    // Calculate voltage
    voltage = (rawValue * 3.3f) / 4095.0f;

    // Calculate moisture percentage
    moisturePercentage = calculateMoisture(rawValue);

    DEBUG_PRINT("[SOIL] Raw: " + String(rawValue) + ", Voltage: " + String(voltage, 2) + "V, Moisture: " + String(moisturePercentage, 1) + "%");
    DEBUG_PRINTLN(", Level: " + getMoistureLevel());

    return true;
}

int SoilMoistureSensor::getRawValue()
{
    return rawValue;
}

float SoilMoistureSensor::getVoltage()
{
    return voltage;
}

float SoilMoistureSensor::getMoisturePercentage()
{
    return moisturePercentage;
}

String SoilMoistureSensor::getMoistureLevel()
{
    if (moisturePercentage < 20.0f)
        return "Very Dry";
    else if (moisturePercentage < 40.0f)
        return "Dry";
    else if (moisturePercentage < 60.0f)
        return "Moist";
    else if (moisturePercentage < 80.0f)
        return "Wet";
    else
        return "Very Wet";
}

void SoilMoistureSensor::calibrateDry(int dryReading)
{
    dryValue = dryReading;
    DEBUG_PRINTLN("[SOIL] Dry calibration set to: " + String(dryValue));
}

void SoilMoistureSensor::calibrateWet(int wetReading)
{
    wetValue = wetReading;
    DEBUG_PRINTLN("[SOIL] Wet calibration set to: " + String(wetValue));
}

float SoilMoistureSensor::calculateMoisture(int rawValue)
{
    // Map raw value to percentage (0-100%)
    // Note: Soil moisture sensors typically give lower values when wet
    // So we invert the mapping: high raw = dry, low raw = wet

    float percentage = map(rawValue, dryValue, wetValue, 0.0f, 100.0f);

    // Ensure percentage is within bounds
    if (percentage < 0.0f)
        percentage = 0.0f;
    if (percentage > 100.0f)
        percentage = 100.0f;

    // Invert if sensor gives high values when wet
    // percentage = 100.0f - percentage;

    return percentage;
}

int SoilMoistureSensor::getAverageRaw()
{
    long sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += sampleBuffer[i];
    }

    return sum / samples;
}
