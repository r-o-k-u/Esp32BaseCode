/**
 * @file MQ135Sensor.cpp
 * @brief MQ135 Air Quality sensor implementation
 * @author Your Name
 * @version 2.0
 */

#include "MQ135Sensor.h"
#include "../utils/Logger.h"

MQ135Sensor::MQ135Sensor(uint8_t sensorPin, int sampleCount)
    : pin(sensorPin), rawValue(0), voltage(0.0f), resistance(0.0f), ppm(0.0f), r0(0.0f),
      samples(sampleCount), sampleIndex(0)
{
    sampleBuffer = new int[samples];
    for (int i = 0; i < samples; i++)
    {
        sampleBuffer[i] = 0;
    }

    initializeCalibration();
}

MQ135Sensor::~MQ135Sensor()
{
    delete[] sampleBuffer;
}

void MQ135Sensor::initializeCalibration()
{
    // MQ135 calibration values for different gases (empirical)
    nh3.a = 110.47f; // Ammonia
    nh3.b = -2.862f;
    nh3.name = "NH3";

    co.a = 100.0f; // Carbon Monoxide
    co.b = -2.75f;
    co.name = "CO";

    nox.a = 76.63f; // Nitrogen Oxides
    nox.b = -3.18f;
    nox.name = "NOx";

    alcohol.a = 102.2f; // Alcohol
    alcohol.b = -2.473f;
    alcohol.name = "Alcohol";

    smoke.a = 98.4f; // Smoke
    smoke.b = -2.862f;
    smoke.name = "Smoke";
}

bool MQ135Sensor::begin()
{
    pinMode(pin, INPUT);
    DEBUG_PRINTLN("[MQ135] MQ135 sensor initialized on pin " + String(pin));
    return true;
}

bool MQ135Sensor::readAirQuality()
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

    // Calculate resistance
    resistance = calculateResistance(rawValue);

    // Calculate PPM (using NH3 as default)
    if (r0 > 0.0f)
    {
        float ratio = resistance / r0;
        ppm = calculatePPM(ratio, nh3);
    }

    DEBUG_PRINT("[MQ135] Raw: " + String(rawValue) + ", Voltage: " + String(voltage, 2) + "V, ");
    DEBUG_PRINT("Resistance: " + String(resistance, 2) + "kΩ, PPM: " + String(ppm, 2));
    DEBUG_PRINTLN(", Level: " + getAirQualityLevel());

    return true;
}

int MQ135Sensor::getRawValue()
{
    return rawValue;
}

float MQ135Sensor::getVoltage()
{
    return voltage;
}

float MQ135Sensor::getResistance()
{
    return resistance;
}

float MQ135Sensor::getPPM()
{
    return ppm;
}

String MQ135Sensor::getAirQualityLevel()
{
    if (ppm < 50.0f)
        return "Excellent";
    else if (ppm < 100.0f)
        return "Good";
    else if (ppm < 200.0f)
        return "Moderate";
    else if (ppm < 500.0f)
        return "Poor";
    else
        return "Very Poor";
}

void MQ135Sensor::calibrateR0(float knownR0)
{
    if (knownR0 > 0.0f)
    {
        r0 = knownR0;
        DEBUG_PRINTLN("[MQ135] R0 calibrated to: " + String(r0, 2) + "kΩ");
    }
    else
    {
        // Calculate R0 in clean air (typically 10-20kΩ for MQ135)
        // This should be done in fresh air for accurate calibration
        float sum = 0.0f;
        for (int i = 0; i < 100; i++)
        {
            int raw = analogRead(pin);
            float voltage = (raw * 3.3f) / 4095.0f;
            float resistance = calculateResistance(raw);
            sum += resistance;
            delay(10);
        }

        r0 = sum / 100.0f;
        DEBUG_PRINTLN("[MQ135] R0 auto-calibrated to: " + String(r0, 2) + "kΩ");
    }
}

float MQ135Sensor::calculatePPM(float ratio, GasCalibration gas)
{
    // MQ135 formula: PPM = a * (Rs/R0)^b
    // Where Rs is current resistance, R0 is clean air resistance
    return gas.a * pow(ratio, gas.b);
}

float MQ135Sensor::getNH3PPM()
{
    if (r0 <= 0.0f)
        return 0.0f;
    float ratio = resistance / r0;
    return calculatePPM(ratio, nh3);
}

float MQ135Sensor::getCOPPM()
{
    if (r0 <= 0.0f)
        return 0.0f;
    float ratio = resistance / r0;
    return calculatePPM(ratio, co);
}

float MQ135Sensor::getNOxPPM()
{
    if (r0 <= 0.0f)
        return 0.0f;
    float ratio = resistance / r0;
    return calculatePPM(ratio, nox);
}

float MQ135Sensor::getAlcoholPPM()
{
    if (r0 <= 0.0f)
        return 0.0f;
    float ratio = resistance / r0;
    return calculatePPM(ratio, alcohol);
}

float MQ135Sensor::getSmokePPM()
{
    if (r0 <= 0.0f)
        return 0.0f;
    float ratio = resistance / r0;
    return calculatePPM(ratio, smoke);
}

float MQ135Sensor::calculateResistance(int rawValue)
{
    // Calculate sensor resistance using voltage divider formula
    // Assuming 10kΩ load resistor
    float vout = (rawValue * 3.3f) / 4095.0f;
    float rLoad = 10.0f; // Load resistor in kΩ

    if (vout >= 3.3f)
        return 0.0f; // Prevent division by zero

    return rLoad * (3.3f - vout) / vout;
}

int MQ135Sensor::getAverageRaw()
{
    long sum = 0;
    for (int i = 0; i < samples; i++)
    {
        sum += sampleBuffer[i];
    }

    return sum / samples;
}
