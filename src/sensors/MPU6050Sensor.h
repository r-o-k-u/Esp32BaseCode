/**
 * @file MPU6050Sensor.h
 * @brief MPU6050 Accelerometer and Gyroscope sensor control
 * @author Your Name
 * @version 2.0
 */

#ifndef MPU6050_SENSOR_H
#define MPU6050_SENSOR_H

#include "../config.h"
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

class MPU6050Sensor
{
private:
    MPU6050 mpu;
    float ax, ay, az; // Acceleration values
    float gx, gy, gz; // Gyroscope values
    float temp;       // Temperature
    bool initialized;

    // Calibration offsets
    float accelBias[3] = {0, 0, 0};
    float gyroBias[3] = {0, 0, 0};

public:
    MPU6050Sensor();

    bool begin();
    bool readSensors();
    bool calibrate();

    // Getters
    float getAccelX() { return ax; }
    float getAccelY() { return ay; }
    float getAccelZ() { return az; }
    float getGyroX() { return gx; }
    float getGyroY() { return gy; }
    float getGyroZ() { return gz; }
    float getTemperature() { return temp; }

    // Calculate derived values
    float getPitch();
    float getRoll();
    float getHeading();
    String getOrientation();

private:
    void applyCalibration();
    void calculateOrientation();
};

#endif // MPU6050_SENSOR_H
