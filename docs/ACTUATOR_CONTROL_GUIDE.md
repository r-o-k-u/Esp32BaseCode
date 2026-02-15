# ESP32 Actuator Control Guide

## Overview

This guide explains how to activate, deactivate, and enable actuators in your ESP32 DualComm System. The system provides multiple control methods including web interface, API calls, and programmatic control.

## Table of Contents

- [Actuator Types](#actuator-types)
- [Web Interface Control](#web-interface-control)
- [API Control](#api-control)
- [Programmatic Control](#programmatic-control)
- [Scene Management](#scene-management)
- [Safety Features](#safety-features)
- [Troubleshooting](#troubleshooting)

## Actuator Types

The system supports 6 main actuator types:

### 1. LED Control
- **Pin**: GPIO 2 (built-in LED)
- **Functions**: On/Off, brightness control
- **Web Interface**: `/actuators.html` - LED Control section

### 2. Buzzer Control
- **Pin**: GPIO 15
- **Functions**: On/Off, tone generation, melodies
- **Web Interface**: `/actuators.html` - Buzzer Control section

### 3. RGB LED Control
- **Pins**: GPIO 16 (Red), 17 (Green), 23 (Blue)
- **Functions**: Color mixing, brightness, effects
- **Web Interface**: `/actuators.html` - RGB LED Control section

### 4. Motor Control (L298N Driver)
- **Driver**: L298N Dual H-Bridge Motor Driver
- **Pins**: GPIO 14 (IN1), 33 (IN2), 32 (Enable PWM)
- **Functions**: Forward/backward, speed control, stop, brake
- **Power**: 5-35V DC, up to 2A per channel
- **Web Interface**: `/actuators.html` - Motor Control section

### 5. Relay Control
- **Pins**: GPIO 27, 26, 25 (3 relays)
- **Functions**: Switch control, pulse mode
- **Web Interface**: `/actuators.html` - Relay Control section

### 6. Servo Control
- **Pins**: GPIO 13, 12 (2 servos)
- **Functions**: Angle control, sweep motion
- **Web Interface**: `/actuators.html` - Servo Control section

## Web Interface Control

### Accessing Actuator Control

1. Connect to your ESP32's WiFi network or local network
2. Open browser and navigate to: `http://[ESP32_IP]/actuators.html`
3. Click the "Actuators" tab in the navigation menu

### Individual Actuator Control

#### LED Control
```javascript
// Turn LED ON
toggleActuator('led', true);

// Turn LED OFF
toggleActuator('led', false);

// Set brightness (0-255)
updateLEDBrightness(128);
```

**Web Interface Elements:**
- Toggle button for ON/OFF
- Brightness slider (0-100%)
- Test and Blink buttons

#### Buzzer Control
```javascript
// Turn buzzer ON
toggleActuator('buzzer', true);

// Turn buzzer OFF
toggleActuator('buzzer', false);

// Play tone
triggerBuzzer(duration_ms, frequency_hz);
```

**Web Interface Elements:**
- Toggle button for ON/OFF
- Frequency slider (100-5000 Hz)
- Duration slider (50-5000 ms)
- Pre-defined melody buttons

#### RGB LED Control
```javascript
// Set color by RGB values
setRGBColor(255, 0, 0); // Red

// Set color by hex
updateRGBColor('#FF0000'); // Red

// Set brightness
setRGBBrightness(128);
```

**Web Interface Elements:**
- Color picker
- RGB sliders
- Pre-defined color buttons
- Effect buttons (Rainbow, Pulse, Chase, Fire)

#### Motor Control
```javascript
// Set direction
setMotorDirection(true);  // Forward
setMotorDirection(false); // Backward

// Set speed (0-100%)
setMotorSpeed(50);

// Stop motor
motorStop();
```

**Web Interface Elements:**
- Direction buttons (Forward, Backward, Stop, Brake)
- Speed slider (0-100%)
- Speed preset buttons (25%, 50%, 75%, 100%)
- Ramp up/down buttons

#### Relay Control
```javascript
// Control individual relays
toggleRelay(1, true);  // Turn relay 1 ON
toggleRelay(2, false); // Turn relay 2 OFF

// Pulse relay
pulseRelay(1, 1000); // Pulse relay 1 for 1 second
```

**Web Interface Elements:**
- Individual relay status indicators
- ON/OFF buttons for each relay
- All On/All Off buttons
- Cycle button
- Pulse duration slider

#### Servo Control
```javascript
// Set servo angle
setServoAngle(1, 90); // Set servo 1 to 90 degrees

// Sweep servo
sweepServo(1, 0, 180, 10); // Sweep servo 1 from 0 to 180 at speed 10
```

**Web Interface Elements:**
- Angle slider (0-180°)
- Preset angle buttons
- Sweep button
- Sweep speed slider

## API Control

### REST API Endpoints

#### Basic Actuator Control
```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "led",
  "value": 1
}
```

**Supported Actuators:**
- `led` - LED control (0/1)
- `buzzer` - Buzzer control (0/1)
- `motor-speed` - Motor speed (0-255)
- `motor-forward` - Motor forward (0/1)
- `motor-backward` - Motor backward (0/1)
- `motor-stop` - Motor stop (0/1)
- `relay1`, `relay2`, `relay3` - Individual relays (0/1)
- `servo1`, `servo2` - Servo angles (0-180)
- `rgb` - RGB color (object with r, g, b values)

#### RGB Color Control
```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "rgb",
  "r": 255,
  "g": 0,
  "b": 0
}
```

#### Buzzer Tone Control
```http
POST /api/actuator
Content-Type: application/json

{
  "actuator": "buzzer",
  "frequency": 1000,
  "duration": 500
}
```

### WebSocket Control

#### Real-time Control
```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://[ESP32_IP]/ws');

// Send actuator commands
ws.send(JSON.stringify({
  type: 'actuator',
  actuator: 'led',
  value: 1
}));
```

#### Status Updates
```javascript
ws.onmessage = function(event) {
  const data = JSON.parse(event.data);
  if (data.type === 'actuatorStatus') {
    console.log('Actuator status:', data);
  }
};
```

## Programmatic Control

### C++ API (Arduino)

#### Basic Usage
```cpp
#include "ActuatorManager.h"

void setup() {
  // Initialize actuator manager
  actuatorManager.begin();
}

void loop() {
  // Control LED
  actuatorManager.setLED(true);
  delay(1000);
  actuatorManager.setLED(false);
  
  // Control RGB LED
  actuatorManager.setRGBColor(255, 0, 0); // Red
  delay(1000);
  actuatorManager.setRGBColor(0, 255, 0); // Green
  delay(1000);
  
  // Control motor
  actuatorManager.setMotorDirection(true);
  actuatorManager.setMotorSpeed(128);
  delay(2000);
  actuatorManager.stopMotor();
}
```

#### Advanced Features
```cpp
// Play melody
const int notes[] = {523, 659, 784};
const int durations[] = {200, 200, 400};
actuatorManager.playMelody(notes, durations, 3);

// Rainbow effect
actuatorManager.rainbowCycle(20);

// Emergency stop
actuatorManager.emergencyStop();

// Scene execution
actuatorManager.executeScene("welcome");
```

### Status Monitoring
```cpp
// Get current status
String status = actuatorManager.getStatus();
Serial.println(status);

// Check individual actuator
bool ledState = actuatorManager.getLED();
int motorSpeed = actuatorManager.getSpeed();
int servoAngle = actuatorManager.getServoAngle(1);
```

## Scene Management

### Pre-defined Scenes

The system includes several pre-defined scenes:

#### Welcome Scene
```javascript
executeScene('welcome');
// Sequence: LED ON → Green RGB → Beep → LED OFF → RGB OFF
```

#### Alert Scene
```javascript
executeScene('alert');
// Sequence: Red RGB → Buzzer ON → Relay pulse → Stop
```

#### Rainbow Scene
```javascript
executeScene('rainbow');
// Continuous rainbow color cycle
```

### Custom Scenes

#### Save Current State as Scene
```javascript
saveCurrentScene();
// Prompts for scene name and saves current actuator states
```

#### Execute Custom Scene
```javascript
executeScene('custom_scene_name');
```

#### Load Scene List
```javascript
loadSceneList();
// Retrieves all saved scenes
```

## Safety Features

### Emergency Stop
```javascript
emergencyStop();
// Immediately turns off all actuators:
// - LED OFF
// - Buzzer OFF
// - Motor STOP
// - RGB OFF
// - All relays OFF
// - Servos to 90°
```

### Safety Status Monitoring
The system monitors:
- Power consumption
- Temperature
- Actuator states
- System health

### Automatic Safety Features
- **Overcurrent Protection**: Automatically disables overloaded actuators
- **Thermal Protection**: Reduces power if temperature too high
- **Timeout Protection**: Automatically turns off actuators after timeout
- **State Validation**: Prevents invalid actuator combinations

## Configuration

### Enabling/Disabling Actuators

In `src/config.h`, you can enable/disable actuators:

```cpp
#define ENABLE_ACTUATORS true  // Enable all actuators
#define ENABLE_LED true        // Enable LED control
#define ENABLE_BUZZER true     // Enable buzzer control
#define ENABLE_RGB true        // Enable RGB control
#define ENABLE_MOTOR true      // Enable motor control
#define ENABLE_RELAY true      // Enable relay control
#define ENABLE_SERVO true      // Enable servo control
```

### Pin Configuration

Actuator pins are defined in `src/config.h`:

```cpp
// LED Configuration
#define LED_PIN 2

// Buzzer Configuration
#define BUZZER_PIN 15

// RGB LED Configuration
#define RGB_R_PIN 16
#define RGB_G_PIN 17
#define RGB_B_PIN 23

// Motor Configuration
#define MOTOR1_IN1 14
#define MOTOR1_IN2 33
#define MOTOR1_EN 32

// Relay Configuration
#define RELAY1_PIN 27
#define RELAY2_PIN 26
#define RELAY3_PIN 25

// Servo Configuration
#define SERVO1_PIN 13
#define SERVO2_PIN 12
```

## Troubleshooting

### Common Issues

#### Actuator Not Responding
1. **Check Power Supply**: Ensure adequate power for the actuator
2. **Check Wiring**: Verify correct pin connections
3. **Check Code**: Ensure actuator is enabled in config
4. **Check Status**: Use getStatus() to verify state

#### Web Interface Not Updating
1. **Check WebSocket**: Verify WebSocket connection is active
2. **Check Network**: Ensure stable WiFi connection
3. **Refresh Page**: Try refreshing the browser page
4. **Check Console**: Look for JavaScript errors

#### API Calls Failing
1. **Check URL**: Verify correct endpoint URL
2. **Check JSON**: Ensure valid JSON format
3. **Check Headers**: Include proper Content-Type
4. **Check Network**: Verify network connectivity

### Debug Mode

Enable debug output in `src/config.h`:

```cpp
#define DEBUG_MODE true
#define DEBUG_ACTUATORS true
```

This will show detailed actuator control messages in the Serial Monitor.

### Testing Individual Actuators

Use the test functions in the web interface:

```javascript
// Test LED
testLED();

// Test buzzer
triggerBuzzer(500);

// Test RGB
setRGBColor(255, 0, 0);

// Test motor
motorForward();
delay(1000);
motorStop();
```

### Power Considerations

#### Current Requirements
- **LED**: ~20mA
- **Buzzer**: ~30mA
- **RGB LED**: ~60mA (full brightness)
- **Motor**: 100-500mA (depending on load)
- **Relay**: 20-100mA per relay
- **Servo**: 100-500mA (depending on load)

#### Power Supply Recommendations
- **USB Power**: Good for testing, limited current
- **External 5V Supply**: Recommended for multiple actuators
- **Battery Power**: Use appropriate voltage regulators
- **Separate Power**: Use separate supplies for high-current devices

## Best Practices

### 1. Always Use Emergency Stop
```javascript
// Before any actuator operation
emergencyStop(); // Ensure clean state

// Then activate desired actuators
setLED(true);
setRGBColor(0, 255, 0);
```

### 2. Check Actuator Status
```javascript
// Before controlling, check current state
String status = actuatorManager.getStatus();
Serial.println("Current status: " + status);
```

### 3. Use Appropriate Power Supplies
- **Low current devices** (LED, buzzer): USB power OK
- **High current devices** (motor, relays): External power recommended
- **Always common ground** between ESP32 and external power

### 4. Implement Safety Timeouts
```cpp
// Example: Turn on motor for 5 seconds then stop
setMotorSpeed(128);
setMotorDirection(true);
delay(5000);
motorStop();
```

### 5. Handle Errors Gracefully
```cpp
// Always check for errors in critical operations
if (!actuatorManager.setLED(true)) {
  Serial.println("Failed to turn on LED");
  // Handle error appropriately
}
```

### 6. Use Scenes for Complex Sequences
```javascript
// Instead of multiple individual commands
executeScene('welcome'); // Clean, tested sequence
```

## Advanced Control Features

### PWM Control and Dimming

#### LED Dimming with PWM
```javascript
// Smooth LED dimming
function smoothDimming(startBrightness, endBrightness, duration) {
  const steps = 50;
  const stepDuration = duration / steps;
  const stepSize = (endBrightness - startBrightness) / steps;
  
  let currentBrightness = startBrightness;
  
  for (let i = 0; i <= steps; i++) {
    updateLEDBrightness(Math.round(currentBrightness));
    currentBrightness += stepSize;
    delay(stepDuration);
  }
}

// Usage: Dim LED from 100% to 0% over 2 seconds
smoothDimming(255, 0, 2000);
```

#### RGB Color Fading
```javascript
// Smooth color transition
function fadeToColor(startColor, endColor, duration) {
  const steps = 100;
  const stepDuration = duration / steps;
  
  const rStep = (endColor.r - startColor.r) / steps;
  const gStep = (endColor.g - startColor.g) / steps;
  const bStep = (endColor.b - startColor.b) / steps;
  
  let r = startColor.r;
  let g = startColor.g;
  let b = startColor.b;
  
  for (let i = 0; i <= steps; i++) {
    setRGBColor(Math.round(r), Math.round(g), Math.round(b));
    r += rStep;
    g += gStep;
    b += bStep;
    delay(stepDuration);
  }
}

// Usage: Fade from red to blue over 3 seconds
fadeToColor({r: 255, g: 0, b: 0}, {r: 0, g: 0, b: 255}, 3000);
```

### Motor Control Advanced Features

#### Speed Ramp Control
```javascript
// Smooth acceleration/deceleration
function rampMotorSpeed(startSpeed, endSpeed, duration) {
  const steps = 50;
  const stepDuration = duration / steps;
  const stepSize = (endSpeed - startSpeed) / steps;
  
  let currentSpeed = startSpeed;
  
  for (let i = 0; i <= steps; i++) {
    setMotorSpeed(Math.round(currentSpeed));
    currentSpeed += stepSize;
    delay(stepDuration);
  }
}

// Usage: Accelerate motor from 0 to 100% over 2 seconds
rampMotorSpeed(0, 100, 2000);
```

#### Position Control with Encoders
```javascript
// Motor position control (requires encoder feedback)
function moveToPosition(targetPosition, maxSpeed = 100) {
  const encoderPin = 2; // Example encoder pin
  let currentPosition = 0;
  
  while (currentPosition < targetPosition) {
    setMotorDirection(true);
    setMotorSpeed(maxSpeed);
    
    // Read encoder
    currentPosition = digitalRead(encoderPin);
    delay(10);
  }
  
  motorStop();
}
```

### Servo Control Advanced Features

#### Smooth Servo Movement
```javascript
// Smooth servo transition
function smoothServoMove(servo, startAngle, endAngle, duration) {
  const steps = 50;
  const stepDuration = duration / steps;
  const stepSize = (endAngle - startAngle) / steps;
  
  let currentAngle = startAngle;
  
  for (let i = 0; i <= steps; i++) {
    setServoAngle(servo, Math.round(currentAngle));
    currentAngle += stepSize;
    delay(stepDuration);
  }
}

// Usage: Move servo 1 from 0° to 180° over 1 second
smoothServoMove(1, 0, 180, 1000);
```

#### Multi-Servo Coordination
```javascript
// Synchronized servo movement
function synchronizedServos(angles, duration) {
  const steps = 100;
  const stepDuration = duration / steps;
  
  const servo1Step = (angles.servo1.end - angles.servo1.start) / steps;
  const servo2Step = (angles.servo2.end - angles.servo2.start) / steps;
  
  let s1 = angles.servo1.start;
  let s2 = angles.servo2.start;
  
  for (let i = 0; i <= steps; i++) {
    setServoAngle(1, Math.round(s1));
    setServoAngle(2, Math.round(s2));
    
    s1 += servo1Step;
    s2 += servo2Step;
    delay(stepDuration);
  }
}

// Usage: Move both servos simultaneously
synchronizedServos({
  servo1: { start: 0, end: 90 },
  servo2: { start: 180, end: 45 }
}, 2000);
```

### Relay Control Advanced Features

#### Sequential Relay Activation
```javascript
// Sequential relay control
function sequentialRelayActivation(relayOrder, delayTime) {
  relayOrder.forEach((relay, index) => {
    setTimeout(() => {
      toggleRelay(relay, true);
      showToast(`Relay ${relay} activated`, 'success');
    }, index * delayTime);
  });
}

// Usage: Activate relays 1, 2, 3 with 1 second delay
sequentialRelayActivation([1, 2, 3], 1000);
```

#### Relay Safety Interlocks
```javascript
// Safety interlock system
function safeRelayControl(relay, state, interlockRelays = []) {
  // Check interlocks
  for (let i = 0; i < interlockRelays.length; i++) {
    if (getRelay(interlockRelays[i])) {
      showToast(`Cannot activate relay ${relay}: Interlock ${interlockRelays[i]} is active`, 'warning');
      return false;
    }
  }
  
  // Activate relay
  toggleRelay(relay, state);
  return true;
}

// Usage: Activate relay 1 only if relays 2 and 3 are off
safeRelayControl(1, true, [2, 3]);
```

### Buzzer Advanced Features

#### Melody Generation
```javascript
// Generate custom melodies
function playCustomMelody(notes, durations, tempo = 1.0) {
  for (let i = 0; i < notes.length; i++) {
    const note = notes[i];
    const duration = durations[i] / tempo;
    
    playTone(note, duration);
    delay(duration + 50); // Small pause between notes
  }
}

// Musical notes (frequencies in Hz)
const notes = {
  'C4': 262, 'D4': 294, 'E4': 330, 'F4': 349,
  'G4': 392, 'A4': 440, 'B4': 494, 'C5': 523
};

// Usage: Play "Twinkle Twinkle Little Star"
const melody = [
  notes.C4, notes.C4, notes.G4, notes.G4,
  notes.A4, notes.A4, notes.G4
];
const durations = [500, 500, 500, 500, 500, 500, 1000];

playCustomMelody(melody, durations);
```

#### Siren Effects
```javascript
// Police siren effect
function policeSiren(duration) {
  const startTime = millis();
  
  while (millis() - startTime < duration) {
    // High pitch
    for (int freq = 800; freq <= 1200; freq += 10) {
      playTone(freq, 10);
    }
    
    // Low pitch
    for (int freq = 1200; freq >= 800; freq -= 10) {
      playTone(freq, 10);
    }
  }
  
  setBuzzer(false);
}

// Usage: Play police siren for 5 seconds
policeSiren(5000);
```

## Integration Examples

### Smart Home Automation System
```javascript
// Complete home automation sequence
function smartHomeAutomation() {
  const time = new Date().getHours();
  
  // Morning routine (6-8 AM)
  if (time >= 6 && time <= 8) {
    // Gradual wake-up light
    fadeToColor({r: 0, g: 0, b: 0}, {r: 255, g: 100, b: 0}, 30000);
    
    // Turn on coffee maker
    toggleRelay(1, true);
    
    // Open curtains (servo)
    smoothServoMove(1, 0, 90, 5000);
  }
  
  // Evening routine (6-10 PM)
  else if (time >= 18 && time <= 22) {
    // Warm lighting
    setRGBColor(255, 100, 0);
    
    // Turn on heater if cold
    const temp = parseFloat(document.getElementById('temperature-value').textContent);
    if (temp < 20) {
      toggleRelay(2, true);
    }
  }
  
  // Night routine (10 PM - 6 AM)
  else {
    // Security mode
    executeScene('security');
    
    // Turn off all non-essential devices
    allRelaysOff();
    setRGBColor(0, 0, 0);
  }
}
```

### Industrial Control System
```javascript
// Conveyor belt control system
function conveyorControlSystem() {
  // Safety checks
  if (!checkSafetyInterlocks()) {
    emergencyStop();
    return;
  }
  
  // Start sequence
  toggleRelay(1, true);  // Power conveyor
  delay(1000);
  
  // Accelerate conveyor
  rampMotorSpeed(0, 80, 3000);
  
  // Run for specified time
  const runTime = 60000; // 1 minute
  const startTime = millis();
  
  while (millis() - startTime < runTime) {
    // Monitor sensors
    const distance = parseFloat(document.getElementById('distance-value').textContent);
    
    // Stop if object detected
    if (distance > 0 && distance < 10) {
      motorStop();
      toggleRelay(2, true); // Activate sorting mechanism
      delay(2000);
      toggleRelay(2, false);
      rampMotorSpeed(0, 80, 2000); // Resume
    }
    
    delay(100);
  }
  
  // Stop sequence
  rampMotorSpeed(80, 0, 3000);
  delay(2000);
  toggleRelay(1, false); // Power off
}
```

### Environmental Control System
```javascript
// Greenhouse environmental control
function greenhouseControl() {
  const temp = parseFloat(document.getElementById('temperature-value').textContent);
  const hum = parseFloat(document.getElementById('humidity-value').textContent);
  const soil = parseInt(document.getElementById('soil-moisture').textContent);
  const light = parseInt(document.getElementById('light-level').textContent);
  
  // Temperature control
  if (temp > 30) {
    toggleRelay(1, true);  // Exhaust fan
    toggleRelay(2, false); // Heater off
  } else if (temp < 20) {
    toggleRelay(1, false); // Exhaust off
    toggleRelay(2, true);  // Heater on
  }
  
  // Humidity control
  if (hum < 40) {
    toggleRelay(3, true);  // Humidifier
  } else if (hum > 80) {
    toggleRelay(1, true);  // Exhaust fan
  }
  
  // Watering control
  if (soil < 30) {
    toggleRelay(4, true);  // Water pump
    delay(10000);          // Water for 10 seconds
    toggleRelay(4, false);
  }
  
  // Light control
  if (light < 500) {
    setRGBColor(255, 255, 255); // Grow lights
  } else {
    setRGBColor(0, 0, 0);       // Lights off
  }
}
```

### Security System
```javascript
// Advanced security system
function securitySystem() {
  const motion = document.getElementById('motion-state').textContent;
  const distance = parseFloat(document.getElementById('distance-value').textContent);
  const time = new Date().getHours();
  
  // Only active at night (8 PM - 6 AM)
  if (time >= 20 || time <= 6) {
    if (motion === 'Active' || (distance > 0 && distance < 50)) {
      // Intruder detected
      
      // Flash lights
      for (let i = 0; i < 10; i++) {
        setRGBColor(255, 0, 0); // Red
        delay(200);
        setRGBColor(0, 0, 0);   // Off
        delay(200);
      }
      
      // Sound alarm
      policeSiren(3000);
      
      // Lock doors (servos)
      setServoAngle(1, 0); // Lock position
      setServoAngle(2, 0);
      
      // Send alert
      triggerAlert();
      
      // Record event
      addActivityLog('Security breach detected and handled');
    }
  }
}
```

## Performance Optimization

### Efficient Control Loops
```javascript
// Optimized control loop
function optimizedControlLoop() {
  const loopInterval = 100; // 10 Hz update rate
  let lastUpdateTime = 0;
  
  setInterval(() => {
    const currentTime = millis();
    
    if (currentTime - lastUpdateTime >= loopInterval) {
      // Read sensors
      refreshSensors();
      
      // Process control logic
      processControlLogic();
      
      // Update actuators
      updateActuators();
      
      lastUpdateTime = currentTime;
    }
  }, 10);
}
```

### Memory Management
```cpp
// Efficient memory usage in C++
void manageMemory() {
  // Use static variables instead of dynamic allocation
  static char buffer[256];
  static int sensorReadings[8];
  
  // Clear arrays efficiently
  memset(sensorReadings, 0, sizeof(sensorReadings));
  
  // Use PROGMEM for constants
  const char message[] PROGMEM = "System ready";
}
```

### Power Management
```cpp
// Power-saving modes
void powerManagement() {
  // Sleep mode when idle
  if (systemIdle) {
    // Turn off non-essential actuators
    allRelaysOff();
    setRGBColor(0, 0, 0);
    setLED(false);
    
    // Enter deep sleep
    esp_sleep_enable_timer_wakeup(60000000); // Wake in 60 seconds
    esp_deep_sleep_start();
  }
}
```

## Troubleshooting Advanced Issues

### Motor Control Problems
```javascript
// Motor troubleshooting
function troubleshootMotor() {
  // Check power supply
  const voltage = getPowerVoltage();
  if (voltage < 11.0) {
    showToast('Low voltage detected', 'warning');
    return;
  }
  
  // Check for overheating
  const temp = getMotorTemperature();
  if (temp > 70) {
    showToast('Motor overheating', 'error');
    emergencyStop();
    return;
  }
  
  // Check encoder feedback
  const position = getMotorPosition();
  if (position === 0) {
    showToast('Encoder fault detected', 'error');
  }
}
```

### Sensor Integration Issues
```javascript
// Sensor-actuator synchronization
function synchronizeSensorActuator() {
  // Ensure sensor reading is complete before actuator response
  refreshSensors();
  
  // Wait for sensor stabilization
  setTimeout(() => {
    // Process sensor data
    const data = getSensorData();
    
    // Update actuators based on processed data
    updateActuatorsBasedOnData(data);
  }, 100);
}
```

### Network Communication Issues
```javascript
// WebSocket reconnection handling
function handleWebSocketReconnect() {
  if (ws && ws.readyState !== WebSocket.OPEN) {
    showToast('Reconnecting to WebSocket...', 'info');
    
    // Attempt reconnection
    setTimeout(() => {
      connectWebSocket();
    }, 5000);
  }
}
```

## Best Practices Summary

### 1. Safety First
```javascript
// Always implement safety checks
function safeActuatorControl() {
  // Check interlocks
  if (!checkSafetyInterlocks()) {
    return false;
  }
  
  // Check limits
  if (!checkActuatorLimits()) {
    return false;
  }
  
  // Enable actuator
  return true;
}
```

### 2. Error Handling
```javascript
// Comprehensive error handling
function robustActuatorControl() {
  try {
    // Control logic
    executeControlLogic();
  } catch (error) {
    // Log error
    logError(error);
    
    // Emergency stop
    emergencyStop();
    
    // Notify user
    showToast('System error occurred', 'error');
  }
}
```

### 3. Testing and Validation
```javascript
// Thorough testing procedures
function testActuatorSystem() {
  // Test individual actuators
  testLED();
  testRGBLED();
  testRelays();
  testServos();
  testMotor();
  testBuzzer();
  
  // Test integrated sequences
  testScene('alert');
  testScene('security');
  testScene('home');
  
  // Test safety systems
  testEmergencyStop();
  testInterlocks();
  testLimits();
}
```

### 4. Documentation and Logging
```javascript
// Maintain comprehensive logs
function logActuatorActivity(action, result, details) {
  const logEntry = {
    timestamp: new Date().toISOString(),
    action: action,
    result: result,
    details: details,
    user: getCurrentUser()
  };
  
  addActivityLog(JSON.stringify(logEntry));
}
```

## Example Applications

### Home Automation
```javascript
// Turn on lights and fan
toggleRelay(1, true);  // Lights
toggleRelay(2, true);  // Fan
setRGBColor(255, 255, 255); // White light
```

### Security System
```javascript
// Alert sequence
executeScene('alert'); // Red lights + siren
pulseRelay(3, 5000);   // Activate alarm
```

### Plant Monitoring
```javascript
// Water pump control
if (soil_moisture < 30) {
  toggleRelay(1, true);  // Turn on pump
  delay(10000);          // Run for 10 seconds
  toggleRelay(1, false); // Turn off pump
}
```

### Motor Control
```javascript
// Conveyor belt control
setMotorDirection(true); // Forward
setMotorSpeed(200);      // High speed
delay(30000);            // Run for 30 seconds
motorStop();             // Stop
```

This guide provides comprehensive information for controlling actuators in your ESP32 system. Always follow safety guidelines and test thoroughly before deploying in production environments.
