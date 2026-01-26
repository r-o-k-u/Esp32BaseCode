```markdown
# ESP32 Dual Communication System

## 📋 Project Overview

A comprehensive IoT system for ESP32 and ESP32-CAM with two-way communication capabilities, supporting multiple sensors, actuators, and a modern web interface.

### ✨ Key Features

- **Two-Way ESP-NOW Communication** - Direct device-to-device messaging without WiFi router
- **WiFi Manager** - Easy network configuration via web interface
- **OTA Updates** - Wireless firmware updates
- **Web Dashboard** - Modern, responsive control interface
- **Multi-Sensor Support** - DHT22, BMP280, Ultrasonic, PIR, LDR, Soil Moisture, MPU6050, MQ135
- **Multi-Actuator Support** - Servos, Relays, LEDs, Buzzer, Motors, RGB LEDs
- **Camera Support** - Live streaming (ESP32-CAM only)
- **Data Logging** - SPIFFS-based data storage
- **Modular Architecture** - Easy to extend and customize

## 🔧 Hardware Requirements

### For ESP32 DevKit:
- ESP32 Development Board (30-pin or 38-pin)
- USB cable for programming
- Sensors (optional):
  - DHT22 Temperature/Humidity
  - BMP280 Pressure/Altitude
  - HC-SR04 Ultrasonic Distance
  - PIR Motion Sensor
  - LDR Light Sensor
  - Soil Moisture Sensor
  - MPU6050 Accelerometer/Gyroscope
  - MQ135 Air Quality Sensor
- Actuators (optional):
  - SG90 Servo Motors (x2)
  - 5V Relay Modules (x3)
  - LEDs
  - Active Buzzer
  - L298N Motor Driver
  - RGB LED Module
- Power supply (5V/2A minimum)

### For ESP32-CAM:
- ESP32-CAM board with OV2640 camera
- FTDI programmer or ESP32-CAM-MB programmer board
- External antenna (optional, for better range)
- Power supply (5V/2A minimum, camera needs stable power)
- Limited GPIO pins (see pin diagram)

## 📦 Software Requirements

- **PlatformIO IDE** (recommended) or Arduino IDE
- **Required Libraries** (auto-installed with PlatformIO):
  - ESP Async WebServer
  - ArduinoJson
  - DHT sensor library
  - Adafruit BMP280
  - MPU6050
  - ESP32Servo
  - And more (see platformio.ini)

## 🚀 Quick Start Guide

### 1. Clone and Setup

```bash
# Clone the repository
git clone 
cd ESP32_DualComm_System

# Copy credentials template
cp src/credentials_template.h src/credentials.h

# Edit credentials.h with your WiFi settings
nano src/credentials.h
```

### 2. Configure Your Device

Edit `src/config.h`:
```cpp
#define DEVICE_NAME "ESP32_Device_1"
#define DEVICE_TYPE 0  // 0 = ESP32, 1 = ESP32-CAM
```

Add peer MAC address:
```cpp
// Get MAC from Serial Monitor when device boots
uint8_t peerMAC[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};
```

### 3. Upload Code

Using PlatformIO:
```bash
# For ESP32
pio run -e esp32 -t upload

# For ESP32-CAM
pio run -e esp32cam -t upload

# Upload filesystem (web files)
pio run -t uploadfs
```

Using Arduino IDE:
1. Install required libraries
2. Select board: ESP32 Dev Module or AI Thinker ESP32-CAM
3. Upload sketch
4. Upload SPIFFS data using ESP32 Sketch Data Upload tool

### 4. Get Device MAC Address

After first boot, check Serial Monitor:
```
Device MAC: 24:6F:28:AA:BB:CC
```

Use this MAC address in the other device's `credentials.h` as `PEER_MAC_1`.

### 5. Access Web Interface

1. Connect to device's WiFi AP (if not connected to your network)
2. Navigate to device IP address (shown in Serial Monitor)
3. Default URL: `http://192.168.4.1` (AP mode) or assigned IP

## 🔌 Wiring Diagrams

### ESP32 DevKit Pin Connections

#### Sensors:
```
DHT22 Temperature/Humidity:
  VCC  → 3.3V
  GND  → GND
  DATA → GPIO 4

BMP280 Pressure (I2C):
  VCC → 3.3V
  GND → GND
  SDA → GPIO 21
  SCL → GPIO 22

HC-SR04 Ultrasonic:
  VCC  → 5V
  GND  → GND
  TRIG → GPIO 5
  ECHO → GPIO 18

PIR Motion:
  VCC → 5V
  GND → GND
  OUT → GPIO 19

LDR Light Sensor:
  One leg → 3.3V
  Other leg → GPIO 34 and 10K resistor to GND

Soil Moisture:
  VCC → 3.3V
  GND → GND
  AO  → GPIO 35

MPU6050 Accelerometer (I2C):
  VCC → 3.3V
  GND → GND
  SDA → GPIO 21
  SCL → GPIO 22

MQ135 Air Quality:
  VCC → 5V
  GND → GND
  AO  → GPIO 32
```

