# ESP32 DualComm System - Examples

This document provides practical examples and use cases for the ESP32 DualComm System, demonstrating how to integrate and use various sensors, actuators, and communication protocols.

## Table of Contents

- [Basic Setup Examples](#basic-setup-examples)
- [Sensor Integration Examples](#sensor-integration-examples)
- [Actuator Control Examples](#actuator-control-examples)
- [Camera Examples](#camera-examples)
- [Communication Examples](#communication-examples)
- [Advanced Projects](#advanced-projects)
- [Troubleshooting Examples](#troubleshooting-examples)

## Basic Setup Examples

### Example 1: Basic System Initialization

```cpp
#include "WiFiManager.h"
#include "SensorManager.h"
#include "ActuatorManager.h"
#include "WebServer.h"
#include "OTAManager.h"

void setup() {
    // Initialize system
    Serial.begin(115200);
    
    // Initialize WiFi
    WiFiManager wifi;
    wifi.begin("MyWiFi", "password123");
    
    // Initialize sensors
    SensorManager sensors;
    sensors.begin();
    
    // Initialize actuators
    ActuatorManager actuators;
    actuators.begin();
    
    // Start web server
    WebServer webServer;
    webServer.begin();
    
    // Initialize OTA
    OTAManager ota;
    ota.begin("esp32-device", "ota-password");
    
    Serial.println("ESP32 DualComm System initialized!");
}

void loop() {
    // Main loop
    sensors.update();
    actuators.update();
    webServer.handleClient();
    ota.handle();
}
```

### Example 2: Configuration Management

```cpp
#include "config.h"

// Load configuration from file or use defaults
void loadConfiguration() {
    // Load WiFi settings
    String ssid = CONFIG_WIFI_SSID;
    String password = CONFIG_WIFI_PASSWORD;
    
    // Load sensor pins
    int dhtPin = CONFIG_DHT_PIN;
    int bmpSDA = CONFIG_BMP_SDA_PIN;
    int bmpSCL = CONFIG_BMP_SCL_PIN;
    
    // Load actuator pins
    int ledPin = CONFIG_LED_PIN;
    int buzzerPin = CONFIG_BUZZER_PIN;
    
    // Apply configuration
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Initialize sensors with configured pins
    dht.setup(dhtPin);
    bmp.begin(bmpSDA, bmpSCL);
    
    // Initialize actuators with configured pins
    led.begin(ledPin);
    buzzer.begin(buzzerPin);
}
```

## Sensor Integration Examples

### Example 1: Environmental Monitoring

```cpp
#include "DHTSensor.h"
#include "BMPSensor.h"
#include "LDRSensor.h"

DHTSensor dht;
BMPSensor bmp;
LDRSensor ldr;

void setup() {
    dht.begin(4);  // DHT on pin 4
    bmp.begin(21, 22);  // BMP on I2C pins
    ldr.begin(34);  // LDR on pin 34
}

void loop() {
    // Read DHT sensor
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    float heatIndex = dht.getHeatIndex();
    
    // Read BMP sensor
    float pressure = bmp.getPressure();
    float altitude = bmp.getAltitude();
    
    // Read LDR sensor
    float lux = ldr.getLux();
    String lightState = ldr.getLightState();
    
    // Print readings
    Serial.print("Temp: "); Serial.print(temperature);
    Serial.print("°C, Hum: "); Serial.print(humidity);
    Serial.print("%, Pressure: "); Serial.print(pressure);
    Serial.print("hPa, Light: "); Serial.print(lux);
    Serial.println(" lux");
    
    delay(2000);
}
```

### Example 2: Motion Detection and Security

```cpp
#include "PIRSensor.h"
#include "UltrasonicSensor.h"
#include "BuzzerController.h"
#include "LEDController.h"

PIRSensor pir;
UltrasonicSensor ultrasonic;
BuzzerController buzzer;
LEDController led;

void setup() {
    pir.begin(13);  // PIR on pin 13
    ultrasonic.begin(5, 18);  // Ultrasonic on pins 5 & 18
    buzzer.begin(25);  // Buzzer on pin 25
    led.begin(2);  // LED on pin 2
    
    // Set up interrupt for PIR
    attachInterrupt(digitalPinToInterrupt(13), motionDetected, RISING);
}

void loop() {
    // Check ultrasonic distance
    float distance = ultrasonic.getDistance();
    
    if (distance < 50.0 && distance > 0) {
        // Object too close - trigger alarm
        triggerAlarm();
    }
    
    delay(100);
}

void motionDetected() {
    Serial.println("Motion detected!");
    triggerAlarm();
}

void triggerAlarm() {
    // Flash LED
    led.setState(true);
    delay(500);
    led.setState(false);
    delay(500);
    
    // Sound buzzer
    buzzer.beep(1000, 1000);  // 1kHz for 1 second
    
    // Log event
    Serial.println("Security alert triggered!");
}
```

### Example 3: Air Quality Monitoring

```cpp
#include "MQ135Sensor.h"
#include "RGBLEDController.h"

MQ135Sensor mq135;
RGBLEDController rgb;

void setup() {
    mq135.begin(32);  // MQ135 on pin 32
    rgb.begin(26, 27, 14);  // RGB on pins 26, 27, 14
}

void loop() {
    float ppm = mq135.getPPM();
    String airQuality = mq135.getAirQuality();
    
    // Set RGB color based on air quality
    if (airQuality == "excellent") {
        rgb.setColor(0, 255, 0);  // Green
    } else if (airQuality == "good") {
        rgb.setColor(255, 255, 0);  // Yellow
    } else if (airQuality == "poor") {
        rgb.setColor(255, 165, 0);  // Orange
    } else {
        rgb.setColor(255, 0, 0);  // Red
    }
    
    Serial.print("Air Quality: "); Serial.print(airQuality);
    Serial.print(", PPM: "); Serial.println(ppm);
    
    delay(1000);
}
```

## Actuator Control Examples

### Example 1: Smart Lighting System

```cpp
#include "LDRSensor.h"
#include "RGBLEDController.h"
#include "Timer.h"

LDRSensor ldr;
RGBLEDController rgb;
Timer lightTimer;

void setup() {
    ldr.begin(34);
    rgb.begin(26, 27, 14);
    lightTimer.start();
}

void loop() {
    float lux = ldr.getLux();
    
    // Automatic lighting based on ambient light
    if (lux < 50) {
        // It's dark - turn on lights
        if (!lightTimer.isRunning() || lightTimer.getElapsedTime() > 60000) {
            // Gradual lighting effect
            rgb.transitionToColor(255, 100, 0, 3000);  // Warm white over 3 seconds
            lightTimer.start();
        }
    } else {
        // It's bright - turn off lights
        rgb.setColor(0, 0, 0);
        lightTimer.stop();
    }
    
    // Schedule lighting scenes
    unsigned long currentTime = millis();
    
    if (currentTime % 86400000 < 1000) {  // Daily at midnight
        morningRoutine();
    } else if (currentTime % 86400000 > 64800000) {  // Evening at 6 PM
        eveningRoutine();
    }
    
    delay(100);
}

void morningRoutine() {
    // Gradual wake-up light
    rgb.transitionToColor(255, 100, 0, 10000);  // Sunrise effect
    delay(10000);
}

void eveningRoutine() {
    // Relaxing evening light
    rgb.setColorHSV(30, 50, 100);  // Warm orange
}
```

### Example 2: Motor Control with Feedback

```cpp
#include "MotorController.h"
#include "UltrasonicSensor.h"
#include "Timer.h"

MotorController motor;
UltrasonicSensor ultrasonic;
Timer motorTimer;

void setup() {
    motor.begin(14, 12, 13);  // IN1, IN2, EN pins
    ultrasonic.begin(5, 18);
    motorTimer.start();
}

void loop() {
    float distance = ultrasonic.getDistance();
    
    if (distance > 0 && distance < 100) {
        // Object detected - stop motor
        motor.stop();
        motorTimer.stop();
        Serial.println("Object detected - motor stopped");
    } else if (distance > 100 && !motorTimer.isRunning()) {
        // Safe to move - start motor
        motor.forward();
        motor.setSpeed(75);
        motorTimer.start();
        Serial.println("Motor started");
    }
    
    // Emergency stop after 30 seconds
    if (motorTimer.isRunning() && motorTimer.getElapsedTime() > 30000) {
        motor.emergencyStop();
        Serial.println("Motor stopped - timeout");
    }
    
    delay(100);
}
```

### Example 3: Servo Automation

```cpp
#include "ServoController.h"
#include "Timer.h"

ServoController servo;
Timer servoTimer;

void setup() {
    servo.begin(15);  // Servo on pin 15
    servoTimer.start();
}

void loop() {
    unsigned long currentTime = millis();
    
    // Automated window control based on time
    if (currentTime % 86400000 < 1000) {  // Morning at midnight
        openWindow();
    } else if (currentTime % 86400000 > 64800000) {  // Evening at 6 PM
        closeWindow();
    }
    
    // Manual override with button press
    if (digitalRead(2) == HIGH) {  // Button on pin 2
        toggleWindow();
        delay(500);  // Debounce
    }
    
    delay(100);
}

void openWindow() {
    servo.sweepTo(180, 2000);  // Open over 2 seconds
    Serial.println("Window opened");
}

void closeWindow() {
    servo.sweepTo(0, 2000);  // Close over 2 seconds
    Serial.println("Window closed");
}

void toggleWindow() {
    int currentAngle = servo.getAngle();
    if (currentAngle < 90) {
        openWindow();
    } else {
        closeWindow();
    }
}
```

## Camera Examples

### Example 1: Basic Camera Capture

```cpp
#include "CameraManager.h"

CameraManager camera;

void setup() {
    Serial.begin(115200);
    
    // Initialize camera
    camera.begin();
    camera.setQuality(10);
    camera.setFrameSize(5);  // QVGA
    camera.setBrightness(0);
    camera.setContrast(0);
}

void loop() {
    // Capture image every 10 seconds
    if (millis() % 10000 < 100) {
        String filename = camera.captureImage();
        if (filename != "") {
            Serial.println("Image captured: " + filename);
        } else {
            Serial.println("Failed to capture image");
        }
    }
    
    delay(100);
}
```

### Example 2: Motion-Activated Camera

```cpp
#include "CameraManager.h"
#include "PIRSensor.h"
#include "MotionDetector.h"

CameraManager camera;
PIRSensor pir;
MotionDetector motionDetector;

void setup() {
    camera.begin();
    pir.begin(13);
    motionDetector.begin();
    
    // Configure camera for motion detection
    camera.setQuality(8);
    camera.setFrameSize(8);  // SVGA for better detail
    motionDetector.setThreshold(50);
    motionDetector.setSensitivity(5);
}

void loop() {
    // Check for motion
    if (pir.isMotionDetected()) {
        // Capture image on motion
        String filename = camera.captureImage();
        if (filename != "") {
            Serial.println("Motion detected! Image captured: " + filename);
            
            // Optional: Send notification
            sendNotification("Motion detected at " + String(millis()));
        }
    }
    
    // Also check camera-based motion detection
    if (motionDetector.isMotionDetected()) {
        String filename = camera.captureImage();
        Serial.println("Camera motion detected! Image: " + filename);
    }
    
    delay(100);
}

void sendNotification(String message) {
    // Implementation depends on your notification method
    // Could be email, SMS, push notification, etc.
    Serial.println("NOTIFICATION: " + message);
}
```

### Example 3: Time-Lapse Photography

```cpp
#include "CameraManager.h"
#include "Timer.h"

CameraManager camera;
Timer captureTimer;

void setup() {
    camera.begin();
    camera.setQuality(15);
    camera.setFrameSize(5);  // QVGA for time-lapse
    
    // Start time-lapse every 30 seconds
    captureTimer.start();
    Serial.println("Time-lapse photography started");
}

void loop() {
    // Capture frame every 30 seconds
    if (captureTimer.getElapsedTime() > 30000) {
        String filename = camera.captureImage();
        if (filename != "") {
            Serial.println("Time-lapse frame: " + filename);
        }
        captureTimer.reset();
    }
    
    // Stop after 1 hour (3600000ms)
    if (captureTimer.getTotalTime() > 3600000) {
        Serial.println("Time-lapse completed");
        while (true);  // Stop
    }
    
    delay(100);
}
```

## Communication Examples

### Example 1: ESP-NOW Sensor Network

```cpp
#include "ESPNowComm.h"
#include "SensorManager.h"

ESPNowComm espNow;
SensorManager sensors;

void setup() {
    Serial.begin(115200);
    
    // Initialize ESP-NOW
    espNow.begin();
    
    // Add peer
    uint8_t peerMac[] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};
    espNow.addPeer(peerMac);
    
    // Initialize sensors
    sensors.begin();
}

void loop() {
    // Read sensors
    sensors.update();
    
    // Send sensor data every 5 seconds
    if (millis() % 5000 < 100) {
        SensorData data = sensors.getLatestData();
        
        // Create message
        String message = "TEMP:" + String(data.temperature) + 
                        ",HUM:" + String(data.humidity) +
                        ",PRES:" + String(data.pressure);
        
        // Send via ESP-NOW
        bool success = espNow.sendMessage(peerMac, message.c_str());
        if (success) {
            Serial.println("Sensor data sent: " + message);
        } else {
            Serial.println("Failed to send sensor data");
        }
    }
    
    delay(100);
}

// Callback for received messages
void onMessageReceived(const uint8_t *mac, const uint8_t *data, int len) {
    String message = String((char*)data);
    Serial.println("Received: " + message);
    
    // Parse and act on received data
    if (message.startsWith("ALERT:")) {
        // Handle alert from peer
        handleAlert(message);
    }
}

void handleAlert(String alert) {
    // Implementation depends on alert type
    Serial.println("ALERT: " + alert);
}
```

### Example 2: MQTT Home Automation

```cpp
#include "WiFiManager.h"
#include "MQTTManager.h"
#include "ActuatorManager.h"

WiFiManager wifi;
MQTTManager mqtt;
ActuatorManager actuators;

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    wifi.begin("MyWiFi", "password123");
    
    // Initialize MQTT
    mqtt.begin("mqtt.example.com", 1883, "esp32-home");
    mqtt.subscribe("home/livingroom/light");
    mqtt.subscribe("home/temperature/set");
    
    // Initialize actuators
    actuators.begin();
    
    // Set callback for MQTT messages
    mqtt.setCallback(mqttCallback);
}

void loop() {
    // Handle MQTT
    mqtt.loop();
    
    // Send sensor data every 10 seconds
    if (millis() % 10000 < 100) {
        sendSensorData();
    }
    
    delay(100);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    if (topicStr == "home/livingroom/light") {
        handleLightControl(message);
    } else if (topicStr == "home/temperature/set") {
        handleTemperatureSet(message);
    }
}

void handleLightControl(String command) {
    if (command == "ON") {
        actuators.getLED()->setState(true);
        mqtt.publish("home/livingroom/light/status", "ON");
    } else if (command == "OFF") {
        actuators.getLED()->setState(false);
        mqtt.publish("home/livingroom/light/status", "OFF");
    } else if (command.startsWith("COLOR:")) {
        // Parse color: COLOR:255,128,0
        String colorStr = command.substring(6);
        int r = colorStr.substring(0, colorStr.indexOf(',')).toInt();
        colorStr = colorStr.substring(colorStr.indexOf(',') + 1);
        int g = colorStr.substring(0, colorStr.indexOf(',')).toInt();
        int b = colorStr.substring(colorStr.indexOf(',') + 1).toInt();
        
        actuators.getRGBLED()->setColor(r, g, b);
        mqtt.publish("home/livingroom/light/status", "COLOR:" + String(r) + "," + String(g) + "," + String(b));
    }
}

void handleTemperatureSet(String tempStr) {
    float targetTemp = tempStr.toFloat();
    // Implementation depends on your heating/cooling system
    Serial.println("Set temperature to: " + tempStr + "°C");
}

void sendSensorData() {
    // Get sensor readings
    float temp = sensors.getTemperature();
    float hum = sensors.getHumidity();
    
    // Publish to MQTT
    mqtt.publish("home/sensors/temperature", String(temp));
    mqtt.publish("home/sensors/humidity", String(hum));
}
```

### Example 3: Serial Communication Bridge

```cpp
#include "SerialBridge.h"
#include "SensorManager.h"
#include "ActuatorManager.h"

SerialBridge serialBridge;
SensorManager sensors;
ActuatorManager actuators;

void setup() {
    Serial.begin(115200);
    Serial2.begin(9600);  // Hardware serial for external device
    
    serialBridge.begin(&Serial2);
    sensors.begin();
    actuators.begin();
}

void loop() {
    // Handle serial communication
    serialBridge.loop();
    
    // Send sensor data every second
    if (millis() % 1000 < 100) {
        sendSensorData();
    }
    
    delay(10);
}

void sendSensorData() {
    SensorData data = sensors.getLatestData();
    
    // Format: TEMP=25.5,HUM=60.2,PRESS=1013.25\n
    String message = "TEMP=" + String(data.temperature, 1) +
                    ",HUM=" + String(data.humidity, 1) +
                    ",PRESS=" + String(data.pressure, 2) + "\n";
    
    serialBridge.send(message);
}

// Handle received serial commands
void onSerialCommand(String command) {
    if (command.startsWith("LED:")) {
        String state = command.substring(4);
        if (state == "ON") {
            actuators.getLED()->setState(true);
        } else if (state == "OFF") {
            actuators.getLED()->setState(false);
        }
    } else if (command.startsWith("MOTOR:")) {
        String params = command.substring(6);
        // Parse: MOTOR:SPEED,DIRECTION
        int commaPos = params.indexOf(',');
        int speed = params.substring(0, commaPos).toInt();
        String direction = params.substring(commaPos + 1);
        
        if (direction == "FORWARD") {
            actuators.getMotor()->forward();
        } else if (direction == "REVERSE") {
            actuators.getMotor()->reverse();
        }
        actuators.getMotor()->setSpeed(speed);
    } else if (command == "STATUS?") {
        sendStatus();
    }
}

void sendStatus() {
    String status = "STATUS:";
    status += "LED=" + String(actuators.getLED()->getState() ? "ON" : "OFF") + ",";
    status += "MOTOR=" + String(actuators.getMotor()->getSpeed()) + ",";
    status += "SENSORS=OK\n";
    
    serialBridge.send(status);
}
```

## Advanced Projects

### Project 1: Smart Garden System

```cpp
#include "SoilMoistureSensor.h"
#include "WaterPumpController.h"
#include "DHTSensor.h"
#include "LDRSensor.h"
#include "Timer.h"

SoilMoistureSensor soil;
WaterPumpController pump;
DHTSensor dht;
LDRSensor ldr;
Timer wateringTimer;

void setup() {
    soil.begin(35);
    pump.begin(23);  // Relay controlling water pump
    dht.begin(4);
    ldr.begin(34);
    
    Serial.println("Smart Garden System started");
}

void loop() {
    // Read sensors
    float moisture = soil.getMoisturePercentage();
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    float light = ldr.getLux();
    
    // Automatic watering based on soil moisture
    if (moisture < 30.0 && !wateringTimer.isRunning()) {
        startWatering();
    }
    
    // Stop watering after 5 minutes or if moisture is sufficient
    if (wateringTimer.isRunning()) {
        if (wateringTimer.getElapsedTime() > 300000 || moisture > 60.0) {
            stopWatering();
        }
    }
    
    // Log data every 5 minutes
    if (millis() % 300000 < 100) {
        logGardenData(moisture, temperature, humidity, light);
    }
    
    delay(1000);
}

void startWatering() {
    pump.start();
    wateringTimer.start();
    Serial.println("Starting watering - soil dry");
}

void stopWatering() {
    pump.stop();
    wateringTimer.stop();
    Serial.println("Stopping watering - soil moist");
}

void logGardenData(float moisture, float temp, float hum, float light) {
    Serial.print("Garden Status - Moisture: ");
    Serial.print(moisture);
    Serial.print("%, Temp: ");
    Serial.print(temp);
    Serial.print("°C, Hum: ");
    Serial.print(hum);
    Serial.print("%, Light: ");
    Serial.print(light);
    Serial.println(" lux");
}
```

### Project 2: Home Security System

```cpp
#include "PIRSensor.h"
#include "UltrasonicSensor.h"
#include "CameraManager.h"
#include "BuzzerController.h"
#include "RGBLEDController.h"
#include "WiFiManager.h"
#include "MQTTManager.h"

PIRSensor pir;
UltrasonicSensor ultrasonic;
CameraManager camera;
BuzzerController buzzer;
RGBLEDController rgb;
WiFiManager wifi;
MQTTManager mqtt;

void setup() {
    // Initialize sensors
    pir.begin(13);
    ultrasonic.begin(5, 18);
    
    // Initialize actuators
    camera.begin();
    buzzer.begin(25);
    rgb.begin(26, 27, 14);
    
    // Initialize communication
    wifi.begin("MyWiFi", "password123");
    mqtt.begin("mqtt.example.com", 1883, "esp32-security");
    mqtt.subscribe("security/arm");
    mqtt.setCallback(securityCallback);
    
    Serial.println("Home Security System armed");
}

void loop() {
    // Check motion sensors
    if (pir.isMotionDetected() || ultrasonic.getDistance() < 100) {
        triggerAlarm();
    }
    
    // Handle MQTT messages
    mqtt.loop();
    
    delay(100);
}

void triggerAlarm() {
    static bool alarmTriggered = false;
    if (!alarmTriggered) {
        alarmTriggered = true;
        
        // Capture image
        String filename = camera.captureImage();
        
        // Sound alarm
        buzzer.playMelody();
        
        // Flash lights
        rgb.rainbowCycle();
        
        // Send notification
        String message = "ALERT: Motion detected at " + String(millis());
        mqtt.publish("security/alert", message);
        
        if (filename != "") {
            mqtt.publish("security/image", filename);
        }
        
        Serial.println("SECURITY ALERT: Motion detected!");
    }
}

void securityCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    if (topicStr == "security/arm") {
        if (message == "DISARM") {
            Serial.println("Security system disarmed");
            // Implementation to disarm system
        } else if (message == "ARM") {
            Serial.println("Security system armed");
            // Implementation to arm system
        }
    }
}
```

### Project 3: Weather Station

```cpp
#include "DHTSensor.h"
#include "BMPSensor.h"
#include "LDRSensor.h"
#include "MQ135Sensor.h"
#include "WiFiManager.h"
#include "WebServer.h"
#include "DataLogger.h"

DHTSensor dht;
BMPSensor bmp;
LDRSensor ldr;
MQ135Sensor mq135;
WiFiManager wifi;
WebServer webServer;
DataLogger logger;

void setup() {
    // Initialize sensors
    dht.begin(4);
    bmp.begin(21, 22);
    ldr.begin(34);
    mq135.begin(32);
    
    // Initialize communication
    wifi.begin("MyWiFi", "password123");
    webServer.begin();
    logger.begin();
    
    Serial.println("Weather Station started");
}

void loop() {
    // Read all sensors
    float temp = dht.getTemperature();
    float hum = dht.getHumidity();
    float pressure = bmp.getPressure();
    float altitude = bmp.getAltitude();
    float lux = ldr.getLux();
    float ppm = mq135.getPPM();
    String airQuality = mq135.getAirQuality();
    
    // Log data
    logger.logWeatherData(temp, hum, pressure, altitude, lux, ppm, airQuality);
    
    // Update web server
    webServer.updateWeatherData(temp, hum, pressure, lux, airQuality);
    
    // Send to external service every 5 minutes
    if (millis() % 300000 < 100) {
        sendToWeatherService(temp, hum, pressure, lux, ppm);
    }
    
    delay(10000);  // Read every 10 seconds
}

void sendToWeatherService(float temp, float hum, float pressure, float lux, float ppm) {
    // Implementation depends on your weather service API
    Serial.print("Sending to weather service: ");
    Serial.print(temp); Serial.print("C, ");
    Serial.print(hum); Serial.print("%, ");
    Serial.print(pressure); Serial.print("hPa, ");
    Serial.print(lux); Serial.print("lux, ");
    Serial.print(ppm); Serial.println("ppm");
}
```

## Troubleshooting Examples

### Example 1: Sensor Calibration

```cpp
#include "SensorManager.h"

SensorManager sensors;

void setup() {
    Serial.begin(115200);
    sensors.begin();
    
    // Calibrate all sensors
    calibrateSensors();
}

void loop() {
    // Normal operation
    sensors.update();
    delay(1000);
}

void calibrateSensors() {
    Serial.println("Starting sensor calibration...");
    
    // Calibrate DHT (usually not needed, but can be done)
    Serial.println("DHT calibration: Ensure sensor is in stable environment");
    delay(5000);
    
    // Calibrate BMP for current altitude
    float currentAltitude = 100.0;  // Your known altitude
    float seaLevelPressure = sensors.getBMP()->seaLevelForAltitude(currentAltitude, 
                                                                   sensors.getBMP()->readPressure());
    Serial.print("BMP calibration: Sea level pressure = ");
    Serial.println(seaLevelPressure);
    
    // Calibrate LDR for current light conditions
    Serial.println("LDR calibration: Measuring current light levels...");
    int avgReading = 0;
    for (int i = 0; i < 10; i++) {
        avgReading += sensors.getLDR()->getADCValue();
        delay(100);
    }
    avgReading /= 10;
    Serial.print("LDR average reading: ");
    Serial.println(avgReading);
    
    // Calibrate MQ135 for clean air
    Serial.println("MQ135 calibration: Expose to clean air for 20 seconds...");
    delay(20000);
    float r0 = sensors.getMQ135()->calibrate();
    Serial.print("MQ135 R0 value: ");
    Serial.println(r0);
    
    Serial.println("Sensor calibration complete!");
}
```

### Example 2: Network Troubleshooting

```cpp
#include "WiFiManager.h"
#include "ESPNowComm.h"

WiFiManager wifi;
ESPNowComm espNow;

void setup() {
    Serial.begin(115200);
    
    // Test WiFi connection
    testWiFiConnection();
    
    // Test ESP-NOW
    testESPNow();
}

void loop() {
    // Check connection status
    checkConnections();
    delay(5000);
}

void testWiFiConnection() {
    Serial.println("Testing WiFi connection...");
    
    // Try multiple SSIDs
    String ssids[] = {"HomeWiFi", "MobileHotspot", "GuestWiFi"};
    String passwords[] = {"password1", "password2", "password3"};
    
    for (int i = 0; i < 3; i++) {
        Serial.print("Trying SSID: ");
        Serial.println(ssids[i]);
        
        if (wifi.begin(ssids[i].c_str(), passwords[i].c_str())) {
            Serial.println("WiFi connected successfully!");
            Serial.print("IP Address: ");
            Serial.println(WiFi.localIP());
            return;
        }
        
        delay(2000);
    }
    
    Serial.println("WiFi connection failed - using AP mode");
    wifi.createAP("ESP32_AP", "12345678");
}

void testESPNow() {
    Serial.println("Testing ESP-NOW...");
    
    if (espNow.begin()) {
        Serial.println("ESP-NOW initialized successfully");
        
        // Test peer connection
        uint8_t testMac[] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56};
        if (espNow.addPeer(testMac)) {
            Serial.println("ESP-NOW peer added successfully");
        } else {
            Serial.println("Failed to add ESP-NOW peer");
        }
    } else {
        Serial.println("ESP-NOW initialization failed");
    }
}

void checkConnections() {
    Serial.println("\n--- Connection Status ---");
    
    // WiFi status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("WiFi: Connected (");
        Serial.print(WiFi.SSID());
        Serial.print(") RSSI: ");
        Serial.println(WiFi.RSSI());
    } else {
        Serial.println("WiFi: Not connected");
    }
    
    // ESP-NOW status
    Serial.print("ESP-NOW: ");
    Serial.println(espNow.isConnected() ? "Connected" : "Disconnected");
    
    // IP address
    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
}
```

### Example 3: Power Management

```cpp
#include "PowerManager.h"
#include "SensorManager.h"
#include "ActuatorManager.h"

PowerManager power;
SensorManager sensors;
ActuatorManager actuators;

void setup() {
    Serial.begin(115200);
    
    // Initialize power management
    power.begin();
    power.setLowPowerMode(false);  // Start in normal mode
    
    sensors.begin();
    actuators.begin();
}

void loop() {
    // Check battery level
    float batteryLevel = power.getBatteryLevel();
    Serial.print("Battery: ");
    Serial.print(batteryLevel, 2);
    Serial.println("%");
    
    // Enter low power mode if battery is low
    if (batteryLevel < 20.0) {
        enterLowPowerMode();
    } else if (batteryLevel > 80.0) {
        exitLowPowerMode();
    }
    
    // Normal operation
    if (!power.isInLowPowerMode()) {
        sensors.update();
        actuators.update();
        delay(1000);
    } else {
        // Low power operation - minimal sensor reading
        sensors.readCriticalSensors();
        delay(10000);  // Longer delay in low power mode
    }
}

void enterLowPowerMode() {
    if (!power.isInLowPowerMode()) {
        Serial.println("Entering low power mode...");
        power.setLowPowerMode(true);
        
        // Reduce sensor update frequency
        sensors.setUpdateInterval(5000);
        
        // Turn off non-essential actuators
        actuators.turnOffNonEssential();
        
        // Reduce WiFi power
        WiFi.setSleep(true);
    }
}

void exitLowPowerMode() {
    if (power.isInLowPowerMode()) {
        Serial.println("Exiting low power mode...");
        power.setLowPowerMode(false);
        
        // Restore normal sensor update frequency
        sensors.setUpdateInterval(1000);
        
        // Restore actuators
        actuators.restoreState();
        
        // Restore WiFi power
        WiFi.setSleep(false);
    }
}
```

## Best Practices

### 1. Error Handling

```cpp
// Always check for errors
bool initializeSensor() {
    if (!sensor.begin()) {
        Serial.println("Failed to initialize sensor");
        return false;
    }
    
    if (!sensor.isReady()) {
        Serial.println("Sensor not ready");
        return false;
    }
    
    return true;
}

// Use timeouts for operations
bool waitForSensorReady(unsigned long timeout = 5000) {
    unsigned long startTime = millis();
    while (!sensor.isReady()) {
        if (millis() - startTime > timeout) {
            Serial.println("Sensor timeout");
            return false;
        }
        delay(100);
    }
    return true;
}
```

### 2. Power Optimization

```cpp
// Use deep sleep when possible
void enterDeepSleep() {
    // Save critical data
    saveState();
    
    // Configure wake-up sources
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 1);  // PIR sensor
    esp_sleep_enable_timer_wakeup(60000000);       // 1 minute
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

// Minimize power consumption
void minimizePower() {
    // Turn off unused peripherals
    btStop();           // Bluetooth
    WiFi.disconnect();  // WiFi (if not needed)
    
    // Reduce CPU frequency
    setCpuFrequencyMhz(80);
    
    // Use efficient data types
    uint8_t smallVar = 0;  // Instead of int
}
```

### 3. Data Validation

```cpp
// Validate sensor readings
float validateTemperature(float temp) {
    if (temp < -40.0 || temp > 85.0) {
        Serial.println("Invalid temperature reading");
        return NAN;
    }
    return temp;
}

// Check for sensor errors
bool checkSensorHealth() {
    float temp = dht.getTemperature();
    float hum = dht.getHumidity();
    
    if (isnan(temp) || isnan(hum)) {
        Serial.println("DHT sensor error");
        return false;
    }
    
    return true;
}
```

### 4. Memory Management

```cpp
// Use dynamic allocation carefully
void manageMemory() {
    // Use static allocation when possible
    static char buffer[256];
    
    // Free dynamic memory
    char* dynamicBuffer = new char[100];
    // ... use buffer ...
    delete[] dynamicBuffer;
    
    // Check available memory
    Serial.print("Free heap: ");
    Serial.println(ESP.getFreeHeap());
}
```

These examples provide a solid foundation for building various IoT projects with the ESP32 DualComm System. Adapt and combine these examples based on your specific requirements and use cases.
