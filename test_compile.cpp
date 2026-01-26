// Simple compilation test
#include <Arduino.h>

// Test basic includes
#include "src/config.h"
#include "src/core/WiFiManager.h"
#include "src/core/WebServer.h"
#include "src/actuators/ActuatorManager.h"
#include "src/sensors/SensorManager.h"

void setup()
{
    // Test basic functionality
    Serial.begin(115200);
    delay(1000);

    WiFiManager wifi;
    WebServerManager web;
    ActuatorManager actuators;
    SensorManager sensors;

    Serial.println("Compilation test successful!");
}

void loop()
{
    // Empty loop
}
