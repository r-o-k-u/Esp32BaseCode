/**
 * ═══════════════════════════════════════════════════════════════════════════
 * ESP32 DUAL COMMUNICATION SYSTEM - CONFIGURATION FILE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * @file config.h
 * @brief Global configuration and pin definitions for ESP32/ESP32-CAM
 * @version 2.0.0
 * @date 2024
 * @author Your Name
 * @license MIT
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * DESCRIPTION:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * This file contains ALL system configuration parameters including:
 * - Device identification
 * - Pin assignments (auto-detected for ESP32 vs ESP32-CAM)
 * - WiFi settings
 * - ESP-NOW configuration
 * - Sensor/actuator parameters
 * - Feature enable/disable flags
 * - Performance tuning parameters
 * - Debug settings
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * IMPORTANT: HOW TO USE THIS FILE
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * 1. Set DEVICE_TYPE in platformio.ini:
 *    -D DEVICE_TYPE=0  for ESP32 DevKit
 *    -D DEVICE_TYPE=1  for ESP32-CAM
 *
 * 2. Adjust pin assignments if using different GPIO pins
 *
 * 3. Enable/disable features as needed
 *
 * 4. Set appropriate intervals for your application
 *
 * 5. Calibrate sensor offsets after testing
 *
 * ═══════════════════════════════════════════════════════════════════════════
 * AUTOMATIC DEVICE DETECTION:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * This file automatically loads the correct pin configuration based on
 * DEVICE_TYPE. You don't need to manually comment/uncomment sections.
 *
 * The preprocessor will include only the relevant configuration for your
 * hardware type.
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ═══════════════════════════════════════════════════════════════════════════
// DEVICE IDENTIFICATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * DEVICE_TYPE is set in platformio.ini build flags:
 * - 0 = ESP32 DevKit (30-pin or 38-pin)
 * - 1 = ESP32-CAM (AI-Thinker)
 *
 * If not defined, defaults to ESP32 DevKit
 */
#ifndef DEVICE_TYPE
#define DEVICE_TYPE 0
#endif

/**
 * Device name - appears in:
 * - WiFi hostname
 * - Web interface title
 * - ESP-NOW peer list
 * - OTA hostname
 *
 * IMPORTANT: Make this unique for each device!
 * Examples: "ESP32_Kitchen", "ESP32_CAM_Garage", "ESP32_Sensor_Node_1"
 */
#define DEVICE_NAME "ESP32_Device_1"

/**
 * Firmware version
 * Update this when you make changes
 * Format: "MAJOR.MINOR.PATCH"
 */
#define FIRMWARE_VERSION "2.0.0"

// ═══════════════════════════════════════════════════════════════════════════
// WIFI CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Access Point (AP) settings for configuration mode
 *
 * When device can't connect to WiFi, it creates its own network
 * with these credentials. Users can connect and configure WiFi.
 *
 * SECURITY WARNING:
 * Change AP_PASSWORD to something more secure!
 * Minimum 8 characters required by WPA2
 */
#define AP_SSID "ESP32_Setup"
#define AP_PASSWORD "12345678"

/**
 * WiFi connection parameters
 *
 * WIFI_TIMEOUT: Maximum time to wait for connection (milliseconds)
 * WIFI_RETRY_DELAY: Delay between connection attempts (milliseconds)
 *
 * Adjust these based on your network:
 * - Fast/reliable network: Lower timeout (10000ms)
 * - Slow/unreliable network: Higher timeout (30000ms)
 */
#define WIFI_TIMEOUT 20000
#define WIFI_RETRY_DELAY 500

