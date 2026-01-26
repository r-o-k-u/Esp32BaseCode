/**
 * @file MPU6050Sensor.cpp
 * @brief MPU6050 Accelerometer and Gyroscope sensor implementation
 * @author Your Name
 * @version 2.0
 */

#include "MPU6050Sensor.h"
#include "../utils/Logger.h"

MPU6050Sensor::MPU6050Sensor() : mpu(), initialized(false)
{
    ax = ay = az = 0.0f;
    gx = gy = gz = 0.0f;
    temp = 0.0f;
}

bool MPU6050Sensor::begin()
{
    Wire.begin(I2C_SDA, I2C_SCL);

    // Initialize MPU6050
    mpu.initialize();

    // Check connection
    if (!mpu.testConnection())
    {
        DEBUG_PRINTLN("[MPU6050] MPU6050 connection failed");
        return false;
    }

    // Configure MPU6050
    mpu.setFullScaleAccelRange(0); // 2g range
    mpu.setFullScaleGyroRange(0);  // 250 deg/s range
    mpu.setDLPFMode(3);            // 44Hz bandwidth

    initialized = true;
    DEBUG_PRINTLN("[MPU6050] MPU6050 sensor initialized successfully");

    return true;
}

bool MPU6050Sensor::readSensors()
{
    if (!initialized)
    {
        DEBUG_PRINTLN("[MPU6050] Sensor not initialized");
        return false;
    }

    // Read raw values
    int16_t ax_raw, ay_raw, az_raw;
    int16_t gx_raw, gy_raw, gz_raw;
    int16_t temp_raw;

    mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);
    temp_raw = mpu.getTemperature();

    // Convert to g and deg/s
    ax = ax_raw / 16384.0f;
    ay = ay_raw / 16384.0f;
    az = az_raw / 16384.0f;

    gx = gx_raw / 131.0f;
    gy = gy_raw / 131.0f;
    gz = gz_raw / 131.0f;

    // Convert temperature
    temp = temp_raw / 340.0f + 36.53f;

    // Apply calibration
    applyCalibration();

    DEBUG_PRINT("[MPU6050] Acc: " + String(ax, 2) + "," + String(ay, 2) + "," + String(az, 2) + "g ");
    DEBUG_PRINT("Gyro: " + String(gx, 1) + "," + String(gy, 1) + "," + String(gz, 1) + "dps ");
    DEBUG_PRINTLN("Temp: " + String(temp, 1) + "C");

    return true;
}

bool MPU6050Sensor::calibrate()
{
    if (!initialized)
    {
        DEBUG_PRINTLN("[MPU6050] Sensor not initialized");
        return false;
    }

    DEBUG_PRINTLN("[MPU6050] Starting calibration...");

    // Reset biases
    for (int i = 0; i < 3; i++)
    {
        accelBias[i] = 0.0f;
        gyroBias[i] = 0.0f;
    }

    // Collect samples for calibration
    const int samples = 100;
    float ax_sum = 0, ay_sum = 0, az_sum = 0;
    float gx_sum = 0, gy_sum = 0, gz_sum = 0;

    for (int i = 0; i < samples; i++)
    {
        readSensors();

        ax_sum += ax;
        ay_sum += ay;
        az_sum += az;

        gx_sum += gx;
        gy_sum += gy;
        gz_sum += gz;

        delay(10);
    }

    // Calculate average biases
    accelBias[0] = ax_sum / samples;
    accelBias[1] = ay_sum / samples;
    accelBias[2] = (az_sum / samples) - 1.0f; // Subtract gravity from Z

    gyroBias[0] = gx_sum / samples;
    gyroBias[1] = gy_sum / samples;
    gyroBias[2] = gz_sum / samples;

    DEBUG_PRINTLN("[MPU6050] Calibration complete");
    DEBUG_PRINT("[MPU6050] Acc bias: " + String(accelBias[0], 3) + "," + String(accelBias[1], 3) + "," + String(accelBias[2], 3));
    DEBUG_PRINTLN(" Gyro bias: " + String(gyroBias[0], 1) + "," + String(gyroBias[1], 1) + "," + String(gyroBias[2], 1));

    return true;
}

float MPU6050Sensor::getPitch()
{
    return atan2(ay, az) * 180.0f / PI;
}

float MPU6050Sensor::getRoll()
{
    return atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;
}

float MPU6050Sensor::getHeading()
{
    // For basic heading calculation using accelerometer only
    // More accurate heading requires magnetometer
    return atan2(ay, ax) * 180.0f / PI;
}

String MPU6050Sensor::getOrientation()
{
    float pitch = getPitch();
    float roll = getRoll();

    if (abs(pitch) > 45.0f)
    {
        if (pitch > 0)
            return "Tilted Forward";
        else
            return "Tilted Backward";
    }
    else if (abs(roll) > 45.0f)
    {
        if (roll > 0)
            return "Tilted Right";
        else
            return "Tilted Left";
    }
    else
    {
        return "Level";
    }
}

void MPU6050Sensor::applyCalibration()
{
    ax -= accelBias[0];
    ay -= accelBias[1];
    az -= accelBias[2];

    gx -= gyroBias[0];
    gy -= gyroBias[1];
    gz -= gyroBias[2];
}

void MPU6050Sensor::calculateOrientation()
{
    // This could implement sensor fusion algorithms like Kalman filter
    // For now, using simple trigonometric calculations
}
