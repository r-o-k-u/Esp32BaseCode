# ESP32 Sensor Control Guide

## Overview

This guide explains how to activate, deactivate, and configure sensors in your ESP32 DualComm System. The system supports multiple sensor types with real-time monitoring, data logging, and alert capabilities.

## Table of Contents

- [Sensor Types](#sensor-types)
- [Web Interface Control](#web-interface-control)
- [API Control](#api-control)
- [Programmatic Control](#programmatic-control)
- [Sensor Calibration](#sensor-calibration)
- [Data Monitoring](#data-monitoring)
- [Alert System](#alert-system)
- [Data Logging](#data-logging)
- [Troubleshooting](#troubleshooting)

## Sensor Types

The system supports 8 main sensor types:

### 1. DHT22 Temperature & Humidity Sensor
- **Pin**: GPIO 4
- **Type**: Digital sensor
- **Range**: Temperature: -40°C to 80°C, Humidity: 0-100%
- **Accuracy**: ±0.5°C, ±2-5% RH
- **Web Interface**: `/sensors.html` - Temperature & Humidity section

### 2. BMP280 Pressure & Altitude Sensor
- **Pins**: I2C (GPIO 21 SDA, GPIO 22 SCL)
- **Type**: I2C sensor
- **Range**: Pressure: 300-1100 hPa, Altitude: 0-3000m
- **Accuracy**: ±1 hPa, ±1m altitude
- **Web Interface**: `/sensors.html` - Pressure & Altitude section

### 3. LDR Light Sensor
- **Pin**: GPIO 34 (ADC1)
- **Type**: Analog sensor
- **Range**: 0-4095 (ADC reading)
- **Applications**: Light level detection, day/night sensing
- **Web Interface**: `/sensors.html` - Light Detection section

### 4. PIR Motion Sensor
- **Pin**: GPIO 19
- **Type**: Digital sensor
- **Range**: 3-7 meters detection
- **Applications**: Security, occupancy detection
- **Web Interface**: `/sensors.html` - Motion Detection section

### 5. HC-SR04 Ultrasonic Distance Sensor
- **Pins**: GPIO 5 (Trig), GPIO 18 (Echo)
- **Type**: Digital sensor
- **Range**: 2cm to 400cm
- **Accuracy**: ±3mm
- **Web Interface**: `/sensors.html` - Distance Measurement section

### 6. MQ135 Air Quality Sensor
- **Pin**: GPIO 32 (ADC1)
- **Type**: Analog sensor
- **Detects**: CO2, NH3, NOx, Alcohol, Benzene, Smoke
- **Applications**: Air quality monitoring
- **Web Interface**: `/sensors.html` - Air Quality section

### 7. Soil Moisture Sensor
- **Pin**: GPIO 35 (ADC1)
- **Type**: Analog sensor
- **Range**: 0-4095 (ADC reading)
- **Applications**: Plant monitoring, irrigation control
- **Web Interface**: `/sensors.html` - Soil Moisture section

### 8. MPU6050 IMU (Accelerometer + Gyroscope)
- **Pins**: I2C (GPIO 21 SDA, GPIO 22 SCL)
- **Type**: I2C sensor
- **Measures**: Acceleration, rotation, orientation
- **Applications**: Motion detection, tilt sensing
- **Web Interface**: `/sensors.html` - IMU section

## Web Interface Control

### Accessing Sensor Control

1. Connect to your ESP32's WiFi network or local network
2. Open browser and navigate to: `http://[ESP32_IP]/sensors.html`
3. Click the "Sensors" tab in the navigation menu

### Individual Sensor Control

#### Temperature & Humidity (DHT22)
```javascript
// Refresh sensor data
refreshSensor('dht');

// Calibrate sensor
calibrateSensor('dht');
```

**Web Interface Elements:**
- Real-time temperature and humidity display
- Heat index calculation
- Comfort level indicator
- Calibration button
- Refresh button

#### Pressure & Altitude (BMP280)
```javascript
// Refresh sensor data
refreshSensor('bmp');

// Calibrate sensor
calibrateSensor('bmp');
```

**Web Interface Elements:**
- Pressure reading in hPa
- Altitude calculation
- Sea level pressure
- Pressure trend indicator
- Calibration button

#### Light Detection (LDR)
```javascript
// Refresh sensor data
refreshSensor('light');
```

**Web Interface Elements:**
- Light level in lux
- Light state (Dark/Light)
- ADC value display
- Visual light bar indicator
- Percentage brightness display

#### Motion Detection (PIR)
```javascript
// Refresh sensor data
refreshSensor('motion');

// Reset motion count
resetMotionCount();

// Update sensitivity
updateMotionSensitivity(7);
```

**Web Interface Elements:**
- Motion state (Active/Inactive)
- Last trigger timestamp
- Motion detection count
- Sensitivity slider (1-10)
- Reset count button

#### Distance Measurement (HC-SR04)
```javascript
// Refresh sensor data
refreshSensor('distance');

// Calibrate sensor
calibrateSensor('ultrasonic');
```

**Web Interface Elements:**
- Distance in centimeters
- Object detection status
- Maximum range indicator
- Accuracy specification
- Calibration button

#### Air Quality (MQ135)
```javascript
// Refresh sensor data
refreshSensor('air');

// Re-calibrate sensor
calibrateAirSensor();
```

**Web Interface Elements:**
- PPM value display
- Air quality status
- ADC reading
- Calibration status
- Re-calibration button

#### Soil Moisture
```javascript
// Refresh sensor data
refreshSensor('soil');

// Calibrate sensor
calibrateSensor('soil');
```

**Web Interface Elements:**
- Moisture level percentage
- Soil condition indicator
- ADC value
- Irrigation status
- Calibration button

#### IMU (MPU6050)
```javascript
// Refresh sensor data
refreshSensor('imu');

// Calibrate sensor
calibrateSensor('imu');
```

**Web Interface Elements:**
- Acceleration values
- Gyroscope readings
- Orientation data
- IMU temperature
- Calibration button

## API Control

### REST API Endpoints

#### Get All Sensor Data
```http
GET /api/sensors
Content-Type: application/json

Response:
{
  "temperature": 25.5,
  "humidity": 60.2,
  "pressure": 1013.25,
  "altitude": 150.5,
  "motion": false,
  "lightLevel": 850,
  "distance": 15.2,
  "airQuality": 450,
  "soilMoisture": 750,
  "acceleration": {
    "x": 0.1,
    "y": 0.2,
    "z": 1.0
  },
  "gyroscope": {
    "x": 0.0,
    "y": 0.0,
    "z": 0.0
  }
}
```

#### Get Specific Sensor Data
```http
GET /api/sensors/temperature
GET /api/sensors/humidity
GET /api/sensors/pressure
GET /api/sensors/motion
GET /api/sensors/light
GET /api/sensors/distance
GET /api/sensors/air
GET /api/sensors/soil
GET /api/sensors/imu
```

#### Sensor Configuration
```http
POST /api/sensors/config
Content-Type: application/json

{
  "sensor": "motion",
  "config": {
    "sensitivity": 7,
    "detectionTimeout": 5000
  }
}
```

#### Sensor Calibration
```http
POST /api/sensors/calibrate
Content-Type: application/json

{
  "sensor": "dht",
  "calibration": {
    "temperatureOffset": -0.5,
    "humidityOffset": 2.0
  }
}
```

### WebSocket Control

#### Real-time Sensor Updates
```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Listen for sensor updates
ws.onmessage = function(event) {
  const data = JSON.parse(event.data);
  if (data.type === 'sensor') {
    console.log('Sensor update:', data);
    updateSensorDisplay(data);
  }
};

// Request specific sensor data
ws.send(JSON.stringify({
  type: 'getSensorData',
  sensor: 'temperature'
}));
```

#### Sensor Status Monitoring
```javascript
ws.onmessage = function(event) {
  const data = JSON.parse(event.data);
  if (data.type === 'sensorStatus') {
    console.log('Sensor status:', data);
    updateSensorStatus(data);
  }
};
```

## Programmatic Control

### C++ API (Arduino)

#### Basic Sensor Usage
```cpp
#include "SensorManager.h"
#include "DHTSensor.h"
#include "BMPSensor.h"

void setup() {
  // Initialize sensor manager
  sensorManager.begin();
  
  // Initialize specific sensors
  dhtSensor.begin();
  bmpSensor.begin();
}

void loop() {
  // Read all sensors
  if (dhtSensor.read()) {
    float temp = dhtSensor.getTemperature();
    float hum = dhtSensor.getHumidity();
    float heatIndex = dhtSensor.getHeatIndex();
    
    Serial.printf("Temp: %.1f°C, Hum: %.1f%%, Heat: %.1f°C\n", 
                  temp, hum, heatIndex);
  }
  
  // Read BMP sensor
  if (bmpSensor.read()) {
    float pressure = bmpSensor.getPressure();
    float altitude = bmpSensor.getAltitude();
    
    Serial.printf("Pressure: %.2f hPa, Altitude: %.2f m\n", 
                  pressure, altitude);
  }
  
  delay(2000); // Wait 2 seconds
}
```

#### Advanced Sensor Features
```cpp
// Get sensor health status
uint8_t sensorCount = sensorManager.getSensorCount();
bool dhtAvailable = dhtSensor.isAvailable();
bool bmpAvailable = bmpSensor.isAvailable();

// Get comprehensive sensor data
DynamicJsonDocument doc(1024);
sensorManager.getAllSensorData(doc);
serializeJson(doc, Serial);
Serial.println();

// Sensor calibration
dhtSensor.calibrate(TEMP_OFFSET, HUMIDITY_OFFSET);
bmpSensor.calibrate(SEA_LEVEL_PRESSURE);
```

### Sensor Reading Functions

#### DHT22 Temperature & Humidity
```cpp
// Read temperature
float temperature = dhtSensor.getTemperature();

// Read humidity
float humidity = dhtSensor.getHumidity();

// Read heat index
float heatIndex = dhtSensor.getHeatIndex();

// Check sensor availability
bool available = dhtSensor.isAvailable();
```

#### BMP280 Pressure & Altitude
```cpp
// Read pressure
float pressure = bmpSensor.getPressure();

// Read altitude
float altitude = bmpSensor.getAltitude();

// Read sea level pressure
float seaLevel = bmpSensor.getSeaLevelPressure();

// Check sensor availability
bool available = bmpSensor.isAvailable();
```

#### PIR Motion Sensor
```cpp
// Read motion state
bool motionDetected = pirSensor.isMotionDetected();

// Get motion count
uint32_t motionCount = pirSensor.getMotionCount();

// Reset motion count
pirSensor.resetMotionCount();
```

#### Ultrasonic Distance Sensor
```cpp
// Read distance
float distance = ultrasonicSensor.getDistance();

// Check object detection
bool objectDetected = ultrasonicSensor.isObjectDetected();

// Set maximum distance
ultrasonicSensor.setMaxDistance(400);
```

#### LDR Light Sensor
```cpp
// Read light level
int lightLevel = ldrSensor.getLightLevel();

// Read ADC value
int adcValue = ldrSensor.getADCValue();

// Get light state
bool isLight = ldrSensor.isLight();
```

## Sensor Calibration

### Automatic Calibration

#### DHT22 Calibration
```javascript
// Web interface
calibrateSensor('dht');

// API call
POST /api/sensors/calibrate
{
  "sensor": "dht",
  "calibration": {
    "temperatureOffset": -0.5,
    "humidityOffset": 2.0
  }
}
```

#### BMP280 Calibration
```javascript
// Web interface
calibrateSensor('bmp');

// API call
POST /api/sensors/calibrate
{
  "sensor": "bmp",
  "calibration": {
    "seaLevelPressure": 1013.25
  }
}
```

### Manual Calibration

#### Temperature Offset Calibration
```cpp
// Compare with reference thermometer
// Adjust offset in config.h
#define TEMP_OFFSET -0.5  // If sensor reads 0.5°C high
```

#### Humidity Offset Calibration
```cpp
// Compare with reference hygrometer
// Adjust offset in config.h
#define HUMIDITY_OFFSET 2.0  // If sensor reads 2% low
```

#### Air Quality Calibration
```javascript
// Calibrate in clean air environment
calibrateAirSensor();

// API call
POST /api/sensors/calibrate
{
  "sensor": "air",
  "calibration": {
    "cleanAirValue": 500
  }
}
```

## Data Monitoring

### Real-time Charts

#### Chart Configuration
```javascript
// Initialize chart
function initializeSensorCharts() {
  const ctx = document.getElementById('sensorChart').getContext('2d');
  sensorChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: [],
      datasets: [{
        label: 'Temperature (°C)',
        data: [],
        borderColor: '#4CAF50',
        backgroundColor: 'rgba(76, 175, 80, 0.1)',
        tension: 0.4,
        fill: true
      }]
    },
    options: {
      responsive: true,
      animation: {
        duration: 1000,
        easing: 'easeOutQuart'
      }
    }
  });
}
```

#### Chart Updates
```javascript
// Update chart with new data
function updateChart() {
  const sensorType = document.getElementById('chart-sensor').value;
  const now = new Date();
  
  sensorChart.data.labels.push(now.toLocaleTimeString());
  sensorChart.data.datasets[0].data.push(getCurrentSensorValue(sensorType));
  
  // Keep only last 20 data points
  if (sensorChart.data.labels.length > 20) {
    sensorChart.data.labels.shift();
    sensorChart.data.datasets[0].data.shift();
  }
  
  sensorChart.update();
}
```

### Data Export

#### JSON Export
```javascript
function exportSensorData() {
  const data = {
    timestamp: new Date().toISOString(),
    sensors: {
      temperature: document.getElementById('temperature-value').textContent,
      humidity: document.getElementById('humidity-value').textContent,
      pressure: document.getElementById('pressure-value').textContent,
      // ... other sensors
    }
  };
  
  const blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
  const url = window.URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `sensor_data_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.json`;
  document.body.appendChild(a);
  a.click();
  window.URL.revokeObjectURL(url);
}
```

#### CSV Export
```javascript
function exportSensorDataCSV() {
  const headers = ['Timestamp', 'Temperature', 'Humidity', 'Pressure', 'Light', 'Distance'];
  const data = [
    new Date().toISOString(),
    document.getElementById('temperature-value').textContent,
    document.getElementById('humidity-value').textContent,
    document.getElementById('pressure-value').textContent,
    document.getElementById('light-level').textContent,
    document.getElementById('distance-value').textContent
  ];
  
  const csv = headers.join(',') + '\n' + data.join(',');
  const blob = new Blob([csv], { type: 'text/csv' });
  const url = window.URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `sensor_data_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.csv`;
  document.body.appendChild(a);
  a.click();
  window.URL.revokeObjectURL(url);
}
```

## Alert System

### Alert Configuration

#### Temperature Alerts
```javascript
// Configure temperature alert thresholds
const tempAlertConfig = {
  min: 10,    // Minimum safe temperature
  max: 35,    // Maximum safe temperature
  enabled: true
};

// Save configuration
localStorage.setItem('tempAlertConfig', JSON.stringify(tempAlertConfig));
```

#### Humidity Alerts
```javascript
// Configure humidity alert thresholds
const humidityAlertConfig = {
  min: 30,    // Minimum safe humidity
  max: 70,    // Maximum safe humidity
  enabled: true
};
```

#### Motion Alerts
```javascript
// Configure motion alert settings
const motionAlertConfig = {
  sensitivity: 7,        // Detection sensitivity (1-10)
  timeout: 5000,         // Timeout between detections
  enabled: true
};
```

### Alert Processing

#### Real-time Alert Checking
```javascript
function checkSensorAlerts() {
  // Check temperature
  const temp = parseFloat(document.getElementById('temperature-value').textContent);
  const tempConfig = JSON.parse(localStorage.getItem('tempAlertConfig') || '{}');
  
  if (tempConfig.enabled && (temp < tempConfig.min || temp > tempConfig.max)) {
    triggerAlert('temperature', `Temperature out of range: ${temp}°C`);
  }
  
  // Check humidity
  const humidity = parseFloat(document.getElementById('humidity-value').textContent);
  const humidityConfig = JSON.parse(localStorage.getItem('humidityAlertConfig') || '{}');
  
  if (humidityConfig.enabled && (humidity < humidityConfig.min || humidity > humidityConfig.max)) {
    triggerAlert('humidity', `Humidity out of range: ${humidity}%`);
  }
  
  // Check motion
  const motionState = document.getElementById('motion-state').textContent;
  if (motionState === 'Active') {
    triggerAlert('motion', 'Motion detected');
  }
}
```

#### Alert Display
```javascript
function triggerAlert(type, message) {
  const alertsList = document.getElementById('alerts-list');
  const alertItem = document.createElement('div');
  alertItem.className = 'alert-item warning';
  alertItem.innerHTML = `
    <i class="fas fa-exclamation-triangle"></i>
    <div>
      <strong>${type.toUpperCase()} Alert</strong>
      <p>${message}</p>
    </div>
    <span class="alert-time">${new Date().toLocaleTimeString()}</span>
  `;
  
  alertsList.insertBefore(alertItem, alertsList.firstChild);
  
  // Update alert count
  const alertCount = document.getElementById('active-alerts');
  alertCount.textContent = parseInt(alertCount.textContent) + 1;
  
  // Show toast notification
  showToast(`${type} alert: ${message}`, 'warning');
}
```

## Data Logging

### Local Storage Logging

#### Log Entry Structure
```javascript
function addSensorLogEntry(type, message, value) {
  const logEntry = {
    timestamp: new Date().toISOString(),
    type: type,
    message: message,
    value: value,
    sensor: getCurrentSensor()
  };
  
  // Get existing logs
  let logs = JSON.parse(localStorage.getItem('sensorLogs') || '[]');
  
  // Add new entry
  logs.push(logEntry);
  
  // Keep only last 1000 entries
  if (logs.length > 1000) {
    logs = logs.slice(-1000);
  }
  
  // Save logs
  localStorage.setItem('sensorLogs', JSON.stringify(logs));
}
```

#### Log Retrieval
```javascript
function getSensorLogs(filter = {}) {
  const logs = JSON.parse(localStorage.getItem('sensorLogs') || '[]');
  
  if (filter.type) {
    return logs.filter(log => log.type === filter.type);
  }
  
  if (filter.date) {
    const date = new Date(filter.date);
    return logs.filter(log => new Date(log.timestamp).toDateString() === date.toDateString());
  }
  
  return logs;
}
```

### File System Logging

#### SPIFFS Logging
```cpp
#include "DataLogger.h"

void logSensorData() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount SPIFFS");
    return;
  }
  
  // Create log entry
  String logEntry = String(millis()) + "," +
                   String(dhtSensor.getTemperature()) + "," +
                   String(dhtSensor.getHumidity()) + "," +
                   String(bmpSensor.getPressure()) + "\n";
  
  // Write to file
  File file = SPIFFS.open("/sensor_log.csv", "a");
  if (file) {
    file.print(logEntry);
    file.close();
    Serial.println("Sensor data logged");
  } else {
    Serial.println("Failed to open log file");
  }
}
```

#### Log Rotation
```cpp
void rotateLogFiles() {
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  while (file) {
    String fileName = file.name();
    if (fileName.startsWith("/sensor_log_") && fileName.endsWith(".csv")) {
      // Check file size
      if (file.size() > MAX_LOG_SIZE) {
        // Rename with timestamp
        String newFileName = "/sensor_log_" + String(millis()) + ".csv";
        SPIFFS.rename(fileName, newFileName);
      }
    }
    file = root.openNextFile();
  }
}
```

## Troubleshooting

### Common Sensor Issues

#### DHT22 Not Reading
1. **Check Wiring**: Verify correct pin connections
2. **Check Power**: Ensure 3.3V or 5V power supply
3. **Check Pullup**: 10K resistor between DATA and VCC
4. **Check Code**: Verify DHT_PIN definition in config.h
5. **Check Timing**: DHT requires 2-second delay between reads

#### BMP280 Not Detected
1. **Check I2C**: Verify SDA and SCL connections
2. **Check Address**: BMP280 address is 0x76 or 0x77
3. **Check Power**: Ensure 3.3V power supply
4. **Check Library**: Install Adafruit_BMP280 library

#### PIR Sensor False Triggers
1. **Check Placement**: Avoid air vents, windows, moving objects
2. **Adjust Sensitivity**: Use potentiometer on sensor
3. **Check Power**: Ensure stable power supply
4. **Add Delay**: Implement debounce logic in code

#### LDR Sensor Inconsistent
1. **Check Circuit**: Verify voltage divider with 10K resistor
2. **Check ADC**: Use ADC1 pins (32-39) only
3. **Average Readings**: Take multiple samples and average
4. **Calibrate**: Compare with known light levels

#### Ultrasonic Sensor Inaccurate
1. **Check Wiring**: Verify Trig and Echo pins
2. **Check Range**: Objects must be 2-400cm away
3. **Check Surface**: Avoid soft/absorbent surfaces
4. **Add Delay**: Wait 60ms between readings

#### MQ135 Sensor Drifting
1. **Preheat Time**: Allow 24-48 hours for stabilization
2. **Clean Air Calibration**: Calibrate in fresh air
3. **Check Power**: Ensure stable 5V supply
4. **Avoid Contaminants**: Keep away from alcohol, cleaning agents

### Debug Mode

#### Enable Debug Output
```cpp
// In config.h
#define DEBUG_MODE true
#define DEBUG_SENSORS true
```

#### Debug Functions
```cpp
// Print sensor status
void printSensorStatus() {
  Serial.println("=== SENSOR STATUS ===");
  Serial.printf("DHT Available: %s\n", dhtSensor.isAvailable() ? "Yes" : "No");
  Serial.printf("BMP Available: %s\n", bmpSensor.isAvailable() ? "Yes" : "No");
  Serial.printf("PIR State: %s\n", pirSensor.isMotionDetected() ? "Active" : "Inactive");
  Serial.printf("LDR Value: %d\n", ldrSensor.getADCValue());
  Serial.printf("Ultrasonic Distance: %.2f cm\n", ultrasonicSensor.getDistance());
  Serial.println("=====================");
}
```

### Sensor Testing

#### Individual Sensor Tests
```javascript
// Test DHT sensor
function testDHT() {
  refreshSensor('dht');
  const temp = document.getElementById('temperature-value').textContent;
  const hum = document.getElementById('humidity-value').textContent;
  
  if (temp !== '-- °C' && hum !== '-- %') {
    showToast('DHT sensor working', 'success');
  } else {
    showToast('DHT sensor error', 'error');
  }
}

// Test all sensors
function testAllSensors() {
  const sensors = ['dht', 'bmp', 'light', 'motion', 'distance', 'air', 'soil', 'imu'];
  let workingCount = 0;
  
  sensors.forEach(sensor => {
    refreshSensor(sensor);
    setTimeout(() => {
      workingCount++;
      if (workingCount === sensors.length) {
        showToast(`All sensors tested: ${workingCount} working`, 'success');
      }
    }, 1000);
  });
}
```

### Power Considerations

#### Current Requirements
- **DHT22**: ~1mA
- **BMP280**: ~1mA
- **LDR**: ~0.1mA
- **PIR**: ~65μA (standby), ~1.5mA (active)
- **Ultrasonic**: ~15mA
- **MQ135**: ~150mA (heater), ~20mA (sensor)
- **Soil Moisture**: ~5mA
- **MPU6050**: ~3.6mA

#### Power Supply Recommendations
- **USB Power**: Good for testing, limited current
- **External 3.3V Supply**: Recommended for multiple sensors
- **Separate Power**: Use separate supplies for high-current sensors
- **Always common ground** between ESP32 and external power

## Best Practices

### 1. Sensor Initialization
```cpp
void setup() {
  // Initialize sensors in order
  dhtSensor.begin();
  bmpSensor.begin();
  pirSensor.begin();
  ultrasonicSensor.begin();
  
  // Check availability
  if (!dhtSensor.isAvailable()) {
    Serial.println("DHT sensor not detected!");
  }
}
```

### 2. Reading Frequency
```cpp
// DHT: Max 1 reading per 2 seconds
// BMP: Can read every 100ms
// PIR: Check every 100ms
// Ultrasonic: Wait 60ms between readings
// LDR: Can read every 100ms
// MQ135: Read every 1-2 seconds
// Soil: Read every 1-2 seconds
// MPU6050: Can read every 10ms
```

### 3. Error Handling
```cpp
// Always check for errors
if (!dhtSensor.read()) {
  Serial.println("Failed to read DHT sensor");
  return;
}

// Check for NaN values
if (isnan(temperature) || isnan(humidity)) {
  Serial.println("Invalid sensor reading");
  return;
}
```

### 4. Calibration
```cpp
// Calibrate sensors regularly
// Store calibration values in EEPROM
// Compare with reference instruments
// Update offsets as needed
```

### 5. Data Validation
```cpp
// Validate sensor readings
if (temperature < -40 || temperature > 80) {
  Serial.println("Temperature out of range");
  return;
}

if (humidity < 0 || humidity > 100) {
  Serial.println("Humidity out of range");
  return;
}
```

## Example Applications

### Home Environment Monitoring
```javascript
// Monitor home environment
function monitorHomeEnvironment() {
  const temp = parseFloat(document.getElementById('temperature-value').textContent);
  const hum = parseFloat(document.getElementById('humidity-value').textContent);
  const air = parseInt(document.getElementById('air-ppm').textContent);
  
  // Check comfort levels
  if (temp >= 20 && temp <= 25 && hum >= 40 && hum <= 60) {
    setEnvironmentStatus('Comfortable');
  } else {
    setEnvironmentStatus('Uncomfortable');
  }
  
  // Check air quality
  if (air > 1000) {
    triggerAlert('air', 'Poor air quality detected');
  }
}
```

### Plant Monitoring System
```javascript
// Monitor plant conditions
function monitorPlants() {
  const soil = parseInt(document.getElementById('soil-moisture').textContent);
  const light = parseInt(document.getElementById('light-level').textContent);
  
  // Check soil moisture
  if (soil < 30) {
    triggerAlert('soil', 'Plant needs watering');
    activateIrrigation(true);
  }
  
  // Check light levels
  if (light < 500) {
    triggerAlert('light', 'Insufficient light for plants');
  }
}
```

### Security System
```javascript
// Security monitoring
function monitorSecurity() {
  const motion = document.getElementById('motion-state').textContent;
  const distance = parseFloat(document.getElementById('distance-value').textContent);
  
  // Check for motion
  if (motion === 'Active') {
    triggerAlert('security', 'Motion detected');
    activateAlarm();
  }
  
  // Check for proximity
  if (distance > 0 && distance < 50) {
    triggerAlert('security', 'Object too close');
  }
}
```

### Weather Station
```javascript
// Weather monitoring
function monitorWeather() {
  const temp = parseFloat(document.getElementById('temperature-value').textContent);
  const hum = parseFloat(document.getElementById('humidity-value').textContent);
  const pressure = parseFloat(document.getElementById('pressure-value').textContent);
  
  // Calculate dew point
  const dewPoint = calculateDewPoint(temp, hum);
  
  // Check pressure trend
  const pressureTrend = checkPressureTrend();
  
  // Update weather status
  updateWeatherStatus(temp, hum, pressure, dewPoint, pressureTrend);
}
```

This guide provides comprehensive information for controlling sensors in your ESP32 system. Always follow safety guidelines and test thoroughly before deploying in production environments.