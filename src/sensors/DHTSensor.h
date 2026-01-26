/**
 * @file DHTSensor.h
 * @brief DHT22 Temperature and Humidity Sensor Interface
 */

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "../config.h"

class DHTSensor
{
private:
    DHT *dht;
    float lastTemp;
    float lastHumidity;
    uint32_t lastReadTime;
    bool initialized;

public:
    DHTSensor();
    ~DHTSensor();

    bool begin();
    bool read();
    float getTemperature();
    float getHumidity();
    float getHeatIndex();
    bool isAvailable();
};

#endif