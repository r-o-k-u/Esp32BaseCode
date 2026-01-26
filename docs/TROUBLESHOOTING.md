# ESP32 DualComm System - Troubleshooting Guide

This comprehensive troubleshooting guide helps you diagnose and resolve common issues with the ESP32 DualComm System. Follow the step-by-step procedures to identify and fix problems efficiently.

## Table of Contents

- [Quick Start Checklist](#quick-start-checklist)
- [Power and Boot Issues](#power-and-boot-issues)
- [WiFi Connection Problems](#wifi-connection-problems)
- [ESP-NOW Communication Issues](#esp-now-communication-issues)
- [Sensor Problems](#sensor-problems)
- [Actuator Issues](#actuator-issues)
- [Camera Problems (ESP32-CAM)](#camera-problems-esp32-cam)
- [Web Interface Issues](#web-interface-issues)
- [OTA Update Problems](#ota-update-problems)
- [Memory and Performance Issues](#memory-and-performance-issues)
- [Data Logging Problems](#data-logging-problems)
- [Serial Communication Issues](#serial-communication-issues)
- [Advanced Diagnostics](#advanced-diagnostics)

## Quick Start Checklist

Before diving into specific issues, run through this checklist:

### Hardware Verification
- [ ] ESP32 board properly powered (5V/1A minimum)
- [ ] USB cable working (try different cable/port)
- [ ] Correct board selected in PlatformIO/Arduino IDE
- [ ] All sensors/actuators connected to correct pins
- [ ] Power supply stable (no voltage drops)

### Software Verification
- [ ] Latest ESP32 board package installed
- [ ] All required libraries installed
- [ ] Code compiles without errors
- [ ] Correct partition scheme selected
- [ ] SPIFFS partition size adequate

### Configuration Verification
- [ ] WiFi credentials correct in `credentials.h`
- [ ] Device MAC addresses properly configured
- [ ] ESP-NOW peer addresses correct
- [ ] Sensor/actuator pins match hardware connections

## Power and Boot Issues

### ESP32 Won't Boot or Respond

**Symptoms:**
- No LED blinking
- No serial output
- Board appears dead

**Diagnostic Steps:**

1. **Check Power Supply**
   ```bash
   # Use multimeter to verify:
   # - Voltage: 5.0V ± 0.1V
   # - Current: >500mA when active
   ```

2. **Verify USB Connection**
   - Try different USB ports
   - Use different USB cable
   - Check device manager (Windows) or `lsusb` (Linux)

3. **Check Boot Mode**
   ```cpp
   // Add to setup() for debugging
   Serial.begin(115200);
   Serial.print("Boot reason: ");
   Serial.println(esp_reset_reason());
   Serial.print("Wakeup cause: ");
   Serial.println(esp_sleep_get_wakeup_cause());
   ```

4. **GPIO0 Check**
   - Ensure GPIO0 not connected to GND
   - Remove any wires from GPIO0 during boot

**Common Solutions:**
- Replace power supply with known good 5V/2A unit
- Use USB hub with external power
- Check for short circuits on power rails

### Random Resets/Crashes

**Symptoms:**
- Device reboots unexpectedly
- Watchdog timer resets
- Brownout detector triggers

**Diagnostic Steps:**

1. **Monitor Reset Reason**
   ```cpp
   void setup() {
       Serial.begin(115200);
       esp_reset_reason_t reason = esp_reset_reason();
       Serial.print("Reset reason: ");
       switch(reason) {
           case ESP_RST_POWERON: Serial.println("Power on"); break;
           case ESP_RST_EXT: Serial.println("External reset"); break;
           case ESP_RST_SW: Serial.println("Software reset"); break;
           case ESP_RST_PANIC: Serial.println("Panic"); break;
           case ESP_RST_INT_WDT: Serial.println("Interrupt WDT"); break;
           case ESP_RST_TASK_WDT: Serial.println("Task WDT"); break;
           case ESP_RST_WDT: Serial.println("Other WDT"); break;
           case ESP_RST_DEEPSLEEP: Serial.println("Deep sleep"); break;
           case ESP_RST_BROWNOUT: Serial.println("Brownout"); break;
           case ESP_RST_SDIO: Serial.println("SDIO"); break;
           default: Serial.println("Unknown"); break;
       }
   }
   ```

2. **Check Power Stability**
   - Monitor voltage during operation
   - Add decoupling capacitors (10µF, 100nF)
   - Use external power supply instead of USB

3. **Memory Issues**
   ```cpp
   // Monitor heap usage
   Serial.print("Free heap: ");
   Serial.println(ESP.getFreeHeap());
   Serial.print("Min free heap: ");
   Serial.println(ESP.getMinFreeHeap());
   Serial.print("Heap fragmentation: ");
   Serial.println(ESP.getHeapFragmentation());
   ```

**Common Solutions:**
- Increase power supply capacity to 2A
- Add large electrolytic capacitor (470µF) across power rails
- Reduce code complexity and memory usage
- Use `delay()` instead of busy-waiting loops

## WiFi Connection Problems

### Cannot Connect to WiFi

**Symptoms:**
- "WiFi connection failed" in serial monitor
- Device creates AP instead of connecting to router

**Diagnostic Steps:**

1. **Verify Credentials**
   ```cpp
   // Test connection manually
   WiFi.begin("Your_SSID", "Your_Password");
   int attempts = 0;
   while (WiFi.status() != WL_CONNECTED && attempts < 20) {
       delay(500);
       Serial.print(".");
       attempts++;
   }
   if (WiFi.status() == WL_CONNECTED) {
       Serial.println("Connected!");
       Serial.print("IP: ");
       Serial.println(WiFi.localIP());
   } else {
       Serial.println("Failed to connect");
       Serial.print("Status: ");
       Serial.println(WiFi.status());
   }
   ```

2. **Check Network Compatibility**
   - Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
   - Verify WPA2 security (not WPA3 or WEP)
   - Check for hidden SSID requirements

3. **Signal Strength Test**
   ```cpp
   // Scan and display networks
   int n = WiFi.scanNetworks();
   for (int i = 0; i < n; i++) {
       Serial.print(WiFi.SSID(i));
       Serial.print(" (");
       Serial.print(WiFi.RSSI(i));
       Serial.print("dBm) ");
       Serial.println(WiFi.encryptionType(i));
   }
   ```

4. **DHCP Issues**
   - Router may not assign IP
   - Try static IP configuration
   ```cpp
   IPAddress staticIP(192, 168, 1, 100);
   IPAddress gateway(192, 168, 1, 1);
   IPAddress subnet(255, 255, 255, 0);
   WiFi.config(staticIP, gateway, subnet);
   ```

**Common Solutions:**
- Move closer to router
- Change WiFi channel (1, 6, or 11 preferred)
- Disable WiFi power saving: `WiFi.setSleep(false)`
- Reset router if DHCP issues

### WiFi Disconnects Frequently

**Symptoms:**
- Connection drops randomly
- Poor stability

**Diagnostic Steps:**

1. **Monitor Connection**
   ```cpp
   void loop() {
       if (WiFi.status() != WL_CONNECTED) {
           Serial.println("WiFi disconnected!");
           Serial.print("Reason: ");
           Serial.println(WiFi.getLastDisconnectReason());
           // Reconnect logic
       }
   }
   ```

2. **Adjust WiFi Settings**
   ```cpp
   // Optimize for stability
   WiFi.setSleep(false);  // Disable power saving
   WiFi.setAutoReconnect(true);
   WiFi.persistent(true);
   ```

3. **Check for Interference**
   - Microwave ovens, cordless phones
   - Bluetooth devices
   - Other 2.4GHz networks

**Common Solutions:**
- Change WiFi channel to less crowded one
- Use external antenna if available
- Add WiFi reconnection logic with exponential backoff

### Slow WiFi Performance

**Symptoms:**
- Web interface slow to load
- OTA updates fail due to timeout

**Diagnostic Steps:**

1. **Check Signal Quality**
   ```cpp
   Serial.print("RSSI: ");
   Serial.print(WiFi.RSSI());
   Serial.println(" dBm");
   // Good: > -50dBm, Poor: < -70dBm
   ```

2. **Monitor Transfer Speeds**
   ```cpp
   // Test upload speed
   unsigned long start = millis();
   // Perform data transfer
   unsigned long duration = millis() - start;
   float speed = (data_size * 8.0) / (duration / 1000.0); // bps
   ```

**Common Solutions:**
- Use WiFi channel with less interference
- Reduce distance to router
- Switch to 5GHz router with ESP32-C3/S3 (if compatible)

## ESP-NOW Communication Issues

### Cannot Send/Receive Messages

**Symptoms:**
- ESP-NOW messages not received
- Peer devices not found

**Diagnostic Steps:**

1. **Verify ESP-NOW Initialization**
   ```cpp
   if (esp_now_init() != ESP_OK) {
       Serial.println("ESP-NOW init failed");
       return;
   }
   Serial.println("ESP-NOW init success");
   ```

2. **Check Peer Addition**
   ```cpp
   esp_now_peer_info_t peerInfo;
   memcpy(peerInfo.peer_addr, peerMac, 6);
   peerInfo.channel = 0;
   peerInfo.encrypt = false;

   if (esp_now_add_peer(&peerInfo) != ESP_OK) {
       Serial.println("Failed to add peer");
   }
   ```

3. **MAC Address Verification**
   ```cpp
   // Print local MAC
   Serial.print("Local MAC: ");
   Serial.println(WiFi.macAddress());

   // Print peer MAC from serial monitor
   uint8_t peerMac[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
   ```

4. **Channel Verification**
   ```cpp
   // Both devices must be on same channel
   int channel = WiFi.channel();
   Serial.print("WiFi Channel: ");
   Serial.println(channel);
   ```

**Common Solutions:**
- Ensure both devices on same WiFi channel
- Verify MAC addresses (case-sensitive)
- Check distance (max ~100m outdoors)
- Disable WiFi power saving

### ESP-NOW Messages Lost

**Symptoms:**
- Intermittent message loss
- Unreliable communication

**Diagnostic Steps:**

1. **Monitor Send Status**
   ```cpp
   esp_now_send_status_t sendStatus = ESP_NOW_SEND_SUCCESS;
   // In send callback
   void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
       if (status != ESP_NOW_SEND_SUCCESS) {
           Serial.println("Send failed");
       }
   }
   ```

2. **Check Message Size**
   ```cpp
   // ESP-NOW max payload: 250 bytes
   if (dataSize > 250) {
       Serial.println("Message too large");
   }
   ```

3. **RSSI Monitoring**
   ```cpp
   // Add RSSI to messages for debugging
   int rssi = WiFi.RSSI();
   Serial.print("Signal strength: ");
   Serial.print(rssi);
   Serial.println(" dBm");
   ```

**Common Solutions:**
- Reduce message frequency
- Increase retransmission attempts
- Use acknowledgments for critical messages
- Implement message queuing

## Sensor Problems

### DHT Sensor Not Reading

**Symptoms:**
- NaN values or no readings
- "DHT read failed" errors

**Diagnostic Steps:**

1. **Check Wiring**
   - Data pin: GPIO4 or GPIO32 (ADC1 preferred)
   - Power: 3.3V (not 5V)
   - Pull-up: 10KΩ resistor on data line

2. **Test Basic Functionality**
   ```cpp
   #include <DHT.h>
   DHT dht(4, DHT22);  // Pin 4, DHT22

   void setup() {
       Serial.begin(115200);
       dht.begin();
   }

   void loop() {
       float h = dht.readHumidity();
       float t = dht.readTemperature();

       if (isnan(h) || isnan(t)) {
           Serial.println("Failed to read from DHT sensor!");
       } else {
           Serial.print("Humidity: ");
           Serial.print(h);
           Serial.print(" %\t");
           Serial.print("Temperature: ");
           Serial.print(t);
           Serial.println(" *C");
       }
       delay(2000);
   }
   ```

3. **Timing Issues**
   - Minimum 2-second delay between readings
   - Check for blocking code in loop()

**Common Solutions:**
- Add 10KΩ pull-up resistor
- Use ADC1 pins (GPIO32-39) instead of ADC2
- Increase delay between readings
- Check sensor power supply stability

### BMP280/I2C Sensor Issues

**Symptoms:**
- Sensor not detected
- Wrong readings or 0 values

**Diagnostic Steps:**

1. **I2C Scanner**
   ```cpp
   #include <Wire.h>

   void setup() {
       Serial.begin(115200);
       Wire.begin(21, 22);  // SDA, SCL
   }

   void loop() {
       Serial.println("Scanning I2C...");
       for (byte address = 1; address < 127; address++) {
           Wire.beginTransmission(address);
           if (Wire.endTransmission() == 0) {
               Serial.print("Found device at 0x");
               Serial.println(address, HEX);
           }
       }
       delay(5000);
   }
   ```

2. **Check Pull-up Resistors**
   - 4.7KΩ resistors on SDA/SCL lines
   - Connected to 3.3V

3. **Address Conflicts**
   - BMP280: 0x76 or 0x77
   - MPU6050: 0x68
   - Change address if conflict

**Common Solutions:**
- Add pull-up resistors (4.7KΩ)
- Check I2C pin assignments (GPIO21 SDA, GPIO22 SCL)
- Verify sensor address
- Test with different I2C speed

### Ultrasonic Sensor Problems

**Symptoms:**
- No distance readings
- Inconsistent measurements

**Diagnostic Steps:**

1. **Basic Test**
   ```cpp
   #define TRIG_PIN 5
   #define ECHO_PIN 18

   void setup() {
       Serial.begin(115200);
       pinMode(TRIG_PIN, OUTPUT);
       pinMode(ECHO_PIN, INPUT);
   }

   void loop() {
       digitalWrite(TRIG_PIN, LOW);
       delayMicroseconds(2);
       digitalWrite(TRIG_PIN, HIGH);
       delayMicroseconds(10);
       digitalWrite(TRIG_PIN, LOW);

       long duration = pulseIn(ECHO_PIN, HIGH);
       float distance = duration * 0.034 / 2;

       Serial.print("Distance: ");
       Serial.print(distance);
       Serial.println(" cm");
       delay(1000);
   }
   ```

2. **Power Supply Check**
   - Must use 5V for reliable operation
   - Check voltage during operation

**Common Solutions:**
- Use 5V power supply
- Add delay between measurements
- Check for acoustic interference
- Clean sensor faces

### PIR Sensor Issues

**Symptoms:**
- False triggers or no detection
- Inconsistent behavior

**Diagnostic Steps:**

1. **Basic Test**
   ```cpp
   void setup() {
       Serial.begin(115200);
       pinMode(19, INPUT);  // PIR pin
   }

   void loop() {
       int pirState = digitalRead(19);
       Serial.println(pirState ? "Motion detected" : "No motion");
       delay(1000);
   }
   ```

2. **Calibration**
   - Adjust sensitivity potentiometer
   - Adjust delay potentiometer
   - Allow 1-minute warm-up time

**Common Solutions:**
- Adjust sensitivity (clockwise = more sensitive)
- Set appropriate delay time
- Avoid thermal sources near sensor
- Use interrupt-capable pins

## Actuator Issues

### Servo Motor Not Moving

**Symptoms:**
- Servo doesn't respond to commands
- Jerky or erratic movement

**Diagnostic Steps:**

1. **Power Supply Check**
   - Servos need 5V/1A per servo
   - Use separate power supply for multiple servos

2. **Basic Test**
   ```cpp
   #include <ESP32Servo.h>
   Servo servo;

   void setup() {
       servo.attach(13);  // GPIO13
   }

   void loop() {
       servo.write(0);
       delay(1000);
       servo.write(90);
       delay(1000);
       servo.write(180);
       delay(1000);
   }
   ```

3. **PWM Pin Verification**
   - Use PWM-capable pins
   - Check for pin conflicts

**Common Solutions:**
- Use external 5V/2A power supply
- Connect ground properly
- Use PWM-capable GPIO pins
- Check servo specifications

### Relay Not Switching

**Symptoms:**
- Relay doesn't click or switch
- Load doesn't turn on/off

**Diagnostic Steps:**

1. **Logic Level Check**
   ```cpp
   // Active LOW relays
   digitalWrite(RELAY_PIN, LOW);  // ON
   delay(1000);
   digitalWrite(RELAY_PIN, HIGH); // OFF
   delay(1000);
   ```

2. **Power Supply**
   - Relay module needs 5V
   - External power for high-current loads

**Common Solutions:**
- Verify active LOW logic
- Check relay module power
- Add flyback diode for inductive loads
- Test with LED first

### Motor Driver Issues

**Symptoms:**
- Motors not spinning
- Only one direction works
- Speed control not working

**Diagnostic Steps:**

1. **L298N Test**
   ```cpp
   // Motor 1 test
   digitalWrite(IN1, HIGH);
   digitalWrite(IN2, LOW);
   analogWrite(EN, 255);  // Full speed

   // Motor 2 test
   digitalWrite(IN3, HIGH);
   digitalWrite(IN4, LOW);
   analogWrite(EN2, 255);
   ```

2. **Power Supply**
   - 12V for motors
   - 5V from L298N for logic
   - Adequate current capacity

**Common Solutions:**
- Check motor power connections
- Verify IN pin logic
- Ensure EN pins have PWM signal
- Add heat sink for high current

### LED/Buzzer Not Working

**Symptoms:**
- No light/sound from components

**Diagnostic Steps:**

1. **Basic Test**
   ```cpp
   pinMode(LED_PIN, OUTPUT);
   pinMode(BUZZER_PIN, OUTPUT);

   digitalWrite(LED_PIN, HIGH);
   digitalWrite(BUZZER_PIN, HIGH);
   delay(1000);
   digitalWrite(LED_PIN, LOW);
   digitalWrite(BUZZER_PIN, LOW);
   ```

2. **Current Limiting**
   - LEDs need resistors (220Ω)
   - Check polarity

**Common Solutions:**
- Add current-limiting resistors
- Check component polarity
- Verify pin assignments
- Test with multimeter

## Camera Problems (ESP32-CAM)

### Camera Initialization Failed

**Symptoms:**
- "Camera init failed" error
- No camera stream

**Diagnostic Steps:**

1. **Power Supply Check**
   - Must be exactly 5V/2A
   - Stable power required
   - Use quality power supply

2. **PSRAM Verification**
   ```cpp
   if (!psramInit()) {
       Serial.println("PSRAM init failed");
   }
   Serial.print("PSRAM size: ");
   Serial.println(ESP.getPsramSize());
   ```

3. **Camera Configuration**
   ```cpp
   camera_config_t config;
   config.pin_pwdn = PWDN_GPIO_NUM;
   config.pin_reset = RESET_GPIO_NUM;
   config.pin_xclk = XCLK_GPIO_NUM;
   // ... complete config
   ```

**Common Solutions:**
- Use 5V/2A power supply
- Enable PSRAM in build settings
- Check camera ribbon cable
- Select correct board type

### Poor Image Quality

**Symptoms:**
- Blurry or distorted images
- Low light performance issues

**Diagnostic Steps:**

1. **Camera Settings Adjustment**
   ```cpp
   sensor_t * s = esp_camera_sensor_get();
   s->set_brightness(s, 1);     // -2 to 2
   s->set_contrast(s, 1);       // -2 to 2
   s->set_saturation(s, 1);     // -2 to 2
   s->set_whitebal(s, 1);       // 0 = disable, 1 = enable
   s->set_awb_gain(s, 1);       // 0 = disable, 1 = enable
   ```

2. **Resolution Check**
   - Lower resolution for better performance
   - SVGA (800x600) often best balance

**Common Solutions:**
- Adjust camera settings
- Improve lighting
- Clean lens
- Reduce frame rate

## Web Interface Issues

### Cannot Access Web Interface

**Symptoms:**
- 192.168.4.1 not accessible
- Page doesn't load

**Diagnostic Steps:**

1. **IP Address Check**
   ```cpp
   Serial.print("IP Address: ");
   Serial.println(WiFi.localIP());
   Serial.print("AP IP: ");
   Serial.println(WiFi.softAPIP());
   ```

2. **Server Status**
   ```cpp
   if (!webServer.begin()) {
       Serial.println("Web server failed to start");
   }
   ```

3. **SPIFFS Check**
   ```cpp
   if (!SPIFFS.begin(true)) {
       Serial.println("SPIFFS mount failed");
   }
   Serial.print("SPIFFS total: ");
   Serial.println(SPIFFS.totalBytes());
   Serial.print("SPIFFS used: ");
   Serial.println(SPIFFS.usedBytes());
   ```

**Common Solutions:**
- Upload SPIFFS data
- Check firewall settings
- Try different browser
- Clear browser cache

### WebSocket Connection Failed

**Symptoms:**
- Real-time updates not working
- No live sensor data

**Diagnostic Steps:**

1. **WebSocket Test**
   ```javascript
   // Browser console
   const ws = new WebSocket('ws://192.168.4.1/ws');
   ws.onopen = () => console.log('Connected');
   ws.onerror = (error) => console.log('Error:', error);
   ```

2. **Server-Side Check**
   ```cpp
   webSocket.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
       Serial.printf("WebSocket event: %d\n", type);
   });
   ```

**Common Solutions:**
- Check WebSocket URL
- Verify firewall allows WebSocket
- Update browser
- Check network connectivity

## OTA Update Problems

### OTA Upload Fails

**Symptoms:**
- "Update failed" error
- Timeout during upload

**Diagnostic Steps:**

1. **OTA Setup Check**
   ```cpp
   ArduinoOTA.setPassword("your_password");
   ArduinoOTA.begin();
   Serial.println("OTA ready");
   ```

2. **Network Connectivity**
   - Device must be on same network
   - Check firewall settings

3. **Memory Check**
   ```cpp
   Serial.print("Free space: ");
   Serial.println(ESP.getFreeSketchSpace());
   ```

**Common Solutions:**
- Ensure same network segment
- Disable firewall temporarily
- Check OTA password
- Free up flash space

### OTA Not Starting

**Symptoms:**
- No OTA server found in IDE

**Diagnostic Steps:**

1. **Port Check**
   ```cpp
   Serial.print("OTA port: ");
   Serial.println(ArduinoOTA.getPort());
   ```

2. **MDNS Check**
   ```cpp
   if (!MDNS.begin("esp32-device")) {
       Serial.println("MDNS failed");
   }
   ```

**Common Solutions:**
- Enable MDNS in code
- Check device hostname
- Restart IDE
- Try IP address instead of hostname

## Memory and Performance Issues

### Out of Memory Errors

**Symptoms:**
- "Out of memory" errors
- Device crashes

**Diagnostic Steps:**

1. **Memory Monitoring**
   ```cpp
   void printMemory() {
       Serial.print("Heap: ");
       Serial.print(ESP.getFreeHeap());
       Serial.print("/");
       Serial.println(ESP.getHeapSize());
       Serial.print("PSRAM: ");
       Serial.print(ESP.getFreePsram());
       Serial.print("/");
       Serial.println(ESP.getPsramSize());
   }
   ```

2. **Memory Leak Detection**
   ```cpp
   // Track allocations
   void* myPtr = malloc(100);
   Serial.printf("Allocated: %p\n", myPtr);
   // Later...
   free(myPtr);
   ```

**Common Solutions:**
- Use static allocation
- Free dynamic memory
- Reduce string usage
- Optimize data structures

### Slow Performance

**Symptoms:**
- Delays in response
- Lag in web interface

**Diagnostic Steps:**

1. **CPU Usage Check**
   ```cpp
   // Monitor loop timing
   unsigned long start = micros();
   // Your code here
   unsigned long duration = micros() - start;
   Serial.printf("Loop time: %lu µs\n", duration);
   ```

2. **Task Monitoring**
   ```cpp
   // FreeRTOS task status
   char *pcTaskList = (char*)malloc(1024);
   vTaskList(pcTaskList);
   Serial.println(pcTaskList);
   free(pcTaskList);
   ```

**Common Solutions:**
- Reduce sensor reading frequency
- Optimize algorithms
- Use interrupts instead of polling
- Increase task priorities

## Data Logging Problems

### SPIFFS Issues

**Symptoms:**
- Cannot write to SPIFFS
- Files not saving

**Diagnostic Steps:**

1. **SPIFFS Status**
   ```cpp
   if (!SPIFFS.begin(true)) {
       Serial.println("SPIFFS mount failed");
       return;
   }
   Serial.printf("Total: %u, Used: %u\n",
                 SPIFFS.totalBytes(), SPIFFS.usedBytes());
   ```

2. **File Operations**
   ```cpp
   File file = SPIFFS.open("/test.txt", "w");
   if (!file) {
       Serial.println("File open failed");
   }
   file.println("Test data");
   file.close();
   ```

**Common Solutions:**
- Format SPIFFS: `SPIFFS.format()`
- Check partition size
- Close files properly
- Avoid concurrent access

### Log Rotation Issues

**Symptoms:**
- Logs not rotating
- File size growing indefinitely

**Diagnostic Steps:**

1. **Size Check**
   ```cpp
   File file = SPIFFS.open("/logs.txt", "r");
   Serial.printf("File size: %u bytes\n", file.size());
   file.close();
   ```

2. **Rotation Logic**
   ```cpp
   if (file.size() > MAX_LOG_SIZE) {
       // Rename and create new file
       SPIFFS.rename("/logs.txt", "/logs_old.txt");
   }
   ```

**Common Solutions:**
- Implement proper rotation
- Compress old logs
- Limit retention period
- Use external storage for large logs

## Serial Communication Issues

### No Serial Output

**Symptoms:**
- Serial monitor empty
- No debug information

**Diagnostic Steps:**

1. **Serial Setup**
   ```cpp
   Serial.begin(115200);
   while (!Serial) {
       ; // Wait for serial port
   }
   Serial.println("Serial ready");
   ```

2. **Baud Rate Check**
   - Ensure IDE matches code (115200)
   - Try different baud rates

**Common Solutions:**
- Check USB cable
- Verify COM port selection
- Restart IDE
- Add delay after Serial.begin()

### Serial Communication Errors

**Symptoms:**
- Garbled output
- Missing characters

**Diagnostic Steps:**

1. **Hardware Serial Test**
   ```cpp
   HardwareSerial mySerial(2);  // UART2
   mySerial.begin(9600, SERIAL_8N1, 16, 17);  // RX, TX
   mySerial.println("Test message");
   ```

2. **Pin Conflicts**
   - GPIO1/TX and GPIO3/RX used by USB
   - Use GPIO16/RX2 and GPIO17/TX2

**Common Solutions:**
- Use UART2 pins (16, 17)
- Check baud rate matching
- Add termination resistors
- Check cable length

## Advanced Diagnostics

### Using ESP Exception Decoder

1. **Install Decoder**
   - Arduino IDE: Tools → ESP Exception Decoder
   - PlatformIO: Built-in

2. **Decode Stack Trace**
   - Copy exception output
   - Paste into decoder
   - Get function names and line numbers

### Core Dump Analysis

1. **Enable Core Dumps**
   ```cpp
   // In code
   #include <esp_system.h>
   // Set core dump to UART
   esp_core_dump_init();
   ```

2. **Analyze Dump**
   - Use esp32-core-dump tool
   - Get crash location and cause

### Network Packet Analysis

1. **Wireshark Setup**
   - Capture WiFi packets
   - Filter for ESP32 MAC address

2. **ESP-NOW Packet Inspection**
   - Monitor packet loss
   - Check timing issues

### Performance Profiling

1. **Function Timing**
   ```cpp
   #define PROFILE_FUNCTION(func) \
       unsigned long start = micros(); \
       func; \
       Serial.printf(#func " took %lu µs\n", micros() - start);
   ```

2. **Memory Profiling**
   ```cpp
   // Track peak usage
   static size_t peakHeap = 0;
   size_t currentHeap = ESP.getFreeHeap();
   if (currentHeap < peakHeap || peakHeap == 0) {
       peakHeap = currentHeap;
       Serial.printf("New peak heap usage: %u\n", peakHeap);
   }
   ```

### System Recovery

#### Factory Reset
```cpp
// Erase all settings
void factoryReset() {
    SPIFFS.format();
    WiFi.disconnect(true);
    ESP.eraseConfig();
    ESP.restart();
}
```

#### Emergency Firmware Upload
1. Hold GPIO0 to GND during boot
2. Upload via USB (not OTA)
3. Release GPIO0 and reset

#### Safe Mode Boot
- Minimal code for testing
- Disable problematic features
- Gradually enable components

### Contact and Support

If you cannot resolve an issue:

1. **Gather Information**
   - ESP32 board type and version
   - Arduino/PlatformIO version
   - Complete error messages
   - Wiring diagram/photos

2. **Check Resources**
   - ESP32 documentation
   - Arduino forums
   - GitHub issues

3. **Community Support**
   - Post detailed problem description
   - Include code snippets
   - Share serial output

Remember: Most issues have simple solutions once properly diagnosed. Work methodically through the troubleshooting steps, and don't hesitate to start over with a minimal test case.