#### Actuators:
```
Servo Motors:
  Servo 1:
    VCC → 5V (external power recommended)
    GND → GND
    SIG → GPIO 13
  Servo 2:
    VCC → 5V
    GND → GND
    SIG → GPIO 12

Relay Modules (active low):
  Relay 1:
    VCC → 5V
    GND → GND
    IN  → GPIO 27
  Relay 2:
    IN  → GPIO 26
  Relay 3:
    IN  → GPIO 25

LED:
  Anode (+) → GPIO 2 (via 220Ω resistor)
  Cathode (−) → GND

Buzzer (Active):
  + → GPIO 15
  − → GND

L298N Motor Driver:
  Motor 1:
    IN1 → GPIO 14
    IN2 → GPIO 33
    EN  → GPIO 32 (PWM)
  Motor 2:
    IN3 → GPIO 16
    IN4 → GPIO 17
    EN  → GPIO 23 (PWM)
  12V → External power supply
  GND → Common ground

RGB LED:
  R → GPIO 16 (via 220Ω)
  G → GPIO 17 (via 220Ω)
  B → GPIO 23 (via 220Ω)
  Common → GND (common cathode)
```

### ESP32-CAM Pin Connections

**⚠️ WARNING:** ESP32-CAM has limited available GPIOs as most are used by the camera!

```
Available GPIO pins:
  GPIO 2  → Can be used (but used for DHT in example)
  GPIO 13 → Can be used (PIR sensor in example)
  GPIO 12 → Can be used (Relay in example)
  GPIO 14 → Can be used (Servo in example)
  GPIO 15 → Can be used (LDR in example)

Camera pins (DO NOT USE):
  GPIO 0, 4, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39

Flash LED:
  GPIO 4 (built-in)

Power:
  Use 5V/2A minimum power supply
  Camera draws significant current
```

## 📡 ESP-NOW Communication

### How It Works

ESP-NOW allows direct communication between ESP32 devices without WiFi router:

1. Both devices can be in STA mode (normal WiFi) or AP mode
2. Messages are sent directly using MAC addresses
3. Supports broadcast and peer-to-peer communication
4. Very low latency (<10ms)
5. Works even when WiFi is not connected

### Message Types

```cpp
MSG_SENSOR_DATA  = 0  // Sensor readings
MSG_ACTUATOR_CMD = 1  // Actuator commands
MSG_STATUS       = 2  // Status updates
MSG_CONFIG       = 3  // Configuration changes
MSG_ACK          = 4  // Acknowledgment
MSG_ALERT        = 5  // Alert/warning messages
MSG_SYNC         = 6  // Time synchronization
MSG_CUSTOM       = 99 // Custom messages
```

### Example Usage

```cpp
// Send sensor data to peer
String jsonData = "{\"temp\":25.5,\"hum\":60}";
espnowComm.sendSensorData(peerMAC, jsonData.c_str());

// Send actuator command to peer
String command = "{\"actuator\":\"led\",\"state\":1}";
espnowComm.sendActuatorCommand(peerMAC, command.c_str());

// Send to all peers
espnowComm.sendToAllPeers(MSG_ALERT, "Motion detected!");
```

## 🌐 Web Interface Features

### Dashboard Tab
- Real-time system information
- Quick sensor readings
- ESP-NOW communication statistics
- Quick control buttons

### Sensors Tab
- Live sensor data from all connected sensors
- Historical temperature chart
- Auto-refreshing readings

### Actuators Tab
- LED, Relay, Servo, Buzzer controls
- Real-time control with immediate feedback
- Slider controls for servo positions

### Communication Tab
- View connected peer devices
- Send custom messages to peers
- Message log with timestamps
- Test connectivity

### Camera Tab (ESP32-CAM only)
- Live camera stream
- Capture photos
- Flash control

### Config Tab
- Change device settings
- System restart
- Factory reset option

## 🔧 API Endpoints

### REST API

```
GET  /api/status          - Get system status
GET  /api/sensors         - Get all sensor readings
GET  /api/peers           - Get ESP-NOW peer list
POST /api/actuator        - Control actuator
     Body: {"actuator":"led", "state":1}
POST /api/servo           - Set servo angle
     Body: {"servo":1, "angle":90}
POST /api/message         - Send ESP-NOW message
     Body: {"peer":"AA:BB:CC:DD:EE:FF", "message":"test"}
POST /api/restart         - Restart device
POST /api/reset           - Factory reset
GET  /cam                 - Camera stream (ESP32-CAM)
```

### WebSocket

Connect to `ws://device-ip/ws` for real-time updates:

```javascript
{
  "type": "sensor",
  "temperature": 25.5,
  "humidity": 60,
  "timestamp": 123456
}

{
  "type": "espnowMessage",
  "from": "AA:BB:CC:DD:EE:FF",
  "data": "message content",
  "timestamp": 123456
}
```

## 🔐 Security Considerations

1. **Change Default Credentials:**
   - Edit `credentials.h`
   - Change `WEB_PASSWORD`
   - Change `OTA_PASSWORD`
   - Change `AP_PASSWORD`

2. **Enable Authentication:**
   - Web interface can use HTTP Basic Auth
   - Configure in `WebServer.cpp`

