/**
 * @file UltrasonicSensor.cpp
 * @brief Ultrasonic sensor implementation
 */

#include "UltrasonicSensor.h"
#include "../config.h"

UltrasonicSensor::UltrasonicSensor()
{
    trigPin = ULTRASONIC_TRIG;
    echoPin = ULTRASONIC_ECHO;
    maxDistance = ULTRASONIC_MAX_DISTANCE;
    lastDistance = 0;
    initialized = false;
}

bool UltrasonicSensor::begin(uint8_t trig, uint8_t echo)
{
    trigPin = trig;
    echoPin = echo;

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    digitalWrite(trigPin, LOW);
    delay(100);

    // Test measurement
    uint16_t distance = read();
    if (distance > 0 && distance < maxDistance)
    {
        initialized = true;
        DEBUG_PRINTLN("Ultrasonic sensor ready!");
        return true;
    }

    DEBUG_PRINTLN("Ultrasonic sensor not detected!");
    return false;
}

long UltrasonicSensor::measurePulse()
{
    // Send 10us pulse
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo pulse duration (timeout after 30ms)
    return pulseIn(echoPin, HIGH, 30000);
}

uint16_t UltrasonicSensor::read()
{
    if (!initialized)
        return 0;

    // Take multiple measurements and average
    long duration = 0;
    int validReadings = 0;

    for (int i = 0; i < 3; i++)
    {
        long d = measurePulse();
        if (d > 0)
        {
            duration += d;
            validReadings++;
        }
        delay(10);
    }

    if (validReadings == 0)
    {
        DEBUG_PRINTLN("No valid ultrasonic readings");
        return lastDistance; // Return last valid reading
    }

    duration /= validReadings;

    // Calculate distance in cm (speed of sound = 343 m/s)
    // distance = (duration * 0.0343) / 2
    uint16_t distance = duration * 0.0343 / 2;

    if (distance > 0 && distance < maxDistance)
    {
        lastDistance = distance;

#if DEBUG_SENSORS
        DEBUG_PRINTF("Ultrasonic distance: %d cm\n", distance);
#endif
    }

    return lastDistance;
}

uint16_t UltrasonicSensor::getDistance()
{
    return lastDistance;
}

bool UltrasonicSensor::isAvailable()
{
    return initialized;
}
