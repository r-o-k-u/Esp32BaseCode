/**
 * ═══════════════════════════════════════════════════════════════════════════
 * BMP280 SENSOR - COMPLETE IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file BMPSensor.cpp
 * @brief Complete implementation of BMP280 pressure sensor
 * @version 2.0.0
 * @date 2024
 */

#include "BMPSensor.h"

/**
 * @brief Constructor
 */
BMPSensor::BMPSensor(uint8_t addr)
{
    bmp = nullptr;
    lastTemperature = 0;
    lastPressure = 0;
    lastAltitude = 0;
    seaLevelPressure = 1013.25; // Standard atmosphere
    i2cAddress = addr;
    initialized = false;
    lastReadTime = 0;
    readCount = 0;
    errorCount = 0;
}

/**
 * @brief Destructor
 */
BMPSensor::~BMPSensor()
{
    if (bmp)
        delete bmp;
}

/**
 * @brief Initialize BMP280 sensor
 */
bool BMPSensor::begin(uint8_t sdaPin, uint8_t sclPin)
{
    Serial.println("Initializing BMP280...");

    // Initialize I2C
    Wire.begin(sdaPin, sclPin);
    delay(100);

    // Create BMP280 object
    bmp = new Adafruit_BMP280(&Wire);

    // Try to initialize sensor
    if (!bmp->begin(i2cAddress))
    {
        Serial.printf("BMP280 not found at address 0x%02X\n", i2cAddress);

        // Try alternate address
        if (i2cAddress == 0x76)
        {
            Serial.println("Trying alternate address 0x77...");
            if (bmp->begin(0x77))
            {
                i2cAddress = 0x77;
                Serial.println("✓ Found BMP280 at 0x77");
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    // Configure sensor for weather station use
    bmp->setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating mode
                     Adafruit_BMP280::SAMPLING_X2,     // Temp oversampling
                     Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                     Adafruit_BMP280::FILTER_X16,      // Filtering
                     Adafruit_BMP280::STANDBY_MS_500); // Standby time

    // Test read
    delay(100);
    if (read())
    {
        initialized = true;
        Serial.println("✓ BMP280 initialized successfully");
        Serial.printf("  Address: 0x%02X\n", i2cAddress);
        Serial.printf("  Temperature: %.1f°C\n", lastTemperature);
        Serial.printf("  Pressure: %.1f hPa\n", lastPressure);
        return true;
    }

    Serial.println("✗ BMP280 test read failed");
    return false;
}

/**
 * @brief Read sensor values
 */
bool BMPSensor::read()
{
    if (!initialized || !bmp)
    {
        errorCount++;
        return false;
    }

    // Read temperature
    float temp = bmp->readTemperature();
    if (isnan(temp) || temp < -40 || temp > 85)
    {
        errorCount++;
        return false;
    }
    lastTemperature = temp;

    // Read pressure
    float pressure = bmp->readPressure() / 100.0F; // Convert Pa to hPa
    if (isnan(pressure) || pressure < 300 || pressure > 1100)
    {
        errorCount++;
        return false;
    }
    lastPressure = pressure;

    // Calculate altitude
    lastAltitude = calculateAltitude(lastPressure, seaLevelPressure);

    lastReadTime = millis();
    readCount++;

    return true;
}

/**
 * @brief Calculate altitude from pressure
 */
float BMPSensor::calculateAltitude(float pressure, float seaLevel)
{
    // International barometric formula
    // altitude = 44330 * (1.0 - pow(pressure/seaLevel, 0.1903))
    return 44330.0 * (1.0 - pow(pressure / seaLevel, 0.1903));
}

/**
 * @brief Get temperature
 */
float BMPSensor::getTemperature()
{
    return lastTemperature;
}

/**
 * @brief Get pressure
 */
float BMPSensor::getPressure()
{
    return lastPressure;
}

/**
 * @brief Get altitude
 */
float BMPSensor::getAltitude()
{
    return lastAltitude;
}

/**
 * @brief Set sea level pressure
 */
void BMPSensor::setSeaLevelPressure(float pressure)
{
    seaLevelPressure = pressure;
    Serial.printf("Sea level pressure set to: %.2f hPa\n", pressure);
}

/**
 * @brief Configure sampling
 */
void BMPSensor::configureSampling(BMPOversampling tempSampling,
                                  BMPOversampling pressSampling,
                                  uint8_t filter)
{
    if (!initialized || !bmp)
        return;

    bmp->setSampling(Adafruit_BMP280::MODE_NORMAL,
                     (Adafruit_BMP280::sensor_sampling)tempSampling,
                     (Adafruit_BMP280::sensor_sampling)pressSampling,
                     (Adafruit_BMP280::sensor_filter)filter,
                     Adafruit_BMP280::STANDBY_MS_500);

    Serial.println("BMP280 sampling configured");
}

/**
 * @brief Get time since last read
 */
uint32_t BMPSensor::getTimeSinceLastRead()
{
    return millis() - lastReadTime;
}

/**
 * @brief Get success rate
 */
float BMPSensor::getSuccessRate()
{
    if (readCount == 0)
        return 100.0;
    return ((float)(readCount - errorCount) / readCount) * 100.0;
}

/**
 * @brief Reset statistics
 */
void BMPSensor::resetStatistics()
{
    readCount = 0;
    errorCount = 0;
}

/**
 * @brief Self-test
 */
bool BMPSensor::selfTest()
{
    if (!initialized)
        return false;

    Serial.println("Running BMP280 self-test...");

    // Perform multiple reads
    bool allPassed = true;
    for (int i = 0; i < 5; i++)
    {
        if (!read())
        {
            allPassed = false;
            Serial.printf("  Test %d: FAILED\n", i + 1);
        }
        else
        {
            Serial.printf("  Test %d: OK (T=%.1f°C, P=%.1fhPa)\n",
                          i + 1, lastTemperature, lastPressure);
        }
        delay(100);
    }

    if (allPassed)
    {
        Serial.println("✓ Self-test PASSED");
    }
    else
    {
        Serial.println("✗ Self-test FAILED");
    }

    return allPassed;
}

/**
 * @brief Get status string
 */
String BMPSensor::getStatusString()
{
    String status = "BMP280: ";
    if (initialized)
    {
        status += "Active, ";
        status += String(lastPressure, 1) + " hPa, ";
        status += String(lastTemperature, 1) + "°C";
    }
    else
    {
        status += "Inactive";
    }
    return status;
}

/**
 * @brief Print sensor info
 */
void BMPSensor::printInfo()
{
    Serial.println("┌─────────────────────────────────────────────────┐");
    Serial.println("│          BMP280 SENSOR INFO                     │");
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.printf("│ Status:         %-28s │\n", initialized ? "Active" : "Inactive");
    Serial.printf("│ I2C Address:    0x%-26X │\n", i2cAddress);
    Serial.printf("│ Temperature:    %-23.1f°C │\n", lastTemperature);
    Serial.printf("│ Pressure:       %-22.1f hPa │\n", lastPressure);
    Serial.printf("│ Altitude:       %-24.1f m │\n", lastAltitude);
    Serial.printf("│ Sea Level:      %-22.1f hPa │\n", seaLevelPressure);
    Serial.println("├─────────────────────────────────────────────────┤");
    Serial.printf("│ Reads:          %-28u │\n", readCount);
    Serial.printf("│ Errors:         %-28u │\n", errorCount);
    Serial.printf("│ Success Rate:   %-23.1f%% │\n", getSuccessRate());
    Serial.println("└─────────────────────────────────────────────────┘");
}
