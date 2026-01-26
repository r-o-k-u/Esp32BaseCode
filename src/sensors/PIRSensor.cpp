/**
 * @file PIRSensor.cpp
 * @brief Passive Infrared (PIR) motion sensor implementation
 * @author Your Name
 * @version 2.0
 */

#include "PIRSensor.h"
#include "../utils/Logger.h"

// Static member initialization
PIRSensor *PIRSensor::instance = nullptr;

PIRSensor::PIRSensor(uint8_t sensorPin, unsigned long debounceMs)
    : pin(sensorPin), motionDetected(false), lastMotionTime(0), debounceTime(debounceMs)
{
    instance = this;
}

bool PIRSensor::begin()
{
    pinMode(pin, INPUT);

    // Attach interrupt for motion detection
    attachInterrupt(digitalPinToInterrupt(pin), motionDetectedISR, RISING);

    DEBUG_PRINTLN("[PIR] PIR sensor initialized on pin " + String(pin));
    return true;
}

bool PIRSensor::readMotion()
{
    // Read the current state
    bool currentMotion = digitalRead(pin) == HIGH;

    if (currentMotion && !motionDetected)
    {
        // New motion detected
        motionDetected = true;
        lastMotionTime = millis();

        DEBUG_PRINTLN("[PIR] Motion detected!");

        // Trigger interrupt-style callback
        motionDetectedISR();
    }
    else if (!currentMotion && motionDetected)
    {
        // Motion ended
        motionDetected = false;
        DEBUG_PRINTLN("[PIR] Motion ended");
    }

    return motionDetected;
}

bool PIRSensor::isMotionDetected()
{
    return motionDetected;
}

unsigned long PIRSensor::getLastMotionTime()
{
    return lastMotionTime;
}

void PIRSensor::resetMotion()
{
    motionDetected = false;
    lastMotionTime = 0;
}

void IRAM_ATTR PIRSensor::motionDetectedISR()
{
    if (instance != nullptr)
    {
        instance->motionDetected = true;
        instance->lastMotionTime = millis();

        // Log motion detection
        DEBUG_PRINTLN("[PIR] Motion detected via interrupt!");
    }
}
