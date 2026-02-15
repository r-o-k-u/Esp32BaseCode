# ESP32 Dual Communication System with GPIOViewer

A comprehensive IoT system that combines WiFi connectivity, ESP-NOW peer-to-peer communication, real-time web monitoring, and GPIO visualization for ESP32 and ESP32-CAM devices.

## 📋 Table of Contents

- [System Overview](#system-overview)
- [Key Features](#key-features)
- [System Architecture](#system-architecture)
- [Hardware Requirements](#hardware-requirements)
- [Installation & Setup](#installation--setup)
- [Configuration](#configuration)
- [Usage Guide](#usage-guide)
- [GPIOViewer Integration](#gpioviewer-integration)
- [API Reference](#api-reference)
- [Troubleshooting](#troubleshooting)
- [Development](#development)
- [License](#license)

## 🎯 System Overview

This project implements a sophisticated ESP32-based IoT system that provides:

- **Dual Communication**: WiFi for internet connectivity + ESP-NOW for device-to-device communication
- **Real-time Monitoring**: Web-based dashboard with live sensor data and GPIO monitoring
- **Multi-device Support**: Network of ESP32 devices that can communicate and share data
- **Comprehensive I/O**: Support for sensors, actuators, and camera (ESP32-CAM)
- **Remote Management**: OTA updates and web-based configuration

## ⭐ Key Features

### Core Functionality
- ✅ **WiFi Connectivity**: Connects to existing WiFi networks or creates access point
- ✅ **ESP-NOW Communication**: Peer-to-peer communication between ESP32 devices
- ✅ **Web Server**: Real-time web dashboard accessible via browser
- ✅ **OTA Updates**: Over-the-air firmware updates
- ✅ **Data Logging**: Persistent storage of sensor data to SPIFFS

### GPIOViewer Integration
- ✅ **Real-time GPIO Monitoring**: Live visualization of all GPIO pin states
- ✅ **Pin Function Detection**: Automatic detection of pin usage (PWM, ADC, digital)
- ✅ **Peripheral Conflict Detection**: Identifies pins used by I2C, SPI, UART peripherals
- ✅ **Web-based Interface**: Access GPIO monitoring via web browser on port 8080
- ✅ **mDNS Support**: Accessible via `gpioviewer.local:8080`

### Sensor & Actuator Support
- ✅ **Multi-sensor Support**: DHT11/DHT22, BMP280, MQ135, LDR, MPU6050, PIR, Ultrasonic, Soil Moisture
- ✅ **Actuator Control**: Relays, servos, L298N motor drivers, RGB LEDs, buzzers
- ✅ **Camera Support**: ESP32-CAM integration with photo capture
- ✅ **WebSocket Communication**: Real-time data streaming to web interface

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        MAIN CONTROLLER                          │
│                        (main.cpp)                               │
└────────┬────────────────────────────────────────────┬──────────┘
         │                                             │
┌────────▼────────┐                          ┌────────▼────────┐
│   CORE MODULES  │                          │  HARDWARE I/O   │
│  - WiFi Manager │                          │   - Sensors     │
│  - OTA Manager  │                          │   - Actuators   │
│  - Web Server   │                          │   - Camera      │
│  - ESP-NOW Comm │                          │   - GPIOViewer  │
│  - Data Logger  │                          │                 │
└─────────────────┘                          └─────────────────┘
         │                                             │
         └─────────────────────────────────────────────┘
                           │
                ┌──────────▼──────────┐
                │   WEB INTERFACE     │
                │   (Real-time UI)    │
                └─────────────────────┘
```

### Communication Flow

1. **Setup Phase**: All subsystems initialize in sequence
2. **Sensor Reading**: Every 2 seconds (configurable)
3. **Data Distribution**: 
   - Web clients via WebSocket
   - ESP-NOW peers via broadcast
   - SPIFFS logging
4. **Actuator Control**: Via web interface or ESP-NOW commands
5. **Status Updates**: Every 5 seconds to peers and web clients

## 🛠️ Hardware Requirements

### Required Components
- **ESP32 Development Board** (any model)
- **ESP32-CAM** (optional, for camera functionality)
- **USB Cable** for programming and power

### Optional Sensors & Actuators
- **Sensors**: DHT11/DHT22, BMP280, MQ135, LDR, MPU6050, PIR, Ultrasonic, Soil Moisture
- **Actuators**: Relays, Servos, DC Motors, RGB LEDs, Buzzers
- **Communication**: I2C devices, SPI devices, UART devices

### Power Requirements
- **USB Power**: 5V/2A minimum recommended
- **External Power**: 3.3V or 5V depending on components
- **Battery Operation**: Supported with appropriate voltage regulation

## 📦 Installation & Setup

### Prerequisites
1. **PlatformIO**: Install PlatformIO Core or VS Code with PlatformIO extension
2. **ESP32 Board Support**: Ensure ESP32 boards are installed in Arduino IDE/PlatformIO
3. **Required Libraries**: All libraries are automatically installed via PlatformIO

### Quick Start

1. **Clone the Repository**
   ```bash
   git clone <repository-url>
   cd ESP32-Dual-Communication-System
   ```

2. **Install Dependencies**
   ```bash
   pio lib install
   ```

3. **Configure WiFi and Credentials**
   - Edit `src/credentials_template.h` and save as `src/credentials.h`
   - Add your WiFi SSID/password and device configurations

4. **Upload to ESP32**
   ```bash
   pio run --target upload
   ```

5. **Upload Filesystem**
   ```bash
   pio run --target uploadfs
   ```

## ⚙️ Configuration

### Main Configuration (`src/config.h`)
```cpp
// Device Configuration
#define DEVICE_TYPE 0           // 0=ESP32, 1=ESP32-CAM
#define DEVICE_NAME "ESP32-Node1"

// Communication Settings
#define ENABLE_ESPNOW true      // Enable ESP-NOW communication
#define ENABLE_WEBSERVER true   // Enable web server
#define ENABLE_OTA true         // Enable OTA updates

// Sensor/Actuator Settings
#define ENABLE_SENSORS true
#define ENABLE_ACTUATORS true
#define ENABLE_CAMERA true      // ESP32-CAM only

// Timing Configuration
#define SENSOR_READ_INTERVAL 2000   // 2 seconds
#define STATUS_UPDATE_INTERVAL 5000 // 5 seconds
```

### WiFi Configuration (`src/credentials.h`)
```cpp
// WiFi Settings
#define WIFI_SSID_1 "YourWiFiSSID"
#define WIFI_PASS_1 "YourWiFiPassword"

// Access Point Settings (fallback)
#define AP_SSID "ESP32-AP"
#define AP_PASSWORD "12345678"

// OTA Settings
#define OTA_HOSTNAME "esp32-node1"
#define OTA_PASSWORD "ota-password"

// ESP-NOW Peer Configuration
uint8_t PEER_MAC_1[] = {0x24, 0x6F, 0x28, 0x12, 0x34, 0x56};
```

### GPIO Pin Configuration
Each sensor/actuator module has its own configuration:
- **DHT Sensor**: `DHT_PIN` (default: 4)
- **BMP280**: Uses I2C (pins 21, 22)
- **Relay**: `RELAY_PIN_1` (default: 5)
- **Servo**: `SERVO_PIN_1` (default: 13)
- **L298N Motor Driver**: `MOTOR1_IN1` (14), `MOTOR1_IN2` (33), `MOTOR1_EN` (32)

## 🚀 Usage Guide

### Initial Setup
1. **Power On**: Connect ESP32 to power via USB
2. **Serial Monitor**: Open Serial Monitor at 115200 baud
3. **Wait for Boot**: System will initialize all components
4. **Check Status**: Look for "🚀 SYSTEM READY 🚀" message

### Web Interface Access
1. **Find IP Address**: Check Serial Monitor for IP address
2. **Open Browser**: Navigate to `http://[IP_ADDRESS]`
3. **GPIOViewer**: Access GPIO monitoring at `http://[IP_ADDRESS]:8080`

### ESP-NOW Network Setup
1. **Configure Peers**: Add peer MAC addresses in `credentials.h`
2. **Power On All Devices**: All ESP32 devices in the network
3. **Check Connectivity**: Serial monitor shows peer status
4. **Data Sharing**: Sensors automatically share data between devices

### Sensor Data Monitoring
- **Real-time Updates**: Data updates every 2 seconds
- **WebSocket Streaming**: Live data to web interface
- **ESP-NOW Broadcasting**: Data shared with peer devices
- **Data Logging**: Automatic logging to SPIFFS filesystem

### Actuator Control
- **Web Interface**: Control relays, servos, LEDs via web UI
- **ESP-NOW Commands**: Send commands from peer devices
- **Real-time Response**: Immediate feedback on actuator state

## 🔌 GPIOViewer Integration

### What is GPIOViewer?
GPIOViewer is a real-time GPIO monitoring system that provides:
- Live visualization of all GPIO pin states
- Automatic detection of pin functions (PWM, ADC, digital)
- Conflict detection with peripheral devices
- Web-based interface accessible on port 8080

### How It Works

#### 1. **Pin State Monitoring**
```cpp
// GPIOViewer continuously monitors all pins
// Updates every 100ms (configurable)
// Detects: HIGH/LOW, PWM duty cycle, ADC values
```

#### 2. **Pin Function Detection**
- **PWM Pins**: Detects pins with PWM signals (LED dimming, servo control)
- **ADC Pins**: Identifies analog input pins (sensors)
- **Digital Pins**: Standard digital I/O pins
- **Peripheral Pins**: Pins used by I2C, SPI, UART (skipped to avoid conflicts)

#### 3. **Conflict Avoidance**
```cpp
// Automatically skips pins used by active peripherals
// Prevents interference with I2C, SPI, UART communication
// Ensures system stability
```

### Accessing GPIOViewer

#### Web Interface
- **URL**: `http://[ESP32_IP]:8080`
- **mDNS**: `http://gpioviewer.local:8080` (if mDNS supported)
- **Real-time Updates**: Live pin state visualization
- **Pin Information**: Shows current state, function, and usage

#### Features
- **Live Pin States**: Real-time HIGH/LOW/PWM/ADC values
- **Pin Function Labels**: Automatically detected pin functions
- **Board Layout**: Visual representation of ESP32 pinout
- **Search & Filter**: Find specific pins quickly
- **Export Data**: Download pin state logs

### GPIOViewer Configuration

#### Port Configuration
```cpp
// Default port: 8080
gpioViewer.setPort(8080);

// Alternative port (if conflict with web server)
gpioViewer.setPort(8081);
```

#### Sampling Interval
```cpp
// Default: 100ms
gpioViewer.setSamplingInterval(100);

// Slower sampling (less CPU usage)
gpioViewer.setSamplingInterval(500);

// Faster sampling (more responsive)
gpioViewer.setSamplingInterval(50);
```

#### Peripheral Pin Skipping
```cpp
// Default: true (recommended)
gpioViewer.setSkipPeripheralPins(true);

// Include all pins (may cause conflicts)
gpioViewer.setSkipPeripheralPins(false);
```

### Troubleshooting GPIOViewer

#### Common Issues

1. **Port Conflict**
   ```
   Problem: Web server uses port 80, GPIOViewer uses port 8080
   Solution: Change GPIOViewer port to 8081 or 8082
   ```

2. **No Pin Detection**
   ```
   Problem: Some pins not showing in GPIOViewer
   Solution: Check if pins are used by peripherals (I2C, SPI, UART)
   ```

3. **Incorrect Pin States**
   ```
   Problem: Pin states don't match actual hardware
   Solution: Check wiring and ensure pins are properly configured
   ```

#### Debug Mode
Enable debug output in GPIOViewer:
```cpp
// Add to setup() for debugging
DEBUG_PRINTLN("GPIOViewer debug enabled");
gpioViewer.begin();
```

## 📡 API Reference

### WebSocket Events

#### Sensor Data
```javascript
// Client receives sensor updates
socket.on('sensor-data', function(data) {
    // data: JSON object with sensor readings
    // Example: {"temperature":25.5,"humidity":60.2,"timestamp":1234567890}
});
```

#### GPIO States
```javascript
// GPIOViewer WebSocket events
socket.on('gpio-state', function(data) {
    // data: JSON object with pin states
    // Example: {"0":256,"2":0,"4":128}
});
```

#### Status Updates
```javascript
// System status from ESP-NOW
socket.on('status', function(data) {
    // data: JSON object with system information
    // Example: {"device":"ESP32-Node1","uptime":3600,"freeHeap":25000}
});
```

### HTTP Endpoints

#### System Information
```http
GET /espinfo
Response: JSON with ESP32 system information
```

#### Sensor Data
```http
GET /sensors
Response: Current sensor readings
```

#### GPIO Information
```http
GET /pinmodes
Response: Configured pin modes
```

#### GPIOViewer Endpoints
```http
GET /release
Response: GPIOViewer version information

GET /free_psram
Response: Available PSRAM

GET /sampling
Response: Current sampling interval
```

### ESP-NOW Message Types

#### Sensor Data
```cpp
// Type: MSG_SENSOR_DATA
// Data: JSON sensor readings
// Example: {"temperature":25.5,"humidity":60.2}
```

#### Actuator Commands
```cpp
// Type: MSG_ACTUATOR_CMD
// Data: JSON actuator control
// Example: {"actuator":"relay1","value":1}
```

#### Status Updates
```cpp
// Type: MSG_STATUS
// Data: System status information
// Example: {"uptime":3600,"freeHeap":25000}
```

## 🔧 Troubleshooting

### Common Issues

#### 1. **WiFi Connection Problems**
```bash
# Check WiFi credentials in credentials.h
# Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
# Verify MAC address not blocked by router
# Try closer to router for better signal
```

#### 2. **ESP-NOW Not Working**
```bash
# Verify MAC addresses are correct in credentials.h
# Ensure all devices on same WiFi channel
# Check distance (max ~100m outdoors, 30m indoors)
# Verify ESP-NOW initialized before WiFi.begin()
```

#### 3. **Web Page Won't Load**
```bash
# Verify SPIFFS uploaded: pio run -t uploadfs
# Check files exist: SPIFFS.exists("/index.html")
# Try accessing directly: http://ip/index.html
# Check browser console for errors
```

#### 4. **Sensors Not Reading**
```bash
# Check wiring (VCC, GND, data pins)
# Verify correct pins in config.h
# Test with I2C scanner for I2C sensors
# Check 3.3V vs 5V requirements
# Verify library versions
```

#### 5. **GPIOViewer Issues**
```bash
# Check port conflicts (default 8080)
# Verify mDNS working (.local domain)
# Check if pins used by peripherals
# Enable debug mode for troubleshooting
```

### Debug Mode

Enable debug output in `src/config.h`:
```cpp
#define DEBUG_MODE true
#define DEBUG_SENSORS true
#define DEBUG_ESPNOW true
```

### Serial Monitor Output
Key messages to look for:
```
[1/9] Initializing GPIO... ✓ GPIO initialized
[2/9] Initializing SPIFFS... ✓ OK
[3/9] Initializing WiFi... ✓ WiFi connected!
[4/9] Initializing ESP-NOW... ✓ ESP-NOW initialized
[5/9] Initializing OTA... ✓ OTA ready
[6/9] Initializing Sensors... ✓ 3 sensor(s) initialized
[7/9] Initializing Actuators... ✓ Actuators initialized
[8/9] Initializing Camera... ✓ Camera ready
[9/9] Initializing Web Server... ✓ Web server started
🚀 SYSTEM READY 🚀
```

### Error Codes

#### ESP-NOW Error Codes
- `MSG_ACK`: Message acknowledgment
- `MSG_SENSOR_DATA`: Sensor data transmission
- `MSG_ACTUATOR_CMD`: Actuator control command
- `MSG_STATUS`: System status update
- `MSG_ALERT`: Alert/warning message

#### System Error Codes
- `ESP_RST_BROWNOUT`: Power supply issue
- `ESP_RST_WDT`: Watchdog timeout
- `ESP_RST_PANIC`: Software crash
- `ESP_RST_DEEPSLEEP`: Deep sleep wake-up

## 🛠️ L298N Motor Driver Integration

### Overview

The system includes comprehensive support for L298N dual H-bridge motor drivers, enabling precise control of DC motors and stepper motors. The L298N driver provides:

- **Dual Channel Control**: Control two DC motors or one stepper motor
- **High Current Capacity**: Up to 2A per channel (4A peak)
- **Wide Voltage Range**: 5V to 35V motor supply
- **PWM Speed Control**: Smooth speed regulation via PWM signals
- **Direction Control**: Forward, backward, and brake modes
- **Built-in Protection**: Overheat and overcurrent protection

### Hardware Connections

#### L298N Pinout
```
L298N Module Connections:
┌─────────────────────────────────┐
│ 12V 5V GND ENA IN1 IN2 IN3 IN4 │
│ OUT1 OUT2 OUT3 OUT4 GND +12V    │
└─────────────────────────────────┘

ESP32 Connections:
- IN1 (GPIO 14)  → Motor 1 Direction 1
- IN2 (GPIO 33)  → Motor 1 Direction 2  
- ENA (GPIO 32)  → Motor 1 Speed (PWM)
- IN3 (GPIO 25)  → Motor 2 Direction 1 (optional)
- IN4 (GPIO 26)  → Motor 2 Direction 2 (optional)
- ENB (GPIO 27)  → Motor 2 Speed (PWM) (optional)
```

#### Wiring Diagram
```
ESP32 GPIO 14 ────► L298N IN1
ESP32 GPIO 33 ────► L298N IN2  
ESP32 GPIO 32 ────► L298N ENA (PWM)
ESP32 GND ───────► L298N GND
L298N OUT1 ──────► Motor 1 Terminal A
L298N OUT2 ──────► Motor 1 Terminal B
L298N +12V ──────► External 12V Power Supply (+)
L298N GND ──────► External Power Supply (-) & ESP32 GND
```

### Configuration

#### Pin Configuration in `src/config.h`
```cpp
// L298N Motor Driver Configuration
#define MOTOR1_IN1 14      // Direction control 1
#define MOTOR1_IN2 33      // Direction control 2  
#define MOTOR1_EN 32       // Speed control (PWM)
#define MOTOR2_IN1 25      // Optional: Second motor
#define MOTOR2_IN2 26      // Optional: Second motor
#define MOTOR2_EN 27       // Optional: Second motor PWM

// Motor Control Settings
#define MOTOR_MAX_SPEED 255    // PWM range 0-255
#define MOTOR_MIN_SPEED 0      // Minimum speed
#define MOTOR_DEFAULT_SPEED 128 // Default speed (50%)
```

#### Power Supply Requirements
- **Logic Supply**: 5V from ESP32 (for L298N logic)
- **Motor Supply**: 6-12V external supply (depending on motor requirements)
- **Current Rating**: Ensure power supply can handle motor current requirements
- **Common Ground**: Essential - connect all grounds together

### Control Methods

#### 1. Web Interface Control
Access motor control via `/actuators.html`:

**Motor Control Panel:**
- **Direction Buttons**: Forward, Backward, Stop, Brake
- **Speed Slider**: 0-100% speed control
- **Speed Presets**: 25%, 50%, 75%, 100% quick buttons
- **Ramp Controls**: Smooth acceleration/deceleration

#### 2. REST API Control
```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "motor-speed",
  "value": 200  // 0-255 PWM value
}
```

```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "motor-forward",
  "value": 1    // 0=stop, 1=forward
}
```

```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "motor-backward", 
  "value": 0    // 0=stop, 1=backward
}
```

#### 3. WebSocket Control
```javascript
// Real-time motor control
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Set motor speed
ws.send(JSON.stringify({
  type: 'actuator',
  actuator: 'motor-speed',
  value: 150
}));

// Set direction
ws.send(JSON.stringify({
  type: 'actuator', 
  actuator: 'motor-forward',
  value: 1
}));
```

#### 4. C++ API (Arduino)
```cpp
#include "MotorController.h"

void setup() {
  // Initialize motor controller
  motorController.begin();
}

void loop() {
  // Basic motor control
  motorController.setSpeed(128);        // 50% speed
  motorController.setDirection(true);   // Forward
  delay(2000);
  
  motorController.setDirection(false);  // Backward
  delay(2000);
  
  motorController.stop();               // Stop motor
  delay(1000);
  
  motorController.brake();              // Brake motor
  delay(1000);
}
```

### Advanced Motor Control Features

#### 1. Speed Ramp Control
```cpp
// Smooth acceleration/deceleration
void rampMotorSpeed(int startSpeed, int endSpeed, int duration) {
  motorController.rampSpeed(startSpeed, endSpeed, duration);
}

// Usage: Accelerate from 0 to 200 over 3 seconds
rampMotorSpeed(0, 200, 3000);
```

#### 2. Position Control (with Encoders)
```cpp
// Motor position control
void moveToPosition(int targetPosition) {
  motorController.moveToPosition(targetPosition);
}

// Get current position
int currentPosition = motorController.getPosition();
```

#### 3. PID Control
```cpp
// Configure PID parameters
motorController.setPID(1.0, 0.1, 0.01);  // Kp, Ki, Kd

// Enable PID control
motorController.enablePID(true);
```

#### 4. Multi-Motor Coordination
```cpp
// Synchronize two motors
void synchronizedMotors(int speed1, int speed2) {
  motorController.setMotor1Speed(speed1);
  motorController.setMotor2Speed(speed2);
}

// Differential drive control
void differentialDrive(int leftSpeed, int rightSpeed) {
  motorController.setLeftMotor(leftSpeed);
  motorController.setRightMotor(rightSpeed);
}
```

### Motor Control Commands

#### Basic Commands
```javascript
// Web Interface Functions
motorForward();        // Set direction forward
motorBackward();       // Set direction backward  
motorStop();           // Stop motor
motorBrake();          // Apply brake

setMotorSpeed(50);     // Set speed to 50%
setMotorSpeed(200);    // Set PWM value to 200
```

#### Advanced Commands
```javascript
// Smooth speed transitions
rampUpSpeed(0, 100, 2000);   // Ramp from 0% to 100% in 2 seconds
rampDownSpeed(100, 0, 1000); // Ramp from 100% to 0% in 1 second

// Direction control with speed
setDirectionAndSpeed('forward', 75);  // Forward at 75% speed
setDirectionAndSpeed('backward', 50); // Backward at 50% speed

// Emergency stop
emergencyStop();  // Immediately stops all motors
```

### Safety Features

#### 1. Overcurrent Protection
```cpp
// Monitor motor current
float current = motorController.getCurrent();
if (current > MAX_CURRENT) {
  motorController.emergencyStop();
  logError("Motor overcurrent detected");
}
```

#### 2. Thermal Protection
```cpp
// Monitor motor temperature
float temp = motorController.getTemperature();
if (temp > MAX_TEMPERATURE) {
  motorController.reduceSpeed();
  logWarning("Motor temperature high, reducing speed");
}
```

#### 3. Stall Detection
```cpp
// Detect motor stall
if (motorController.isStalled()) {
  motorController.emergencyStop();
  logError("Motor stalled, stopping motor");
}
```

### Troubleshooting

#### Common Issues

1. **Motor Not Spinning**
   ```
   Check: Power supply voltage and current
   Check: Wiring connections (IN1, IN2, ENA)
   Check: PWM signal on ENA pin
   Check: Motor supply voltage
   ```

2. **Motor Spins Erratically**
   ```
   Check: PWM frequency (use 1kHz-20kHz)
   Check: Power supply stability
   Check: Ground connections
   Check: Motor load (too heavy?)
   ```

3. **L298N Gets Hot**
   ```
   Check: Current draw exceeds 2A limit
   Check: Adequate heat sinking
   Check: PWM frequency (too low causes heating)
   Check: Motor supply voltage (too high?)
   ```

4. **Direction Control Not Working**
   ```
   Check: IN1 and IN2 logic levels
   Check: PWM signal on ENA pin
   Check: Motor wiring polarity
   Check: L298N enable pins
   ```

#### Debug Mode
```cpp
// Enable motor debugging
#define DEBUG_MOTOR true

// Monitor motor status
void debugMotorStatus() {
  Serial.print("Speed: ");
  Serial.println(motorController.getSpeed());
  Serial.print("Direction: ");
  Serial.println(motorController.getDirection());
  Serial.print("Current: ");
  Serial.println(motorController.getCurrent());
  Serial.print("Temperature: ");
  Serial.println(motorController.getTemperature());
}
```

### Integration Examples

#### 1. Conveyor Belt Control
```cpp
void conveyorControl() {
  // Start conveyor
  motorController.setSpeed(200);
  motorController.setDirection(true);  // Forward
  
  // Run for 30 seconds
  delay(30000);
  
  // Stop conveyor
  motorController.stop();
}
```

#### 2. Robotic Platform Control
```cpp
void robotMovement() {
  // Move forward
  motorController.setLeftMotor(150);
  motorController.setRightMotor(150);
  delay(2000);
  
  // Turn right
  motorController.setLeftMotor(150);
  motorController.setRightMotor(50);
  delay(1000);
  
  // Stop
  motorController.emergencyStop();
}
```

#### 3. Automated Door Control
```cpp
void doorControl() {
  // Open door
  motorController.rampSpeed(0, 200, 1000);  // Smooth start
  delay(5000);  // Hold open
  
  // Close door
  motorController.setDirection(false);  // Reverse
  motorController.rampSpeed(200, 0, 1000);  // Smooth stop
}
```

### Performance Optimization

#### 1. PWM Frequency Optimization
```cpp
// Optimal PWM frequency for L298N
#define MOTOR_PWM_FREQ 20000  // 20kHz (inaudible)
#define MOTOR_PWM_CHANNEL 0
#define MOTOR_PWM_RESOLUTION 8  // 8-bit (0-255)
```

#### 2. Power Efficiency
```cpp
// Reduce power consumption when idle
void powerSavingMode() {
  if (motorController.isIdle()) {
    motorController.setSpeed(0);
    // Optionally disable motor driver
  }
}
```

#### 3. Heat Management
```cpp
// Monitor and manage temperature
void thermalManagement() {
  float temp = motorController.getTemperature();
  if (temp > 60) {
    motorController.reduceSpeed(20);  // Reduce speed by 20%
  }
  if (temp > 75) {
    motorController.stop();  // Stop motor
  }
}
```

### Best Practices

1. **Always use common ground** between ESP32, L298N, and power supplies
2. **Use appropriate power supply** for motor voltage and current requirements
3. **Implement safety interlocks** to prevent damage
4. **Use PWM for speed control** rather than voltage regulation
5. **Monitor temperature and current** during operation
6. **Implement smooth acceleration/deceleration** to reduce mechanical stress
7. **Use appropriate PWM frequency** (1-20kHz) to avoid motor noise
8. **Add flyback diodes** if not already present on the L298N module

This comprehensive L298N motor driver integration provides robust, safe, and efficient motor control for your ESP32 projects.

## 🛠️ Development

### Adding New Sensors

1. **Create Sensor Class**
   ```cpp
   // src/sensors/NewSensor.h
   class NewSensor {
   public:
       bool begin();
       float readValue();
       void printStatus();
   };
   ```

2. **Implement Sensor Logic**
   ```cpp
   // src/sensors/NewSensor.cpp
   bool NewSensor::begin() {
       // Initialize sensor
       return true;
   }
   ```

3. **Add to Sensor Manager**
   ```cpp
   // src/sensors/SensorManager.cpp
   void SensorManager::addNewSensor() {
       // Add sensor to management
   }
   ```

### Adding New Actuators

1. **Create Actuator Class**
   ```cpp
   // src/actuators/NewActuator.h
   class NewActuator {
   public:
       bool begin();
       void setValue(int value);
       int getValue();
   };
   ```

2. **Implement Actuator Logic**
   ```cpp
   // src/actuators/NewActuator.cpp
   void NewActuator::setValue(int value) {
       // Control actuator
   }
   ```

3. **Add to Actuator Manager**
   ```cpp
   // src/actuators/ActuatorManager.cpp
   void ActuatorManager::addNewActuator() {
       // Add actuator to management
   }
   ```

### Customizing GPIOViewer

1. **Modify Pin Detection**
   ```cpp
   // src/gpio_viewer_fixed.h
   bool isPinOnPeripheralBus(int gpioNum) {
       // Add custom peripheral detection
   }
   ```

2. **Custom Pin Functions**
   ```cpp
   // Add new pin function detection
   bool isPinCustomFunction(int gpioNum) {
       // Custom logic
   }
   ```

3. **UI Customization**
   - Modify web assets in `data/` directory
   - Update CSS in `data/style.css`
   - Modify JavaScript in `data/script.js`

### PlatformIO Configuration

#### Custom Build Flags
```ini
; platformio.ini
build_flags =
    -DDEBUG_MODE
    -DENABLE_CUSTOM_FEATURE
    -DCUSTOM_PIN_CONFIG
```

#### Custom Libraries
```ini
; platformio.ini
lib_deps =
    your-custom-library
    https://github.com/user/repo.git
```

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## 📞 Support

For support and questions:
- Create an issue on GitHub
- Check the troubleshooting section
- Review the documentation
- Check Serial Monitor output for error messages

## 🙏 Acknowledgments

- ESP32 Arduino Core Team
- PlatformIO Development Team
- All library contributors
- GPIOViewer original author

---

**Note**: This system is designed for educational and development purposes. Always follow safety guidelines when working with electrical components and IoT devices.