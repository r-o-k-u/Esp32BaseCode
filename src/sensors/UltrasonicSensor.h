/**
 * @file UltrasonicSensor.h
 * @brief HC-SR04 ultrasonic distance sensor interface
 * @author Your Name
 * @version 2.0
 */

#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include <Arduino.h>

class UltrasonicSensor
{
private:
    uint8_t trigPin;
    uint8_t echoPin;
    uint16_t maxDistance;
    uint16_t lastDistance;
    bool initialized;

    long measurePulse();

public:
    UltrasonicSensor();
    bool begin(uint8_t trig, uint8_t echo);
    uint16_t read();
    uint16_t getDistance();
    bool isAvailable();
};

#endif // ULTRASONIC_SENSOR_H