3. **Network Security:**
   - Use WPA2 WiFi encryption
   - Consider VPN for remote access
   - ESP-NOW can use encryption (currently disabled for simplicity)

## 📊 Data Logging

### Logging to SPIFFS

Data is stored in JSON format:
```json
{
  "timestamp": 123456,
  "device": "ESP32_Device_1",
  "sensors": {
    "temperature": 25.5,
    "humidity": 60
  }
}
```

### Viewing Logs

Access via web interface or download:
```
GET /api/logs?type=sensors&limit=100
```

### Log Rotation

Automatic rotation when file exceeds `MAX_LOG_SIZE` (100KB default).

## 🐛 Troubleshooting

### WiFi Connection Issues
```
Problem: Device not connecting to WiFi
Solutions:
1. Check SSID and password in credentials.h
2. Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
3. Check if network is WPA2 (WPA3 not supported)
4. Try moving closer to router
5. Check Serial Monitor for error messages
```

### ESP-NOW Not Working
```
Problem: Devices not communicating
Solutions:
1. Verify both devices have correct MAC addresses
2. Check both devices are on same WiFi channel
3. Ensure ESP-NOW is initialized before WiFi connection
4. Maximum 6 peers on ESP32
5. Check distance (max ~100m outdoors, ~30m indoors)
```

### Camera Not Starting (ESP32-CAM)
```
Problem: Camera initialization failed
Solutions:
1. Ensure adequate power supply (5V/2A minimum)
2. Check camera ribbon cable connection
3. Use correct board definition (AI Thinker ESP32-CAM)
4. Ensure PSRAM is enabled in build flags
5. Try lowering camera resolution
```

### OTA Upload Fails
```
Problem: Can't upload over WiFi
Solutions:
1. Check device is on same network
2. Verify OTA_PASSWORD matches
3. Ensure no firewall blocking port 3232
4. Check device has enough free space
5. Restart device and try again
```

### SPIFFS Upload Issues
```
Problem: Web pages not loading
Solutions:
1. Upload filesystem with PlatformIO: pio run -t uploadfs
2. Check SPIFFS partition size in platformio.ini
3. Verify files are in data/ directory
4. Check Serial Monitor for SPIFFS mount errors
5. Format SPIFFS: SPIFFS.format()
```

## 🔄 Updating Firmware

### Over-The-Air (OTA)

1. Device must be connected to WiFi
2. In PlatformIO:
   ```bash
   pio run -e esp32 --target upload --upload-port <device-ip>
   ```
3. Or use Arduino IDE with network port selected

### USB Upload

1. Connect device via USB
2. Regular upload process
3. Don't forget to upload filesystem for web file updates

## 🛠️ Customization

### Adding New Sensors

1. Create sensor class in `src/sensors/`:
```cpp
// MySensor.h
class MySensor {
public:
    void begin();
    float read();
};
```

2. Add to `SensorManager.cpp`:
```cpp
#include "sensors/MySensor.h"
MySensor mySensor;

void SensorManager::begin() {
    mySensor.begin();
}

void SensorManager::getAllSensorData(JsonDocument& doc) {
    doc["myValue"] = mySensor.read();
}
```

### Adding New Actuators

Similar process in `src/actuators/`.

### Customizing Web Interface

Edit files in `data/` directory:
- `index.html` - Structure
- `style.css` - Styling
- `script.js` - Functionality

Upload with: `pio run -t uploadfs`

## 📈 Performance Tips

1. **Optimize Sensor Reading:**
   - Adjust `SENSOR_READ_INTERVAL` based on needs
   - Use averaging for noisy sensors
   
2. **ESP-NOW Range:**
   - Use external antenna on ESP32-CAM
   - Avoid obstacles between devices
   - Stay within 100m range
   
3. **Power Consumption:**
   - Use deep sleep when possible
   - Reduce WiFi transmit power if close to router
   - Turn off unused peripherals
   
4. **Memory Management:**
   - Monitor free heap regularly
   - Use static JSON documents
   - Clear buffers after use

## 📚 Additional Resources

- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [ESP-NOW Protocol](https://www.espressif.com/en/products/software/esp-now/overview)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ArduinoJson Documentation](https://arduinojson.org/)

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Open Pull Request

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

## ⚠️ Disclaimer

This is a development/educational project. Use at your own risk. Ensure proper electrical safety when working with hardware.

## 💡 Project Ideas

What you can build with this system:

1. **Home Automation**
   - Control lights, fans, appliances
   - Monitor temperature, humidity
   - Motion-based automation

2. **Greenhouse Monitor**
   - Soil moisture sensing
   - Automatic watering system
   - Temperature/humidity logging

3. **Security System**
   - Motion detection alerts
   - Camera monitoring
   - Multi-device coordination

4. **Weather Station**
   - Multiple sensor deployment
   - Data aggregation
   - Historical tracking

5. **Robot Control**
   - Motor control
   - Sensor feedback
   - Remote operation

6. **Smart Parking**
   - Ultrasonic distance sensing
   - Availability notification
   - Multi-spot monitoring

---

**Happy Making! 🚀**
```