// ═══════════════════════════════════════════════════════════════════════════
// WEB SERVER CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * HTTP and WebSocket port configuration
 *
 * WEB_SERVER_PORT: Standard HTTP port (80 = no port in URL)
 * WEBSOCKET_PORT: WebSocket for real-time updates (81 by default)
 * MAX_CLIENTS: Maximum simultaneous WebSocket connections
 *
 * NOTE: Port 80 requires no ":80" in URL (http://ip/)
 *       Other ports require explicit specification (http://ip:8080/)
 */
#define WEB_SERVER_PORT 80
#define WEBSOCKET_PORT 81
#define MAX_CLIENTS 4

// ═══════════════════════════════════════════════════════════════════════════
// OTA (OVER-THE-AIR UPDATE) CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * OTA update settings
 *
 * OTA_HOSTNAME: Network name for finding device (.local domain)
 * OTA_PASSWORD: Password required for OTA updates (CHANGE THIS!)
 * OTA_PORT: Network port for OTA (default 3232)
 *
 * USAGE:
 * Arduino IDE: Tools -> Port -> <OTA_HOSTNAME>
 * PlatformIO: pio run -t upload --upload-port <OTA_HOSTNAME>.local
 *
 * SECURITY:
 * Always set a strong OTA password to prevent unauthorized updates!
 */
#define OTA_HOSTNAME "ESP32-OTA"
#define OTA_PASSWORD "admin123" // ⚠️ CHANGE THIS!
#define OTA_PORT 3232

// ═══════════════════════════════════════════════════════════════════════════
// ESP-NOW CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * ESP-NOW communication parameters
 *
 * ESPNOW_CHANNEL: WiFi channel (1-13, must match all peers)
 * MAX_ESPNOW_PEERS: Maximum number of peer devices (ESP32 limit: 20)
 * ESPNOW_RETRY_COUNT: Number of retries for failed transmissions
 * ESPNOW_ACK_TIMEOUT: Timeout for acknowledgment (milliseconds)
 *
 * CHANNEL SELECTION:
 * - Must be same for all communicating devices
 * - Should match your WiFi router channel for best performance
 * - Channels 1, 6, 11 have least overlap in 2.4GHz band
 *
 * PEER LIMIT:
 * - ESP32 supports up to 20 encrypted or 6 unencrypted peers
 * - We use unencrypted for simplicity (6 peer limit)
 * - Use encryption in production for security
 */
#define ESPNOW_CHANNEL 1
#define MAX_ESPNOW_PEERS 5
#define ESPNOW_RETRY_COUNT 3
#define ESPNOW_ACK_TIMEOUT 200

/**
 * Default peer device MAC address
 *
 * This is a placeholder. Get actual MAC addresses by:
 * 1. Upload code to each ESP32
 * 2. Check Serial Monitor on boot
 * 3. Copy the MAC address shown
 * 4. Set in credentials.h as PEER_MAC_1, PEER_MAC_2, etc.
 *
 * Format: {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
 *
 * Broadcast address (FF:FF:FF:FF:FF:FF) sends to all nearby devices
 */
extern uint8_t peerMAC[]; // Broadcast

// ═══════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONS - ESP32 DEVKIT (DEVICE_TYPE == 0)
// ═══════════════════════════════════════════════════════════════════════════

#if DEVICE_TYPE == 0

// ───────────────────────────────────────────────────────────────────────
// SENSOR PINS - ESP32 DEVKIT
// ───────────────────────────────────────────────────────────────────────

/**
 * DHT22 Temperature & Humidity Sensor
 * - VCC: 3.3V or 5V
 * - GND: Ground
 * - DATA: GPIO 4 (with 10K pullup resistor to VCC)
 *
 * DHT_TYPE options: DHT11, DHT21, DHT22 (DHT22 recommended)
 */
#define DHT_PIN 4
#define DHT_TYPE DHT22

/**
 * HC-SR04 Ultrasonic Distance Sensor
 * - VCC: 5V
 * - GND: Ground
 * - TRIG: GPIO 5 (trigger pulse output)
 * - ECHO: GPIO 18 (echo pulse input)
 *
 * Range: 2cm to 400cm
 * Accuracy: ±3mm
 */
#define ULTRASONIC_TRIG 5
#define ULTRASONIC_ECHO 18

/**
 * PIR Motion Sensor (HC-SR501)
 * - VCC: 5V
 * - GND: Ground
 * - OUT: GPIO 19 (HIGH when motion detected)
 *
 * Detection range: 3-7 meters
 * Detection angle: 120 degrees
 */
#define PIR_PIN 19

/**
 * LDR Light Sensor (Photoresistor)
 * - One leg: 3.3V
 * - Other leg: GPIO 34 AND 10K resistor to GND
 *
 * Forms voltage divider circuit
 * Reading: 0 (dark) to 4095 (bright)
 *
 * NOTE: Use ADC1 pins (32-39) only!
 * ADC2 pins conflict with WiFi
 */
#define LDR_PIN 34 // ADC1_CH6

/**
 * Soil Moisture Sensor
 * - VCC: 3.3V (or 5V with voltage divider)
 * - GND: Ground
 * - AO: GPIO 35 (analog output)
 *
 * Reading: Lower = wetter, Higher = drier
 * Calibration needed for accurate readings
 */
#define SOIL_MOISTURE_PIN 35 // ADC1_CH7

/**
 * MQ135 Air Quality Sensor
 * - VCC: 5V
 * - GND: Ground
 * - AO: GPIO 32 (analog output)
 *
 * Detects: CO2, NH3, NOx, Alcohol, Benzene, Smoke
 * Requires 24-48h preheat for accurate readings
 */
#define MQ135_PIN 32 // ADC1_CH4

/**
 * I2C Bus for BMP280, MPU6050, and other I2C devices
 * - SDA: GPIO 21 (data line)
 * - SCL: GPIO 22 (clock line)
 *
 * Multiple devices can share same I2C bus
 * Each device has unique address (e.g., 0x76, 0x68)
 *
 * Pull-up resistors (4.7K) to 3.3V recommended
 */
#define I2C_SDA 21
#define I2C_SCL 22

// ───────────────────────────────────────────────────────────────────────
// ACTUATOR PINS - ESP32 DEVKIT
// ───────────────────────────────────────────────────────────────────────

/**
 * Servo Motors (SG90 or similar)
 * - VCC: 5V (separate power recommended for multiple servos)
 * - GND: Common ground
 * - Signal: PWM pin
 *
 * IMPORTANT: Servos can draw high current!
 * Use external 5V power supply for more than 2 servos
 * Common ground between ESP32 and servo power supply
 */
#define SERVO1_PIN 13
#define SERVO2_PIN 12

/**
 * Relay Modules (5V relay boards)
 * - VCC: 5V
 * - GND: Ground
 * - IN: Control pin (active LOW typically)
 *
 * Most relay modules are active LOW:
 * - LOW (0V) = Relay ON
 * - HIGH (3.3V) = Relay OFF
 *
 * Use for controlling:
 * - Lights
 * - Fans
 * - Pumps
 * - Heaters
 * - AC appliances (BE CAREFUL!)
 */
#define RELAY1_PIN 27
#define RELAY2_PIN 26
#define RELAY3_PIN 25

/**
 * Built-in LED
 * - GPIO 2 (onboard LED on most ESP32 boards)
 *
 * Used for:
 * - Status indication
 * - Heartbeat
 * - Error codes
 */
#define LED_PIN 2

/**
 * Buzzer (Active buzzer)
 * - Positive: GPIO 15
 * - Negative: GND
 *
 * Active buzzer: ON/OFF control
 * Passive buzzer: Requires PWM for tones
 *
 * Can use tone() function for melodies
 */
#define BUZZER_PIN 15

/**
 * L298N Motor Driver
 * - VCC: 5-12V (motor power)
 * - GND: Common ground
 * - IN1-IN4: Direction control
 * - EN: Speed control (PWM)
 *
 * Controls 2 DC motors:
 * - Forward: IN1=HIGH, IN2=LOW
 * - Backward: IN1=LOW, IN2=HIGH
 * - Stop: Both LOW
 * - Speed: PWM on EN pin (0-255)
 */
#define MOTOR1_IN1 14
#define MOTOR1_IN2 33
#define MOTOR1_EN 32
#define MOTOR2_IN3 16
#define MOTOR2_IN4 17
#define MOTOR2_EN 23

/**
 * RGB LED (Common Cathode)
 * - R: GPIO 16 (via 220Ω resistor)
 * - G: GPIO 17 (via 220Ω resistor)
 * - B: GPIO 23 (via 220Ω resistor)
 * - Common: GND
 *
 * Use PWM for color mixing:
 * - analogWrite() for brightness (0-255)
 * - Combine R, G, B for any color
 *
 * Common Anode: Resistors to 3.3V, control with LOW
 */
#define RGB_R_PIN 16
#define RGB_G_PIN 17
#define RGB_B_PIN 23

/**
 * Available GPIO pins (not used above):
 * - GPIO 0 (boot button, use with caution)
 * - GPIO 36 (input only, no pullup)
 * - GPIO 39 (input only, no pullup)
 *
 * AVOID THESE PINS:
 * - GPIO 1, 3 (UART TX/RX - used for Serial)
 * - GPIO 6-11 (connected to flash - don't use!)
 */

// ═══════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONS - ESP32-CAM (DEVICE_TYPE == 1)
// ═══════════════════════════════════════════════════════════════════════════

#elif DEVICE_TYPE == 1

/**
 * ⚠️ WARNING: ESP32-CAM HAS VERY LIMITED GPIO!
 *
 * Most pins are used by the camera module.
 * Only a few pins are available for sensors/actuators.
 *
 * Available GPIOs: 2, 12, 13, 14, 15
 * Input-only: 34, 35, 36, 39
 *
 * Camera uses: 0, 4, 5, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33
 */

// ───────────────────────────────────────────────────────────────────────
// BUILT-IN PINS
// ───────────────────────────────────────────────────────────────────────

/**
 * Built-in Flash LED
 * - GPIO 4 (white LED next to camera)
 * - Active HIGH
 *
 * Use for:
 * - Photo illumination
 * - Status indication
 */
#define LED_PIN 33  // Red LED
#define FLASH_PIN 4 // White flash LED
#define LED_BUILTIN 33

// ───────────────────────────────────────────────────────────────────────
// AVAILABLE SENSOR PINS (Limited!)
// ───────────────────────────────────────────────────────────────────────

/**
 * DHT22 - Can use GPIO 2
 * WARNING: GPIO 2 must be LOW during boot
 */
#define DHT_PIN 2
#define DHT_TYPE DHT22

/**
 * PIR Motion Sensor - GPIO 13
 */
#define PIR_PIN 13

/**
 * LDR Light Sensor - GPIO 15 (ADC2_CH3)
 * NOTE: ADC2 doesn't work with WiFi active!
 * Use for pre-WiFi readings only
 */
#define LDR_PIN 15

/**
 * I2C Bus (shared with camera but can be used carefully)
 * - SDA: GPIO 26
 * - SCL: GPIO 27
 *
 * Can add BMP280, OLED, etc. if needed
 * Test thoroughly as these are camera pins!
 */
#define I2C_SDA 26
#define I2C_SCL 27

// ───────────────────────────────────────────────────────────────────────
// AVAILABLE ACTUATOR PINS
// ───────────────────────────────────────────────────────────────────────

/**
 * Relay - GPIO 12
 */
#define RELAY1_PIN 12

/**
 * Servo - GPIO 14
 */
#define SERVO1_PIN 14

/**
 * Camera-specific pins (DO NOT USE FOR OTHER PURPOSES!)
 *
 * Y9    GPIO 36
 * Y8    GPIO 37
 * Y7    GPIO 38
 * Y6    GPIO 39
 * Y5    GPIO 35
 * Y4    GPIO 34
 * Y3    GPIO 5
 * Y2    GPIO 18
 * VSYNC GPIO 25
 * HREF  GPIO 23
 * PCLK  GPIO 22
 * XCLK  GPIO 21
 * SIOD  GPIO 26 (SDA)
 * SIOC  GPIO 27 (SCL)
 * D0    GPIO 19
 * D1    GPIO 36
 * D2    GPIO 37
 * D3    GPIO 38
 * D4    GPIO 39
 * D5    GPIO 35
 * D6    GPIO 34
 * D7    GPIO 5
 * D8    GPIO 17
 * D9    GPIO 16
 */

#endif

// ═══════════════════════════════════════════════════════════════════════════
// SENSOR CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Sensor reading parameters
 *
 * SENSOR_READ_INTERVAL: How often to read sensors (milliseconds)
 *   - 2000ms (2 seconds) is good for most applications
 *   - Increase for battery-powered devices
 *   - Decrease for fast-response applications
 *
 * ULTRASONIC_MAX_DISTANCE: Maximum distance for HC-SR04 (cm)
 *   - 400cm is sensor maximum
 *   - Set lower to filter spurious readings
 *
 * LDR_SAMPLES: Number of samples to average for LDR
 *   - More samples = smoother readings
 *   - More samples = slower response
 *
 * TEMP/HUMIDITY_OFFSET: Calibration offsets
 *   - Compare with reference thermometer
 *   - Adjust these values to match reference
 *   - Example: If reads 2°C high, set TEMP_OFFSET = -2.0
 */
#define SENSOR_READ_INTERVAL 2000   // 2 seconds
#define ULTRASONIC_MAX_DISTANCE 400 // 400 cm
#define LDR_SAMPLES 10              // Average 10 readings
#define TEMP_OFFSET 0.0             // Temperature calibration
#define HUMIDITY_OFFSET 0.0         // Humidity calibration

// ═══════════════════════════════════════════════════════════════════════════
// DATA LOGGING CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Data logging settings
 *
 * LOG_TO_SERIAL: Print data to Serial Monitor
 *   - Enable for debugging
 *   - Disable in production to save processing time
 *
 * LOG_TO_SPIFFS: Save data to filesystem
 *   - Enable for data collection
 *   - Monitor SPIFFS space usage
 *
 * MAX_LOG_SIZE: Maximum log file size (bytes)
 *   - 100KB = about 1000 sensor readings
 *   - Files rotate when this size reached
 *
 * LOG_ROTATION: Auto-rotate logs when full
 *   - Old logs renamed with timestamp
 *   - Prevents filling entire filesystem
 */
#define LOG_TO_SERIAL true
#define LOG_TO_SPIFFS true
#define MAX_LOG_SIZE 100000 // 100 KB
#define LOG_ROTATION true

// ═══════════════════════════════════════════════════════════════════════════
// SYSTEM TIMING
// ═══════════════════════════════════════════════════════════════════════════

/**
 * System timing parameters (milliseconds)
 *
 * These control how often various system tasks run.
 * Adjust based on your application needs.
 */
#define WATCHDOG_TIMEOUT 30000      // Watchdog reset timeout
#define HEARTBEAT_INTERVAL 1000     // LED blink rate
#define STATUS_UPDATE_INTERVAL 5000 // Status broadcast interval
#define SERIAL_BAUD 115200          // Serial Monitor baud rate

// ═══════════════════════════════════════════════════════════════════════════
// BUFFER SIZES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Buffer size configuration
 *
 * Larger buffers = more memory usage but can handle bigger messages
 * Smaller buffers = less memory but may truncate data
 *
 * JSON_BUFFER_SIZE: For sensor data JSON (typical: 1-2KB)
 * HTTP_BUFFER_SIZE: For HTTP responses (typical: 512B-2KB)
 * ESPNOW_BUFFER_SIZE: ESP-NOW max payload (250 bytes fixed)
 */
#define JSON_BUFFER_SIZE 2048
#define HTTP_BUFFER_SIZE 1024
#define ESPNOW_BUFFER_SIZE 250

// ═══════════════════════════════════════════════════════════════════════════
// FEATURE ENABLES
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Feature enable/disable flags
 *
 * Set to true/false to enable/disable features.
 * Disabling unused features saves memory and improves performance.
 *
 * ENABLE_OTA: Over-the-air updates
 * ENABLE_WEBSERVER: Web interface
 * ENABLE_ESPNOW: Device-to-device communication
 * ENABLE_DATA_LOGGING: Data persistence
 * ENABLE_SENSORS: Sensor reading
 * ENABLE_ACTUATORS: Actuator control
 * ENABLE_CAMERA: Camera (ESP32-CAM only)
 */
#define ENABLE_OTA true
#define ENABLE_WEBSERVER true
#define ENABLE_ESPNOW true
#define ENABLE_DATA_LOGGING true
#define ENABLE_SENSORS true
#define ENABLE_ACTUATORS true
#define ENABLE_CAMERA (DEVICE_TYPE == 1) // Auto-detect

// ═══════════════════════════════════════════════════════════════════════════
// DEBUG SETTINGS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * Debug configuration
 *
 * Enable debug output for specific subsystems.
 * Set to false in production to improve performance.
 *
 * DEBUG_MODE: Master debug flag
 * DEBUG_ESPNOW: ESP-NOW communication debug
 * DEBUG_SENSORS: Sensor reading debug
 * DEBUG_ACTUATORS: Actuator control debug
 */
#define DEBUG_MODE true
#define DEBUG_ESPNOW true
#define DEBUG_SENSORS true
#define DEBUG_ACTUATORS true

/**
 * Debug print macros
 *
 * Use these instead of Serial.print() for conditional debugging
 * Automatically disabled when DEBUG_MODE = false
 *
 * Usage:
 *   DEBUG_PRINT("Value: ");
 *   DEBUG_PRINTLN(value);
 *   DEBUG_PRINTF("Value: %d\n", value);
 */
#if DEBUG_MODE
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif

// ═══════════════════════════════════════════════════════════════════════════
// COMPILE-TIME VALIDATIONS
// ═══════════════════════════════════════════════════════════════════════════

/**
 * These checks ensure configuration is valid at compile time
 * Prevents common configuration mistakes
 */

// Check device type is valid
#if DEVICE_TYPE != 0 && DEVICE_TYPE != 1
#error "DEVICE_TYPE must be 0 (ESP32) or 1 (ESP32-CAM)"
#endif

// Check buffer sizes are reasonable
#if JSON_BUFFER_SIZE < 512
#warning "JSON_BUFFER_SIZE is very small, may cause data truncation"
#endif

// Check camera only enabled on ESP32-CAM
#if ENABLE_CAMERA && (DEVICE_TYPE != 1)
#warning "Camera enabled but device type is not ESP32-CAM"
#endif

// Check ESP-NOW peer limit
#if MAX_ESPNOW_PEERS > 6
#warning "ESP32 supports max 6 unencrypted peers"
#endif

#endif // CONFIG_H

/**
 * ═══════════════════════════════════════════════════════════════════════════
 * CONFIGURATION TIPS:
 * ═══════════════════════════════════════════════════════════════════════════
 *
 * MEMORY OPTIMIZATION:
 * - Disable unused features
 * - Reduce buffer sizes
 * - Decrease MAX_CLIENTS
 * - Shorten update intervals
 *
 * PERFORMANCE TUNING:
 * - Increase sensor intervals for slower reads
 * - Disable serial debugging in production
 * - Use smaller JSON buffers
 * - Reduce WebSocket broadcast frequency
 *
 * BATTERY OPERATION:
 * - Increase all intervals
 * - Disable OTA and WebServer
 * - Use deep sleep between readings
 * - Lower WiFi transmit power
 *
 * REAL-TIME APPLICATIONS:
 * - Decrease sensor intervals
 * - Increase WebSocket update rate
 * - Use faster WiFi mode
 * - Prioritize critical sensors
 *
 * ═══════════════════════════════════════════════════════════════════════════
 */
