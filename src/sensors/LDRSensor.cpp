/**
 * @file LDRSensor.cpp
 * @brief Light Dependent Resistor (LDR) light sensor implementation
 * @author Your Name
 * @version 2.0
 */

#include "LDRSensor.h"
#include "../utils/Logger.h"

LDRSensor::LDRSensor(uint8_t sensorPin, int sampleCount)
    : pin(sensorPin), rawValue(0), voltage(0.0f), lux(0.0f),
      samples(sampleCount), sampleIndex(0)
{
    sampleBuffer = new int[samples];
    for (int i = 0; i < samples; i++)
    {
        sampleBuffer[i] = 0;
    }
}

LDRSensor::~LDRSensor()
{
    delete[] sampleBuffer;
}

bool LDRSensor::begin()
{
    pinMode(pin, INPUT);
    DEBUG_PRINTLN("[LDR] LDR sensor initialized on pin " + String(pin));
    return true;
}

bool LDRSensor::readLight()
{
    // Read raw ADC value
    int raw = analogRead(pin);

    // Store in circular buffer
    sampleBuffer[sampleIndex] = raw;
    sampleIndex = (sampleIndex + 1) % samples;

    // Calculate average voltage
    voltage = getAverageVoltage();

    // Calculate lux
    lux = calculateLux(raw);
    rawValue = raw;

    DEBUG_PRINT("[LDR] Raw: " + String(rawValue) + ", Voltage: " + String(voltage, 2) + "V, Lux: " + String(lux, 2));
    DEBUG_PRINTLN(", Level: " + getLightLevel());

    return true;
}

int LDRSensor::getRawValue()
{
    return rawValue;
}

float LDRSensor::getVoltage()
{
    return voltage;
}

float LDRSensor::getLux()
{
    return lux;
}

String LDRSensor::getLightLevel()
{
    if (lux < 1.0f)
        return "Dark";
    else if (lux < 10.0f)
        return "Dim";
    else if (lux < 100.0f)
        return "Indoor";
    else if (lux < 1000.0f)
        return "Cloudy";
    else if (lux < 10000.0f)
        return "Sunny";
    else
        return "Very Bright";
}

float LDRSensor::calculateLux(int rawValue)
{
    // Convert raw ADC value to voltage (assuming 3.3V reference)
    float voltage = (rawValue * 3.3f) / 4095.0f;

    // Simple approximation for lux calculation
    // This is a rough estimate and may need calibration for your specific LDR
    if (voltage < 0.1f)
        return 0.0f;

    // LDR resistance decreases with light, so voltage increases
    // Using a simplified formula: Lux = k * (Vref - Vout) / Vout
    float resistanceRatio = (3.3f - voltage) / voltage;

    // Empirical constant for typical LDR (may need calibration)
    float lux = 1000.0f / (resistanceRatio + 1.0f);

    return lux;
}

float LDRSensor::getAverageVoltage()
{
    long sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += sampleBuffer[i];
    }

    int average = sum / samples;
    return (average * 3.3f) / 4095.0f;
}
