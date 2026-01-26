/**
 * @file DHTSensor.cpp
 * @brief DHT22 sensor implementation
 */

#include "DHTSensor.h"

DHTSensor::DHTSensor()
{
    dht = nullptr;
    lastTemp = 0;
    lastHumidity = 0;
    lastReadTime = 0;
    initialized = false;
}

DHTSensor::~DHTSensor()
{
    if (dht)
        delete dht;
}

bool DHTSensor::begin()
{
    DEBUG_PRINT("Initializing DHT sensor on pin ");
    DEBUG_PRINTLN(DHT_PIN);

    dht = new DHT(DHT_PIN, DHT_TYPE);
    dht->begin();

    delay(2000); // DHT needs time to stabilize

    // Test read
    float t = dht->readTemperature();
    if (isnan(t))
    {
        DEBUG_PRINTLN("DHT sensor not detected!");
        initialized = false;
        return false;
    }

    initialized = true;
    DEBUG_PRINTLN("DHT sensor ready!");
    return true;
}

bool DHTSensor::read()
{
    if (!initialized)
        return false;

    // Don't read more than once every 2 seconds
    if (millis() - lastReadTime < 2000)
        return true;

    float temp = dht->readTemperature() + TEMP_OFFSET;
    float hum = dht->readHumidity() + HUMIDITY_OFFSET;

    if (isnan(temp) || isnan(hum))
    {
        DEBUG_PRINTLN("Failed to read from DHT sensor!");
        return false;
    }

    lastTemp = temp;
    lastHumidity = hum;
    lastReadTime = millis();

#if DEBUG_SENSORS
    DEBUG_PRINTF("DHT - Temp: %.1f°C, Humidity: %.1f%%\n", temp, hum);
#endif

    return true;
}

float DHTSensor::getTemperature()
{
    return lastTemp;
}

float DHTSensor::getHumidity()
{
    return lastHumidity;
}

float DHTSensor::getHeatIndex()
{
    if (!initialized)
        return 0;
    return dht->computeHeatIndex(lastTemp, lastHumidity, false);
}

bool DHTSensor::isAvailable()
{
    return initialized;
}