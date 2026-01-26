# ESP32 DualComm System - API Reference

This document provides comprehensive API documentation for the ESP32 DualComm System, covering all available endpoints for sensors, actuators, camera, communication, and system management.

## Table of Contents

- [System Management](#system-management)
- [Sensor Management](#sensor-management)
- [Actuator Control](#actuator-control)
- [Camera Control](#camera-control)
- [Communication](#communication)
- [Logging](#logging)
- [OTA Updates](#ota-updates)
- [Error Handling](#error-handling)
- [Examples](#examples)

## System Management

### Device Information

#### GET `/api/system/info`
Get comprehensive system information including hardware details, firmware version, and current status.

**Response:**
```json
{
  "device": {
    "name": "ESP32_DualComm",
    "model": "ESP32-WROOM-32",
    "chipId": "1234567890ABCDEF",
    "coreVersion": "3.0.0",
    "sdkVersion": "v4.4.1"
  },
  "firmware": {
    "version": "2.0.0",
    "buildDate": "2024-01-15",
    "gitHash": "abc123def456"
  },
  "status": {
    "uptime": 3600,
    "freeHeap": 256000,
    "sketchSize": 1024000,
    "freeSketchSpace": 3145728,
    "resetReason": "Power on"
  },
  "network": {
    "wifi": {
      "status": "connected",
      "ssid": "MyWiFi",
      "rssi": -45,
      "ip": "192.168.1.100",
      "mac": "24:0A:C4:12:34:56"
    },
    "espnow": {
      "status": "enabled",
      "channel": 1,
      "peers": 2
    }
  }
}
```

#### GET `/api/system/health`
Get real-time system health metrics.

**Response:**
```json
{
  "uptime": 3600,
  "memory": {
    "total": 524288,
    "used": 268288,
    "free": 256000,
    "usage": 51.2
  },
  "storage": {
    "total": 4194304,
    "used": 1048576,
    "free": 3145728,
    "usage": 25.0
  },
  "temperature": 45.2,
  "voltage": 3.3
}
```

### Configuration Management

#### GET `/api/config`
Get current system configuration.

**Response:**
```json
{
  "system": {
    "deviceName": "ESP32_DualComm",
    "otaPassword": "secure_password"
  },
  "wifi": {
    "ssid": "MyWiFi",
    "password": "wifi_password",
    "apMode": false
  },
  "sensors": {
    "interval": 1000,
    "pins": {
      "dht": 4,
      "bmp": 21,
      "ldr": 34,
      "soil": 35,
      "pir": 13,
      "mq135": 32,
      "mpuSDA": 21,
      "mpuSCL": 22,
      "ultrasonicTrig": 5,
      "ultrasonicEcho": 18
    }
  },
  "actuators": {
    "pins": {
      "led": 2,
      "buzzer": 25,
      "motorIN1": 14,
      "motorIN2": 12,
      "motorEN": 13,
      "rgbR": 26,
      "rgbG": 27,
      "rgbB": 14,
      "relay1": 23,
      "relay2": 22,
      "relay3": 21,
      "servo1": 15
    }
  },
  "camera": {
    "enabled": true,
    "quality": 10,
    "frameSize": 5,
    "brightness": 0,
    "contrast": 0,
    "saturation": 0,
    "flashPin": 4
  },
  "espnow": {
    "enabled": true,
    "channel": 1,
    "peerMac": "24:0A:C4:12:34:56"
  }
}
```

#### POST `/api/config/system`
Update system configuration.

**Request:**
```json
{
  "system": {
    "deviceName": "New_Device_Name",
    "otaPassword": "new_password"
  },
  "wifi": {
    "ssid": "NewWiFi",
    "password": "new_password"
  }
}
```

#### POST `/api/config/sensors`
Update sensor configuration.

**Request:**
```json
{
  "sensors": {
    "interval": 2000,
    "pins": {
      "dht": 5,
      "bmp": 22
    }
  }
}
```

#### POST `/api/config/actuators`
Update actuator configuration.

**Request:**
```json
{
  "actuators": {
    "pins": {
      "led": 3,
      "buzzer": 26
    }
  }
}
```

#### POST `/api/config/camera`
Update camera configuration.

**Request:**
```json
{
  "camera": {
    "enabled": true,
    "quality": 5,
    "frameSize": 8,
    "brightness": 1,
    "contrast": -1
  }
}
```

#### POST `/api/config/espnow`
Update ESP-NOW configuration.

**Request:**
```json
{
  "espnow": {
    "enabled": true,
    "channel": 6,
    "peerMac": "AA:BB:CC:DD:EE:FF"
  }
}
```

### System Actions

#### POST `/api/reboot`
Reboot the ESP32 device.

**Response:**
```json
{
  "status": "rebooting",
  "message": "Device will reboot in 3 seconds"
}
```

#### POST `/api/factory-reset`
Perform factory reset (clears all configuration).

**Response:**
```json
{
  "status": "success",
  "message": "Factory reset completed"
}
```

## Sensor Management

### Sensor Data

#### GET `/api/sensors`
Get current readings from all sensors.

**Response:**
```json
{
  "dht": {
    "temperature": 25.5,
    "humidity": 60.2,
    "heatIndex": 26.8,
    "timestamp": 1642267800000
  },
  "bmp": {
    "pressure": 1013.25,
    "altitude": 150.5,
    "seaLevelPressure": 1020.15,
    "timestamp": 1642267800000
  },
  "ldr": {
    "lux": 500,
    "lightState": "bright",
    "adcValue": 2048,
    "timestamp": 1642267800000
  },
  "soil": {
    "moisturePercentage": 45.0,
    "moistureState": "dry",
    "adcValue": 1500,
    "timestamp": 1642267800000
  },
  "pir": {
    "motionDetected": false,
    "lastMotion": 1642267700000,
    "motionCount": 5,
    "timestamp": 1642267800000
  },
  "mq135": {
    "ppm": 400,
    "airQuality": "good",
    "adcValue": 1024,
    "timestamp": 1642267800000
  },
  "mpu6050": {
    "accelX": 0.02,
    "accelY": -0.01,
    "accelZ": 0.98,
    "gyroX": 0.5,
    "gyroY": -0.3,
    "gyroZ": 0.1,
    "pitch": 1.2,
    "roll": -0.8,
    "yaw": 0.5,
    "timestamp": 1642267800000
  },
  "ultrasonic": {
    "distance": 15.5,
    "objectDetected": true,
    "timestamp": 1642267800000
  }
}
```

#### GET `/api/sensors/dht`
Get DHT sensor readings.

**Response:**
```json
{
  "temperature": 25.5,
  "humidity": 60.2,
  "heatIndex": 26.8,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/bmp`
Get BMP sensor readings.

**Response:**
```json
{
  "pressure": 1013.25,
  "altitude": 150.5,
  "seaLevelPressure": 1020.15,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/ldr`
Get LDR sensor readings.

**Response:**
```json
{
  "lux": 500,
  "lightState": "bright",
  "adcValue": 2048,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/soil`
Get soil moisture sensor readings.

**Response:**
```json
{
  "moisturePercentage": 45.0,
  "moistureState": "dry",
  "adcValue": 1500,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/pir`
Get PIR sensor readings.

**Response:**
```json
{
  "motionDetected": false,
  "lastMotion": 1642267700000,
  "motionCount": 5,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/mq135`
Get MQ135 sensor readings.

**Response:**
```json
{
  "ppm": 400,
  "airQuality": "good",
  "adcValue": 1024,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/mpu6050`
Get MPU6050 sensor readings.

**Response:**
```json
{
  "accelX": 0.02,
  "accelY": -0.01,
  "accelZ": 0.98,
  "gyroX": 0.5,
  "gyroY": -0.3,
  "gyroZ": 0.1,
  "pitch": 1.2,
  "roll": -0.8,
  "yaw": 0.5,
  "timestamp": 1642267800000
}
```

#### GET `/api/sensors/ultrasonic`
Get ultrasonic sensor readings.

**Response:**
```json
{
  "distance": 15.5,
  "objectDetected": true,
  "timestamp": 1642267800000
}
```

### Sensor Actions

#### POST `/api/sensors/dht/calibrate`
Calibrate DHT sensor.

**Response:**
```json
{
  "status": "success",
  "message": "DHT sensor calibrated"
}
```

#### POST `/api/sensors/bmp/calibrate`
Calibrate BMP sensor.

**Response:**
```json
{
  "status": "success",
  "message": "BMP sensor calibrated"
}
```

#### POST `/api/sensors/pir/reset`
Reset PIR motion count.

**Response:**
```json
{
  "status": "success",
  "message": "PIR motion count reset",
  "motionCount": 0
}
```

#### GET `/api/sensors/log`
Get sensor reading history.

**Query Parameters:**
- `limit`: Number of entries to return (default: 100)
- `from`: Start timestamp (optional)
- `to`: End timestamp (optional)

**Response:**
```json
[
  {
    "timestamp": 1642267800000,
    "dht": {
      "temperature": 25.5,
      "humidity": 60.2
    },
    "bmp": {
      "pressure": 1013.25
    }
  }
]
```

## Actuator Control

### LED Control

#### POST `/api/actuators/led/toggle`
Toggle LED state.

**Response:**
```json
{
  "status": "success",
  "ledState": true
}
```

#### POST `/api/actuators/led/on`
Turn LED on.

**Response:**
```json
{
  "status": "success",
  "ledState": true
}
```

#### POST `/api/actuators/led/off`
Turn LED off.

**Response:**
```json
{
  "status": "success",
  "ledState": false
}
```

#### POST `/api/actuators/led/test`
Test LED functionality.

**Response:**
```json
{
  "status": "success",
  "message": "LED test completed"
}
```

### Buzzer Control

#### POST `/api/actuators/buzzer/toggle`
Toggle buzzer state.

**Response:**
```json
{
  "status": "success",
  "buzzerState": true
}
```

#### POST `/api/actuators/buzzer/beep`
Play a beep sound.

**Request:**
```json
{
  "frequency": 1000,
  "duration": 500
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Beep played"
}
```

#### POST `/api/actuators/buzzer/melody`
Play a melody.

**Response:**
```json
{
  "status": "success",
  "message": "Melody played"
}
```

### RGB LED Control

#### POST `/api/actuators/rgb/color`
Set RGB LED color.

**Request (Option 1 - RGB values):**
```json
{
  "red": 255,
  "green": 128,
  "blue": 0
}
```

**Request (Option 2 - Hex color):**
```json
{
  "color": "#FF8000"
}
```

**Response:**
```json
{
  "status": "success",
  "color": {
    "r": 255,
    "g": 128,
    "b": 0
  }
}
```

#### POST `/api/actuators/rgb/brightness`
Set RGB LED brightness.

**Request:**
```json
{
  "brightness": 200
}
```

**Response:**
```json
{
  "status": "success",
  "brightness": 200
}
```

#### POST `/api/actuators/rgb/rainbow`
Start rainbow effect.

**Response:**
```json
{
  "status": "success",
  "message": "Rainbow effect started"
}
```

#### POST `/api/actuators/rgb/colorwipe`
Start color wipe effect.

**Response:**
```json
{
  "status": "success",
  "message": "Color wipe effect started"
}
```

#### POST `/api/actuators/rgb/theater`
Start theater chase effect.

**Response:**
```json
{
  "status": "success",
  "message": "Theater chase effect started"
}
```

#### POST `/api/actuators/rgb/twinkle`
Start twinkle effect.

**Response:**
```json
{
  "status": "success",
  "message": "Twinkle effect started"
}
```

### Motor Control

#### POST `/api/actuators/motor/forward`
Move motor forward.

**Response:**
```json
{
  "status": "success",
  "motorState": "forward"
}
```

#### POST `/api/actuators/motor/reverse`
Move motor in reverse.

**Response:**
```json
{
  "status": "success",
  "motorState": "reverse"
}
```

#### POST `/api/actuators/motor/stop`
Stop motor.

**Response:**
```json
{
  "status": "success",
  "motorState": "stopped"
}
```

#### POST `/api/actuators/motor/brake`
Brake motor.

**Response:**
```json
{
  "status": "success",
  "motorState": "braked"
}
```

#### POST `/api/actuators/motor/speed`
Set motor speed.

**Request:**
```json
{
  "speed": 75
}
```

**Response:**
```json
{
  "status": "success",
  "speed": 75
}
```

#### POST `/api/actuators/motor/accelerate`
Accelerate motor.

**Response:**
```json
{
  "status": "success",
  "message": "Motor accelerating"
}
```

#### POST `/api/actuators/motor/decelerate`
Decelerate motor.

**Response:**
```json
{
  "status": "success",
  "message": "Motor decelerating"
}
```

#### POST `/api/actuators/motor/rampup`
Ramp up motor speed.

**Response:**
```json
{
  "status": "success",
  "message": "Motor ramping up"
}
```

#### POST `/api/actuators/motor/rampdown`
Ramp down motor speed.

**Response:**
```json
{
  "status": "success",
  "message": "Motor ramping down"
}
```

### Relay Control

#### POST `/api/actuators/relay/{id}/toggle`
Toggle relay state (replace {id} with relay number 1-3).

**Response:**
```json
{
  "status": "success",
  "relay": 1,
  "state": true
}
```

#### POST `/api/actuators/relay/{id}/on`
Turn relay on.

**Response:**
```json
{
  "status": "success",
  "relay": 1,
  "state": true
}
```

#### POST `/api/actuators/relay/{id}/off`
Turn relay off.

**Response:**
```json
{
  "status": "success",
  "relay": 1,
  "state": false
}
```

#### POST `/api/actuators/relay/{id}/pulse`
Pulse relay for 1 second.

**Response:**
```json
{
  "status": "success",
  "relay": 1,
  "message": "Relay pulsed"
}
```

#### POST `/api/actuators/relay/all/on`
Turn all relays on.

**Response:**
```json
{
  "status": "success",
  "message": "All relays turned on"
}
```

#### POST `/api/actuators/relay/all/off`
Turn all relays off.

**Response:**
```json
{
  "status": "success",
  "message": "All relays turned off"
}
```

### Servo Control

#### POST `/api/actuators/servo/angle`
Set servo angle.

**Request:**
```json
{
  "angle": 90
}
```

**Response:**
```json
{
  "status": "success",
  "angle": 90
}
```

#### POST `/api/actuators/servo/sweep`
Sweep servo from 0 to 180 degrees.

**Response:**
```json
{
  "status": "success",
  "message": "Servo sweeping"
}
```

### General Actuator Control

#### POST `/api/actuators/emergency-stop`
Emergency stop all actuators.

**Response:**
```json
{
  "status": "success",
  "message": "Emergency stop activated"
}
```

#### POST `/api/actuators/all/on`
Turn all actuators on.

**Response:**
```json
{
  "status": "success",
  "message": "All actuators turned on"
}
```

#### POST `/api/actuators/all/off`
Turn all actuators off.

**Response:**
```json
{
  "status": "success",
  "message": "All actuators turned off"
}
```

#### GET `/api/actuators/status`
Get status of all actuators.

**Response:**
```json
{
  "led": true,
  "buzzer": false,
  "rgb": {
    "state": true,
    "color": {
      "r": 255,
      "g": 128,
      "b": 0
    },
    "brightness": 255
  },
  "motor": {
    "state": "stopped",
    "speed": 0
  },
  "relays": [true, false, true],
  "servo": 90
}
```

## Camera Control

### Camera Management

#### POST `/api/camera/start`
Start camera module.

**Response:**
```json
{
  "status": "success",
  "message": "Camera started"
}
```

#### POST `/api/camera/stop`
Stop camera module.

**Response:**
```json
{
  "status": "success",
  "message": "Camera stopped"
}
```

#### POST `/api/camera/capture`
Capture a still image.

**Response:**
```json
{
  "status": "success",
  "filename": "image_20240115_143022.jpg",
  "size": 156234
}
```

#### GET `/api/camera/status`
Get camera status and settings.

**Response:**
```json
{
  "enabled": true,
  "resolution": "320x240",
  "frameSize": 5,
  "quality": 10,
  "brightness": 0,
  "contrast": 0,
  "saturation": 0,
  "flashEnabled": false,
  "fps": 15
}
```

### Camera Settings

#### POST `/api/camera/settings`
Update camera settings.

**Request:**
```json
{
  "resolution": 5,
  "quality": 8,
  "brightness": 1,
  "contrast": -1,
  "saturation": 0
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Camera settings applied"
}
```

#### POST `/api/camera/flash/on`
Turn flash LED on.

**Response:**
```json
{
  "status": "success",
  "flashState": true
}
```

#### POST `/api/camera/flash/off`
Turn flash LED off.

**Response:**
```json
{
  "status": "success",
  "flashState": false
}
```

#### POST `/api/camera/flash/blink`
Blink flash LED.

**Response:**
```json
{
  "status": "success",
  "message": "Flash blinking"
}
```

### Image Gallery

#### GET `/api/camera/gallery`
Get list of captured images.

**Response:**
```json
[
  {
    "filename": "image_20240115_143022.jpg",
    "timestamp": 1642267822000,
    "size": 156234
  },
  {
    "filename": "image_20240115_142945.jpg",
    "timestamp": 1642267785000,
    "size": 142567
  }
]
```

#### DELETE `/api/camera/gallery/{filename}`
Delete an image (replace {filename} with actual filename).

**Response:**
```json
{
  "status": "success",
  "message": "Image deleted"
}
```

#### POST `/api/camera/gallery/clear`
Clear all images.

**Response:**
```json
{
  "status": "success",
  "message": "Gallery cleared"
}
```

### Motion Detection

#### POST `/api/camera/motion/enable`
Enable motion detection.

**Response:**
```json
{
  "status": "success",
  "message": "Motion detection enabled"
}
```

#### POST `/api/camera/motion/disable`
Disable motion detection.

**Response:**
```json
{
  "status": "success",
  "message": "Motion detection disabled"
}
```

#### POST `/api/camera/motion/calibrate`
Calibrate motion detection.

**Response:**
```json
{
  "status": "success",
  "message": "Motion detection calibrated"
}
```

#### POST `/api/camera/motion/reset`
Reset motion detection statistics.

**Response:**
```json
{
  "status": "success",
  "message": "Motion detection reset"
}
```

### Live Streaming

#### GET `/api/camera/stream`
Get current frame for live streaming.

**Response:**
```json
{
  "frame": "base64_encoded_image_data",
  "timestamp": 1642267800000
}
```

## Communication

### ESP-NOW

#### POST `/api/espnow/start`
Start ESP-NOW communication.

**Response:**
```json
{
  "status": "success",
  "message": "ESP-NOW started"
}
```

#### POST `/api/espnow/stop`
Stop ESP-NOW communication.

**Response:**
```json
{
  "status": "success",
  "message": "ESP-NOW stopped"
}
```

#### POST `/api/espnow/scan`
Scan for ESP-NOW peers.

**Response:**
```json
{
  "status": "success",
  "message": "Scanning for peers"
}
```

#### GET `/api/espnow/peers`
Get list of configured peers.

**Response:**
```json
[
  {
    "mac": "24:0A:C4:12:34:56",
    "name": "ESP32_Node_1",
    "lastSeen": 1642267800000
  }
]
```

#### POST `/api/espnow/peers/add`
Add a new peer.

**Request:**
```json
{
  "mac": "AA:BB:CC:DD:EE:FF",
  "name": "New_Peer"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Peer added"
}
```

#### DELETE `/api/espnow/peers/{mac}`
Remove a peer (replace {mac} with actual MAC address).

**Response:**
```json
{
  "status": "success",
  "message": "Peer removed"
}
```

#### POST `/api/espnow/message/send`
Send a message to a specific peer.

**Request:**
```json
{
  "mac": "24:0A:C4:12:34:56",
  "type": "sensor",
  "content": "Temperature: 25.5°C"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Message sent"
}
```

#### POST `/api/espnow/message/broadcast`
Broadcast a message to all peers.

**Request:**
```json
{
  "content": "System status update"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Message broadcasted"
}
```

#### GET `/api/espnow/messages`
Get message history.

**Response:**
```json
[
  {
    "timestamp": 1642267800000,
    "type": "sensor",
    "content": "Temperature: 25.5°C",
    "direction": "sent",
    "peer": "24:0A:C4:12:34:56"
  }
]
```

### WiFi Management

#### POST `/api/wifi/connect`
Connect to WiFi network.

**Request:**
```json
{
  "ssid": "MyWiFi",
  "password": "password123"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Connecting to WiFi"
}
```

#### POST `/api/wifi/disconnect`
Disconnect from WiFi.

**Response:**
```json
{
  "status": "success",
  "message": "Disconnected from WiFi"
}
```

#### POST `/api/wifi/ap`
Create WiFi access point.

**Request:**
```json
{
  "ssid": "ESP32_AP",
  "password": "12345678"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Access point created"
}
```

#### GET `/api/wifi/networks`
Get list of available WiFi networks.

**Response:**
```json
[
  {
    "ssid": "MyWiFi",
    "rssi": -45,
    "channel": 6,
    "secure": true
  }
]
```

### MQTT

#### POST `/api/mqtt/connect`
Connect to MQTT broker.

**Request:**
```json
{
  "broker": "mqtt.example.com",
  "port": 1883,
  "username": "user",
  "password": "pass",
  "clientId": "esp32_client"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Connecting to MQTT"
}
```

#### POST `/api/mqtt/disconnect`
Disconnect from MQTT broker.

**Response:**
```json
{
  "status": "success",
  "message": "Disconnected from MQTT"
}
```

#### POST `/api/mqtt/publish`
Publish MQTT message.

**Request:**
```json
{
  "topic": "sensors/temperature",
  "message": "25.5",
  "qos": 1
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Message published"
}
```

#### POST `/api/mqtt/subscribe`
Subscribe to MQTT topic.

**Request:**
```json
{
  "topic": "actuators/led",
  "qos": 1
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Subscribed to topic"
}
```

#### POST `/api/mqtt/unsubscribe`
Unsubscribe from MQTT topic.

**Request:**
```json
{
  "topic": "actuators/led"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Unsubscribed from topic"
}
```

#### GET `/api/mqtt/messages`
Get MQTT message history.

**Response:**
```json
[
  {
    "timestamp": 1642267800000,
    "topic": "sensors/temperature",
    "message": "25.5",
    "qos": 1
  }
]
```

### Serial Communication

#### POST `/api/serial/open`
Open serial port.

**Request:**
```json
{
  "baudRate": 115200,
  "dataBits": 8,
  "stopBits": 1,
  "parity": "none"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Serial port opened"
}
```

#### POST `/api/serial/close`
Close serial port.

**Response:**
```json
{
  "status": "success",
  "message": "Serial port closed"
}
```

#### POST `/api/serial/send`
Send serial message.

**Request:**
```json
{
  "message": "Hello Serial"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Message sent"
}
```

#### GET `/api/serial/output`
Get serial output buffer.

**Response:**
```json
[
  "Serial output line 1",
  "Serial output line 2"
]
```

## Logging

### Log Management

#### POST `/api/logs/start`
Start logging system.

**Response:**
```json
{
  "status": "success",
  "message": "Logging started"
}
```

#### POST `/api/logs/stop`
Stop logging system.

**Response:**
```json
{
  "status": "success",
  "message": "Logging stopped"
}
```

#### GET `/api/logs`
Get log entries with filtering.

**Query Parameters:**
- `level`: Log level (error, warning, info, debug, all)
- `module`: Module name
- `timeRange`: Time range (1h, 6h, 24h, 7d, all)
- `search`: Search text

**Response:**
```json
[
  {
    "timestamp": 1642267800000,
    "level": "info",
    "module": "system",
    "message": "System started"
  }
]
```

#### POST `/api/logs/clear`
Clear all logs.

**Response:**
```json
{
  "status": "success",
  "message": "Logs cleared"
}
```

#### GET `/api/logs/export`
Export logs to text format.

**Response:**
```
[2024-01-15 14:30:00] [INFO] [system] System started
[2024-01-15 14:30:01] [INFO] [wifi] Connected to MyWiFi
```

#### GET `/api/logs/stats`
Get log statistics.

**Response:**
```json
{
  "total": 1500,
  "error": 5,
  "warning": 12,
  "info": 1400,
  "debug": 83
}
```

#### GET `/api/logs/settings`
Get logging configuration.

**Response:**
```json
{
  "enabled": true,
  "logToFile": true,
  "logToSerial": true,
  "logToWeb": true,
  "maxEntries": 1000,
  "retentionDays": 7
}
```

#### POST `/api/logs/settings`
Update logging configuration.

**Request:**
```json
{
  "enabled": true,
  "logToFile": true,
  "maxEntries": 2000,
  "retentionDays": 14
}
```

**Response:**
```json
{
  "status": "success",
  "message": "Log settings updated"
}
```

#### POST `/api/logs/rotate`
Rotate log files.

**Response:**
```json
{
  "status": "success",
  "message": "Logs rotated"
}
```

## OTA Updates

### OTA Management

#### POST `/api/ota/start`
Start OTA update process.

**Request:**
```json
{
  "url": "http://example.com/firmware.bin"
}
```

**Response:**
```json
{
  "status": "success",
  "message": "OTA update started"
}
```

#### POST `/api/ota/stop`
Stop OTA update process.

**Response:**
```json
{
  "status": "success",
  "message": "OTA update stopped"
}
```

#### GET `/api/ota/status`
Get OTA update status.

**Response:**
```json
{
  "status": "idle",
  "progress": 0,
  "message": "No update in progress"
}
```

#### POST `/api/ota/upload`
Upload firmware file for OTA update.

**Request:**
- Form data with file field named "firmware"

**Response:**
```json
{
  "status": "success",
  "message": "Firmware uploaded"
}
```

## Error Handling

### Error Response Format

All API endpoints return standard error responses when operations fail:

```json
{
  "status": "error",
  "message": "Error description",
  "code": 400,
  "details": "Additional error details"
}
```

### Common Error Codes

- `400`: Bad Request - Invalid parameters
- `401`: Unauthorized - Authentication required
- `403`: Forbidden - Access denied
- `404`: Not Found - Resource not found
- `422`: Unprocessable Entity - Validation failed
- `500`: Internal Server Error - Server error
- `503`: Service Unavailable - Service temporarily unavailable

### Error Examples

#### Invalid Parameters
```json
{
  "status": "error",
  "message": "Invalid sensor ID",
  "code": 400,
  "details": "Sensor ID must be between 1 and 10"
}
```

#### Authentication Required
```json
{
  "status": "error",
  "message": "Authentication required",
  "code": 401,
  "details": "Please provide valid credentials"
}
```

#### Resource Not Found
```json
{
  "status": "error",
  "message": "Sensor not found",
  "code": 404,
  "details": "No sensor with ID 999 exists"
}
```

## Examples

### Example 1: Complete Sensor Reading

```bash
curl -X GET "http://esp32.local/api/sensors" \
     -H "Content-Type: application/json"
```

### Example 2: Set RGB LED Color

```bash
curl -X POST "http://esp32.local/api/actuators/rgb/color" \
     -H "Content-Type: application/json" \
     -d '{"red": 255, "green": 128, "blue": 0}'
```

### Example 3: Capture Camera Image

```bash
curl -X POST "http://esp32.local/api/camera/capture" \
     -H "Content-Type: application/json"
```

### Example 4: Send ESP-NOW Message

```bash
curl -X POST "http://esp32.local/api/espnow/message/send" \
     -H "Content-Type: application/json" \
     -d '{
       "mac": "24:0A:C4:12:34:56",
       "type": "sensor",
       "content": "Temperature: 25.5°C"
     }'
```

### Example 5: Subscribe to MQTT Topic

```bash
curl -X POST "http://esp32.local/api/mqtt/subscribe" \
     -H "Content-Type: application/json" \
     -d '{
       "topic": "sensors/temperature",
       "qos": 1
     }'
```

## Notes

- All timestamps are in Unix timestamp format (milliseconds since epoch)
- Color values are in RGB format (0-255) or hex format (#RRGGBB)
- Angles are in degrees (0-180 for servos)
- Speed values are percentages (0-100)
- All API endpoints require proper authentication when enabled
- Some endpoints may require specific hardware configurations
- Response times may vary based on network conditions and device load
