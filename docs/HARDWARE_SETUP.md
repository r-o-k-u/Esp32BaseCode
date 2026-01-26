# ESP32 DualComm System - Hardware Setup

This document provides comprehensive hardware setup instructions for the ESP32 DualComm System, including pin connections, schematics, and assembly instructions.

## Table of Contents

- [Hardware Requirements](#hardware-requirements)
- [ESP32 DevKit Pinout](#esp32-devkit-pinout)
- [ESP32-CAM Pinout](#esp32-cam-pinout)
- [Sensor Connections](#sensor-connections)
- [Actuator Connections](#actuator-connections)
- [Power Supply](#power-supply)
- [Schematics](#schematics)
- [Assembly Instructions](#assembly-instructions)
- [Testing Hardware](#testing-hardware)
- [Troubleshooting Hardware Issues](#troubleshooting-hardware-issues)

## Hardware Requirements

### Core Components

#### For ESP32 DevKit:
- **ESP32 Development Board** (30-pin or 38-pin version)
  - Recommended: ESP32-WROOM-32 DevKit V1 or V4
  - Compatible: ESP32-S2, ESP32-S3 (limited sensor support)
- **USB Cable** (Type A to Micro-USB for programming)
- **Power Supply** (5V/2A minimum, stable voltage required)
- **Breadboard** (for prototyping, optional)
- **Jumper Wires** (male-male, male-female, female-female)

#### For ESP32-CAM:
- **ESP32-CAM Board** (AI Thinker or similar)
- **FTDI Programmer** or **ESP32-CAM-MB Programmer Board**
- **External Antenna** (optional, improves range)
- **Power Supply** (5V/2A minimum, camera draws significant current)
- **Ribbon Cable** (for camera connection, usually included)

### Sensors (Optional, based on project needs)

#### Environmental Sensors:
- **DHT22 Temperature/Humidity Sensor** (3-pin or 4-pin)
- **BMP280 Pressure/Altitude Sensor** (I2C interface)
- **HC-SR04 Ultrasonic Distance Sensor**
- **PIR Motion Sensor** (HC-SR501)
- **LDR Light Sensor** (with resistor)
- **Soil Moisture Sensor** (capacitive or resistive)
- **MPU6050 Accelerometer/Gyroscope** (I2C interface)
- **MQ135 Air Quality Sensor**

### Actuators (Optional, based on project needs)

#### Output Devices:
- **SG90 Servo Motors** (x2 recommended)
- **5V Relay Modules** (active low, x3 recommended)
- **LEDs** (various colors)
- **Active Buzzer** (5V)
- **L298N Motor Driver** (for DC motors)
- **RGB LED Module** (common cathode)
- **DC Motors** (with appropriate power supply)

### Power and Miscellaneous:
- **5V/2A Power Supply** (for ESP32-CAM and actuators)
- **3.3V Regulator** (if needed for sensors)
- **Resistors** (220Ω, 10KΩ, 4.7KΩ pull-ups)
- **Capacitors** (10µF, 100nF for power filtering)
- **Diodes** (1N4001 for motor protection)

## ESP32 DevKit Pinout

### ESP32-WROOM-32 DevKit (30-pin)

```
                    USB
                 ┌─────┐
          GND ── │○ ○○├─ 3.3V
          GPIO13│○ ○○├─ GPIO12
          GPIO15│○ ○○├─ GPIO14
          GPIO2 ──│○ ○○├─ GPIO27
          GPIO4 ──│○ ○○├─ GPIO26
          GPIO5 ──│○ ○○├─ GPIO25
          GPIO18 ─│○ ○○├─ GPIO33
          GPIO19 ─│○ ○○├─ GPIO32
          GPIO21 ─│○ ○○├─ GPIO35 (ADC)
          GPIO22 ─│○ ○○├─ GPIO34 (ADC)
          GPIO23 ─│○ ○○├─ GPIO39 (ADC)
                 └─────┘
                    ESP32
```

### Pin Functions and Constraints

| Pin | Function | Notes | Safe for Sensors? | Safe for Actuators? |
|-----|----------|-------|-------------------|---------------------|
| GPIO0 | Boot/Program | Pulled up, don't use | ❌ | ❌ |
| GPIO1 | UART TX | Serial output | ❌ | ❌ |
| GPIO2 | LED_BUILTIN | On-board LED | ❌ | ✅ |
| GPIO3 | UART RX | Serial input | ❌ | ❌ |
| GPIO4 | ADC2_CH0 | DHT22, LDR | ✅ | ✅ |
| GPIO5 | ADC2_CH1 | Ultrasonic TRIG | ✅ | ✅ |
| GPIO6-11 | SPI Flash | Internal use | ❌ | ❌ |
| GPIO12 | ADC2_CH2 | Boot control | ⚠️ | ✅ |
| GPIO13 | ADC2_CH3 | Servo 1, Buzzer | ✅ | ✅ |
| GPIO14 | ADC2_CH4 | Motor IN1, Relay | ✅ | ✅ |
| GPIO15 | ADC2_CH5 | Servo 2, Boot control | ⚠️ | ✅ |
| GPIO16 | UART RX2 | Serial comm | ✅ | ✅ |
| GPIO17 | UART TX2 | Serial comm | ✅ | ✅ |
| GPIO18 | ADC2_CH6 | Ultrasonic ECHO | ✅ | ✅ |
| GPIO19 | ADC2_CH7 | PIR sensor | ✅ | ✅ |
| GPIO21 | I2C SDA | BMP280, MPU6050 | ✅ | ✅ |
| GPIO22 | I2C SCL | BMP280, MPU6050 | ✅ | ✅ |
| GPIO23 | ADC2_CH7 | Motor EN, Relay | ✅ | ✅ |
| GPIO25 | ADC2_CH8 | Relay 2, Buzzer | ✅ | ✅ |
| GPIO26 | ADC2_CH9 | RGB R, Motor IN2 | ✅ | ✅ |
| GPIO27 | ADC2_CH10 | Relay 3, RGB G | ✅ | ✅ |
| GPIO32 | ADC1_CH4 | MQ135, Motor IN3 | ✅ | ✅ |
| GPIO33 | ADC1_CH5 | Motor IN4, RGB B | ✅ | ✅ |
| GPIO34 | ADC1_CH6 | Soil Moisture | ✅ | ❌ (input only) |
| GPIO35 | ADC1_CH7 | LDR alt, Soil alt | ✅ | ❌ (input only) |
| GPIO36-39 | ADC1 | Internal sensors | ✅ | ❌ (input only) |

**⚠️ Important Notes:**
- GPIO12 and GPIO15 are boot control pins - avoid using if possible
- ADC2 pins (GPIO0-15, 25-27) may not work when WiFi is active
- Use ADC1 pins (GPIO32-39) for sensors when possible
- GPIO34, 35, 36, 39 are input-only (no internal pull-up)

## ESP32-CAM Pinout

### AI Thinker ESP32-CAM

```
Camera Connector (top):
┌─────────────────────────────────┐
│ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
│ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ ○ │
└─────────────────────────────────┘

Pin Functions:
1:  D0 (GPIO36)  9:  D8 (GPIO9)   17: CMD (GPIO11)
2:  D1 (GPIO39)  10: D9 (GPIO10)  18: PCLK (GPIO22)
3:  D2 (GPIO34)  11: D10 (GPIO12) 19: VSYNC (GPIO25)
4:  D3 (GPIO35)  12: D11 (GPIO13) 20: HREF (GPIO23)
5:  D4 (GPIO32)  13: D12 (GPIO14) 21: SDA (GPIO26)
6:  D5 (GPIO33)  14: D13 (GPIO15) 22: SCL (GPIO27)
7:  GND          15: GND          23: GND
8:  D6 (GPIO25)  16: D7 (GPIO26)  24: Y9 (GPIO19)
```

### Available GPIO Pins on ESP32-CAM

**⚠️ WARNING:** ESP32-CAM has very limited available GPIOs!

```
Available for user:
- GPIO2   (limited use)
- GPIO13  (PIR sensor)
- GPIO12  (Relay)
- GPIO14  (Servo)
- GPIO15  (LDR)
- GPIO4   (Flash LED)

Used by camera (DO NOT USE):
- GPIO0, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 34, 35, 36, 39

Power pins:
- 3.3V, 5V, GND
```

### ESP32-CAM Limitations

1. **Limited GPIO:** Only 6 pins available for sensors/actuators
2. **No USB:** Requires external programmer (FTDI/USB-TTL)
3. **High Power Draw:** Camera + ESP32 requires 5V/2A minimum
4. **Heat Generation:** Camera module gets hot during operation
5. **No Built-in LED:** GPIO2 not easily accessible for status LED

## Sensor Connections

### DHT22 Temperature/Humidity Sensor

#### 4-Pin Version (with pull-up):
```
ESP32 GPIO4 ──────┬───── DHT22 Data
                  │
               10KΩ Resistor
                  │
ESP32 3.3V ───────┼───── DHT22 VCC
ESP32 GND ────────────── DHT22 GND
```

#### 3-Pin Version:
```
ESP32 GPIO4 ────── DHT22 Data
ESP32 3.3V ─────── DHT22 VCC
ESP32 GND ──────── DHT22 GND
```

**Notes:**
- Use GPIO4 (ADC2) or GPIO32 (ADC1) for best compatibility
- 10KΩ pull-up resistor required for 4-pin version
- Keep wiring short (<20cm)
- Power from 3.3V, not 5V

### BMP280 Pressure Sensor (I2C)

```
ESP32 GPIO21 (SDA) ────── BMP280 SDA
ESP32 GPIO22 (SCL) ────── BMP280 SCL
ESP32 3.3V ────────────── BMP280 VCC
ESP32 GND ─────────────── BMP280 GND
```

**Notes:**
- Default I2C address: 0x76 or 0x77 (check your module)
- 4.7KΩ pull-up resistors recommended on SDA/SCL
- Can share I2C bus with MPU6050

### HC-SR04 Ultrasonic Sensor

```
ESP32 GPIO5 ────── HC-SR04 TRIG
ESP32 GPIO18 ───── HC-SR04 ECHO
ESP32 5V ───────── HC-SR04 VCC
ESP32 GND ──────── HC-SR04 GND
```

**Notes:**
- TRIG pin can be any GPIO
- ECHO pin must be interrupt-capable (GPIO18, 19, 21-23, 25-27)
- Power from 5V for reliable operation
- Maximum range: ~4 meters
- Minimum range: ~2 cm

### PIR Motion Sensor (HC-SR501)

```
ESP32 GPIO19 ───── PIR OUT
ESP32 5V ───────── PIR VCC
ESP32 GND ──────── PIR GND
```

**Notes:**
- OUT pin can be any GPIO with interrupt capability
- Power from 5V (some modules work with 3.3V)
- Adjustable sensitivity and delay via onboard potentiometers
- Warm-up time: ~1 minute after power-on

### LDR Light Sensor

```
ESP32 GPIO34 ──────┬───── LDR
                   │
                10KΩ Resistor
                   │
ESP32 GND ─────────┼───── GND
```

**Notes:**
- GPIO34 is ADC1 (recommended) or GPIO35
- 10KΩ resistor creates voltage divider
- ADC value increases with light intensity
- Calibrate for your specific LDR

### Soil Moisture Sensor

```
ESP32 GPIO35 ────── Soil AO (Analog Out)
ESP32 3.3V ──────── Soil VCC
ESP32 GND ───────── Soil GND
```

**Notes:**
- Use ADC1 pin (GPIO32-35, 39)
- Power from 3.3V to avoid corrosion
- Clean sensor regularly to prevent corrosion
- Some sensors have DO (digital out) pin - not used here

### MPU6050 Accelerometer/Gyroscope (I2C)

```
ESP32 GPIO21 (SDA) ────── MPU6050 SDA
ESP32 GPIO22 (SCL) ────── MPU6050 SCL
ESP32 3.3V ────────────── MPU6050 VCC
ESP32 GND ─────────────── MPU6050 GND
ESP32 GPIO4 ───────────── MPU6050 INT (optional)
```

**Notes:**
- Default I2C address: 0x68
- Can share I2C bus with BMP280
- INT pin optional for interrupt-driven reading
- Requires calibration for accurate readings

### MQ135 Air Quality Sensor

```
ESP32 GPIO32 ────── MQ135 AO (Analog Out)
ESP32 5V ────────── MQ135 VCC
ESP32 GND ───────── MQ135 GND
```

**Notes:**
- Use ADC1 pin for WiFi compatibility
- Requires 5V power
- Warm-up time: 24-48 hours for accurate readings
- Preheat for 20 minutes before calibration

## Actuator Connections

### Servo Motors (SG90)

#### Servo 1:
```
ESP32 GPIO13 ───── Servo Signal
ESP32 5V ───────── Servo VCC (external power recommended)
ESP32 GND ──────── Servo GND
```

#### Servo 2:
```
ESP32 GPIO12 ───── Servo Signal
ESP32 5V ───────── Servo VCC
ESP32 GND ──────── Servo GND
```

**Notes:**
- Use PWM-capable pins (GPIO12, 13, 14, 15, 16, 17, 18, 19, 21-23, 25-27)
- External 5V/2A power supply recommended for multiple servos
- Signal wire is usually orange/yellow
- Pulse width: 500-2500µs (0-180°)

### Relay Modules (5V Active Low)

#### Relay 1:
```
ESP32 GPIO27 ───── Relay IN
ESP32 5V ───────── Relay VCC
ESP32 GND ──────── Relay GND
```

#### Relay 2:
```
ESP32 GPIO26 ───── Relay IN
ESP32 5V ───────── Relay VCC
ESP32 GND ──────── Relay GND
```

#### Relay 3:
```
ESP32 GPIO25 ───── Relay IN
ESP32 5V ───────── Relay VCC
ESP32 GND ──────── Relay GND
```

**Notes:**
- Active LOW: HIGH = relay OFF, LOW = relay ON
- Each relay can switch AC/DC loads up to rated capacity
- Use external power for high-current loads
- Add flyback diode for inductive loads

### LED (Status LED)

```
ESP32 GPIO2 ──────┬───── LED Anode (+)
                  │
               220Ω Resistor
                  │
ESP32 GND ────────┼───── LED Cathode (-)
```

**Notes:**
- GPIO2 has built-in LED on most ESP32 DevKit boards
- 220Ω resistor protects LED (3.3V operation)
- Can use any GPIO pin
- Current: ~10mA maximum

### Active Buzzer

```
ESP32 GPIO15 ───── Buzzer + (Positive)
ESP32 GND ──────── Buzzer - (Negative)
```

**Notes:**
- Active buzzer has built-in oscillator
- Just needs power to make sound
- Frequency depends on buzzer specifications
- Can be PWM controlled for different tones

### L298N Motor Driver

```
ESP32 GPIO14 ───── L298N IN1
ESP32 GPIO33 ───── L298N IN2
ESP32 GPIO32 ───── L298N EN (PWM for speed)
ESP32 GPIO16 ───── L298N IN3
ESP32 GPIO17 ───── L298N IN4
ESP32 GPIO23 ───── L298N EN2 (PWM for speed)

External 12V ───── L298N 12V
ESP32 GND ──────── L298N GND

Motor 1:
L298N OUT1 ─────── Motor 1 +
L298N OUT2 ─────── Motor 1 -

Motor 2:
L298N OUT3 ─────── Motor 2 +
L298N OUT4 ─────── Motor 2 -
```

**Notes:**
- 12V input for motors, 5V output for logic
- EN pins control speed via PWM
- IN1-IN4 control direction
- Add heat sink for high current applications

### RGB LED (Common Cathode)

```
ESP32 GPIO16 ──────┬───── RGB Red +
                   │
                220Ω Resistor
                   │
ESP32 GPIO17 ──────┬───── RGB Green +
                   │
                220Ω Resistor
                   │
ESP32 GPIO23 ──────┬───── RGB Blue +
                   │
                220Ω Resistor
                   │
ESP32 GND ─────────┼───── RGB Common (-)
```

**Notes:**
- Common cathode: connect common to GND
- 220Ω resistors for each color
- PWM pins for smooth color transitions
- Can mix colors for any hue

## Power Supply

### ESP32 DevKit Power Requirements

- **Voltage:** 5V via USB or VIN pin
- **Current:** 500mA typical, 1A peak
- **3.3V Output:** 1A maximum (for sensors)

### ESP32-CAM Power Requirements

- **Voltage:** 5V stable supply
- **Current:** 2A minimum (camera + ESP32)
- **Ripple:** <100mV (use capacitor filtering)
- **Connector:** Micro-USB or external power

### Power Supply Circuit

```
5V Power Supply ──┬─── 100µF Capacitor ──┬─── ESP32 VIN
                  │                       │
                  └───── 10µF Capacitor ──┼─── ESP32 GND
                                          │
                                          └──── Sensors/Actuators
```

### Battery Power (Optional)

For portable applications:

```
LiPo Battery ──┬─── TP4056 Charger ──┬─── Boost Converter (5V) ──┬─── ESP32
               │                     │                           │
               └───── Protection ────┼─── Battery Monitor ──────┼─── Voltage Divider
                                     │                           │
                                     └───────────────────────────┼─── ESP32 ADC (GPIO34)
                                                                 │
                                                                 └─── ESP32 GND
```

## Schematics

### Complete ESP32 DevKit Schematic

```
ESP32 DevKit           Sensors                     Actuators
┌─────────────────┐    ┌─────────────────────┐    ┌─────────────────────┐
│                 │    │ DHT22   BMP280      │    │ LED   Buzzer        │
│ GPIO4 ──►[DHT] │    │ GPIO4   GPIO21/22   │    │ GPIO2  GPIO15        │
│ GPIO21/22 ─►[BMP]│    │ PIR     MPU6050     │    │ Relay1 Relay2       │
│ GPIO19 ──►[PIR] │    │ GPIO19  GPIO21/22   │    │ GPIO27 GPIO26       │
│ GPIO34 ──►[LDR] │    │ HC-SR04 Ultrasonic  │    │ Relay3 Servo1       │
│ GPIO35 ──►[SOIL]│    │ GPIO5/18 GPIO5/18   │    │ GPIO25 GPIO13       │
│ GPIO32 ──►[MQ135│    │ MQ135   Soil        │    │ Servo2 RGB          │
│                 │    │ GPIO32  GPIO35      │    │ GPIO12 GPIO16/17/23 │
│ 3.3V ──────────┼────┼─► Power              │    │ Motor  L298N        │
│ GND ───────────┼────┼─► Ground             │    │ GPIO14/33/32/16/17/23│
└─────────────────┘    └─────────────────────┘    └─────────────────────┘
```

### ESP32-CAM Limited Connections

```
ESP32-CAM             Available Pins
┌─────────────────┐    ┌─────────────────────┐
│ Camera Module   │    │ GPIO2  - Limited   │
│ (OV2640)        │    │ GPIO13 - PIR       │
│ Connected       │    │ GPIO12 - Relay     │
│                 │    │ GPIO14 - Servo     │
│ GPIO13 ──►[PIR] │    │ GPIO15 - LDR       │
│ GPIO12 ──►[RELAY│    │ GPIO4  - Flash LED │
│ GPIO14 ──►[SERVO│    │                     │
│ GPIO15 ──►[LDR] │    │ 3.3V - Power       │
│                 │    │ GND   - Ground     │
└─────────────────┘    └─────────────────────┘
```

### Breadboard Layout Example

```
Power Rails:
+5V ──────────────────────────────────────────────────────────────────────
GND ───────────────────────────────────────────────────────────────────────

ESP32 DevKit:
[ESP32] GPIO4 ──► DHT22 ──► 3.3V
[ESP32] GPIO21 ──► BMP SDA
[ESP32] GPIO22 ──► BMP SCL ──► MPU SCL
[ESP32] GPIO19 ──► PIR OUT
[ESP32] GPIO34 ──► LDR ──► 10K ──► GND
[ESP32] GPIO35 ──► Soil AO
[ESP32] GPIO32 ──► MQ135 AO
[ESP32] GPIO5  ──► Ultrasonic TRIG
[ESP32] GPIO18 ──► Ultrasonic ECHO

Actuators:
[ESP32] GPIO2  ──► LED ──► 220Ω ──► GND
[ESP32] GPIO15 ──► Buzzer ──► GND
[ESP32] GPIO27 ──► Relay1 IN
[ESP32] GPIO26 ──► Relay2 IN
[ESP32] GPIO25 ──► Relay3 IN
[ESP32] GPIO13 ──► Servo1 Signal
[ESP32] GPIO12 ──► Servo2 Signal
[ESP32] GPIO16 ──► RGB R ──► 220Ω
[ESP32] GPIO17 ──► RGB G ──► 220Ω
[ESP32] GPIO23 ──► RGB B ──► 220Ω ──► GND (Common)

Motor Driver (L298N):
[ESP32] GPIO14 ──► IN1
[ESP32] GPIO33 ──► IN2
[ESP32] GPIO32 ──► EN (PWM)
[ESP32] GPIO16 ──► IN3
[ESP32] GPIO17 ──► IN4
[ESP32] GPIO23 ──► EN2 (PWM)
```

## Assembly Instructions

### Step 1: Prepare Components

1. Gather all required components
2. Check pinouts and specifications
3. Prepare jumper wires and breadboard
4. Ensure power supply is appropriate

### Step 2: ESP32 Setup

1. Connect ESP32 to computer via USB
2. Verify board recognition in Arduino IDE/PlatformIO
3. Upload basic blink sketch to test
4. Note MAC address from serial monitor

### Step 3: Sensor Assembly

1. Start with simple sensors (DHT22, PIR)
2. Connect power and ground first
3. Connect data/signal pins
4. Test each sensor individually

### Step 4: Actuator Assembly

1. Connect actuators one by one
2. Use appropriate power supplies
3. Add protection (resistors, diodes)
4. Test each actuator separately

### Step 5: Integration Testing

1. Connect all components
2. Upload full system code
3. Test sensor readings
4. Test actuator control
5. Verify communication

### Step 6: Final Assembly

1. Secure all connections
2. Add strain relief to wires
3. Clean up breadboard layout
4. Add labeling for future reference

## Testing Hardware

### Basic Power Test

```cpp
void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);  // Built-in LED
    
    Serial.println("ESP32 Power Test");
    Serial.print("Chip ID: ");
    Serial.println(ESP.getChipId());
    Serial.print("Flash Size: ");
    Serial.println(ESP.getFlashChipSize());
}

void loop() {
    digitalWrite(2, HIGH);
    delay(1000);
    digitalWrite(2, LOW);
    delay(1000);
}
```

### Sensor Test Code

```cpp
#include "DHTSensor.h"
#include "BMPSensor.h"

DHTSensor dht;
BMPSensor bmp;

void setup() {
    Serial.begin(115200);
    dht.begin(4);
    bmp.begin(21, 22);
}

void loop() {
    float temp = dht.getTemperature();
    float hum = dht.getHumidity();
    float pressure = bmp.getPressure();
    
    Serial.print("Temp: "); Serial.print(temp);
    Serial.print("°C, Hum: "); Serial.print(hum);
    Serial.print("%, Pressure: "); Serial.print(pressure);
    Serial.println(" hPa");
    
    delay(2000);
}
```

### Actuator Test Code

```cpp
void setup() {
    pinMode(2, OUTPUT);   // LED
    pinMode(15, OUTPUT);  // Buzzer
    pinMode(27, OUTPUT);  // Relay
    
    Serial.begin(115200);
}

void loop() {
    // Test LED
    digitalWrite(2, HIGH);
    Serial.println("LED ON");
    delay(1000);
    digitalWrite(2, LOW);
    Serial.println("LED OFF");
    
    // Test Buzzer
    digitalWrite(15, HIGH);
    Serial.println("Buzzer ON");
    delay(500);
    digitalWrite(15, LOW);
    Serial.println("Buzzer OFF");
    
    // Test Relay
    digitalWrite(27, LOW);  // Active low
    Serial.println("Relay ON");
    delay(1000);
    digitalWrite(27, HIGH);
    Serial.println("Relay OFF");
    
    delay(2000);
}
```

## Troubleshooting Hardware Issues

### ESP32 Won't Boot

**Symptoms:** No response, LED not blinking
**Possible Causes:**
- Power supply insufficient (<5V/500mA)
- GPIO0 grounded (boot mode)
- Damaged USB cable
- Wrong board selection

**Solutions:**
1. Check power supply voltage/current
2. Ensure GPIO0 not connected to GND
3. Try different USB cable/port
4. Verify board selection in IDE

### Sensors Not Reading

**Symptoms:** Invalid readings, NaN values
**Possible Causes:**
- Wrong pin connections
- Insufficient power
- Missing pull-up resistors
- I2C address conflicts

**Solutions:**
1. Verify wiring against schematics
2. Check power supply to sensors
3. Add pull-up resistors (4.7KΩ for I2C)
4. Check I2C addresses with scanner

### Actuators Not Working

**Symptoms:** No response from motors/servos
**Possible Causes:**
- Insufficient power supply
- Wrong pin assignments
- Missing PWM signals
- Overloaded circuits

**Solutions:**
1. Verify power supply capacity
2. Check pin assignments in code
3. Ensure PWM-capable pins for servos
4. Add external power for motors

### WiFi Issues

**Symptoms:** Can't connect to WiFi
**Possible Causes:**
- Wrong credentials
- Out of range
- 5GHz network (ESP32 only supports 2.4GHz)
- Power supply interference

**Solutions:**
1. Verify SSID and password
2. Check signal strength
3. Ensure 2.4GHz network
4. Use clean power supply

### ESP-NOW Not Working

**Symptoms:** Can't communicate between devices
**Possible Causes:**
- Wrong MAC addresses
- Different WiFi channels
- Out of range
- Antenna issues

**Solutions:**
1. Verify MAC addresses
2. Set same WiFi channel
3. Check distance/line-of-sight
4. Use external antennas

### Camera Issues (ESP32-CAM)

**Symptoms:** Camera not initializing
**Possible Causes:**
- Insufficient power (needs 5V/2A)
- Loose ribbon cable
- Wrong board selection
- PSRAM not enabled

**Solutions:**
1. Verify 5V/2A power supply
2. Check camera ribbon cable
3. Select correct board in IDE
4. Enable PSRAM in build settings

### Memory Issues

**Symptoms:** Crashes, watchdog resets
**Possible Causes:**
- Insufficient heap memory
- Stack overflow
- Memory leaks
- Large data structures

**Solutions:**
1. Monitor free heap memory
2. Reduce array sizes
3. Free dynamic memory
4. Use static allocation

## Safety Precautions

### Electrical Safety

1. **Power Supply:** Use appropriate voltage/current ratings
2. **Short Circuits:** Double-check all connections
3. **Heat Dissipation:** Add heat sinks to high-power components
4. **Insulation:** Cover exposed conductors
5. **Grounding:** Proper grounding for AC devices

### Component Safety

1. **Polarity:** Check diode/transistor polarity
2. **Current Limits:** Don't exceed component ratings
3. **Temperature:** Monitor component temperatures
4. **Ventilation:** Ensure adequate airflow
5. **Protection:** Use fuses and protection circuits

### Best Practices

1. **Start Small:** Test individual components first
2. **Incremental Build:** Add components one at a time
3. **Documentation:** Label all connections
4. **Backup:** Keep backup of working configurations
5. **Version Control:** Track hardware changes

This comprehensive hardware setup guide should help you successfully assemble and configure your ESP32 DualComm System. Remember to proceed carefully, test thoroughly, and consult datasheets for specific component requirements.
