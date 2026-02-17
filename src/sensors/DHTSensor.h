/**
 * @file DHTSensor.h
 * @brief DHT11 Temperature and Humidity Sensor Interface
 */

#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H

#include <Arduino.h>
#include "../config.h"

// Include DHT library first to capture its definitions
#include <DHT.h>

// Undefine DEBUG_PRINT and DEBUG_PRINTLN if already defined by DHT library
#ifdef DEBUG_PRINT
#undef DEBUG_PRINT
#endif
#ifdef DEBUG_PRINTLN
#undef DEBUG_PRINTLN
#endif

// Re-define our debug macros after undefining DHT library's
#ifndef DEBUG_PRINT
#define DEBUG_PRINT(x) Serial.print(x)
#endif
#ifndef DEBUG_PRINTLN
#define DEBUG_PRINTLN(x) Serial.println(x)
#endif

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